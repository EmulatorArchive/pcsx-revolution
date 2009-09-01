#include "../PsxCommon.h"
#include "R3000A.h"
#include "R3000AOpcodeDispatcher.h"
#include "R3000AOpcodeTable.h"
#include "Gte.h"

#if 0	/* Enable it, when new intrepeter will be done */

#define ex(func)  case (__COUNTER__-baseval): { psx##func(); return; }
#define gte(func) case (__COUNTER__-baseval): { gte##func(); return; }
#define su(func)  case (__COUNTER__-baseval): { _dispatch_##func(code); return; }
#define null()    case (__COUNTER__-baseval): break;

static void _dispatch_SPECIAL( u32 code );
static void _dispatch_REGIMM( u32 code );
static void _dispatch_COP0( u32 code );
static void _dispatch_COP2( u32 code );
static void _dispatch_COP2B( u32 code );

static void _dispatch_SPECIAL( u32 code )
{
	static const int baseval = __COUNTER__ + 1;

	switch( _fFunct_(code) )
	{
		ex(SLL)    null()     ex(SRL)   ex(SRA)     ex(SLLV)    null()    ex(SRLV)  ex(SRAV)
		ex(JR)     ex(JALR)   null()    null()      ex(SYSCALL) ex(BREAK) null()    null()
		ex(MFHI)   ex(MTHI)   ex(MFLO)  ex(MTLO)    null()      null()    null()    null()
		ex(MULT)   ex(MULTU)  ex(DIV)   ex(DIVU)    null()      null()    null()    null()
		ex(ADD)    ex(ADDU)   ex(SUB)   ex(SUBU)    ex(AND)     ex(OR)    ex(XOR)   ex(NOR)
		null()     null()     ex(SLT)   ex(SLTU)    null()      null()    null()    null()

		// R3000A is MIPSI, and does not support TRAP instructions.
	}
	psxNULL();
}

static void _dispatch_REGIMM( u32 code )
{
	static const int baseval = __COUNTER__ + 1;

	switch( _fRt_(code) )
	{
		case 0x00: psxBLTZ(); return;
		case 0x01: psxBGEZ(); return;
		case 0x10: psxBLTZAL(); return;
		case 0x11: psxBGEZAL(); return;
	}
	psxNULL();
}

static void _dispatch_COP0( u32 code )
{
	static const int baseval = __COUNTER__ + 1;

	switch( _fRs_(code) )
	{
		case 0x00: psxMFC0(); return;
		case 0x02: psxCFC0(); return;
		case 0x04: psxMTC0(); return;
		case 0x06: psxCTC0(); return;
		case 0x10: psxRFE(); return;
	}
	psxNULL();
}

static void _dispatch_COP2( u32 code )
{
	static const int baseval = __COUNTER__ + 1;

	switch( _fFunct_(code) )
	{
		su(COP2B)  gte(RTPS)   null()      null()     null()    null()      gte(NCLIP)  null()
		null()     null()      null()      null()     gte(OP)   null()      null()      null()
		gte(DPCS)  gte(INTPL)  gte(MVMVA)  gte(NCDS)  gte(CDP)  null()      gte(NCDT)   null()
		null()     null()      null()      gte(NCCS)  gte(CC)   null()      gte(NCS)    null()
		gte(NCT)   null()      null()      null()     null()    null()      null()      null()
		gte(SQR)   gte(DCPL)   gte(DPCT)   null()     null()    gte(AVSZ3)  gte(AVSZ4)  null()
		gte(RTPT)  null()      null()      null()     null()    null()      null()      null()
		null()     null()      null()      null()     null()    gte(GPF)    gte(GPL)    gte(NCCT)
	}
	psxNULL();
}

static void _dispatch_COP2B( u32 code )
{
	static const int baseval = __COUNTER__ + 1;

	switch( _fRs_(code) )
	{
		case 0x00: gteMFC2(); return;
		case 0x02: gteCFC2(); return;
		case 0x04: gteMTC2(); return;
		case 0x06: gteCTC2(); return;
	}
	psxNULL();
}

void OpcodeDispatcher( u32 code )
{
	static const int baseval = __COUNTER__ + 1;

	switch( code >> 26 )
	{
		su(SPECIAL)  su(REGIMM)  ex(J)      ex(JAL)      ex(BEQ)     ex(BNE)     ex(BLEZ)   ex(BGTZ)
		ex(ADDI)     ex(ADDIU)   ex(SLTI)   ex(SLTIU)    ex(ANDI)    ex(ORI)     ex(XORI)   ex(LUI)
		su(COP0)     null()      su(COP2)   null()       null()      null()      null()     null()
		null()       null()      null()     null()       null()      null()      null()     null()
		ex(LB)       ex(LH)      ex(LWL)    ex(LW)       ex(LBU)     ex(LHU)     ex(LWR)    null()
		ex(SB)       ex(SH)      ex(SWL)    ex(SW)       null()      null()      ex(SWR)    null() 
		null()       null()      gte(LWC2)  null()       null()      null()      null()     null()
		null()       null()      gte(SWC2)  ex(HLE)      null()      null()      null()     null()
	}
	psxNULL();
}

#undef ex
#undef gte
#undef su
#undef null

#else

// Subsets
void (*psxBSC[64])();
void (*psxSPC[64])();
void (*psxREG[32])();
void (*psxCP0[32])();
void (*psxCP2[64])();
void (*psxCP2BSC[32])();

void psxSPECIAL() {
	psxSPC[_Funct_]();
}

void psxREGIMM() {
	psxREG[_Rt_]();
}

void psxCOP0() {
	psxCP0[_Rs_]();
}

void psxCOP2() {
	psxCP2[_Funct_]();
}

void psxBASIC() {
	psxCP2BSC[_Rs_]();
}

void (*psxBSC[64])() = {
	psxSPECIAL, psxREGIMM, psxJ   , psxJAL  , psxBEQ , psxBNE , psxBLEZ, psxBGTZ,
	psxADDI   , psxADDIU , psxSLTI, psxSLTIU, psxANDI, psxORI , psxXORI, psxLUI ,
	psxCOP0   , psxNULL  , psxCOP2, psxNULL , psxNULL, psxNULL, psxNULL, psxNULL,
	psxNULL   , psxNULL  , psxNULL, psxNULL , psxNULL, psxNULL, psxNULL, psxNULL,
	psxLB     , psxLH    , psxLWL , psxLW   , psxLBU , psxLHU , psxLWR , psxNULL,
	psxSB     , psxSH    , psxSWL , psxSW   , psxNULL, psxNULL, psxSWR , psxNULL, 
	psxNULL   , psxNULL  , gteLWC2, psxNULL , psxNULL, psxNULL, psxNULL, psxNULL,
	psxNULL   , psxNULL  , gteSWC2, psxHLE  , psxNULL, psxNULL, psxNULL, psxNULL 
};


void (*psxSPC[64])() = {
	psxSLL , psxNULL , psxSRL , psxSRA , psxSLLV   , psxNULL , psxSRLV, psxSRAV,
	psxJR  , psxJALR , psxNULL, psxNULL, psxSYSCALL, psxBREAK, psxNULL, psxNULL,
	psxMFHI, psxMTHI , psxMFLO, psxMTLO, psxNULL   , psxNULL , psxNULL, psxNULL,
	psxMULT, psxMULTU, psxDIV , psxDIVU, psxNULL   , psxNULL , psxNULL, psxNULL,
	psxADD , psxADDU , psxSUB , psxSUBU, psxAND    , psxOR   , psxXOR , psxNOR ,
	psxNULL, psxNULL , psxSLT , psxSLTU, psxNULL   , psxNULL , psxNULL, psxNULL,
	psxNULL, psxNULL , psxNULL, psxNULL, psxNULL   , psxNULL , psxNULL, psxNULL,
	psxNULL, psxNULL , psxNULL, psxNULL, psxNULL   , psxNULL , psxNULL, psxNULL
};

void (*psxREG[32])() = {
	psxBLTZ  , psxBGEZ  , psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL,
	psxNULL  , psxNULL  , psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL,
	psxBLTZAL, psxBGEZAL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL,
	psxNULL  , psxNULL  , psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL
};

void (*psxCP0[32])() = {
	psxMFC0, psxNULL, psxCFC0, psxNULL, psxMTC0, psxNULL, psxCTC0, psxNULL,
	psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL,
	psxRFE , psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL,
	psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL
};

void (*psxCP2[64])() = {
	psxBASIC, gteRTPS , psxNULL , psxNULL, psxNULL, psxNULL , gteNCLIP, psxNULL, // 00
	psxNULL , psxNULL , psxNULL , psxNULL, gteOP  , psxNULL , psxNULL , psxNULL, // 08
	gteDPCS , gteINTPL, gteMVMVA, gteNCDS, gteCDP , psxNULL , gteNCDT , psxNULL, // 10
	psxNULL , psxNULL , psxNULL , gteNCCS, gteCC  , psxNULL , gteNCS  , psxNULL, // 18
	gteNCT  , psxNULL , psxNULL , psxNULL, psxNULL, psxNULL , psxNULL , psxNULL, // 20
	gteSQR  , gteDCPL , gteDPCT , psxNULL, psxNULL, gteAVSZ3, gteAVSZ4, psxNULL, // 28 
	gteRTPT , psxNULL , psxNULL , psxNULL, psxNULL, psxNULL , psxNULL , psxNULL, // 30
	psxNULL , psxNULL , psxNULL , psxNULL, psxNULL, gteGPF  , gteGPL  , gteNCCT  // 38
};

void (*psxCP2BSC[32])() = {
	gteMFC2, psxNULL, gteCFC2, psxNULL, gteMTC2, psxNULL, gteCTC2, psxNULL,
	psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL,
	psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL,
	psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL
};

void inline OpcodeDispatcher( u32 code )
{
	psxBSC[code >> 26]();
}

#endif
