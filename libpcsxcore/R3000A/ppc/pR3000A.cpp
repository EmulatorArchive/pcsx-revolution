/*  PCSX-Revolution - PS Emulator for Nintendo Wii
 *  Copyright (C) 2009-2010  PCSX-Revolution Dev Team
 *
 *  PCSX-Revolution is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public 
 *  License as published by the Free Software Foundation, either 
 *  version 2 of the License, or (at your option) any later version.
 *
 *  PCSX-Revolution is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with PCSX-Revolution.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

#include "plugins.h"
#include "psxcommon.h"
#include "psxmem.h"
#include "psxcounters.h"
#include "ppc.h"
#include "../r3000a.h"
#include "psxhle.h"
#include "mdec.h"
#include "plugins.h"

using namespace R3000A;

#ifdef GEKKO
#	define _LANGUAGE_ASSEMBLY
#	include <ogc/machine/asm.h>
#endif

#define REG_LO 32
#define REG_HI 33

// #define NO_CONSTANT

#undef PC_REC
#undef PC_REC32
#define PC_REC(x)	(psxRecLUT[(x) >> 16] + ((x) & 0xffff))
#define PC_REC32(x) (*(uptr*)PC_REC(x))

#if defined(HW_DOL)
#define RECMEM_SIZE		(7*1024*1024)
#elif defined(HW_RVL)
#define RECMEM_SIZE		(8*1024*1024)
#endif

static void RecTestBranch(u32 amount) {
	LWPRtoR(r4, &psxRegs.evtCycleCountdown);
	SUBI(r4, r4, amount);
	STWRtoPR(&psxRegs.evtCycleCountdown, r4);
	CMPWI(r4, 0);
	BranchTarget branchtest(BT_GT);
	CALLFunc((uptr)psxBranchTest);
	branchtest.setTarget();
}

#define REC_MEM2

#ifdef REC_MEM2
/* variable declarations */
static uptr psxRecLUT[0x010000 * sizeof(uptr)] __attribute__((aligned(32)));
static char recMem[RECMEM_SIZE + 0x1000]  __attribute__((aligned(32), section(".text#")));        /* the recompiled blocks will be here */
static char recRAM[0x200000] __attribute__((aligned(32)));       /* and the ptr to the blocks here */
static char recROM[0x080000] __attribute__((aligned(32)));       /* and here */
#else
static char *recMem;	/* the recompiled blocks will be here */
static char *recRAM;	/* and the ptr to the blocks here */
static char *recROM;	/* and here */
uptr *psxRecLUT;
#endif

static u32 pc;			/* recompiler pc */
static u32 pcold;		/* recompiler oldpc */
static int count;		/* recompiler intruction count */
static int branch;		/* set for branch */
static u32 target;		/* branch target */

u32 cop2readypc = 0;
u32 idlecyclecount = 0;

typedef struct {
	int state;
	u32 k;
	int reg;
} iRegisters;

#define NUM_REGISTERS 34

static iRegisters iRegs[NUM_REGISTERS];

#define ST_UNK      0x00
#define ST_CONST    0x01
#define ST_MAPPED   0x02

#ifdef NO_CONSTANT
#define IsConst(reg) 0
#else
#define IsConst(reg)  (iRegs[reg].state == ST_CONST)
#endif
#define IsMapped(reg) (iRegs[reg].state == ST_MAPPED)

#define NEW_REGS

#ifdef NEW_REGS

#include "regAlloc.h"

using namespace JIT;

RegAlloc HWRegs;

#endif

#define DYNAREC_BLOCK 50

void SetArg_OfB( int arg ) {
	if(IsConst(_Rs_)) {
		LIW((arg), iRegs[_Rs_].k + _Imm_);
	}
	else {
		MR((arg), HWRegs.Get(_Rs_));
		if(_Imm_) ADDI((arg), (arg), _Imm_);
	}
}

static void recRecompile();
static void recError();

static void MapConst(int reg, u32 _const) {
#ifdef NEW_REGS
	HWRegs.Dispose(reg);
#endif
	iRegs[reg].k = _const;
	iRegs[reg].state = ST_CONST;
}

static void iFlushReg(int reg) {
	if (IsConst(reg)) {
		LIW(r3, iRegs[reg].k);
		STWRtoPR(&psxRegs.GPR.r[reg].d, r3);
	}
	iRegs[reg].state = ST_UNK;
}

static void iFlushRegs() {
#ifdef NEW_REGS
	HWRegs.FlushRegs();
#endif
	int i;

	for (i = 1; i < NUM_REGISTERS; i++) {
		iFlushReg( i );
	}
}

static void Return()
{
	iFlushRegs();
#if 1
	if (((u32)returnPC & 0x1fffffc) == (u32)returnPC) {
		BA((u32)returnPC);
	}
	else {
		LIW(r0, (u32)returnPC);
		MTLR(r0);
		BLR();
	}
#else
	LWZ(r0, ((1*4)+8)+4, r1);
	ADDI(r1, r1, ((1*4)+8));
	MTLR(r0);
	LMW(r31, -(1*4), r1);
	BLR();
#endif
}

static void UpdateCycle(u32 amount) {
	LWPRtoR(r4, &psxRegs.evtCycleCountdown);
	SUBI(r4, r4, amount);
	STWRtoPR(&psxRegs.evtCycleCountdown, r4);
}

static void iRet() {
    /* store cycle */
	count = (idlecyclecount + (pc - pcold)) / 4;
	UpdateCycle(count);
	Return();
}

static void invalidateCache(u32 from, u32 to) {
	while(from < to) {
		__asm__ __volatile__("dcbst 0,%0" : : "r" (from));
		__asm__ __volatile__("icbi 0,%0" : : "r" (from));
		from += 4;
	}
	__asm__ __volatile__("sync");
	__asm__ __volatile__("isync");
}

static void SetBranch();
static void iBranch(u32 branchPC, int savectx);
static void iJump(u32 branchPC);

static void recSPECIAL();
static void recREGIMM();
static void recCOP0();
static void recCOP2();
static void recBASIC();

#include "recOpcodeTable.h"
#include "pGte.h"

static void (*recBSC[64])() = {
	recSPECIAL, recREGIMM, recJ   , recJAL  , recBEQ , recBNE , recBLEZ, recBGTZ,
	recADDI   , recADDIU , recSLTI, recSLTIU, recANDI, recORI , recXORI, recLUI ,
	recCOP0   , recNULL  , recCOP2, recNULL , recNULL, recNULL, recNULL, recNULL,
	recNULL   , recNULL  , recNULL, recNULL , recNULL, recNULL, recNULL, recNULL,
	recLB     , recLH    , recLWL , recLW   , recLBU , recLHU , recLWR , recNULL,
	recSB     , recSH    , recSWL , recSW   , recNULL, recNULL, recSWR , recNULL,
	recNULL   , recNULL  , recLWC2, recNULL , recNULL, recNULL, recNULL, recNULL,
	recNULL   , recNULL  , recSWC2, recHLE  , recNULL, recNULL, recNULL, recNULL
};

static void (*recSPC[64])() = {
	recSLL , recNULL, recSRL , recSRA , recSLLV   , recNULL , recSRLV, recSRAV,
	recJR  , recJALR, recNULL, recNULL, recSYSCALL, recBREAK, recNULL, recNULL,
	recMFHI, recMTHI, recMFLO, recMTLO, recNULL   , recNULL , recNULL, recNULL,
	recMULT, recMULTU, recDIV, recDIVU, recNULL   , recNULL , recNULL, recNULL,
	recADD , recADDU, recSUB , recSUBU, recAND    , recOR   , recXOR , recNOR ,
	recNULL, recNULL, recSLT , recSLTU, recNULL   , recNULL , recNULL, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL   , recNULL , recNULL, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL   , recNULL , recNULL, recNULL
};

static void (*recREG[32])() = {
	recBLTZ  , recBGEZ  , recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recNULL  , recNULL  , recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recBLTZAL, recBGEZAL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recNULL  , recNULL  , recNULL, recNULL, recNULL, recNULL, recNULL, recNULL
};

static void (*recCP0[32])() = {
	recMFC0, recNULL, recCFC0, recNULL, recMTC0, recNULL, recCTC0, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recRFE , recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL
};

static void (*recCP2[64])() = {
	recBASIC, recRTPS , recNULL , recNULL, recNULL, recNULL , recNCLIP, recNULL, // 00
	recNULL , recNULL , recNULL , recNULL, recOP  , recNULL , recNULL , recNULL, // 08
	recDPCS , recINTPL, recMVMVA, recNCDS, recCDP , recNULL , recNCDT , recNULL, // 10
	recNULL , recNULL , recNULL , recNCCS, recCC  , recNULL , recNCS  , recNULL, // 18
	recNCT  , recNULL , recNULL , recNULL, recNULL, recNULL , recNULL , recNULL, // 20
	recSQR  , recDCPL , recDPCT , recNULL, recNULL, recAVSZ3, recAVSZ4, recNULL, // 28 
	recRTPT , recNULL , recNULL , recNULL, recNULL, recNULL , recNULL , recNULL, // 30
	recNULL , recNULL , recNULL , recNULL, recNULL, recGPF  , recGPL  , recNCCT  // 38
};

static void (*recCP2BSC[32])() = {
	recMFC2, recNULL, recCFC2, recNULL, recMTC2, recNULL, recCTC2, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL,
	recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL, recNULL
};

static void recSPECIAL() {
	recSPC[_Funct_]();
}

static void recREGIMM() {
	recREG[_Rt_]();
}

static void recCOP0() {
	recCP0[_Rs_]();
}

static void recCOP2() {
	recCP2[_Funct_]();
}

static void recBASIC() {
	recCP2BSC[_Rs_]();
}

static int iLoadTest() {
	u32 tmp;

	// check for load delay
	tmp = psxRegs.code >> 26;
	switch (tmp) {
		case 0x10: // COP0
			switch (_Rs_) {
				case 0x00: // MFC0
				case 0x02: // CFC0
					return 1;
			}
			break;
		case 0x12: // COP2
			switch (_Funct_) {
				case 0x00:
					switch (_Rs_) {
						case 0x00: // MFC2
						case 0x02: // CFC2
							return 1;
					}
					break;
			}
			break;
		case 0x32: // LWC2
			return 1;
		default:
			if (tmp >= 0x20 && tmp <= 0x26) { // LB/LH/LWL/LW/LBU/LHU/LWR
				return 1;
			}
			break;
	}
	return 0;
}

/* set a pending branch */
static void SetBranch() {
	branch = 1;
	psxRegs.code = PSXMu32(pc);
	pc += 4;

	if (iLoadTest() == 1) {
		iFlushRegs();
		LIW(r4, psxRegs.code);
		LIW(r3, pc);
		STWRtoPR(&psxRegs.code, r4);
		STWRtoPR(&psxRegs.pc, r3);

		// store cycle 
		count = (idlecyclecount + (pc - pcold)) / 4;
		UpdateCycle(count);

		LI(PPCARG1, _Rt_);
		LWMtoR(PPCARG2, (uptr)&target);

		CALLFunc((u32)psxDelayTest);
		
		Return();
		return;
	}

	recBSC[_Op_]();

	iFlushRegs();
	LWMtoR(r4, (uptr)&target);
	STWRtoPR(&psxRegs.pc, r4);
	
	count = (idlecyclecount + (pc - pcold)) / 4;
// 	UpdateCycle(count);
	
	RecTestBranch(count);
	
	// TODO: don't return if target is compiled

#if 1
	Return();
#else
	// maybe just happened an interruption, check so
	LWMtoR(r0, (uptr)&target);
	LWPRtoR(r3, &psxRegs.pc);
	CMPLW(r3, r0);
	BNE_L(b32Ptr[0]);

	LIW(r3, PC_REC(SWAPu32(target)));
	LWZ(r3, 0, r3);
	MTCTR(r3);
	CMPLWI(r3, 0);
	BNE_L(b32Ptr[1]);

	B_DST(b32Ptr[0]);
	iRet();

	// next bit is already compiled - jump right to it
	B_DST(b32Ptr[1]);
	//Return();
	BCTR();
#endif
}

static void iJump(u32 branchPC) {
	branch = 1;
	psxRegs.code = PSXMu32(pc);
	pc+=4;

	if (iLoadTest() == 1) {
		iFlushRegs();
		LIW(r0, psxRegs.code);
		LIW(r3, pc);
		STWRtoPR(&psxRegs.code, r0);
		STWRtoPR(&psxRegs.pc, r3);

		count = (idlecyclecount + (pc - pcold)) / 4;
		UpdateCycle(count);

		LI(PPCARG1, _Rt_);
		LIW(PPCARG2, branchPC);

		CALLFunc((u32)psxDelayTest);

		Return();
		return;
	}

	recBSC[_Op_]();

	iFlushRegs();
	LIW(r4, branchPC);
	STWRtoPR(&psxRegs.pc, r4);
	
	count = (idlecyclecount + (pc - pcold)) / 4;
// 	UpdateCycle(count);

	RecTestBranch(count);

	/*if (!Config.HLE && Config.PsxOut &&
	    ((branchPC & 0x1fffff) == 0xa0 ||
	     (branchPC & 0x1fffff) == 0xb0 ||
	     (branchPC & 0x1fffff) == 0xc0))
	  CALLFunc((u32)psxJumpTest);*/
#if 1
	// always return for now...
	Return();
#else
	LIW(r0, branchPC);
	LWPRtoR(r3, &psxRegs.pc);
	CMPLW(r3, r0);

	BranchTarget ne(BT_NE);

	LIW(r3, PC_REC(branchPC));
	LWZ(r3, 0, r3);
	CMPLWI(r3, 0);
	BranchTarget ne2(BT_NE);

	ne.setTarget();
	Return();
	
	ne2.setTarget();

	MTCTR(r3);
	BCTRL();
#endif
}

static void iBranch(u32 branchPC, int savectx) {
	iRegisters iRegsS[NUM_REGISTERS];
#ifdef NEW_REGS
	RegAlloc HWRegsS;
#endif

	if (savectx) {
		memcpy(iRegsS, iRegs, sizeof(iRegs));
#ifdef NEW_REGS
// 		memcpy(&HWRegsS, &HWRegs, sizeof(HWRegs));
		HWRegsS = HWRegs;
#endif
	}

	branch = 1;
	psxRegs.code = PSXMu32(pc);

	// the delay test is only made when the branch is taken
	// savectx == 0 will mean that :)
	if (savectx == 0 && iLoadTest() == 1) {
		iFlushRegs();
		LIW(r4, psxRegs.code);
		LIW(r0, pc);
		STWRtoPR(&psxRegs.code, r4);
		STWRtoPR(&psxRegs.pc, r0);

		count = (idlecyclecount + ((pc + 4) - pcold)) / 4;
		UpdateCycle(count);

		LI(PPCARG1, _Rt_);
		LIW(PPCARG2, branchPC);

		CALLFunc((u32)psxDelayTest);

		Return();
		return;
	}

	pc += 4;
	recBSC[_Op_]();

	iFlushRegs();
	LIW(r4, branchPC);
	STWRtoPR(&psxRegs.pc, r4);

	count = (idlecyclecount + (pc - pcold)) / 4;
// 	UpdateCycle(count);

	RecTestBranch(count);

#if 1
	Return();
#else
	LIW(r4, branchPC);
	LWPRtoR(r3, &psxRegs.pc);
	CMPLW(r3, r4);

	BranchTarget eq(BT_EQ);

	Return();

	eq.setTarget();
	LIW(r3, PC_REC(branchPC));
	LWZ(r3, 0, r3);

	CMPLWI(r3, 0);
	BranchTarget ne(BT_NE);

	Return();

	ne.setTarget();

	MTCTR(r3);
	BCTRL();
#endif
	pc -= 4;
	if (savectx) {
		memcpy(iRegs, iRegsS, sizeof(iRegs));
#ifdef NEW_REGS
// 		memcpy(&HWRegs, &HWRegsS, sizeof(HWRegsS));
		HWRegs = HWRegsS;
#endif
	}
}


void iDumpRegs() {
	int i, j;

	//printf("%08x %08x\n", psxRegs.pc, psxRegs.evtCycleCountdown);
	for (i=0; i<4; i++) {
		for (j=0; j<8; j++)
			printf("%08x ", psxRegs.GPR.r[j*i].d);
		printf("\n");
	}
}

void iDumpBlock(char *ptr) {
/*	FILE *f;
	u32 i;

	SysPrintf("dump1 %x:%x, %x\n", psxRegs.pc, pc, psxCurrentCycle);

	for (i = psxRegs.pc; i < pc; i+=4)
		SysPrintf("%s\n", disR3000AF(PSXMu32(i), i));

	fflush(stdout);
	f = fopen("dump1", "w");
	fwrite(ptr, 1, (u32)x86Ptr - (u32)ptr, f);
	fclose(f);
	system("ndisasmw -u dump1");
	fflush(stdout);*/
}
static void freeMem(int all)
{
#ifndef REC_MEM2
    if (recMem) free(recMem);
    if (recRAM) free(recRAM);
    if (recROM) free(recROM);
    recMem = recRAM = recROM = NULL;
    
    if (all && psxRecLUT) {
        free(psxRecLUT); 
		psxRecLUT = NULL;
    }
#endif
}

static int allocMem() {
	int i;
#ifndef REC_MEM2
	freeMem(0);
        
	if (psxRecLUT==NULL)
		psxRecLUT = (uptr*) memalign(32,0x010000 * sizeof(uptr));

	recMem = (char*) memalign(32,RECMEM_SIZE + 0x1000);
	recRAM = (char*) memalign(32,0x200000);
	recROM = (char*) memalign(32,0x080000);
	if (recRAM == NULL || recROM == NULL || recMem == NULL/*(void *)-1*/ || psxRecLUT == NULL) {
		freeMem(1);
		SysMessage("Error allocating memory"); return -1;
	}
#endif
	memset(recMem, 0, RECMEM_SIZE);
	memset(recRAM, 0, 0x200000);
	memset(recROM, 0, 0x080000);

	for (i=0; i<0x80; i++) psxRecLUT[i + 0x0000] = (uptr)&recRAM[(i & 0x1f) << 16];
	memcpy(psxRecLUT + 0x8000, psxRecLUT, 0x80 * sizeof(uptr));
	memcpy(psxRecLUT + 0xa000, psxRecLUT, 0x80 * sizeof(uptr));

	for (i=0; i<0x08; i++) psxRecLUT[i + 0xbfc0] = (uptr)&recROM[i << 16];

	return 0;
}

static int recInit() {
	return allocMem();
}

static void recReset() {
	SysPrintf("reset");
	memset(recRAM, 0, 0x200000);
	memset(recROM, 0, 0x080000);

	ppcInit();
	ppcSetPtr((u32 *)recMem);

	branch = 0;
	memset(iRegs, 0, sizeof(iRegs));
	iRegs[0].state = ST_CONST;
	iRegs[0].k     = 0;
#ifdef NEW_REGS
	HWRegs.Reset();
#endif
}

static void recShutdown() {
	freeMem(1);
	ppcShutdown();
}

static void recError() {
	SysReset();
	ClosePlugins();
	SysMessage("Unrecoverable error while running recompiler\n");
	SysRunGui();
}

__inline static void execute() {
	void (**recFunc)();
	char *p;

	p =	(char*)PC_REC(psxRegs.pc);
	/*if (p != NULL)*/ 
	/*else { recError(); return; }*/

	if (*p == 0) {
		recRecompile();
	}

	recFunc = (void (**)()) (u32)p;
#if 1
	recRun(*recFunc, (u32)&psxRegs, (u32)&psxM);
#else
	(*recFunc)();
#endif
}

static void recExecute() {
	for (;;) execute();
}

static void recExecuteBlock() {
	execute();
}

static void recClear(u32 Addr, u32 Size) {
	u32 start = (u32)(u8 *)PC_REC(Addr);
	
	u32 bank,offset;

	bank = Addr >> 24;
	offset = Addr & 0xffffff;


	// Pitfall 3D - clear dynarec slots that contain 'stale' ram data
	// - fixes stage 1 loading crash
	if( bank == 0x80 || bank == 0xa0 || bank == 0x00 ) {
		offset &= 0x1fffff;

		if( offset >= DYNAREC_BLOCK * 4 )
			memset((void*)PC_REC(Addr - DYNAREC_BLOCK * 4), 0, DYNAREC_BLOCK * 4);
		else
			memset((void*)PC_REC(Addr - offset), 0, offset);
	}

	memset((void*)PC_REC(Addr), 0, Size * 4);
	invalidateCache(start, start + (Size * 4));
}

#ifdef NEW_REC_TEST

void psxRecompileNextInstruction(int delayslot)
{
	p = (char *)PSXM(pc);
	if (p == NULL) recError();
	psxRegs.code = GETLE32((u32 *)p);

	pc+=4;
	count++;

	recBSC[_Op_]();
}

void psxSetBranchImm( u32 imm )
{
	branch = 1;

	// end the current block
	LIW(r3, imm);
	iFlushRegs();
	STWRtoPR(&psxRegs.pc, r3);

	count = (idlecyclecount + (pc - pcold)) / 4;
	
	RecTestBranch(count);
	
	// return for now
	Return();

	//recBlocks.Link(HWADDR(imm), xJcc32());
}

#endif

static void recRecompile() {
	//static int recCount = 0;
	char *p;
	u32 *ptr;
	int i;
	cop2readypc = 0;
	idlecyclecount = 0;
	
	for (i=0; i<NUM_REGISTERS; i++) {
		iRegs[i].state = ST_UNK;
		iRegs[i].reg = -1;
	}
	iRegs[0].k = 0;
	iRegs[0].state = ST_CONST;
	
	/* if ppcPtr reached the mem limit reset whole mem */
	if (((u32)ppcPtr - (u32)recMem) >= (RECMEM_SIZE - 0x10000))
		recReset();

	ppcAlign(4);
	ptr = ppcPtr;

	// tell the LUT where to find us
	PC_REC32(psxRegs.pc) = (u32)ppcPtr;

	pcold = pc = psxRegs.pc;
#if 0
	MFLR(r0);
	STMW(r31, -(1*4), r1)
	STW(r0, 4, r1);
	STWU(r1, -((1*4)+8), r1);

	LIW(r31, &psxRegs);
#endif

	for (count = 0; count < DYNAREC_BLOCK;) {
#ifdef NEW_REC_TEST
		psxRecompileNextInstruction(0);
#else
		p = (char *)PSXM(pc);
		if (p == NULL) recError();
		psxRegs.code = GETLE32((u32 *)p);

		pc+=4;
		count++;

		recBSC[_Op_]();
#endif

		if (branch) {
			branch = 0;
			goto done;
		}
	}

	iFlushRegs();
	LIW(r0, pc);

	STWRtoPR(&psxRegs.pc, r0);

	iRet();

done:;

	invalidateCache((u32)(u8*)ptr, (u32)(u8*)ppcPtr);

	sprintf((char *)ppcPtr, "PC=%08x", pcold);
	ppcPtr += strlen((char *)ppcPtr);
}

R3000Acpu R3000A::psxRec = {
	recInit,
	recReset,
	recExecute,
	recExecuteBlock,
	recClear,
	recShutdown
};
