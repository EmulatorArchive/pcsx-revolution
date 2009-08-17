#include "R3000AOpcodeTable.h"
#include "R3000A.h"
#include "../PsxHLE.h"

/*
 * Arithmetic with immediate operand
 * Format:  OP rt, rs, immediate
 */

__inline void psxADDI()  {	     // Rt = Rs + Im   (Exception on Integer Overflow)
	if (!_Rt_) return; _i32(_rRt_) = _i32(_rRs_) + _Imm_ ;
}
__inline void psxADDIU() { if (!_Rt_) return; _i32(_rRt_) = _i32(_rRs_) + _Imm_ ; }          // Rt = Rs + Im
__inline void psxANDI()  { if (!_Rt_) return; _rRt_ = _u32(_rRs_) & _ImmU_; }                // Rt = Rs And Im
__inline void psxORI()   { if (!_Rt_) return; _rRt_ = _u32(_rRs_) | _ImmU_; }                // Rt = Rs Or  Im
__inline void psxXORI()  { if (!_Rt_) return; _rRt_ = _u32(_rRs_) ^ _ImmU_; }                // Rt = Rs Xor Im
__inline void psxSLTI()  { if (!_Rt_) return; _i32(_rRt_) = _i32(_rRs_) < _Imm_ ; }          // Rt = Rs < Im    (Signed)
__inline void psxSLTIU() { if (!_Rt_) return; _rRt_ = _u32(_rRs_) < ((u32)_Imm_); }          // Rt = Rs < Im    (Unsigned)

/*
 * Register arithmetic
 * Format:  OP rd, rs, rt
 */
__inline void psxADD()  {       // Rd = Rs + Rt  (Exception on Integer Overflow)
	if (!_Rd_) return; _i32(_rRd_) = _i32(_rRs_) + _i32(_rRt_);
}

__inline void psxSUB()  {       // Rd = Rs - Rt  (Exception on Integer Overflow)
	if (!_Rd_) return; _i32(_rRd_) = _i32(_rRs_) - _i32(_rRt_);
}

__inline void psxADDU() { if (!_Rd_) return; _i32(_rRd_) = _i32(_rRs_) + _i32(_rRt_); }      // Rd = Rs + Rt
__inline void psxSUBU() { if (!_Rd_) return; _i32(_rRd_) = _i32(_rRs_) - _i32(_rRt_); }      // Rd = Rs - Rt
__inline void psxAND()  { if (!_Rd_) return; _rRd_ = _u32(_rRs_) & _u32(_rRt_); }            // Rd = Rs And Rt
__inline void psxOR()   { if (!_Rd_) return; _rRd_ = _u32(_rRs_) | _u32(_rRt_); }            // Rd = Rs Or  Rt
__inline void psxXOR()  { if (!_Rd_) return; _rRd_ = _u32(_rRs_) ^ _u32(_rRt_); }            // Rd = Rs Xor Rt
__inline void psxNOR()  { if (!_Rd_) return; _rRd_ =~(_u32(_rRs_) | _u32(_rRt_)); }          // Rd = Rs Nor Rt
__inline void psxSLT()  { if (!_Rd_) return; _rRd_ = _i32(_rRs_) < _i32(_rRt_); }            // Rd = Rs < Rt  (Signed)
__inline void psxSLTU() { if (!_Rd_) return; _rRd_ = _u32(_rRs_) < _u32(_rRt_); }            // Rd = Rs < Rt  (Unsigned)

/*
 * Register mult/div & Register trap logic
 * Format:  OP rs, rt
 */
__inline void psxDIV() {
	const s32 Rt = _i32(_rRt_);
	const s32 Rs = _i32(_rRs_);

	if( Rt == 0 )
	{
		_i32(_rHi_) = Rs;
		_i32(_rLo_) = (Rs >= 0) ? -1 : 1;
		return;
	}
	else if( Rt == -1 )
	{
		if( Rs == (s32)0x80000000 )
		{
			_i32(_rHi_) = 0;
			_i32(_rLo_) = 0x80000000;
			return;
		}
	}
	_i32(_rHi_) = Rs % Rt;
	_i32(_rLo_) = Rs / Rt;
}

__inline void psxDIVU() {
	const u32 Rt = _rRt_;
	const u32 Rs = _rRs_;

	if( Rt == 0 )
	{
		_rHi_ = Rs;
		_rLo_ = (u32)(-1);	    // unsigned div by zero always returns Rs, 0xffffffff
		return;
	}
	_rHi_ = Rs % Rt;
	_rLo_ = Rs / Rt;
}

__inline void MultHelper( u64 result )
{
	_rHi_ = (u32)(result >> 32);
	_rLo_ = (u32)result;
}

__inline void psxMULT()
{
	MultHelper( (s64)_i32(_rRs_) * _i32(_rRt_) );
}

__inline void psxMULTU()
{
	MultHelper( (u64)_u32(_rRs_) * _u32(_rRt_) );
}

/*
 * Register branch logic
 * Format:  OP rs, offset
 */
#define RepZBranchi32(op)      if(_i32(_rRs_) op 0) doBranch(_BranchTarget_);
#define RepZBranchLinki32(op)  if(_i32(_rRs_) op 0) { _SetLink(31); doBranch(_BranchTarget_); }

__inline void psxBGEZ()   { RepZBranchi32(>=) }      // Branch if Rs >= 0
__inline void psxBGTZ()   { RepZBranchi32(>) }       // Branch if Rs >  0
__inline void psxBLEZ()   { RepZBranchi32(<=) }      // Branch if Rs <= 0
__inline void psxBLTZ()   { RepZBranchi32(<) }       // Branch if Rs <  0

__inline void psxBGEZAL()       // Branch if Rs >= 0 and link
{
	_SetLink(31);
	psxBGEZ();
}

__inline void psxBLTZAL()       // Branch if Rs <  0 and link
{
	_SetLink(31);
	psxBLTZ();
}

/*
 * Shift arithmetic with constant shift
 * Format:  OP rd, rt, sa
 */
__inline void psxSLL() { if (!_Rd_) return; _u32(_rRd_) = _u32(_rRt_) << _Sa_; } // Rd = Rt << sa
__inline void psxSRA() { if (!_Rd_) return; _i32(_rRd_) = _i32(_rRt_) >> _Sa_; } // Rd = Rt >> sa (arithmetic)
__inline void psxSRL() { if (!_Rd_) return; _u32(_rRd_) = _u32(_rRt_) >> _Sa_; } // Rd = Rt >> sa (logical)

/*
 * Shift arithmetic with variant register shift
 * Format:  OP rd, rt, rs
 */
__inline void psxSLLV() { if (!_Rd_) return; _u32(_rRd_) = _u32(_rRt_) << (_u32(_rRs_) & 0x1f); } // Rd = Rt << rs
__inline void psxSRAV() { if (!_Rd_) return; _i32(_rRd_) = _i32(_rRt_) >> (_u32(_rRs_) & 0x1f); } // Rd = Rt >> rs (arithmetic)
__inline void psxSRLV() { if (!_Rd_) return; _u32(_rRd_) = _u32(_rRt_) >> (_u32(_rRs_) & 0x1f); } // Rd = Rt >> rs (logical)

/*
 * Load higher 16 bits of the first word in GPR with imm
 * Format:  OP rt, immediate
 */
__inline void psxLUI() { if (!_Rt_) return; _i32(_rRt_) = _Imm_ << 16; } // Upper halfword of Rt = Im

/*
 * Move from HI/LO to GPR
 * Format:  OP rd
 */
__inline void psxMFHI() { if (!_Rd_) return; _rRd_ = _rHi_; } // Rd = Hi
__inline void psxMFLO() { if (!_Rd_) return; _rRd_ = _rLo_; } // Rd = Lo

/*
 * Move to GPR to HI/LO & Register jump
 * Format:  OP rs
 */
__inline void psxMTHI() { _rHi_ = _rRs_; } // Hi = Rs
__inline void psxMTLO() { _rLo_ = _rRs_; } // Lo = Rs

/*
 * Special purpose instructions
 * Format:  OP
 */
__inline void psxBREAK() {
	// Break exception - psx rom doens't handles this
}

__inline void psxSYSCALL() {
	psxRegs.pc -= 4;
	psxException(0x20, psxRegs.IsDelaySlot);
}

__inline void psxRFE() {
//      SysPrintf("psxRFE\n");
	psxRegs.CP0.n.Status = (psxRegs.CP0.n.Status & 0xfffffff0) |
						  ((psxRegs.CP0.n.Status & 0x3c) >> 2);
}

/*
 * Register branch logic
 * Format:  OP rs, rt, offset
 */
#define RepBranchi32(op)      if(_i32(_rRs_) op _i32(_rRt_)) doBranch(_BranchTarget_);

__inline void psxBEQ() {	RepBranchi32(==) }  // Branch if Rs == Rt
__inline void psxBNE() {	RepBranchi32(!=) }  // Branch if Rs != Rt

/*
 * Jump to target
 * Format:  OP target
 */
__inline void psxJ()   { doBranch(_JumpTarget_); }
__inline void psxJAL() { _SetLink(31); psxJ(); }

/*
 * Register jump
 * Format:  OP rs, rd
 */
__inline void psxJR()   {
	doBranch(_u32(_rRs_));
	psxJumpTest();
}

__inline void psxJALR() {
	_SetLink(_Rd_);
	psxJR();
}

/*
 * Load and store for GPR
 * Format:  OP rt, offset(base)
 */

#define _oB_ (_u32(_rRs_) + _Imm_)

__inline void psxLB() {
	if (_Rt_) {
		_i32(_rRt_) = (s8)psxMemRead8(_oB_);
	} else {
		psxMemRead8(_oB_);
	}
}

__inline void psxLBU() {
	if (_Rt_) {
		_u32(_rRt_) = psxMemRead8(_oB_);
	} else {
		psxMemRead8(_oB_);
	}
}

__inline void psxLH() {
	if (_Rt_) {
		_i32(_rRt_) = (s16)psxMemRead16(_oB_);
	} else {
		psxMemRead16(_oB_);
	}
}

__inline void psxLHU() {
	if (_Rt_) {
		_u32(_rRt_) = psxMemRead16(_oB_);
	} else {
		psxMemRead16(_oB_);
	}
}

__inline void psxLW() {
	if (_Rt_) {
		_u32(_rRt_) = psxMemRead32(_oB_);
	} else {
		psxMemRead32(_oB_);
	}
}

__inline void psxLWL() {
	const u32 addr = _oB_;
	const u32 shift = (addr & 3) << 3;
	const u32 mem = psxMemRead32( addr & 0xfffffffc );

	if (!_Rt_) return;
	_u32(_rRt_) = (_u32(_rRt_) & (0x00ffffff >> shift)) | (mem << (24 - shift));

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
	_u32(_rRt_) = (_u32(_rRt_) & (0xffffff00 << (24-shift))) | (mem >> shift);

	/*
	Mem = 1234.  Reg = abcd

	0   1234   (mem      ) | (reg & 0x00000000)
	1   a123   (mem >>  8) | (reg & 0xff000000)
	2   ab12   (mem >> 16) | (reg & 0xffff0000)
	3   abc1   (mem >> 24) | (reg & 0xffffff00)
	*/
}

__inline void psxSB() { psxMemWrite8 (_oB_, _u8 (_rRt_)); }
__inline void psxSH() { psxMemWrite16(_oB_, _u16(_rRt_)); }
__inline void psxSW() { psxMemWrite32(_oB_, _u32(_rRt_)); }

__inline void psxSWL() {
	const u32 addr = _oB_;
	const u32 shift = (addr & 3) << 3;
	const u32 mem = psxMemRead32(addr & 0xfffffffc);

	psxMemWrite32( (addr & 0xfffffffc),
		(( _u32(_rRt_) >> (24 - shift) )) | (mem & (0xffffff00 << shift))
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
		( (_u32(_rRt_) << shift) | (mem & (0x00ffffff >> (24 - shift))) )
	);

	/*
	Mem = 1234.  Reg = abcd

	0   abcd   (reg      ) | (mem & 0x00000000)
	1   bcd4   (reg <<  8) | (mem & 0x000000ff)
	2   cd34   (reg << 16) | (mem & 0x0000ffff)
	3   d234   (reg << 24) | (mem & 0x00ffffff)
	*/
}

/*
 * Moves between GPR and COPx
 * Format:  OP rt, fs
 */

__inline void psxMFC0() { if (!_Rt_) return; _rRt_ = _rFs_; }
__inline void psxCFC0() { if (!_Rt_) return; _rRt_ = _rFs_; }

__inline void psxMTC0()
{
	psxRegs.CP0.r[_Rd_] = _rRt_;
}

__inline void psxCTC0()
{
	psxRegs.CP0.r[_Rd_] = _rRt_;
}

/*
 * Unknow instruction (would generate an exception)
 * Format:  ?
 */
void psxNULL() {
#ifdef PSXCPU_LOG
	PSXCPU_LOG("psx: Unimplemented op 0x%x\n", psxRegs.code);
#endif
	SysPrintf("psx: Unimplemented op 0x%x\n", psxRegs.code);
}

void psxHLE() {
//      psxHLEt[psxRegs.code & 0xffff]();
	psxHLEt[psxRegs.code & 0x07]();  // HDHOSHY experimental patch
}
