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

/*
 * PSX assembly interpreter.
 */

#include "psxcommon.h"
#include "psxevents.h"
#include "r3000a.h"
#include "gte.h"
#include "psxhle.h"
#include "psxhw.h"
#include "R3000AOpcodeTable.h"

using namespace R3000A;

static int branch2 = 0;
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

// Subsets
static void (*psxBSC[64])() = {
	psxSPECIAL, psxREGIMM, psxJ   , psxJAL  , psxBEQ , psxBNE , psxBLEZ, psxBGTZ,
	psxADDI   , psxADDIU , psxSLTI, psxSLTIU, psxANDI, psxORI , psxXORI, psxLUI ,
	psxCOP0   , psxNULL  , psxCOP2, psxNULL , psxNULL, psxNULL, psxNULL, psxNULL,
	psxNULL   , psxNULL  , psxNULL, psxNULL , psxNULL, psxNULL, psxNULL, psxNULL,
	psxLB     , psxLH    , psxLWL , psxLW   , psxLBU , psxLHU , psxLWR , psxNULL,
	psxSB     , psxSH    , psxSWL , psxSW   , psxNULL, psxNULL, psxSWR , psxNULL, 
	psxNULL   , psxNULL  , gteLWC2, psxNULL , psxNULL, psxNULL, psxNULL, psxNULL,
	psxNULL   , psxNULL  , gteSWC2, psxHLE  , psxNULL, psxNULL, psxNULL, psxNULL 
};


static void (*psxSPC[64])() = {
	psxSLL , psxNULL , psxSRL , psxSRA , psxSLLV   , psxNULL , psxSRLV, psxSRAV,
	psxJR  , psxJALR , psxNULL, psxNULL, psxSYSCALL, psxBREAK, psxNULL, psxNULL,
	psxMFHI, psxMTHI , psxMFLO, psxMTLO, psxNULL   , psxNULL , psxNULL, psxNULL,
	psxMULT, psxMULTU, psxDIV , psxDIVU, psxNULL   , psxNULL , psxNULL, psxNULL,
	psxADD , psxADDU , psxSUB , psxSUBU, psxAND    , psxOR   , psxXOR , psxNOR ,
	psxNULL, psxNULL , psxSLT , psxSLTU, psxNULL   , psxNULL , psxNULL, psxNULL,
	psxNULL, psxNULL , psxNULL, psxNULL, psxNULL   , psxNULL , psxNULL, psxNULL,
	psxNULL, psxNULL , psxNULL, psxNULL, psxNULL   , psxNULL , psxNULL, psxNULL
};

static void (*psxREG[32])() = {
	psxBLTZ  , psxBGEZ  , psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL,
	psxNULL  , psxNULL  , psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL,
	psxBLTZAL, psxBGEZAL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL,
	psxNULL  , psxNULL  , psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL
};

static void (*psxCP0[32])() = {
	psxMFC0, psxNULL, psxCFC0, psxNULL, psxMTC0, psxNULL, psxCTC0, psxNULL,
	psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL,
	psxRFE , psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL,
	psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL
};

static void (*psxCP2[64])() = {
	psxBASIC, gteRTPS , psxNULL , psxNULL, psxNULL, psxNULL , gteNCLIP, psxNULL, // 00
	psxNULL , psxNULL , psxNULL , psxNULL, gteOP  , psxNULL , psxNULL , psxNULL, // 08
	gteDPCS , gteINTPL, gteMVMVA, gteNCDS, gteCDP , psxNULL , gteNCDT , psxNULL, // 10
	psxNULL , psxNULL , psxNULL , gteNCCS, gteCC  , psxNULL , gteNCS  , psxNULL, // 18
	gteNCT  , psxNULL , psxNULL , psxNULL, psxNULL, psxNULL , psxNULL , psxNULL, // 20
	gteSQR  , gteDCPL , gteDPCT , psxNULL, psxNULL, gteAVSZ3, gteAVSZ4, psxNULL, // 28 
	gteRTPT , psxNULL , psxNULL , psxNULL, psxNULL, psxNULL , psxNULL , psxNULL, // 30
	psxNULL , psxNULL , psxNULL , psxNULL, psxNULL, gteGPF  , gteGPL  , gteNCCT  // 38
};

static void (*psxCP2BSC[32])() = {
	gteMFC2, psxNULL, gteCFC2, psxNULL, gteMTC2, psxNULL, gteCTC2, psxNULL,
	psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL,
	psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL,
	psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL, psxNULL
};

#ifdef GTE_TIMING
/*
	If an instruction that reads a GTE register or a GTE command is executed
	before the current GTE command is finished, the cpu will hold until the
	instruction has finished.
	GTE instructions and functions should not be used in delay slots of jumps and branches
*/
static __inline void GteUnitStall( u32 newStall )
{
	if( newStall == 0 ) return;		// not a GTE instruction?

	if( psxRegs.GteUnitCycles > 0 )
	{
		psxRegs.evtCycleCountdown -= psxRegs.GteUnitCycles;
		//SysPrintf("Psx GTE Stall for %d cycles.\n", psxRegs.GteUnitCycles );
	}
	psxRegs.GteUnitCycles = newStall;
}
#endif

static __inline void AddCycles( int amount )
{
	psxRegs.evtCycleCountdown	-= amount;
#ifdef GTE_TIMING
	psxRegs.GteUnitCycles		-= amount;
#endif
}

static void delayRead(int reg, u32 bpc) {
	u32 rold, rnew;

//	SysPrintf("delayRead at %x!\n", psxRegs.pc);
	rold = psxRegs.GPR.r[reg].d;
	psxBSC[_Op_](); // branch delay load
	rnew = psxRegs.GPR.r[reg].d;

	psxRegs.pc = bpc;

	psxRegs.IsDelaySlot = false;

	psxRegs.GPR.r[reg].d = rold;
	execI(); // first branch opcode
	psxRegs.GPR.r[reg].d = rnew;
	TEST_BRANCH();
}

static void delayWrite(int reg, u32 bpc) {

/*	SysPrintf("delayWrite at %x!\n", psxRegs.pc);

	SysPrintf("%s\n", disR3000AF(psxRegs.code, psxRegs.pc-4));
	SysPrintf("%s\n", disR3000AF(PSXMu32(bpc), bpc));*/

	// no changes from normal behavior

	psxBSC[_Op_]();

	psxRegs.IsDelaySlot = false;

	psxRegs.pc = bpc;

	TEST_BRANCH();
}

static void delayReadWrite(int reg, u32 bpc) {

//	SysPrintf("delayReadWrite at %x!\n", psxRegs.pc);

	// the branch delay load is skipped

	psxRegs.IsDelaySlot = false;

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

int R3000A::psxTestLoadDelay(int reg, u32 tmp) {
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
				case 0x00: case 0x01:
				case 0x10: case 0x11: // BLTZ/BGEZ...
					// Xenogears - lbu v0 / beq v0
					// - no load delay (fixes battle loading)
					break;

					if (_tRs_ == reg) return 2;
					break;
			}
			break;

		// J would be just a break;
		case 0x03: // JAL
			if (31 == reg) return 3;
			break;

		case 0x04: case 0x05: // BEQ/BNE
			// Xenogears - lbu v0 / beq v0
			// - no load delay (fixes battle loading)
			break;

			if (_tRs_ == reg || _tRt_ == reg) return 2;
			break;

		case 0x06: case 0x07: // BLEZ/BGTZ
			// Xenogears - lbu v0 / beq v0
			// - no load delay (fixes battle loading)
			break;

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
					if (_tRt_ == reg) return 3;
					break;
				case 0x02: // CFC0
					if (_tRt_ == reg) return 3;
					break;
				case 0x04: // MTC0
					if (_tRt_ == reg) return 2;
					break;
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
							if (_tRt_ == reg) return 3;
							break;
						case 0x02: // CFC2
							if (_tRt_ == reg) return 3;
							break;
						case 0x04: // MTC2
							if (_tRt_ == reg) return 2;
							break;
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

void R3000A::psxDelayTest(int reg, u32 bpc) {
	u32 *code;
	u32 tmp;

	code = (u32 *)PSXM(bpc);
	tmp = ((code == NULL) ? 0 : GETLE32(code));
	psxRegs.IsDelaySlot = true;

	switch (psxTestLoadDelay(reg, tmp)) {
		case 1:
			delayReadWrite(reg, bpc); return;
		case 2:
			delayRead(reg, bpc); return;
		case 3:
			delayWrite(reg, bpc); return;
	}
	psxBSC[_Op_]();

	psxRegs.IsDelaySlot = false;

	psxRegs.pc = bpc;

	TEST_BRANCH();
}

__inline void doBranch(u32 tar) {
	u32 *code;
	u32 tmp;

	branch2 = psxRegs.IsDelaySlot = true;

	branchPC = tar;

	code = (u32 *)PSXM(psxRegs.pc);
	psxRegs.code = ((code == NULL) ? 0 : GETLE32(code));

	debugI();

	psxRegs.pc += 4;
	AddCycles(1);

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

	psxBSC[_Op_]();

	psxRegs.IsDelaySlot = false;

	psxRegs.pc = branchPC;

	TEST_BRANCH();
}

/*********************************************************
* Arithmetic with immediate operand                      *
* Format:  OP rt, rs, immediate                          *
*********************************************************/
void psxADDI()	{ if (!_Rt_) return; _rRtS_ = _rRsS_ + _Imm_ ; }		// Rt = Rs + Im 	(Exception on Integer Overflow)
void psxADDIU()	{ if (!_Rt_) return; _rRtS_ = _rRsS_ + _Imm_ ; }		// Rt = Rs + Im
void psxANDI()	{ if (!_Rt_) return; _rRtU_ = _rRsU_ & _ImmU_; }		// Rt = Rs And Im
void psxORI()	{ if (!_Rt_) return; _rRtU_ = _rRsU_ | _ImmU_; }		// Rt = Rs Or  Im
void psxXORI()	{ if (!_Rt_) return; _rRtU_ = _rRsU_ ^ _ImmU_; }		// Rt = Rs Xor Im
void psxSLTI()	{ if (!_Rt_) return; _rRtS_ = _rRsS_ < _Imm_ ; }		// Rt = Rs < Im		(Signed)
void psxSLTIU()	{ if (!_Rt_) return; _rRtU_ = _rRsU_ < ((u32)_Imm_); }		// Rt = Rs < Im		(Unsigned)

/*********************************************************
* Register arithmetic                                    *
* Format:  OP rd, rs, rt                                 *
*********************************************************/
void psxADD()	{ if (!_Rd_) return; _rRdS_ = _rRsS_ + _rRtS_; }	// Rd = Rs + Rt		(Exception on Integer Overflow)
void psxADDU()	{ if (!_Rd_) return; _rRdS_ = _rRsS_ + _rRtS_; }	// Rd = Rs + Rt
void psxSUB()	{ if (!_Rd_) return; _rRdS_ = _rRsS_ - _rRtS_; }	// Rd = Rs - Rt		(Exception on Integer Overflow)
void psxSUBU()	{ if (!_Rd_) return; _rRdS_ = _rRsS_ - _rRtS_; }	// Rd = Rs - Rt
void psxAND()	{ if (!_Rd_) return; _rRdU_ = _rRsU_ & _rRtU_; }	// Rd = Rs And Rt
void psxOR()	{ if (!_Rd_) return; _rRdU_ = _rRsU_ | _rRtU_; }	// Rd = Rs Or  Rt
void psxXOR()	{ if (!_Rd_) return; _rRdU_ = _rRsU_ ^ _rRtU_; }	// Rd = Rs Xor Rt
void psxNOR()	{ if (!_Rd_) return; _rRdU_ =~(_rRsU_ | _rRtU_); }	// Rd = Rs Nor Rt
void psxSLT()	{ if (!_Rd_) return; _rRdU_ = _rRsS_ < _rRtS_; }	// Rd = Rs < Rt		(Signed)
void psxSLTU()	{ if (!_Rd_) return; _rRdU_ = _rRsU_ < _rRtU_; }	// Rd = Rs < Rt		(Unsigned)

/*********************************************************
* Register mult/div & Register trap logic                *
* Format:  OP rs, rt                                     *
*********************************************************/
__inline void psxDIV() {
	const s32 Rt = _rRtS_;
	const s32 Rs = _rRsS_;

	if( Rt == 0 )
	{
		_rHiS_ = Rs;
		_rLoS_ = (Rs >= 0) ? -1 : 1;
		return;
	}
	if( Rs == 0x80000000 && Rt == 0xffffffff )
	{
		_rHiS_ = 0;
		_rLoS_ = Rs;
		return;
	}

	_rHiS_ = Rs % Rt;
	_rLoS_ = Rs / Rt;
}

__inline void psxDIVU() {
	const u32 Rt = _rRtU_;
	const u32 Rs = _rRsU_;

	if( Rt == 0 )
	{
		_rHiU_ = Rs;
		_rLoU_ = (u32)(-1);	    // unsigned div by zero always returns Rs, 0xffffffff
		return;
	}
	_rHiU_ = Rs % Rt;
	_rLoU_ = Rs / Rt;
}

#ifdef MAME_MULT
static inline void
unsigned_multiply (u32 v1, u32 v2)
{
	u32 a, b, c, d;
	u32 bd, ad, cb, ac;
	u32 mid, mid2, carry_mid = 0;

	a = (v1 >> 16) & 0xffff;
	b = v1 & 0xffff;
	c = (v2 >> 16) & 0xffff;
	d = v2 & 0xffff;

	bd = b * d;
	ad = a * d;
	cb = c * b;
	ac = a * c;

	mid = ad + cb;
	if (mid < ad || mid < cb)
		/* Arithmetic overflow or carry-out */
		carry_mid = 1;

	mid2 = mid + ((bd >> 16) & 0xffff);
	if (mid2 < mid || mid2 < ((bd >> 16) & 0xffff))
		/* Arithmetic overflow or carry-out */
		carry_mid += 1;

	_rLoU_ = (bd & 0xffff) | ((mid2 & 0xffff) << 16);
	_rHiU_ = ac + (carry_mid << 16) + ((mid2 >> 16) & 0xffff);
}


static inline void
signed_multiply (s32 v1, s32 v2)
{
	int neg_sign = 0;

	if (v1 < 0)
	{
		v1 = - v1;
		neg_sign = 1;
	}
	if (v2 < 0)
	{
		v2 = - v2;
		neg_sign = ! neg_sign;
	}

	unsigned_multiply (v1, v2);
	if (neg_sign)
	{
		_rLoU_ = ~ _rLoU_;
		_rHiU_ = ~ _rHiU_;
		_rLoU_ += 1;
		if (_rLoU_ == 0)
			_rHiU_ += 1;
	}
}
#else
__inline void MultHelper( u64 result )
{
	_rHiU_ = (u32)(result >> 32);
	_rLoU_ = (u32)result;
}
#endif
__inline void psxMULT()
{
#ifdef MAME_MULT
	signed_multiply( _rRsS_, _rRtS_ );
#else
	MultHelper( (s64)_rRsS_ * _rRtS_ );
#endif
}

__inline void psxMULTU()
{
#ifdef MAME_MULT
	unsigned_multiply( _rRsU_, _rRtU_ );
#else
	MultHelper( (u64)_rRsU_ * _rRtU_ );
#endif
}

/*********************************************************
* Register branch logic                                  *
* Format:  OP rs, offset                                 *
*********************************************************/
#define RepZBranchi32(op)      if(_rRsS_ op 0) doBranch(_BranchTarget_);
#define RepZBranchLinki32(op)  if(_rRsS_ op 0) { _SetLink(31); doBranch(_BranchTarget_); }

void psxBGEZ()   { RepZBranchi32(>=) }      // Branch if Rs >= 0
void psxBGEZAL() { RepZBranchLinki32(>=) }  // Branch if Rs >= 0 and link
void psxBGTZ()   { RepZBranchi32(>) }       // Branch if Rs >  0
void psxBLEZ()   { RepZBranchi32(<=) }      // Branch if Rs <= 0
void psxBLTZ()   { RepZBranchi32(<) }       // Branch if Rs <  0
void psxBLTZAL() { RepZBranchLinki32(<) }   // Branch if Rs <  0 and link

/*********************************************************
* Shift arithmetic with constant shift                   *
* Format:  OP rd, rt, sa                                 *
*********************************************************/
void psxSLL() { if (!_Rd_) return; _rRdU_ = _rRtU_ << _Sa_; } // Rd = Rt << sa
void psxSRA() { if (!_Rd_) return; _rRdS_ = _rRtS_ >> _Sa_; } // Rd = Rt >> sa (arithmetic)
void psxSRL() { if (!_Rd_) return; _rRdU_ = _rRtU_ >> _Sa_; } // Rd = Rt >> sa (logical)

/*********************************************************
* Shift arithmetic with variant register shift           *
* Format:  OP rd, rt, rs                                 *
*********************************************************/
__inline u32 Shamt() {
	int shamt = (_rRsU_ & 0x1f);
	if(shamt >= 0 && shamt < 32) return shamt;
	return 0;
}

__inline void psxSLLV() { if (!_Rd_) return; _rRdU_ = _rRtU_ << Shamt(); } // Rd = Rt << rs
__inline void psxSRAV() { if (!_Rd_) return; _rRdS_ = _rRtS_ >> Shamt(); } // Rd = Rt >> rs (arithmetic)
__inline void psxSRLV() { if (!_Rd_) return; _rRdU_ = _rRtU_ >> Shamt(); } // Rd = Rt >> rs (logical)

/*********************************************************
* Load higher 16 bits of the first word in GPR with imm  *
* Format:  OP rt, immediate                              *
*********************************************************/
void psxLUI() { if (!_Rt_) return; _rRtS_ = psxRegs.code << 16; } // Upper halfword of Rt = Im

/*********************************************************
* Move from HI/LO to GPR                                 *
* Format:  OP rd                                         *
*********************************************************/
void psxMFHI() { if (!_Rd_) return; _rRdU_ = _rHiU_; } // Rd = Hi
void psxMFLO() { if (!_Rd_) return; _rRdU_ = _rLoU_; } // Rd = Lo

/*********************************************************
* Move to GPR to HI/LO & Register jump                   *
* Format:  OP rs                                         *
*********************************************************/
void psxMTHI() { _rHiU_ = _rRsU_; } // Hi = Rs
void psxMTLO() { _rLoU_ = _rRsU_; } // Lo = Rs

/*********************************************************
* Special purpose instructions                           *
* Format:  OP                                            *
*********************************************************/
void psxBREAK() {
	// Break exception - psx rom doens't handles this
}

void psxSYSCALL() {
	psxRegs.pc -= 4;

	psxException(0x20, psxRegs.IsDelaySlot);
}

void psxRFE() {
//	SysPrintf("psxRFE\n");
	psxRegs.CP0.n.Status = (psxRegs.CP0.n.Status & 0xfffffff0) |
						  ((psxRegs.CP0.n.Status & 0x3c) >> 2);
}

/*********************************************************
* Register branch logic                                  *
* Format:  OP rs, rt, offset                             *
*********************************************************/
#define RepBranchi32(op)      if(_rRsS_ op _rRtS_) doBranch(_BranchTarget_);

void psxBEQ() {	RepBranchi32(==) }  // Branch if Rs == Rt
void psxBNE() {	RepBranchi32(!=) }  // Branch if Rs != Rt

/*********************************************************
* Jump to target                                         *
* Format:  OP target                                     *
*********************************************************/
void psxJ()   {               doBranch(_JumpTarget_); }
void psxJAL() {	_SetLink(31); doBranch(_JumpTarget_); }

/*********************************************************
* Register jump                                          *
* Format:  OP rs, rd                                     *
*********************************************************/
void psxJR()   {
	doBranch(_rRsU_);
	psxJumpTest();
}

void psxJALR() {
	u32 temp = _rRsU_;
	if (_Rd_) { _SetLink(_Rd_); }
	doBranch(temp);
}

/*********************************************************
* Load and store for GPR                                 *
* Format:  OP rt, offset(base)                           *
*********************************************************/

#define _oB_ (_rRsU_ + _Imm_)

void psxLB() {
	if (_Rt_) {
		_rRtS_ = (s8)psxMemRead8(_oB_); 
	} else {
		psxMemRead8(_oB_); 
	}
}

void psxLBU() {
	if (_Rt_) {
		_rRtU_ = psxMemRead8(_oB_);
	} else {
		psxMemRead8(_oB_); 
	}
}

void psxLH() {
	if (_Rt_) {
		_rRtS_ = (s16)psxMemRead16(_oB_);
	} else {
		psxMemRead16(_oB_);
	}
}

void psxLHU() {
	if (_Rt_) {
		_rRtU_ = psxMemRead16(_oB_);
	} else {
		psxMemRead16(_oB_);
	}
}

void psxLW() {
	if (_Rt_) {
		_rRtS_ = psxMemRead32(_oB_);
	} else {
		psxMemRead32(_oB_);
	}
}

__inline void psxLWL() {
	const u32 addr = _oB_;
	const u32 shift = (addr & 3) << 3;
	const u32 mem = psxMemRead32( addr & 0xfffffffc );

	if (!_Rt_) return;
	_rRtU_ = (_rRtU_ & (0x00ffffff >> shift)) | (mem << (24 - shift));

	/*
	Mem = 1234.  Reg = abcd

	0   4bcd   (mem << 24) | (reg & 0x00ffffff)
	1   34cd   (mem << 16) | (reg & 0x0000ffff)
	2   234d   (mem <<  8) | (reg & 0x000000ff)
	3   1234   (mem      ) | (reg & 0x00000000)
	*/
}

__inline void psxLWR() {
	const u32 addr = _oB_;
	const u32 shift = (addr & 3) << 3;
	const u32 mem = psxMemRead32( addr & 0xfffffffc );

	if (!_Rt_) return;
	_rRtU_ = (_rRtU_ & (0xffffff00 << (24-shift))) | (mem >> shift);

	/*
	Mem = 1234.  Reg = abcd

	0   1234   (mem      ) | (reg & 0x00000000)
	1   a123   (mem >>  8) | (reg & 0xff000000)
	2   ab12   (mem >> 16) | (reg & 0xffff0000)
	3   abc1   (mem >> 24) | (reg & 0xffffff00)
	*/
}

void psxSB() { psxMemWrite8 (_oB_, (psxRegs.GPR.r[_Rt_].b.l)); }
void psxSH() { psxMemWrite16(_oB_, (psxRegs.GPR.r[_Rt_].w.l)); }
void psxSW() { psxMemWrite32(_oB_, _rRtU_); }

__inline void psxSWL() {
	const u32 addr = _oB_;
	const u32 shift = (addr & 3) << 3;
	const u32 mem = psxMemRead32(addr & 0xfffffffc);

	psxMemWrite32( (addr & 0xfffffffc),
		(( _rRtU_ >> (24 - shift) )) | (mem & (0xffffff00 << shift))
	);

	/*
	Mem = 1234.  Reg = abcd

	0   123a   (reg >> 24) | (mem & 0xffffff00)
	1   12ab   (reg >> 16) | (mem & 0xffff0000)
	2   1abc   (reg >>  8) | (mem & 0xff000000)
	3   abcd   (reg      ) | (mem & 0x00000000)
	*/
}

__inline void psxSWR() {
	const u32 addr = _oB_;
	const u32 shift = (addr & 3) << 3;
	const u32 mem = psxMemRead32(addr & 0xfffffffc);

	psxMemWrite32( (addr & 0xfffffffc),
		( (_rRtU_ << shift) | (mem & (0x00ffffff >> (24 - shift))) )
	);

	/*
	Mem = 1234.  Reg = abcd

	0   abcd   (reg      ) | (mem & 0x00000000)
	1   bcd4   (reg <<  8) | (mem & 0x000000ff)
	2   cd34   (reg << 16) | (mem & 0x0000ffff)
	3   d234   (reg << 24) | (mem & 0x00ffffff)
	*/
}

/*********************************************************
* Moves between GPR and COPx                             *
* Format:  OP rt, fs                                     *
*********************************************************/
void psxMFC0() { if (!_Rt_) return; _rRtU_ = _rFsU_; }
void psxCFC0() { if (!_Rt_) return; _rRtU_ = _rFsU_; }

void R3000A::psxTestSWInts() {
	// the next code is untested, if u know please
	// tell me if it works ok or not (linuzappz)
	if (psxRegs.CP0.n.Cause & psxRegs.CP0.n.Status & 0x0300 &&
		psxRegs.CP0.n.Status & 0x1) {

		psxException(psxRegs.CP0.n.Cause, psxRegs.IsDelaySlot);
	}
}
#if 1
__inline void MTC0(int reg, u32 val) {
//	SysPrintf("MTC0 %d: %x\n", reg, val);
	switch (reg) {
		case 12: // Status
			psxTestSWInts();
			psxTestIntc();
			break;

		case 13: // Cause
			val &= ~(0xfc00);
			psxTestSWInts();
			break;
	}
	psxRegs.CP0.r[reg].d = val;
}

void psxMTC0() { MTC0(_Rd_, _rRtU_); }
void psxCTC0() { MTC0(_Rd_, _rRtU_); }
#else
__inline void psxMTC0()
{
	_rFsU_ = _rRtU_;
}

__inline void psxCTC0()
{
	_rFsU_ = _rRtU_;
}
#endif
/*********************************************************
* Unknow instruction (would generate an exception)       *
* Format:  ?                                             *
*********************************************************/
void psxNULL() { 
#ifdef PSXCPU_LOG
	PSXCPU_LOG("psx: Unimplemented op %x\n", psxRegs.code);
#endif
SysPrintf("psx: Unimplemented op %x\n", psxRegs.code);
}

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
	if ((psxRegs.CP0.n.Status & 0x40000000) == 0 )
		return;

	psxCP2[_Funct_]();
}

void psxBASIC() {
	psxCP2BSC[_Rs_]();
}

void psxHLE() {
//	psxHLEt[psxRegs.code & 0xffff]();
	psxHLEt[psxRegs.code & 0x07]();		// HDHOSHY experimental patch
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

static void intExecuteBlock() {
	branch2 = 0;
	while (!branch2) execI();
}

static void intClear(u32 Addr, u32 Size) {
}

static void intShutdown() {
}

// interpreter execution
inline void execI() { 
#ifdef GTE_TIMING
	GteStall = 0;
#endif
	u32 *code = (u32 *)PSXM(psxRegs.pc); 
	psxRegs.code = ((code == NULL) ? 0 : GETLE32(code));

	debugI();
#ifndef GEKKO
	if (Config.Debug) ProcessDebug();
#endif
	psxRegs.pc += 4;
	AddCycles(1);

	psxBSC[_Op_]();
#ifdef GTE_TIMING
	GteUnitStall(GteStall);
#endif
}

R3000Acpu R3000A::psxInt = {
	intInit,
	intReset,
	intExecute,
	intExecuteBlock,
	intClear,
	intShutdown
};
