/***************************************************************************
 *   Copyright (C) 2007 Ryan Schultz, PCSX-df Team, PCSX team              *
 *   schultz.ryan@gmail.com, http://rschultz.ath.cx/code.php               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02111-1307 USA.            *
 ***************************************************************************/

/*
 * PSX assembly interpreter.
 */

#include "../PsxCommon.h"
#include "../PsxMem.h"
#include "../PsxHLE.h"
#include "R3000A.h"
#include "R3000AOpcodeTable.h"
#include "R3000AOpcodeDispatcher.h"
#include "Gte.h"
#ifndef __GAMECUBE__
/*FIXME*/
#include "../gui/hdebug.h"
#endif

static u32 branchPC;

#define TEST_BRANCH() \
	if (psxRegs.evtCycleCountdown <= 0) \
		psxBranchTest();

// These macros are used to assemble the repassembler functions

#ifdef PSXCPU_LOG
#define debugI() PSXCPU_LOG("%s\n", disR3000AF(psxRegs.code, psxRegs.pc));
#else
#define debugI()
#endif

inline void execI();

static void delayRead(int reg, u32 bpc) {
	u32 rold, rnew;

//      SysPrintf("delayRead at %x!\n", psxRegs.pc);

	rold = psxRegs.GPR.r[reg];
	OpcodeDispatcher( psxRegs.code ); // branch delay load
	rnew = psxRegs.GPR.r[reg];

	psxRegs.pc = bpc;

	TEST_BRANCH();

	psxRegs.GPR.r[reg] = rold;
	execI(); // first branch opcode
	psxRegs.GPR.r[reg] = rnew;

	psxRegs.IsDelaySlot = 0;
}

static void delayWrite(int reg, u32 bpc) {

/*      SysPrintf("delayWrite at %x!\n", psxRegs.pc);

	SysPrintf("%s\n", disR3000AF(psxRegs.code, psxRegs.pc-4));
	SysPrintf("%s\n", disR3000AF(PSXMu32(bpc), bpc));*/

	// no changes from normal behavior

	OpcodeDispatcher( psxRegs.code );

	psxRegs.IsDelaySlot = 0;
	psxRegs.pc = bpc;

	TEST_BRANCH();
}

static void delayReadWrite(int reg, u32 bpc) {

//      SysPrintf("delayReadWrite at %x!\n", psxRegs.pc);

	// the branch delay load is skipped

	psxRegs.IsDelaySlot = 0;
	psxRegs.pc = bpc;

	TEST_BRANCH();
}

// this defines shall be used with the tmp
// of the next func (instead of _Funct_...)
#define _tFunct_  ((tmp      ) & 0x3F)  // The funct part of the instruction register
#define _tRd_     ((tmp >> 11) & 0x1F)  // The rd part of the instruction register
#define _tRt_     ((tmp >> 16) & 0x1F)  // The rt part of the instruction register
#define _tRs_     ((tmp >> 21) & 0x1F)  // The rs part of the instruction register
#define _tSa_     ((tmp >>  6) & 0x1F)  // The sa part of the instruction register

int psxTestLoadDelay(int reg, u32 tmp) {
	if (tmp == 0) return 0; // NOP
	switch (tmp >> 26) {
		case 0x00: // SPECIAL
			switch (_tFunct_) {
				case 0x00: // SLL
				case 0x02: case 0x03: // SRL/SRA
					if (_tRd_ == reg && _tRt_ == reg) return 1; else
					if (_tRt_ == reg) return 2; else
					if (_tRd_ == reg) return 3;
					break;

				case 0x08: // JR
					if (_tRs_ == reg) return 2;
					break;
				case 0x09: // JALR
					if (_tRd_ == reg && _tRs_ == reg) return 1; else
					if (_tRs_ == reg) return 2; else
					if (_tRd_ == reg) return 3;
					break;

				// SYSCALL/BREAK just a break;

				case 0x20: case 0x21: case 0x22: case 0x23:
				case 0x24: case 0x25: case 0x26: case 0x27:
				case 0x2a: case 0x2b: // ADD/ADDU...
				case 0x04: case 0x06: case 0x07: // SLLV...
					if (_tRd_ == reg && (_tRt_ == reg || _tRs_ == reg)) return 1; else
					if (_tRt_ == reg || _tRs_ == reg) return 2; else
					if (_tRd_ == reg) return 3;
					break;

				case 0x10: case 0x12: // MFHI/MFLO
					if (_tRd_ == reg) return 3;
					break;
				case 0x11: case 0x13: // MTHI/MTLO
					if (_tRs_ == reg) return 2;
					break;

				case 0x18: case 0x19:
				case 0x1a: case 0x1b: // MULT/DIV...
					if (_tRt_ == reg || _tRs_ == reg) return 2;
					break;
			}
			break;

		case 0x01: // REGIMM
			switch (_tRt_) {
				case 0x00: case 0x02:
				case 0x10: case 0x12: // BLTZ/BGEZ...
					if (_tRs_ == reg) return 2;
					break;
			}
			break;

		// J would be just a break;
		case 0x03: // JAL
			if (31 == reg) return 3;
			break;

		case 0x04: case 0x05: // BEQ/BNE
			if (_tRs_ == reg || _tRt_ == reg) return 2;
			break;

		case 0x06: case 0x07: // BLEZ/BGTZ
			if (_tRs_ == reg) return 2;
			break;

		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c: case 0x0d: case 0x0e: // ADDI/ADDIU...
			if (_tRt_ == reg && _tRs_ == reg) return 1; else
			if (_tRs_ == reg) return 2; else
			if (_tRt_ == reg) return 3;
			break;

		case 0x0f: // LUI
			if (_tRt_ == reg) return 3;
			break;

		case 0x10: // COP0
			switch (_tFunct_) {
				case 0x00: // MFC0
				case 0x02: // CFC0
					if (_tRt_ == reg) return 3;
					break;
				case 0x04: // MTC0
				case 0x06: // CTC0
					if (_tRt_ == reg) return 2;
					break;
				// RFE just a break;
			}
			break;



		case 0x12: // COP2
			switch (_tFunct_) {
				case 0x00:
					switch (_tRs_) {
						case 0x00: // MFC2
						case 0x02: // CFC2
							if (_tRt_ == reg) return 3;
							break;
						case 0x04: // MTC2
						case 0x06: // CTC2
							if (_tRt_ == reg) return 2;
							break;
					}
					break;
				// RTPS... break;
			}
			break;

		case 0x22: case 0x26: // LWL/LWR
			if (_tRt_ == reg) return 3; else
			if (_tRs_ == reg) return 2;
			break;

		case 0x20: case 0x21: case 0x23:
		case 0x24: case 0x25: // LB/LH/LW/LBU/LHU
			if (_tRt_ == reg && _tRs_ == reg) return 1; else
			if (_tRs_ == reg) return 2; else
			if (_tRt_ == reg) return 3;
			break;

		case 0x28: case 0x29: case 0x2a:
		case 0x2b: case 0x2e: // SB/SH/SWL/SW/SWR
			if (_tRt_ == reg || _tRs_ == reg) return 2;
			break;

		case 0x32: case 0x3a: // LWC2/SWC2
			if (_tRs_ == reg) return 2;
			break;
	}

	return 0;
}

void psxDelayTest(int reg, u32 bpc) {
	u32 *code;
	u32 tmp;

	code = (u32 *)PSXM(bpc);
	tmp = ((code == NULL) ? 0 : SWAP32(*code));
	psxRegs.IsDelaySlot = 1;

	switch (psxTestLoadDelay(reg, tmp)) {
		case 1:
			delayReadWrite(reg, bpc); return;
		case 2:
			delayRead(reg, bpc); return;
		case 3:
			delayWrite(reg, bpc); return;
	}
	OpcodeDispatcher( psxRegs.code );

	psxRegs.IsDelaySlot = 0;
	psxRegs.pc = bpc;

	TEST_BRANCH();
}

__inline void doBranch(u32 tar) {
	u32 *code;
	u32 tmp;

	psxRegs.IsDelaySlot = 1;
	branchPC = tar;

	code = (u32 *)PSXM(psxRegs.pc);

	//psxRegs.code = ((code == NULL) ? 0 : SWAP32(*code));
	if(code == NULL)
	{
		psxRegs.pc += 4;
		psxRegs.evtCycleCountdown--;
		code = (u32 *)PSXM(psxRegs.pc);
		psxRegs.code = ((code == NULL) ? 0 : SWAP32(*code));
	}
	else psxRegs.code = SWAP32(*code);

	debugI();

	psxRegs.pc += 4;
	psxRegs.evtCycleCountdown--;

	// check for load delay
	tmp = psxRegs.code >> 26;
	switch (tmp) {
		case 0x10: // COP0
			switch (_Rs_) {
				case 0x00: // MFC0
				case 0x02: // CFC0
					psxDelayTest(_Rt_, branchPC);
					return;
			}
			break;
		case 0x12: // COP2
			switch (_Funct_) {
				case 0x00:
					switch (_Rs_) {
						case 0x00: // MFC2
						case 0x02: // CFC2
							psxDelayTest(_Rt_, branchPC);
							return;
					}
					break;
			}
			break;
		case 0x32: // LWC2
			psxDelayTest(_Rt_, branchPC);
			return;
		default:
			if (tmp >= 0x20 && tmp <= 0x26) { // LB/LH/LWL/LW/LBU/LHU/LWR
				psxDelayTest(_Rt_, branchPC);
				return;
			}
			break;
	}

	OpcodeDispatcher( psxRegs.code );

	psxRegs.IsDelaySlot = 0;
	psxRegs.pc = branchPC;

	TEST_BRANCH();
}

///////////////////////////////////////////

static int intInit() {
	return 0;
}

static void intReset() {
}

static void intExecute() {
	for (;;)
		execI();
}

static void intExecuteBlock(u32 addr) {
	while(psxRegs.pc != addr) execI();
}

static void intClear(u32 Addr, u32 Size) {
}

static void intShutdown() {
}

// interpreter execution
inline void execI() {
	u32 *code = (u32 *)PSXM(psxRegs.pc);
	if(code == NULL)
	{
		psxRegs.pc += 4;
		psxRegs.evtCycleCountdown--;
		code = (u32 *)PSXM(psxRegs.pc);
		psxRegs.code = ((code == NULL) ? 0 : SWAP32(*code));
	}
	else psxRegs.code = SWAP32(*code);

	debugI();

	psxRegs.pc += 4;
	psxRegs.evtCycleCountdown--;
	OpcodeDispatcher( psxRegs.code );
}

R3000Acpu psxInt = {
	intInit,
	intReset,
	intExecute,
	intExecuteBlock,
	intClear,
	intShutdown
};

#ifndef __GAMECUBE__
/* debugger version */
static inline void execIDbg() { 
	u32 *code = (u32 *)PSXM(psxRegs.pc);
	psxRegs.code = ((code == NULL) ? 0 : SWAP32(*code));

	// dump opcode when LOG_CPU is enabled
	debugI();

	// normal execution
	if (!hdb_pause) {
		psxRegs.pc += 4;
		AddCycles( 1 );
		OpcodeDispatcher( psxRegs.code );
	}

	// trace one instruction
	if(hdb_pause == 2) {
		psxRegs.pc += 4;
		AddCycles( 1 );
		OpcodeDispatcher( psxRegs.code );
		hdb_pause = 1;
	}
	
	// wait for breakpoint
	if(hdb_pause == 3) {
		psxRegs.pc+= 4; 
		AddCycles( 1 );
		OpcodeDispatcher( psxRegs.code );
		if(psxRegs.pc == hdb_break) hdb_pause = 1;
	}
}

static void intExecuteDbg() {
	for (;;)
		execIDbg();
}

static void intExecuteBlockDbg(u32 addr) {
	while(psxRegs.pc != addr) execIDbg();
}

R3000Acpu psxIntDbg = {
	intInit,
	intReset,
	intExecuteDbg,
	intExecuteBlockDbg,
	intClear,
	intShutdown
};
#endif