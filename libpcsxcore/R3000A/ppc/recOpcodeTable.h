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

#ifndef _recR3000ATABLE_H_
#define _recR3000ATABLE_H_

#include "../R3000AOpcodeTable.h"

#define REC_FUNC(f) \
static void rec##f() { \
	iFlushRegs(); \
	LIW(r4, (u32)psxRegs.code); \
	LIW(r3, (u32)pc); \
	STWRtoPR(&psxRegs.code, r4); \
	STWRtoPR(&psxRegs.pc, r3); \
	CALLFunc((u32)psx##f); \
}

#define REC_SYS(f) \
void psx##f();\
static void rec##f() { \
	iFlushRegs(); \
	LIW(r4, (u32)psxRegs.code); \
	LIW(r3, (u32)pc); \
	STWRtoPR(&psxRegs.code, r4); \
	STWRtoPR(&psxRegs.pc, r3); \
	CALLFunc((u32)psx##f); \
	branch = 2; \
	iRet(); \
}

#define REC_BRANCH(f) \
static void rec##f() { \
	iFlushRegs(); \
	LIW(r4, (u32)psxRegs.code); \
	LIW(r3, (u32)pc); \
	STWRtoPR(&psxRegs.code, r4); \
	STWRtoPR(&psxRegs.pc, r3); \
	CALLFunc((u32)psx##f); \
	branch = 2; \
	iRet(); \
}

/* - Arithmetic with immediate operand - */
/*********************************************************
* Arithmetic with immediate operand                      *
* Format:  OP rt, rs, immediate                          *
*********************************************************/

#if 0
REC_FUNC(SLTI);
REC_FUNC(SLTIU);
REC_FUNC(ADDI);
REC_FUNC(ADDIU);
REC_FUNC(ANDI);
REC_FUNC(ORI);
REC_FUNC(XORI)
#else
//CR0:	SIGN      | POSITIVE | ZERO  | SOVERFLOW | SOVERFLOW | OVERFLOW | CARRY
#ifdef NEW_REGS
static void recSLTI() {
// Rt = Rs < Im (signed)
	if (!_Rt_) return;

	if (IsConst(_Rs_)) {
		MapConst(_Rt_, (s32)iRegs[_Rs_].k < _Imm_);
	} else {
		int Rs = HWRegs.Get(_Rs_);
		int Rt = HWRegs.Put(_Rt_);
		CMPI(cr7, Rs, _Imm_);
		MFCR(Rt);
		RLWINM(Rt, Rt, 29, 31, 31);
	}
}

static void recSLTIU() {
// Rt = Rs < Im (unsigned)
	if (!_Rt_) return;

	if (IsConst(_Rs_)) {
		MapConst(_Rt_, iRegs[_Rs_].k < (u32)_ImmU_);
	} else {
		int Rs = HWRegs.Get(_Rs_);
		int Rt = HWRegs.Put(_Rt_);
		SUBIC(Rt, Rs, _ImmU_);
		SUBFE(Rt, Rt, Rt);
		NEG(Rt, Rt)
	}
}

static void recADDIU() {
// Rt = Rs + Im
	if (!_Rt_) return;

	if (IsConst(_Rs_)) {
		MapConst(_Rt_, iRegs[_Rs_].k + _Imm_);
	} else {
		int RS = HWRegs.Get(_Rs_);
		int RT = HWRegs.Put(_Rt_);
		ADDI(RT, RS, _Imm_);
	}
}

static void recADDI() {
// Rt = Rs + Im
	recADDIU();
}
static void recANDI() {
// Rt = Rs And Im
	if (!_Rt_) return;

	if (IsConst(_Rs_)) {
		MapConst(_Rt_, iRegs[_Rs_].k & _ImmU_);
	} else {
		int RS = HWRegs.Get(_Rs_);
		int RT = HWRegs.Put(_Rt_);
		ANDI_(RT, RS, _ImmU_);
	}
}

static void recORI() {
// Rt = Rs Or Im
	if (!_Rt_) return;

	if (IsConst(_Rs_)) {
		MapConst(_Rt_, iRegs[_Rs_].k | _ImmU_);
	} else {
		int RS = HWRegs.Get(_Rs_);
		int RT = HWRegs.Put(_Rt_);
		ORI(RT, RS, _ImmU_);
	}
}

static void recXORI() {
// Rt = Rs Xor Im
	if (!_Rt_) return;

	if (IsConst(_Rs_)) {
		MapConst(_Rt_, iRegs[_Rs_].k ^ _ImmU_);
	} else {
		XORI(HWRegs.Put(_Rt_), HWRegs.Get(_Rs_), _ImmU_);
	}
}

#else // NEW_REGS

static void recSLTI() {
// Rt = Rs < Im (signed)
	if (!_Rt_) return;

	if (IsConst(_Rs_)) {
		MapConst(_Rt_, (s32)iRegs[_Rs_].k < _Imm_);
	} else {
		iRegs[_Rt_].state = ST_UNK;

		LWPRtoR(r3, &_rRsS_);
		LIW(r4, _Imm_);
		CMP(cr7, r3, r4);
		MFCR(r3);
		RLWINM(r3, r3, 29, 31, 31);
		STWRtoPR(&_rRtS_, r3);
	}
}

static void recSLTIU() {
// Rt = Rs < Im (unsigned)
	if (!_Rt_) return;

	if (IsConst(_Rs_)) {
		MapConst(_Rt_, iRegs[_Rs_].k < (u32)_ImmU_);
	} else {
		iRegs[_Rt_].state = ST_UNK;

		LWPRtoR(r3, &_rRsU_);
		LIW(r4, _ImmU_);
		SUBFC(r4, r4, r3);
		SUBFE(r4, r4, r4);
		NEG(r4, r4);
		STWRtoPR(&_rRtU_, r4);
	}
}

static void recADDIU() {
// Rt = Rs + Im
	if (!_Rt_) return;

	if (IsConst(_Rs_)) {
		MapConst(_Rt_, iRegs[_Rs_].k + _Imm_);
	} else {
		iRegs[_Rt_].state = ST_UNK;

		LWPRtoR(r3, &_rRsS_);
		LIW(r4, _Imm_);
		ADD(r3, r4, r3)
		STWRtoPR(&_rRtS_, r3);
	}
}

static void recADDI() {
// Rt = Rs + Im
	recADDIU();
}
static void recANDI() {
// Rt = Rs And Im
	if (!_Rt_) return;

	if (IsConst(_Rs_)) {
		MapConst(_Rt_, iRegs[_Rs_].k & _ImmU_);
	} else {
		iRegs[_Rt_].state = ST_UNK;
		LWPRtoR(r3, &_rRsU_);
		ANDI_(r3, r3, _ImmU_);
		//RLWINM(r3, r3, 0, 16, 31);
		STWRtoPR(&_rRtU_, r3);
	}
}

static void recORI() {
// Rt = Rs Or Im
	if (!_Rt_) return;

	if (IsConst(_Rs_)) {
		MapConst(_Rt_, iRegs[_Rs_].k | _ImmU_);
	} else {
		iRegs[_Rt_].state = ST_UNK;

		LWPRtoR(r3, &_rRsU_);
		ORI(r3, r3, _ImmU_);
		STWRtoPR(&_rRtU_, r3);
	}
}

static void recXORI() {
// Rt = Rs Xor Im
	if (!_Rt_) return;

	if (IsConst(_Rs_)) {
		MapConst(_Rt_, iRegs[_Rs_].k ^ _ImmU_);
	} else {
		iRegs[_Rt_].state = ST_UNK;
		LWPRtoR(r3, &_rRsU_);
		XORI(r3, r3, _ImmU_);
		STWRtoPR(&_rRtU_, r3);
	}
}
#endif // NEW_REGS
#endif
//end of * Arithmetic with immediate operand  

/*********************************************************
* Load higher 16 bits of the first word in GPR with imm  *
* Format:  OP rt, immediate                              *
*********************************************************/

static void recLUI()  {
// Rt = Imm << 16
	if (!_Rt_) return;

	MapConst(_Rt_, _Imm_ << 16);
}

//End of Load Higher .....

/* - Register arithmetic - */
/*********************************************************
* Register arithmetic                                    *
* Format:  OP rd, rs, rt                                 *
*********************************************************/

#if 0
REC_FUNC(ADD);
REC_FUNC(ADDU);
REC_FUNC(SUB);
REC_FUNC(SUBU);
REC_FUNC(AND);
REC_FUNC(OR);
REC_FUNC(XOR);
REC_FUNC(NOR);
REC_FUNC(SLT);
REC_FUNC(SLTU);
#else
#ifdef NEW_REGS

static void recADDU() {
// Rd = Rs + Rt 
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, (s32)iRegs[_Rs_].k + (s32)iRegs[_Rt_].k);
		return;
	}

	ADD(HWRegs.Put(_Rd_), HWRegs.Get(_Rs_), HWRegs.Get(_Rt_));
}

static void recADD() {
// Rd = Rs + Rt
	recADDU();
}

static void recSUBU() {
// Rd = Rs - Rt
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, (s32)iRegs[_Rs_].k - (s32)iRegs[_Rt_].k);
		return;
	}
	SUB(HWRegs.Put(_Rd_), HWRegs.Get(_Rs_), HWRegs.Get(_Rt_));
}

static void recSUB() {
// Rd = Rs - Rt
	recSUBU();
}

static void recAND() {
// Rd = Rs And Rt
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rs_].k & iRegs[_Rt_].k);
		return;
	}

	AND(HWRegs.Put(_Rd_), HWRegs.Get(_Rs_), HWRegs.Get(_Rt_));
}

static void recOR() {
// Rd = Rs Or Rt
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rs_].k | iRegs[_Rt_].k);
		return;
	}

	OR(HWRegs.Put(_Rd_), HWRegs.Get(_Rs_), HWRegs.Get(_Rt_));
}

static void recXOR() {
// Rd = Rs Xor Rt
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rs_].k ^ iRegs[_Rt_].k);
		return;
	}

	int Rs = HWRegs.Get(_Rs_);
	int Rt = HWRegs.Get(_Rt_);
	int Rd = HWRegs.Put(_Rd_);

	XOR(Rd, Rs, Rt);
}

static void recNOR() {
// Rd = Rs Nor Rt
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, ~(iRegs[_Rs_].k | iRegs[_Rt_].k));
		return;
	}

	int Rs = HWRegs.Get(_Rs_);
	int Rt = HWRegs.Get(_Rt_);
	int Rd = HWRegs.Put(_Rd_);

	NOR(Rd, Rs, Rt);
}

static void recSLT() {
// Rd = Rs < Rt (signed)
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, (s32)iRegs[_Rs_].k < (s32)iRegs[_Rt_].k);
		return;
	}
	int Rs = HWRegs.Get(_Rs_);
	int Rt = HWRegs.Get(_Rt_);
	int Rd = HWRegs.Put(_Rd_);
	CMP(cr7, Rs, Rt);
	MFCR(Rd);
	RLWINM(Rd, Rd, 29, 31, 31);
}

static void recSLTU() { 
// Rd = Rs < Rt (unsigned)
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rs_].k < iRegs[_Rt_].k);
		return;
	}
	int Rs = HWRegs.Get(_Rs_);
	int Rt = HWRegs.Get(_Rt_);
	int Rd = HWRegs.Put(_Rd_);
	SUBFC(Rd, Rt, Rs);
	SUBFE(Rd, Rd, Rd);
	NEG(Rd, Rd);

}

#else // NEW_REGS

static void recADDU() {
// Rd = Rs + Rt 
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, (s32)iRegs[_Rs_].k + (s32)iRegs[_Rt_].k);
		return;
	} else if (IsConst(_Rs_)) {
		LWPRtoR(r4, &_rRtS_);
		LIW(r3, (s32)iRegs[_Rs_].k);
	} else if (IsConst(_Rt_)) {
		LWPRtoR(r3, &_rRsS_);
		LIW(r4, (s32)iRegs[_Rt_].k);
	}
	else {
		LWPRtoR(r3, &_rRsS_);
		LWPRtoR(r4, &_rRtS_);
	}
	iRegs[_Rd_].state = ST_UNK;
	
	ADD(r3, r3, r4);
	STWRtoPR(&_rRdS_, r3);
}

static void recADD() {
// Rd = Rs + Rt
	recADDU();
}

static void recSUBU() {
// Rd = Rs - Rt
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, (s32)iRegs[_Rs_].k - (s32)iRegs[_Rt_].k);
		return;
	} else if (IsConst(_Rs_)) {
		LIW(r3, (s32)iRegs[_Rs_].k);
		LWPRtoR(r4, &_rRtS_);
	} else if (IsConst(_Rt_)) {
		LWPRtoR(r3, &_rRsS_);
		LIW(r4, (s32)iRegs[_Rt_].k);
	}
	else {
		LWPRtoR(r3, &_rRsS_);
		LWPRtoR(r4, &_rRtS_);
	}
	iRegs[_Rd_].state = ST_UNK;

	SUBF(r3, r4, r3);
	STWRtoPR(&_rRdS_, r3);
}

static void recSUB() {
// Rd = Rs - Rt
	recSUBU();
}

static void recAND() {
// Rd = Rs And Rt
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rs_].k & iRegs[_Rt_].k);
		return;
	} else if (IsConst(_Rs_)) {
		LIW(r4, iRegs[_Rs_].k);
		LWPRtoR(r3, &_rRtU_);
	} else if (IsConst(_Rt_)) {
		LIW(r3, iRegs[_Rt_].k);
		LWPRtoR(r4, &_rRsU_);
	} else {
		LWPRtoR(r3, &_rRtU_);
		LWPRtoR(r4, &_rRsU_);
	}
	iRegs[_Rd_].state = ST_UNK;

	AND(r3, r3, r4);
	STWRtoPR(&_rRdU_, r3);
}

static void recOR() {
// Rd = Rs Or Rt
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rs_].k | iRegs[_Rt_].k);
		return;
	} else if (IsConst(_Rs_)) {
		LIW(r4, iRegs[_Rs_].k);
		LWPRtoR(r3, &_rRtU_);
	} else if (IsConst(_Rt_)) {
		LIW(r3, iRegs[_Rt_].k);
		LWPRtoR(r4, &_rRsU_);
	} else {
		LWPRtoR(r3, &_rRtU_);
		LWPRtoR(r4, &_rRsU_);
	}
	iRegs[_Rd_].state = ST_UNK;

	OR(r3, r3, r4);
	STWRtoPR(&_rRdU_, r3);
}

static void recXOR() {
// Rd = Rs Xor Rt
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rs_].k ^ iRegs[_Rt_].k);
		return;
	} else if (IsConst(_Rs_)) {
		LIW(r4, iRegs[_Rs_].k);
		LWPRtoR(r3, &_rRtU_);
	} else if (IsConst(_Rt_)) {
		LIW(r3, iRegs[_Rt_].k);
		LWPRtoR(r4, &_rRsU_);
	} else {
		LWPRtoR(r3, &_rRtU_);
		LWPRtoR(r4, &_rRsU_);
	}
	iRegs[_Rd_].state = ST_UNK;

	XOR(r3, r3, r4);
	STWRtoPR(&_rRdU_, r3);
}

static void recNOR() {
// Rd = Rs Nor Rt
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, ~(iRegs[_Rs_].k | iRegs[_Rt_].k));
		return;
	} else if (IsConst(_Rs_)) {
		LIW(r4, iRegs[_Rs_].k);
		LWPRtoR(r3, &_rRtU_);
	} else if (IsConst(_Rt_)) {
		LIW(r3, iRegs[_Rt_].k);
		LWPRtoR(r4, &_rRsU_);
	} else {
		LWPRtoR(r3, &_rRtU_);
		LWPRtoR(r4, &_rRsU_);
	}
	iRegs[_Rd_].state = ST_UNK;

	NOR(r3, r3, r4);
	STWRtoPR(&_rRdU_, r3);
}

static void recSLT() {
// Rd = Rs < Rt (signed)
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, (s32)iRegs[_Rs_].k < (s32)iRegs[_Rt_].k);
		return;
	} else if(IsConst(_Rs_)) {
		LWPRtoR(r4, &_rRtS_);
		LIW(r3, (s32)iRegs[_Rs_].k);
	} else if (IsConst(_Rt_)) {
		LIW(r4, (s32)iRegs[_Rt_].k);
		LWPRtoR(r3, &_rRsS_);
	} else {
		LWPRtoR(r4, &_rRtS_);
		LWPRtoR(r3, &_rRsS_);
	}
	iRegs[_Rd_].state = ST_UNK;

	CMP(cr7, r3, r4);
	MFCR(r3);
	RLWINM(r3, r3, 29, 31, 31);
	STWRtoPR(&_rRdS_, r3);
}

static void recSLTU() { 
// Rd = Rs < Rt (unsigned)
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rs_].k < iRegs[_Rt_].k);
		return;
	} else if(IsConst(_Rs_)) {
		LWPRtoR(r4, &_rRtU_);
		LIW(r3, iRegs[_Rs_].k);
	} else if (IsConst(_Rt_)) {
		LIW(r4, iRegs[_Rt_].k);
		LWPRtoR(r3, &_rRsU_);
	} else {
		LWPRtoR(r4, &_rRtU_);
		LWPRtoR(r3, &_rRsU_);
	}
	iRegs[_Rd_].state = ST_UNK;

	SUBFC(r4, r4, r3);
	SUBFE(r4, r4, r4);
	NEG(r4, r4);
	STWRtoPR(&_rRdU_, r4);
}
#endif
#endif
//End of * Register arithmetic

/* - mult/div & Register trap logic - */
/*********************************************************
* Register mult/div & Register trap logic                *
* Format:  OP rs, rt                                     *
*********************************************************/

#if 0
REC_FUNC(MULT);
REC_FUNC(MULTU);
#else
static void recMULT() {
// Lo/Hi = Rs * Rt (signed)
	if(IsConst(_Rs_) && IsConst(_Rt_)) {
		s64 res = iRegs[_Rs_].k * iRegs[_Rt_].k;
		MapConst(REG_LO, (u32)(res >> 32));
		MapConst(REG_HI, (u32)res);
		return;
	}
	// TODO: Const cases

	MULHW(HWRegs.Put(REG_HI), HWRegs.Get(_Rs_), HWRegs.Get(_Rt_));
	MULLW(HWRegs.Put(REG_LO), HWRegs.Get(_Rs_), HWRegs.Get(_Rt_));
}

static void recMULTU() {
// Lo/Hi = Rs * Rt (unsigned)
	if(IsConst(_Rs_) && IsConst(_Rt_)) {
		u64 res = iRegs[_Rs_].k * iRegs[_Rt_].k;
		MapConst(REG_LO, (u32)(res >> 32));
		MapConst(REG_HI, (u32)res);
		return;
	}
	// TODO: Const cases

	MULHWU(HWRegs.Put(REG_HI), HWRegs.Get(_Rs_), HWRegs.Get(_Rt_));
	MULLW(HWRegs.Put(REG_LO), HWRegs.Get(_Rs_), HWRegs.Get(_Rt_));
}
#endif

#if 1
REC_FUNC(DIV);
REC_FUNC(DIVU);
#else
#ifdef NEW_REGS
static void recDIV() {
// Lo/Hi = Rs / Rt (signed)

	if(IsConst(_Rs_) && IsConst(_Rt_)) {
		if( iRegs[_Rt_].k == 0 )
		{
			MapConst(REG_LO, ((iRegs[_Rs_].k >= 0) ? -1 : 1));
			MapConst(REG_HI, iRegs[_Rs_].k);
		}
		else if( iRegs[_Rs_].k == 0x80000000 && iRegs[_Rt_].k == 0xffffffff )
		{
			MapConst(REG_LO, iRegs[_Rs_].k);
			MapConst(REG_HI, 0);
		}
		else {
			MapConst(REG_LO, iRegs[_Rs_].k / iRegs[_Rt_].k);
			MapConst(REG_HI, iRegs[_Rs_].k % iRegs[_Rt_].k);
		}
		return;
	}
	//TODO: cons cases
	/*
	else if(IsConst(_Rs_)) {
		LWPRtoR(r4, &_rRtS_);
		LIW(r3, iRegs[_Rs_].k);
	}
	else if(IsConst(_Rt_)) {
		LWPRtoR(r3, &_rRsS_);
		LIW(r4, iRegs[_Rt_].k);
	}*/

	CMPWI(HWRegs.Get(_Rt_), 0);

	BranchTarget rteq0(BT_NE);

	NOT(HWRegs.Put(REG_LO), HWRegs.Get(_Rs_));
	MR(HWRegs.Put(REG_HI), HWRegs.Get(_Rs_));

	SRAWI(HWRegs.Put(REG_LO), HWRegs.Get(REG_LO), 31);
	ORI(HWRegs.Put(REG_LO), HWRegs.Get(REG_LO), 1);

	BranchTarget exit0(BT_UN);

	rteq0.setTarget();

	LIS(r3, -32768);
	CMPW(HWRegs.Get(_Rs_), r3);

	BranchTarget rscheck(BT_NE);

	CMPWI(HWRegs.Get(_Rt_), -1);

	BranchTarget rtcheck(BT_NE);

	MR(HWRegs.Put(REG_LO), HWRegs.Get(_Rs_));
	LI(HWRegs.Put(REG_HI), 0);

	BranchTarget exit1(BT_UN);

	rscheck.setTarget();
	rtcheck.setTarget();

	DIVW(HWRegs.Put(REG_LO), HWRegs.Get(_Rs_), HWRegs.Get(_Rt_));	// rLo = Rs / Rt 
	MULLW(HWRegs.Put(REG_HI), HWRegs.Get(REG_LO), HWRegs.Get(_Rt_));
	SUBF(HWRegs.Put(REG_HI), HWRegs.Get(REG_HI), HWRegs.Get(_Rs_));	// rHi = Rs % Rt

	exit0.setTarget();
	exit1.setTarget();
}

static void recDIVU() {
// Lo/Hi = Rs / Rt (unsigned)

	if(IsConst(_Rs_) && IsConst(_Rt_)) {
		if( iRegs[_Rt_].k == 0 )
		{
			MapConst(REG_LO, (u32)(-1));
			MapConst(REG_HI, iRegs[_Rs_].k);
		}
		else {
			MapConst(REG_LO, iRegs[_Rs_].k / iRegs[_Rt_].k);
			MapConst(REG_HI, iRegs[_Rs_].k % iRegs[_Rt_].k);
		}
		return;
	}
	// TODO: const cases
	/*else if(IsConst(_Rs_)) {
		LWPRtoR(r4, &_rRtU_);
		LIW(r3, iRegs[_Rs_].k);
	}
	else if(IsConst(_Rt_)) {
		LWPRtoR(r3, &_rRsU_);
		LIW(r4, iRegs[_Rt_].k);
	}*/

	CMPWI(HWRegs.Get(_Rt_), 0);

	BranchTarget eq(BT_EQ);

	DIVWU(HWRegs.Put(REG_LO), HWRegs.Get(_Rs_), HWRegs.Get(_Rt_));	// rLo = Rs / Rt 
	MULLW(HWRegs.Put(REG_HI), HWRegs.Get(REG_LO), HWRegs.Get(_Rt_));
	SUBF(HWRegs.Put(REG_HI), HWRegs.Get(REG_HI), HWRegs.Get(_Rs_));	// rHi = Rs % Rt

	BranchTarget exit(BT_UN);

	eq.setTarget();

	MR(HWRegs.Put(REG_HI), HWRegs.Get(_Rs_));
	LIW(HWRegs.Put(REG_LO), (u32)(-1));

	exit.setTarget();
}
#else // NEW_REGS
static void recDIV() {
// Lo/Hi = Rs / Rt (signed)

	if(IsConst(_Rs_) && IsConst(_Rt_)) {
		if( iRegs[_Rt_].k == 0 )
		{
			MapConst(REG_LO, ((iRegs[_Rs_].k >= 0) ? -1 : 1));
			MapConst(REG_HI, iRegs[_Rs_].k);
		}
		else if( iRegs[_Rs_].k == 0x80000000 && iRegs[_Rt_].k == 0xffffffff )
		{
			MapConst(REG_LO, iRegs[_Rs_].k);
			MapConst(REG_HI, 0);
		}
		else {
			MapConst(REG_LO, iRegs[_Rs_].k / iRegs[_Rt_].k);
			MapConst(REG_HI, iRegs[_Rs_].k % iRegs[_Rt_].k);
		}
		return;
	}
	else if(IsConst(_Rs_)) {
		LWPRtoR(r4, &_rRtS_);
		LIW(r3, iRegs[_Rs_].k);
	}
	else if(IsConst(_Rt_)) {
		LWPRtoR(r3, &_rRsS_);
		LIW(r4, iRegs[_Rt_].k);
	}
	else {
		LWPRtoR(r4, &_rRtS_);
		LWPRtoR(r3, &_rRsS_);
	}
	iRegs[REG_LO].state = ST_UNK;
	iRegs[REG_HI].state = ST_UNK;

	CMPWI(r4, 0);

	BranchTarget rteq0(BT_NE);

	NOT(r4, r3);
	STWRtoPR(&_rHiS_, r3);
	SRAWI(r4, r4, 31);
	ORI(r4, r4, 1);
	STWRtoPR(&_rLoS_, r4);

	BranchTarget exit0(BT_UN);

	rteq0.setTarget();

	LIS(r5, -32768);
	CMPW(r3, r5);

	BranchTarget rscheck(BT_NE);

	CMPWI(r4, -1);

	BranchTarget rtcheck(BT_NE);

	LI(r4, 0);
	STWRtoPR(&_rLoS_, r3);
	STWRtoPR(&_rHiS_, r4);

	BranchTarget exit1(BT_UN);

	rscheck.setTarget();
	rtcheck.setTarget();
	
	DIVW(r5, r3, r4);	// rLo = Rs / Rt 
	MULLW(r4, r5, r4);
	STWRtoPR(&_rLoS_, r5);
	SUBF(r4, r4, r3);	// rHi = Rs % Rt
	STWRtoPR(&_rHiS_, r4);

	exit0.setTarget();
	exit1.setTarget();
}

static void recDIVU() {
// Lo/Hi = Rs / Rt (unsigned)

	if(IsConst(_Rs_) && IsConst(_Rt_)) {
		if( iRegs[_Rt_].k == 0 )
		{
			MapConst(REG_LO, (u32)(-1));
			MapConst(REG_HI, iRegs[_Rs_].k);
		}
		else {
			MapConst(REG_LO, iRegs[_Rs_].k / iRegs[_Rt_].k);
			MapConst(REG_HI, iRegs[_Rs_].k % iRegs[_Rt_].k);
		}
		return;
	}
	else if(IsConst(_Rs_)) {
		LWPRtoR(r4, &_rRtU_);
		LIW(r3, iRegs[_Rs_].k);
	}
	else if(IsConst(_Rt_)) {
		LWPRtoR(r3, &_rRsU_);
		LIW(r4, iRegs[_Rt_].k);
	}
	else {
		LWPRtoR(r3, &_rRsU_);
		LWPRtoR(r4, &_rRtU_);
	}
	iRegs[REG_LO].state = ST_UNK;
	iRegs[REG_HI].state = ST_UNK;

	CMPWI(r4, 0);

	BranchTarget eq(BT_EQ);

	DIVWU(r5, r3, r4);	// rLo = Rs / Rt 
	MULLW(r4, r5, r4);
	STWRtoPR(&_rLoU_, r5);
	SUBF(r4, r4, r3);	// rHi = Rs % Rt
	STWRtoPR(&_rHiU_, r4);

	BranchTarget exit(BT_UN);

	eq.setTarget();

	LI(r4, -1);
	STWRtoPR(&_rHiU_, r3);
	STWRtoPR(&_rLoU_, r4);

	exit.setTarget();
}
#endif // NEW_REGS
#endif
//End of * Register mult/div & Register trap logic  

/* - memory access - */

#if 1
REC_FUNC(SWL);
REC_FUNC(SWR);
#else
static void recSWL() {

#if 0
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		const u32 shift = (addr & 3) << 3;
		addr &= ~3;
		int t = addr >> 16;

		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			LWMtoR(r3, (uptr)&psxM[addr & 0x1ffffc]);
			if (IsConst(_Rt_)) {
				LIW(r11, iRegs[_Rt_].k);
			} 
			else {
				LWPRtoR(r11, &_rRtU_);
			}
			LI(r9, -256);
			SLWI(r9, r9, shift);
			SRWI(r11, r11, (24-shift));
			AND(r3, r3, r9);
			OR(r3, r11, r3);
			STWRtoM((uptr)&psxM[addr & 0x1ffffc], r3);
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			LWMtoR(r3, (uptr)&psxH[addr & 0xffc]);
			if (IsConst(_Rt_)) {
				LIW(r11, iRegs[_Rt_].k);
			} 
			else {
				LWPRtoR(r11, &_rRtU_);
			}
			LI(r9, -256);
			SLWI(r9, r9, shift);
			SRWI(r11, r11, (24-shift));
			AND(r3, r3, r9);
			OR(r3, r11, r3);
			STWRtoM((uptr)&psxH[addr & 0xffc], r3);
			return;
		}
#if 1
		if (t == 0x1f80) {
			switch (addr) {
				case 0x1f801080: case 0x1f801084: 
				case 0x1f801090: case 0x1f801094: 
				case 0x1f8010a0: case 0x1f8010a4: 
				case 0x1f8010b0: case 0x1f8010b4: 
				case 0x1f8010c0: case 0x1f8010c4: 
				case 0x1f8010d0: case 0x1f8010d4: 
				case 0x1f8010e0: case 0x1f8010e4: 
				case 0x1f801074:
				case 0x1f8010f0:
					LWMtoR(r3, (uptr)&psxH[addr & 0xfffc]);
					if (IsConst(_Rt_)) {
						LIW(r11, iRegs[_Rt_].k);
					} 
					else {
						LWPRtoR(r11, &_rRtU_);
					}
					LI(r9, -256);
					SLWI(r9, r9, shift);
					SRWI(r11, r11, (24-shift));
					AND(r3, r3, r9);
					OR(r3, r11, r3);
					STWRtoM((uptr)&psxH[addr & 0xfffc], r3);
					return;

				case 0x1f801810:
					CALLFunc((u32)&GPU_readData);
					if (IsConst(_Rt_)) {
						LIW(r11, iRegs[_Rt_].k);
					} 
					else {
						LWPRtoR(r11, &_rRtU_);
					}
					LI(r9, -256);
					SLWI(r9, r9, shift);
					SRWI(r11, r11, (24-shift));
					AND(r3, r3, r9);
					OR(r3, r11, r3);
					CALLFunc((u32)&GPU_writeData);
					return;

				case 0x1f801814:
					CALLFunc((uptr)&GPU_readStatus);
					if (IsConst(_Rt_)) {
						LIW(r11, iRegs[_Rt_].k);
					} 
					else {
						LWPRtoR(r11, &_rRtU_);
					}
					LI(r9, -256);
					SLWI(r9, r9, shift);
					SRWI(r11, r11, (24-shift));
					AND(r3, r3, r9);
					OR(r3, r11, r3);
					CALLFunc((uptr)&GPU_writeStatus);
					return;
					
				case 0x1f801820:
					CALLFunc((uptr)&mdecRead0);
					if (IsConst(_Rt_)) {
						LIW(r11, iRegs[_Rt_].k);
					} 
					else {
						LWPRtoR(r11, &_rRtU_);
					}
					LI(r9, -256);
					SLWI(r9, r9, shift);
					SRWI(r11, r11, (24-shift));
					AND(r3, r3, r9);
					OR(r3, r11, r3);
					CALLFunc((uptr)&mdecWrite0);
					return;

				case 0x1f801824:
					CALLFunc((uptr)&mdecRead1);
					if (IsConst(_Rt_)) {
						LIW(r11, iRegs[_Rt_].k);
					} 
					else {
						LWPRtoR(r11, &_rRtU_);
					}
					LI(r9, -256);
					SLWI(r9, r9, shift);
					SRWI(r11, r11, (24-shift));
					AND(r3, r3, r9);
					OR(r3, r11, r3);
					CALLFunc((uptr)&mdecWrite1);
					return;
			}
		}
#endif

	}
#endif

	SetArg_OfB(r6);
	RLWINM(r5, r6, 3, 27, 28);	// (addr & 3) << 3
	RLWINM(r6, r6, 0, 0, 29);		// (addr & ~3)
	MR(PPCARG1, r6);
	CALLFunc((u32)psxMemRead32);

	LI(r4, -256);
	SLW(r4, r4, r5);
	if (IsConst(_Rt_)) {
		LIW(r8, iRegs[_Rt_].k);
	}
	else {
		LWPRtoR(r8, &_rRtU_);
	}
	SUBFIC(r5, r5, 24);
	AND(r4, r3, r4);
	SRW(r5, r8, r5);
	MR(PPCARG1, r6);

	OR(PPCARG2, r4, r5);
	CALLFunc((uptr)psxMemWrite32);
}

static void recSWR() {
#if 0
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		const u32 shift = (addr & 3) << 3;
		addr &= ~3;
		int t = addr >> 16;

		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			LWMtoR(r12, (uptr)&psxM[addr & 0x1ffffc]);
			if (IsConst(_Rt_)) {
				LIW(r10, iRegs[_Rt_].k);
			} 
			else {
				LWPRtoR(r10, &_rRtU_);
			}
			LIS(r9, 255);
			ORI(r9, r9, 65535);
			SLWI(r10, r10, shift);
			SRAWI(r9, r9, (24-shift));
			AND(r9, r12, r9);
			OR(r9, r9, r10);
			STWRtoM((uptr)&psxM[addr & 0x1ffffc], r9);
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			LWMtoR(r12, (uptr)&psxH[addr & 0xffc]);
			if (IsConst(_Rt_)) {
				LIW(r10, iRegs[_Rt_].k);
			}
			else {
				LWPRtoR(r10, &_rRtU_);
			}
			LIS(r9, 255);
			ORI(r9, r9, 65535);
			SLWI(r10, r10, shift);
			SRAWI(r9, r9, (24-shift));
			AND(r9, r12, r9);
			OR(r9, r9, r10);
			STWRtoM((uptr)&psxH[addr & 0xffc], r9);
			return;
		}
#if 1
		if (t == 0x1f80) {
			switch (addr) {
				case 0x1f801080: case 0x1f801084: 
				case 0x1f801090: case 0x1f801094: 
				case 0x1f8010a0: case 0x1f8010a4: 
				case 0x1f8010b0: case 0x1f8010b4: 
				case 0x1f8010c0: case 0x1f8010c4: 
				case 0x1f8010d0: case 0x1f8010d4: 
				case 0x1f8010e0: case 0x1f8010e4: 
				case 0x1f801074:
				case 0x1f8010f0:
					LWMtoR(r3, (uptr)&psxH[addr & 0xfffc]);
					if (IsConst(_Rt_)) {
						LIW(r10, iRegs[_Rt_].k);
					}
					else {
						LWPRtoR(r10, &_rRtU_);
					}
					LIS(r9, 255);
					ORI(r9, r9, 65535);
					SLWI(r10, r10, shift);
					SRAWI(r9, r9, (24-shift));
					AND(r9, r12, r9);
					OR(r9, r9, r10);
					STWRtoM((uptr)&psxH[addr & 0xfffc], r3);
					return;

				case 0x1f801810:
					CALLFunc((u32)&GPU_readData);
					if (IsConst(_Rt_)) {
						LIW(r10, iRegs[_Rt_].k);
					}
					else {
						LWPRtoR(r10, &_rRtU_);
					}
					LIS(r9, 255);
					ORI(r9, r9, 65535);
					SLWI(r10, r10, shift);
					SRAWI(r9, r9, (24-shift));
					AND(r9, r12, r9);
					OR(r9, r9, r10);
					CALLFunc((u32)&GPU_writeData);
					return;

				case 0x1f801814:
					CALLFunc((uptr)&GPU_readStatus);
					if (IsConst(_Rt_)) {
						LIW(r10, iRegs[_Rt_].k);
					}
					else {
						LWPRtoR(r10, &_rRtU_);
					}
					LIS(r9, 255);
					ORI(r9, r9, 65535);
					SLWI(r10, r10, shift);
					SRAWI(r9, r9, (24-shift));
					AND(r9, r12, r9);
					OR(r9, r9, r10);
					CALLFunc((uptr)&GPU_writeStatus);
					return;
					
				case 0x1f801820:
					CALLFunc((uptr)&mdecRead0);
					if (IsConst(_Rt_)) {
						LIW(r10, iRegs[_Rt_].k);
					}
					else {
						LWPRtoR(r10, &_rRtU_);
					}
					LIS(r9, 255);
					ORI(r9, r9, 65535);
					SLWI(r10, r10, shift);
					SRAWI(r9, r9, (24-shift));
					AND(r9, r12, r9);
					OR(r9, r9, r10);
					CALLFunc((uptr)&mdecWrite0);
					return;

				case 0x1f801824:
					CALLFunc((uptr)&mdecRead1);
					if (IsConst(_Rt_)) {
						LIW(r10, iRegs[_Rt_].k);
					}
					else {
						LWPRtoR(r10, &_rRtU_);
					}
					LIS(r9, 255);
					ORI(r9, r9, 65535);
					SLWI(r10, r10, shift);
					SRAWI(r9, r9, (24-shift));
					AND(r9, r12, r9);
					OR(r9, r9, r10);
					CALLFunc((uptr)&mdecWrite1);
					return;
			}
		}
#endif
	}
#endif

	SetArg_OfB(r6);
	RLWINM(r5, r6, 3, 27, 28);	// (addr & 3) << 3
	RLWINM(r6, r6, 0, 0, 29);		// (addr & ~3)
	MR(PPCARG1, r6);
	CALLFunc((u32)psxMemRead32);

	LIS(r4, 255);
	SUBFIC(r9, r5, 24);
	ORI(r4, r4, 65535);

	if (IsConst(_Rt_)) {
		LIW(r8, iRegs[_Rt_].k);
	}
	else {
		LWPRtoR(r8, &_rRtU_);
	}

	SRAW(r4, r4, r9);

	AND(r4, r3, r4);
	SLW(r5, r8, r5);
	MR(PPCARG1, r6);

	OR(PPCARG2, r4, r5);

	CALLFunc((uptr)psxMemWrite32);
}
#endif

#if 1
REC_FUNC(LWL);
REC_FUNC(LWR);
#else
#ifdef NEW_REGS
static void recLWL() {
#if 1
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		const u32 shift = (addr & 3) << 3;
		addr &= ~3;
		int t = addr >> 16;

		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (!_Rt_) return;

			LWMtoR(r3, (uptr)&psxM[addr & 0x1ffffc]);
			LIS(r4, 255);
			ORI(r4, r4, 65535);
			SLWI(r3, r3, (24-shift));
			SRAWI(r4, r4, shift);

			AND(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_), r4);
			OR(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_), r3);

			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (!_Rt_) return;

			LWMtoR(r3, (uptr)&psxH[addr & 0xffc]);
			LIS(r4, 255);
			ORI(r4, r4, 65535);
			SLWI(r3, r3, (24-shift));
			SRAWI(r4, r4, shift);

			AND(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_), r4);
			OR(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_), r3);
			return;
		}
	}
#endif
	SetArg_OfB(PPCARG1);
	RLWINM(PPCARG1, PPCARG1, 0, 0, 29);		// (add & ~3)
	CALLFunc((u32)psxMemRead32);
	if(_Rt_) {
		SetArg_OfB(r4);

		LIS(r5, 255);
		RLWINM(r4, r4, 3, 27, 28);	// (addr & 3) << 3

		ORI(r5, r5, 65535);
		SRAW(r5, r5, r4);
		SUBFIC(r4, r4, 24);
		AND(r5, r5, HWRegs.Get(_Rt_));
		SLW(HWRegs.Put(_Rt_), r3, r4);
		OR(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_), r5);
	}
}

static void recLWR() {
#if 1
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		const u32 shift = (addr & 3) << 3;
		addr &= ~3;
		int t = addr >> 16;
		

		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (!_Rt_) return;

			LWMtoR(r3, (uptr)&psxM[addr & 0x1ffffc]);
			LI(r4, -256);
			SRWI(r3, r3, shift);
			SLWI(r4, r4, (24-shift));

			AND(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_), r4);
			OR(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_), r3);
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (!_Rt_) return;

			LWMtoR(r3, (uptr)&psxH[addr & 0xffc]);
			SRWI(r3, r3, shift);

			LI(r4, -256);
			SLWI(r4, r4, (24-shift));

			AND(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_), r4);
			OR(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_), r3);
			return;
		}
	}
#endif

	SetArg_OfB(PPCARG1);
	RLWINM(PPCARG1, PPCARG1, 0, 0, 29);		// (addr & ~3)
	CALLFunc((u32)psxMemRead32);
	if(_Rt_) {
		SetArg_OfB(r4);

		RLWINM(r4, r4, 3, 27, 28);	// shift = (addr & 3) << 3

		SUBFIC(r5, r4, 24);
		LI(r6, -256);					// 0xffffff00
		SRW(r3, r3, r4);
		SLW(r5, r6, r5);

		AND(r5, r5, HWRegs.Get(_Rt_));
		OR(HWRegs.Put(_Rt_), r3, r5);
	}
}
#else // NEW_REGS
static void recLWL() {
#if 0
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		const u32 shift = (addr & 3) << 3;
		addr &= ~3;
		int t = addr >> 16;

		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (!_Rt_) return;
			iRegs[_Rt_].state = ST_UNK;

			LWMtoR(r10, (uptr)&psxM[addr & 0x1ffffc]);
			LIS(r9, 255);
			ORI(r9, r9, 65535);
			SLWI(r10, r10, (24-shift));
			SRAWI(r6, r9, shift);
			if (IsConst(_Rt_)) {
				LIW(r9, iRegs[_Rt_].k);
			} 
			else {
				LWPRtoR(r9, &_rRtU_);
			}
			AND(r9, r9, r6);
			OR(r9, r9, r10);
			STWRtoPR(&_rRtU_, r9);
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (!_Rt_) return;
			iRegs[_Rt_].state = ST_UNK;

			LWMtoR(r10, (uptr)&psxH[addr & 0xffc]);
			LIS(r9, 255);
			ORI(r9, r9, 65535);
			SLWI(r10, r10, (24-shift));
			SRAWI(r6, r9, shift);
			if (IsConst(_Rt_)) {
				LIW(r9, iRegs[_Rt_].k);
			} 
			else {
				LWPRtoR(r9, &_rRtU_);
			}
			AND(r9, r9, r6);
			OR(r9, r9, r10);
			STWRtoPR(&_rRtU_, r9);
			return;
		}
	}
#endif
	SetArg_OfB(r6);
	RLWINM(PPCARG1, r6, 0, 0, 29);		// (add & ~3)
	CALLFunc((u32)psxMemRead32);
	if(_Rt_) {
		iRegs[_Rt_].state = ST_UNK;

		LIS(r9, 255);
		RLWINM(r6, r6, 3, 27, 28);	// (addr & 3) << 3

		if (IsConst(_Rt_)) {
			LIW(r5, iRegs[_Rt_].k);
		}
		else {
			LWPRtoR(r5, &_rRtU_);
		}
		ORI(r9, r9, 65535);
		SRAW(r9, r9, r6);
		SUBFIC(r6, r6, 24);
		AND(r9, r9, r5);
		SLW(r3, r3, r6);
		OR(r3, r3, r9);

		STWRtoPR(&_rRtU_, r3);
	}
}

static void recLWR() {
#if 0
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		const u32 shift = (addr & 3) << 3;
		addr &= ~3;
		int t = addr >> 16;
		

		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (!_Rt_) return;
			iRegs[_Rt_].state = ST_UNK;

			LWMtoR(r10, (uptr)&psxM[addr & 0x1ffffc]);
			SRWI(r10, r10, shift);

			LI(r9, -256);
			SLWI(r12, r9, (24-shift));
			if (IsConst(_Rt_)) {
				LIW(r9, iRegs[_Rt_].k);
			} 
			else {
				LWPRtoR(r9, &_rRtU_);
			}
			AND(r9, r9, r12);
			OR(r9, r9, r10);
			STWRtoPR(&_rRtU_, r9);
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (!_Rt_) return;
			iRegs[_Rt_].state = ST_UNK;

			LWMtoR(r10, (uptr)&psxH[addr & 0xffc]);
			SRWI(r10, r10, shift);

			LI(r9, -256);
			SLWI(r12, r9, (24-shift));
			if (IsConst(_Rt_)) {
				LIW(r9, iRegs[_Rt_].k);
			} 
			else {
				LWPRtoR(r9, &_rRtU_);
			}
			AND(r9, r9, r12);
			OR(r9, r9, r10);
			STWRtoPR(&_rRtU_, r9);
			return;
		}
	}
#endif

	SetArg_OfB(r6);
	RLWINM(PPCARG1, r6, 0, 0, 29);		// (addr & ~3)
	CALLFunc((u32)psxMemRead32);
	if(_Rt_) {
		iRegs[_Rt_].state = ST_UNK;

		RLWINM(r6, r6, 3, 27, 28);	// shift = (addr & 3) << 3
		if (IsConst(_Rt_)) {
			LIW(r5, iRegs[_Rt_].k);
		}
		else {
			LWPRtoR(r5, &_rRtU_);
		}
		SUBFIC(r9, r6, 24);
		LI(r10, -256);					// 0xffffff00
		SRW(r3, r3, r6);
		SLW(r9, r10, r9);

		AND(r9, r9, r5);
		OR(r3, r3, r9);
		STWRtoPR(&_rRtU_, r3);
	}
}
#endif // NEW_REGS
#endif

#if 0
REC_FUNC(LB);
REC_FUNC(LBU);
REC_FUNC(LH);
REC_FUNC(LHU);
#else
static void recLB() {
// Rt = mem[Rs + Im] (signed)
#if 1
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;

		if ((t & 0xfff0) == 0xbfc0) {
			if (!_Rt_) return;
			// since bios is readonly it won't change
			MapConst(_Rt_, psxRs8(addr));
			return;
		}
		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (!_Rt_) return;

			LWMtoR(HWRegs.Put(_Rt_), (uptr)&psxM[addr & 0x1fffff]);
			EXTSB(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_));
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (!_Rt_) return;

			LWMtoR(HWRegs.Put(_Rt_), (uptr)&psxH[addr & 0xfff]);
			EXTSB(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_));
			return;
		}
	//	SysPrintf("unhandled r8 %x\n", addr);
	}
#endif
	SetArg_OfB(PPCARG1);
	CALLFunc((u32)psxMemRead8);
	if (_Rt_) {
		EXTSB(HWRegs.Put(_Rt_), r3);
	}
}

static void recLBU() {
// Rt = mem[Rs + Im] (unsigned)
#if 1
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;

		if ((t & 0xfff0) == 0xbfc0) {
			if (!_Rt_) return;
			// since bios is readonly it won't change
			MapConst(_Rt_, psxRu8(addr));
			return;
		}
		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (!_Rt_) return;

			LWMtoR(HWRegs.Put(_Rt_), (uptr)&psxM[addr & 0x1fffff]);
			RLWINM(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_), 0, 24, 31);
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (!_Rt_) return;

			LWMtoR(HWRegs.Put(_Rt_), (uptr)&psxH[addr & 0xfff]);
			RLWINM(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_), 0, 24, 31);
			return;
		}
	}
#endif
	SetArg_OfB(PPCARG1);
	CALLFunc((u32)psxMemRead8);

	if (_Rt_) {
		MR(HWRegs.Put(_Rt_), r3);
	}
}

static void recLH() {
// Rt = mem[Rs + Im] (signed)
#if 1
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;
	
		if ((t & 0xfff0) == 0xbfc0) {
			if (!_Rt_) return;
			// since bios is readonly it won't change
			MapConst(_Rt_, psxRu16(addr));
			return;
		}
		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (!_Rt_) return;

			LWMtoR(HWRegs.Put(_Rt_), (uptr)&psxM[addr & 0x1fffff]);
			EXTSH(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_));
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (!_Rt_) return;

			LWMtoR(HWRegs.Put(_Rt_), (uptr)&psxH[addr & 0xfff]);
			EXTSH(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_));
			return;
		}
	//	SysPrintf("unhandled r16 %x\n", addr);
	}
#endif
	SetArg_OfB(PPCARG1);
	CALLFunc((u32)psxMemRead16);
	if (_Rt_) {
		EXTSH(HWRegs.Put(_Rt_), r3);
	}
}

static void recLHU() {
// Rt = mem[Rs + Im] (unsigned)
#if 1
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;
	
		if ((t & 0xfff0) == 0xbfc0) {
			if (!_Rt_) return;
			// since bios is readonly it won't change
			MapConst(_Rt_, psxRu16(addr));
			return;
		}
		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (!_Rt_) return;

			LWMtoR(HWRegs.Put(_Rt_), (uptr)&psxM[addr & 0x1fffff]);
			RLWINM(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_), 0, 16, 31);
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (!_Rt_) return;

			LWMtoR(HWRegs.Put(_Rt_), (uptr)&psxH[addr & 0xfff]);
			RLWINM(HWRegs.Put(_Rt_), HWRegs.Get(_Rt_), 0, 16, 31);
			return;
		}

		if (t == 0x1f80) {
			if (addr >= 0x1f801c00 && addr < 0x1f801e00) {
					if (!_Rt_) return;

					LIW(PPCARG1, addr);
					CALLFunc((uptr)SPU_readRegister);
					RLWINM(HWRegs.Put(_Rt_), r3, 0, 16, 31);
					return;
			}
			switch (addr) {
				case 0x1f801100: case 0x1f801110: case 0x1f801120:
					if (!_Rt_) return;

					LIW(PPCARG1, (addr >> 4) & 0x3);
					CALLFunc((uptr)psxRcntRcount);
					RLWINM(HWRegs.Put(_Rt_), r3, 0, 16, 31);
					return;

				case 0x1f801104: case 0x1f801114: case 0x1f801124:
					if (!_Rt_) return;

					LIW(PPCARG1, (addr >> 4) & 0x3);
					CALLFunc((uptr)psxRcntRmode);
					RLWINM(HWRegs.Put(_Rt_), r3, 0, 16, 31);
					return;
	
				case 0x1f801108: case 0x1f801118: case 0x1f801128:
					if (!_Rt_) return;

					LIW(PPCARG1, (addr >> 4) & 0x3);
					CALLFunc((uptr)psxRcntRtarget);
					RLWINM(HWRegs.Put(_Rt_), r3, 0, 16, 31);
					return;
			}
		}

	//	SysPrintf("unhandled r16u %x\n", addr);
	}
#endif
	SetArg_OfB(PPCARG1);
	CALLFunc((u32)psxMemRead16);
	if (_Rt_) {
		MR(HWRegs.Put(_Rt_), r3);
	}
}
#endif

#if 0
REC_FUNC(LW);
#else
static void recLW() {
// Rt = mem[Rs + Im] (unsigned)
#if 1
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;

		if ((t & 0xfff0) == 0xbfc0) {
			if (!_Rt_) return;
			// since bios is readonly it won't change
			MapConst(_Rt_, psxRu32(addr));
			return;
		}
		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (!_Rt_) return;

			LWMtoR(HWRegs.Put(_Rt_), (uptr)&psxM[addr & 0x1fffff]);
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (!_Rt_) return;

			LWMtoR(HWRegs.Put(_Rt_), (uptr)&psxH[addr & 0xfff]);
			return;
		}

		if (t == 0x1f80) {
			switch (addr) {
				case 0x1f801080: case 0x1f801084: case 0x1f801088: 
				case 0x1f801090: case 0x1f801094: case 0x1f801098: 
				case 0x1f8010a0: case 0x1f8010a4: case 0x1f8010a8: 
				case 0x1f8010b0: case 0x1f8010b4: case 0x1f8010b8: 
				case 0x1f8010c0: case 0x1f8010c4: case 0x1f8010c8: 
				case 0x1f8010d0: case 0x1f8010d4: case 0x1f8010d8: 
				case 0x1f8010e0: case 0x1f8010e4: case 0x1f8010e8: 
				case 0x1f801070: case 0x1f801074:
				case 0x1f8010f0: case 0x1f8010f4:
					if (!_Rt_) return;
					
					LWMtoR(HWRegs.Put(_Rt_), (uptr)&psxH[addr & 0xffff]);
					return;

				case 0x1f801810:
					if (!_Rt_) return;

					CALLFunc((uptr)GPU_readData);
					MR(HWRegs.Put(_Rt_), r3);
					return;

				case 0x1f801814:
					if (!_Rt_) return;

					CALLFunc((uptr)GPU_readStatus);
					MR(HWRegs.Put(_Rt_), r3);
					return;
			}
		}

//		SysPrintf("unhandled r32 %x\n", addr);
	}
#endif
	SetArg_OfB(PPCARG1);
	CALLFunc((uptr)psxMemRead32);
	if (_Rt_) {
		MR(HWRegs.Put(_Rt_), r3);
	}
}
#endif

#if 0
REC_FUNC(SB);
REC_FUNC(SH);
REC_FUNC(SW);
#else
static void recSB() {
// mem[Rs + Im] = Rt
	
	/*if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;

		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (IsConst(_Rt_)) {
				LIW(r3, (u8)iRegs[_Rt_].k);
				STBRtoM((uptr)&psxM[addr & 0x1fffff], r3);
			} else {
				LWPRtoR(r3, &_rRtU_);
				RLWINM(PPCARG2, PPCARG2, 0, 24, 31);
				STBRtoM((uptr)&psxM[addr & 0x1fffff], r3);
			}
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (IsConst(_Rt_)) {
				LIW(r3, (u8)iRegs[_Rt_].k);
				STBRtoM((uptr)&psxH[addr & 0xfff], r3);
			} else {
				LWPRtoR(r3, &_rRtU_);
				RLWINM(PPCARG2, PPCARG2, 0, 24, 31);
				STBRtoM((uptr)&psxH[addr & 0xfff], r3);
			}
			return;
		}
//		SysPrintf("unhandled w8 %x\n", addr);
	}*/

	SetArg_OfB(PPCARG1);
	MR(PPCARG2, HWRegs.Get(_Rt_));
	CALLFunc((u32)psxMemWrite8);
}

static void recSH() {
// mem[Rs + Im] = Rt
#if 0
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;

		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (IsConst(_Rt_)) {
				LIW(r3, (u16)iRegs[_Rt_].k);
			} else {
				LWPRtoR(r3, &_rRtU_);
			}
			STHRtoM((uptr)&psxM[addr & 0x1fffff], r3);
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (IsConst(_Rt_)) {
				LIW(r3, (u16)iRegs[_Rt_].k);
			} else {
				LWPRtoR(r3, &_rRtU_);
			}
			STHRtoM((uptr)&psxH[addr & 0xfff], r3);
			return;
		}
		if (t == 0x1f80) {
			/*if (addr >= 0x1f801c00 && addr < 0x1f801e00) {
				LIW(PPCARG1, addr);
				if (IsConst(_Rt_)) {
					LIW(PPCARG2, (u16)iRegs[_Rt_].k);
				} else {
					LWPRtoR(PPCARG2, &_rRtU_);
					RLWINM(PPCARG2, PPCARG2, 0, 16, 31);
				}
				CALLFunc((uptr)&SPU_writeRegister);
				return;
			}*/
			switch (addr) {
				case 0x1f801100: case 0x1f801110: case 0x1f801120:
					LIW(PPCARG1, (addr >> 4) & 0x3);
					if (IsConst(_Rt_)) {
						LIW(PPCARG2, (u16)iRegs[_Rt_].k);
					} else {
						LHPRtoR(PPCARG2, &psxRegs.GPR.r[_Rt_].w.l);
					}
					CALLFunc((uptr)psxRcntWcount);
					return;

				case 0x1f801104: case 0x1f801114: case 0x1f801124:
					LIW(PPCARG1, (addr >> 4) & 0x3);
					if (IsConst(_Rt_)) {
						LIW(PPCARG2, (u16)iRegs[_Rt_].k);
					} else {
						LHPRtoR(PPCARG2, &psxRegs.GPR.r[_Rt_].w.l);
					}
					CALLFunc((uptr)psxRcntWmode);
					return;
	
				case 0x1f801108: case 0x1f801118: case 0x1f801128:
					LIW(PPCARG1, (addr >> 4) & 0x3);
					if (IsConst(_Rt_)) {
						LIW(PPCARG2, (u16)iRegs[_Rt_].k);
					} else {
						LHPRtoR(PPCARG2, &psxRegs.GPR.r[_Rt_].w.l);
					}
					CALLFunc((uptr)psxRcntWtarget);
					return;
			}
		}
//		SysPrintf("unhandled w16 %x\n", addr);
	}
#endif
	SetArg_OfB(PPCARG1);
	MR(PPCARG2, HWRegs.Get(_Rt_));
	CALLFunc((u32)psxMemWrite16);
}

static void recSW() {
// mem[Rs + Im] = Rt
#if 0
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;

		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (IsConst(_Rt_)) {
				LIW(r3, (u32)iRegs[_Rt_].k);
			} else {
				LWPRtoR(r3, &_rRtU_);
			}
			STWRtoM((uptr)&psxM[addr & 0x1fffff], r3);
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (IsConst(_Rt_)) {
				LIW(r3, (u32)iRegs[_Rt_].k);
			} else {
				LWPRtoR(r3, &_rRtU_);
			}
			STWRtoM((uptr)&psxH[addr & 0xfff], r3);
			return;
		}
		if (t == 0x1f80) {
			switch (addr) {
				case 0x1f801080: case 0x1f801084: 
				case 0x1f801090: case 0x1f801094: 
				case 0x1f8010a0: case 0x1f8010a4: 
				case 0x1f8010b0: case 0x1f8010b4: 
				case 0x1f8010c0: case 0x1f8010c4: 
				case 0x1f8010d0: case 0x1f8010d4: 
				case 0x1f8010e0: case 0x1f8010e4: 
				case 0x1f801074:
				case 0x1f8010f0:
					if (IsConst(_Rt_)) {
						LIW(PPCARG1, iRegs[_Rt_].k);
					} else {
						LWPRtoR(PPCARG1, &_rRtU_);
					}
					STWRtoM((uptr)&psxH[addr & 0xffff], r3);
					return;

				case 0x1f801810:
					if (IsConst(_Rt_)) {
						LIW(PPCARG1, iRegs[_Rt_].k);
					} else {
						LWPRtoR(PPCARG1, &_rRtU_);
					}
					CALLFunc((u32)&GPU_writeData);
					return;

				case 0x1f801814:
					if (IsConst(_Rt_)) {
						LIW(PPCARG1, iRegs[_Rt_].k);
					} else {
						LWPRtoR(PPCARG1, &_rRtU_);
					}
					CALLFunc((u32)&GPU_writeStatus);
					return;
					
				case 0x1f801820:
					if (IsConst(_Rt_)) {
						LIW(PPCARG1, iRegs[_Rt_].k);
					} else {
						LWPRtoR(PPCARG1, &_rRtU_);
					}
					CALLFunc((u32)&mdecWrite0);
					return;

				case 0x1f801824:
					if (IsConst(_Rt_)) {
						LIW(PPCARG1, iRegs[_Rt_].k);
					} else {
						LWPRtoR(PPCARG1, &_rRtU_);
					}
					CALLFunc((u32)&mdecWrite1);
					return;
			}
		}

	}
#endif
	SetArg_OfB(PPCARG1);
	MR(PPCARG2, HWRegs.Get(_Rt_));
	CALLFunc((u32)psxMemWrite32);
}
#endif

#if 0
REC_FUNC(SLL);
REC_FUNC(SRL);
REC_FUNC(SRA);
#else

#ifdef NEW_REGS
static void recSLL() {
// Rd = Rt << Sa
	if (!_Rd_) return;

	if (IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rt_].k << _Sa_);
	} else {

		if(_Sa_) {
			SLWI(HWRegs.Put(_Rd_), HWRegs.Get(_Rt_), _Sa_);
		}
		else if(_Rt_ != _Rd_) {
			MR(HWRegs.Put(_Rd_), HWRegs.Get(_Rt_));
		}
	}
}

static void recSRL() {
// Rd = Rt >> Sa
	if (!_Rd_) return;

	if (IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rt_].k >> _Sa_);
	} else {
		if(_Sa_) {
			SRWI(HWRegs.Put(_Rd_), HWRegs.Get(_Rt_), _Sa_);
		}
		else if(_Rt_ != _Rd_) {
			MR(HWRegs.Put(_Rd_), HWRegs.Get(_Rt_));
		}
	}
}

static void recSRA() {
// Rd = Rt >> Sa
	if (!_Rd_) return;

	if (IsConst(_Rt_)) {
		MapConst(_Rd_, (s32)iRegs[_Rt_].k >> _Sa_);
	} else {
		if(_Sa_) {
			SRAWI(HWRegs.Put(_Rd_), HWRegs.Get(_Rt_), _Sa_);
		}
		else if(_Rt_ != _Rd_) {
			MR(HWRegs.Put(_Rd_), HWRegs.Get(_Rt_));
		}
	}
}
#else // NEW_REGS

static void recSLL() {
// Rd = Rt << Sa
	if (!_Rd_) return;

	if (IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rt_].k << _Sa_);
	} else {
		iRegs[_Rd_].state = ST_UNK;

		LWPRtoR(r3, &_rRtU_);
		if(_Sa_) SLWI(r3, r3, _Sa_);
		STWRtoPR(&_rRdU_, r3);
	}
}

static void recSRL() {
// Rd = Rt >> Sa
	if (!_Rd_) return;

	if (IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rt_].k >> _Sa_);
	} else {
		iRegs[_Rd_].state = ST_UNK;

		LWPRtoR(r3, &_rRtU_);
		if(_Sa_) SRWI(r3, r3, _Sa_);
		STWRtoPR(&_rRdU_, r3);
	}
}

static void recSRA() {
// Rd = Rt >> Sa
	if (!_Rd_) return;

	if (IsConst(_Rt_)) {
		MapConst(_Rd_, (s32)iRegs[_Rt_].k >> _Sa_);
	} else {
		iRegs[_Rd_].state = ST_UNK;

		LWPRtoR(r3, &_rRtS_);
		if(_Sa_) SRAWI(r3, r3, _Sa_);
		STWRtoPR(&_rRdS_, r3);
	}
}
#endif // NEW_REGS
#endif

/* - shift ops - */

#if 0
REC_FUNC(SLLV);
REC_FUNC(SRLV);
REC_FUNC(SRAV);
#else

#ifdef NEW_REGS

static void recSLLV() {
// Rd = Rt << Rs
	if (!_Rd_) return;

	if (IsConst(_Rt_) && IsConst(_Rs_)) {
		MapConst(_Rd_, iRegs[_Rt_].k << (iRegs[_Rs_].k & 0x1f));
		return;
	}
	else if (IsConst(_Rs_)) {
		if(iRegs[_Rs_].k & 0x1f) {
			SLWI(HWRegs.Put(_Rd_), HWRegs.Get(_Rt_), (iRegs[_Rs_].k & 0x1f));
		}
		else if(_Rd_ != _Rt_) {
			MR(HWRegs.Put(_Rd_), HWRegs.Get(_Rt_));
		}
	}
	else {

		RLWINM(r3, HWRegs.Get(_Rs_), 0, 27, 31); // &0x1f
		SLW(HWRegs.Put(_Rd_), HWRegs.Get(_Rt_), r3);
	}
}

static void recSRLV() {
// Rd = Rt >> Rs
	if (!_Rd_) return;

	if (IsConst(_Rt_) && IsConst(_Rs_)) {
		MapConst(_Rd_, iRegs[_Rt_].k >> (iRegs[_Rs_].k & 0x1f));
		return;
	}
	else if (IsConst(_Rs_)) {
		if(iRegs[_Rs_].k & 0x1f) {
			SRWI(HWRegs.Put(_Rd_), HWRegs.Get(_Rt_), (iRegs[_Rs_].k & 0x1f));
		}
		else if(_Rd_ != _Rt_) {
			MR(HWRegs.Put(_Rd_), HWRegs.Get(_Rt_));
		}
	}
	else {

		RLWINM(r3, HWRegs.Get(_Rs_), 0, 27, 31); // &0x1f
		SRW(HWRegs.Put(_Rd_), HWRegs.Get(_Rt_), r3);
	}
}

static void recSRAV() {
// Rd = Rt >> Rs
	if (!_Rd_) return;

	if (IsConst(_Rt_) && IsConst(_Rs_)) {
		MapConst(_Rd_, (s32)iRegs[_Rt_].k >> (iRegs[_Rs_].k & 0x1f));
		return;
	}
	else if (IsConst(_Rs_)) {
		if(iRegs[_Rs_].k & 0x1f) {
			SRAWI(HWRegs.Put(_Rd_), HWRegs.Get(_Rt_), (iRegs[_Rs_].k & 0x1f));
		}
		else if(_Rd_ != _Rt_) {
			MR(HWRegs.Put(_Rd_), HWRegs.Get(_Rt_));
		}
	}
	else {
		RLWINM(r3, HWRegs.Get(_Rs_), 0, 27, 31); // &0x1f
		SRAW(HWRegs.Put(_Rd_), HWRegs.Get(_Rt_), r3);
	}
}

#else //NEW_REGS
static void recSLLV() {
// Rd = Rt << Rs
	if (!_Rd_) return;

	if (IsConst(_Rt_) && IsConst(_Rs_)) {
		MapConst(_Rd_, iRegs[_Rt_].k << (iRegs[_Rs_].k & 0x1f));
		return;
	}
	else if (IsConst(_Rs_)) {
		iRegs[_Rd_].state = ST_UNK;
		LWPRtoR(r3, &_rRtU_);
// 		LIW(r9, (iRegs[_Rs_].k & 0x1f));
		SLWI(r3, r3, (iRegs[_Rs_].k & 0x1f));
		STWRtoPR(&_rRdU_, r3);
		return;
	}
	else if (IsConst(_Rt_)) {
		LWPRtoR(r9, &_rRsU_);
		LIW(r3, iRegs[_Rt_].k);
		RLWINM(r9, r9, 0, 27, 31); // &0x1f
	}
	else {
		LWPRtoR(r9, &_rRsU_);
		LWPRtoR(r3, &_rRtU_);
		RLWINM(r9, r9, 0, 27, 31); // &0x1f
	}
	iRegs[_Rd_].state = ST_UNK;

	SLW(r3, r3, r9);
	STWRtoPR(&_rRdU_, r3);
}

static void recSRLV() {
// Rd = Rt >> Rs
	if (!_Rd_) return;

	if (IsConst(_Rt_) && IsConst(_Rs_)) {
		MapConst(_Rd_, iRegs[_Rt_].k >> (iRegs[_Rs_].k & 0x1f));
		return;
	}
	else if (IsConst(_Rs_)) {
		iRegs[_Rd_].state = ST_UNK;
		LWPRtoR(r3, &_rRtU_);
// 		LIW(r9, (iRegs[_Rs_].k & 0x1f));
		SRWI(r3, r3, (iRegs[_Rs_].k & 0x1f));
		STWRtoPR(&_rRdU_, r3);
		return;
	}
	else if (IsConst(_Rt_)) {
		LWPRtoR(r9, &_rRsU_);
		LIW(r3, iRegs[_Rt_].k);
		RLWINM(r9, r9, 0, 27, 31); // &0x1f
	}
	else {
		LWPRtoR(r9, &_rRsU_);
		LWPRtoR(r3, &_rRtU_);
		RLWINM(r9, r9, 0, 27, 31); // &0x1f
	}
	iRegs[_Rd_].state = ST_UNK;

	SRW(r3, r3, r9);
	STWRtoPR(&_rRdU_, r3);
}

static void recSRAV() {
// Rd = Rt >> Rs
	if (!_Rd_) return;

	if (IsConst(_Rt_) && IsConst(_Rs_)) {
		MapConst(_Rd_, (s32)iRegs[_Rt_].k >> (iRegs[_Rs_].k & 0x1f));
		return;
	}
	else if (IsConst(_Rs_)) {
		iRegs[_Rd_].state = ST_UNK;
		LWPRtoR(r3, &_rRtU_);
// 		LIW(r9, (iRegs[_Rs_].k & 0x1f));
		SRAWI(r3, r3, (iRegs[_Rs_].k & 0x1f));
		STWRtoPR(&_rRdU_, r3);
		return;
	}
	else if (IsConst(_Rt_)) {
		LWPRtoR(r9, &_rRsU_);
		LIW(r3, iRegs[_Rt_].k);
		RLWINM(r9, r9, 0, 27, 31); // &0x1f
	}
	else {
		LWPRtoR(r9, &_rRsU_);
		LWPRtoR(r3, &_rRtU_);
		RLWINM(r9, r9, 0, 27, 31); // &0x1f
	}
	iRegs[_Rd_].state = ST_UNK;

	SRAW(r3, r3, r9);
	STWRtoPR(&_rRdU_, r3);
}
#endif // NEW_REGS
#endif

#if 0
REC_SYS(SYSCALL);
REC_SYS(BREAK);
#else
static void recSYSCALL() {
	LIW(r4, pc - 4);

	iFlushRegs();

	STWRtoPR(&psxRegs.pc, r4);

	LI(PPCARG1, 0x20);
	LI(PPCARG2, (branch == 1 ? 1 : 0));
	CALLFunc ((u32)psxException);

	branch = 2;
	iRet();
}

static void recBREAK() {
}
#endif

#if 0
REC_FUNC(MFHI);
REC_FUNC(MTHI);
REC_FUNC(MFLO);
REC_FUNC(MTLO);
#else
#ifdef NEW_REGS
static void recMFHI() {
// Rd = Hi
	if (!_Rd_) return;

	if(IsConst(REG_HI)) {
		MapConst(_Rd_, iRegs[REG_HI].k);
	}
	else {
		MR(HWRegs.Put(_Rd_), HWRegs.Get(REG_HI));
	}
}

static void recMTHI() {
// Hi = Rs

	if (IsConst(_Rs_)) {
		MapConst(REG_HI, iRegs[_Rs_].k);
	} else {
		MR(HWRegs.Put(REG_HI), HWRegs.Get(_Rs_));
	}
}

static void recMFLO() {
// Rd = Lo
	if (!_Rd_) return;

	if(IsConst(REG_LO)) {
		MapConst(_Rd_, iRegs[REG_LO].k);
	}
	else {
		MR(HWRegs.Put(_Rd_), HWRegs.Get(REG_LO));
	}
}

static void recMTLO() {
// Lo = Rs

	if (IsConst(_Rs_)) {
		MapConst(REG_LO, iRegs[_Rs_].k);
	} else {
		MR(HWRegs.Put(REG_LO), HWRegs.Get(_Rs_));
	}
}
#else
static void recMFHI() {
// Rd = Hi
	if (!_Rd_) return;

	if(IsConst(REG_HI)) {
		MapConst(_Rd_, iRegs[REG_HI].k);
	}
	else {
		iRegs[_Rd_].state = ST_UNK;
		LWPRtoR(r9, &_rHiU_);
		STWRtoPR(&_rRdU_, r9);
	}
}

static void recMTHI() {
// Hi = Rs

	if (IsConst(_Rs_)) {
		MapConst(REG_HI, iRegs[_Rs_].k);
	} else {
		iRegs[REG_HI].state = ST_UNK;
		LWPRtoR(r9, &_rRsU_);
		STWRtoPR(&_rHiU_, r9);
	}
}

static void recMFLO() {
// Rd = Lo
	if (!_Rd_) return;

	if(IsConst(REG_LO)) {
		MapConst(_Rd_, iRegs[REG_LO].k);
	}
	else {
		iRegs[_Rd_].state = ST_UNK;
		LWPRtoR(r9, &_rLoU_);
		STWRtoPR(&_rRdU_, r9);
	}
}

static void recMTLO() {
// Lo = Rs

	if (IsConst(_Rs_)) {
		MapConst(REG_LO, iRegs[_Rs_].k);
	} else {
		iRegs[REG_LO].state = ST_UNK;
		LWPRtoR(r9, &_rRsU_);
		STWRtoPR(&_rLoU_, r9);
	}
}
#endif // NEW_REGS
#endif
/* - branch ops - */

#define REC_GEN_BRANCH_FUNC_C(name, btcond, exp) \
static void rec##name() { \
	u32 bpc = _Imm_ * 4 + pc; \
	\
	if (bpc == pc+4 && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) { \
		return; \
	} \
	\
	if (_Rs_ == _Rt_) { \
		iJump(bpc); \
	} \
	else { \
		if (IsConst(_Rs_) && IsConst(_Rt_)) { \
			if (!(iRegs[_Rs_].k exp iRegs[_Rt_].k)) { \
				bpc = pc+4; \
			} \
			iJump(bpc); \
			return; \
		} \
	\
		CMPLW(HWRegs.Get(_Rs_), HWRegs.Get(_Rt_)); \
	\
		BranchTarget branch(btcond); \
	\
		iBranch(pc+4, 1); \
	\
		branch.setTarget(); \
	\
		iBranch(bpc, 0); \
		pc+=4; \
	} \
}

#define REC_GEN_BRANCH_FUNC(name, btcond, exp) \
	static void rec##name() { \
		u32 bpc = _Imm_ * 4 + pc; \
 \
		if (bpc == pc+4 && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) { \
			return; \
		} \
 \
		if (IsConst(_Rs_)) { \
			if ((s32)iRegs[_Rs_].k exp 0) { \
				iJump(bpc); return; \
			} else { \
				iJump(pc+4); return; \
			} \
		} \
 \
		CMPWI(HWRegs.Get(_Rs_), 0); \
 \
		BranchTarget branch(btcond); \
 \
		iBranch(pc+4, 1); \
 \
		branch.setTarget(); \
 \
		iBranch(bpc, 0); \
		pc+=4; \
	}

#define REC_GEN_BRANCH_FUNC_L(name, btcond, exp)  \
	static void rec##name() { \
		u32 bpc = _Imm_ * 4 + pc; \
 \
		if (bpc == pc+4 && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) { \
			return; \
		} \
 \
		if (IsConst(_Rs_)) { \
			if ((s32)iRegs[_Rs_].k exp 0) { \
				MapConst(31, pc + 4); \
				iJump(bpc); return; \
			} else { \
				iJump(pc+4); return; \
			} \
		} \
 \
		CMPWI(HWRegs.Get(_Rs_), 0); \
 \
		BranchTarget branch(btcond); \
 \
		iBranch(pc+4, 1); \
 \
		branch.setTarget(); \
		MapConst(31, pc + 4); \
 \
		iBranch(bpc, 0); \
		pc+=4; \
	}


#if 0
REC_BRANCH(BEQ);     // *FIXME
#else
#ifdef NEW_REGS

REC_GEN_BRANCH_FUNC_C(BEQ, BT_EQ, ==);

#else // NEW_REGS
static void recBEQ() {
// Branch if Rs == Rt

	u32 bpc = _Imm_ * 4 + pc;
	
	if (bpc == pc+4 && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) {
		return;
	}

	if (_Rs_ == _Rt_) {
		iJump(bpc);
	}
	else {
		if (IsConst(_Rs_) && IsConst(_Rt_)) {
			if (iRegs[_Rs_].k == iRegs[_Rt_].k) {
				iJump(bpc); return;
			} else {
				iJump(pc+4); return;
			}
		}
		else if (IsConst(_Rs_)) {
			LWPRtoR(r3, &_rRtU_);
			LIW(r9, iRegs[_Rs_].k);
		}
		else if (IsConst(_Rt_)) {
			LWPRtoR(r9, &_rRsU_);
			LIW(r3, iRegs[_Rt_].k);
		}
		else {
			LWPRtoR(r3, &_rRtU_);
			LWPRtoR(r9, &_rRsU_);
		}
		CMPW(r9, r3);

		BranchTarget eq(BT_EQ);

		iBranch(pc+4, 1);

		eq.setTarget();

		iBranch(bpc, 0);
		pc+=4;
	}
}
#endif // NEW_REGS
#endif

#if 0
REC_BRANCH(BNE);     // *FIXME
#else
#ifdef NEW_REGS

REC_GEN_BRANCH_FUNC_C(BNE, BT_NE, !=);

#else // NEW_REGS
static void recBNE() {
// Branch if Rs != Rt

	u32 bpc = _Imm_ * 4 + pc;

	if (bpc == pc+4 && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) {
		return;
	}

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		if (iRegs[_Rs_].k != iRegs[_Rt_].k) {
			iJump(bpc); return;
		} else {
			iJump(pc+4); return;
		}
	}
	else if (IsConst(_Rs_)) {
		LWPRtoR(r3, &_rRtU_);
		LIW(r9, iRegs[_Rs_].k);

	}
	else if (IsConst(_Rt_)) {
		LWPRtoR(r9, &_rRsU_);
		LIW(r3, iRegs[_Rt_].k);
	}
	else {
		LWPRtoR(r3, &_rRtU_);
		LWPRtoR(r9, &_rRsU_);
	}
	CMPW(r9, r3);
	BranchTarget ne(BT_NE);

	iBranch(pc+4, 1);

	ne.setTarget();

	iBranch(bpc, 0);
	pc+=4;
}
#endif // NEW_REGS
#endif

#if 0
REC_BRANCH(BLTZ);
REC_BRANCH(BGTZ);
REC_BRANCH(BLTZAL);
REC_BRANCH(BLEZ);
REC_BRANCH(BGEZ);
REC_BRANCH(BGEZAL);
#else
#ifdef NEW_REGS

REC_GEN_BRANCH_FUNC(BLTZ, BT_LT, <);
REC_GEN_BRANCH_FUNC(BGTZ, BT_GT, >);
REC_GEN_BRANCH_FUNC(BLEZ, BT_LE, <=);
REC_GEN_BRANCH_FUNC(BGEZ, BT_GE, >=);

REC_GEN_BRANCH_FUNC_L(BLTZAL, BT_LT, <);
REC_GEN_BRANCH_FUNC_L(BGEZAL, BT_GE, >);

#else	// NEW_REGS

static void recBLTZ() {
// Branch if Rs < 0
	u32 bpc = _Imm_ * 4 + pc;

	if (bpc == pc+4 && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) {
		return;
	}

	if (IsConst(_Rs_)) {
		if ((s32)iRegs[_Rs_].k < 0) {
			iJump(bpc); return;
		} else {
			iJump(pc+4); return;
		}
	}

	LWPRtoR(r3, &_rRsU_);
	CMPWI(r3, 0);
	
	BranchTarget lt(BT_LT);

	iBranch(pc+4, 1);

	lt.setTarget();

	iBranch(bpc, 0);
	pc+=4;
}

static void recBGTZ() {
// Branch if Rs > 0
	u32 bpc = _Imm_ * 4 + pc;

	if (bpc == pc+4 && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) {
		return;
	}

	if (IsConst(_Rs_)) {
		if ((s32)iRegs[_Rs_].k > 0) {
			iJump(bpc); return;
		} else {
			iJump(pc+4); return;
		}
	}

	LWPRtoR(r3, &_rRsU_);
	CMPWI(r3, 0);
	
	BranchTarget gt(BT_GT);

	iBranch(pc+4, 1);

	gt.setTarget();

	iBranch(bpc, 0);
	pc += 4;
}

static void recBLTZAL() {
// Branch if Rs < 0
	u32 bpc = _Imm_ * 4 + pc;

	if (bpc == pc+4 && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) {
		return;
	}

	if (IsConst(_Rs_)) {
		if ((s32)iRegs[_Rs_].k < 0) {
			MapConst(31, pc + 4);

			iJump(bpc); return;
		} else {
			iJump(pc+4); return;
		}
	}

	LWPRtoR(r3, &_rRsU_);
	CMPWI(r3, 0);
	
	BranchTarget lt(BT_LT);

	iBranch(pc+4, 1);

	lt.setTarget();

	MapConst(31, pc + 4);

	iBranch(bpc, 0);
	pc+=4;
}

static void recBGEZAL() {
// Branch if Rs >= 0
	u32 bpc = _Imm_ * 4 + pc;

	if (bpc == pc+4 && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) {
		return;
	}

	if (IsConst(_Rs_)) {
		if ((s32)iRegs[_Rs_].k >= 0) {
			MapConst(31, pc + 4);

			iJump(bpc); return;
		} else {
			iJump(pc+4); return;
		}
	}

	LWPRtoR(r3, &_rRsU_);
	CMPWI(r3, 0);
	
	BranchTarget ge(BT_GE);

	iBranch(pc+4, 1);

	ge.setTarget();

	MapConst(31, pc + 4);

	iBranch(bpc, 0);
	pc += 4;
}

static void recBLEZ() {
// Branch if Rs <= 0
	u32 bpc = _Imm_ * 4 + pc;
	u32 *b;
	
	if (bpc == pc+4 && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) {
		return;
	}

	if (IsConst(_Rs_)) {
		if ((s32)iRegs[_Rs_].k <= 0) {
			iJump(bpc); return;
		} else {
			iJump(pc+4); return;
		}
	}

	LWPRtoR(r3, &_rRsU_);
	CMPWI(r3, 0);
	
	BranchTarget le(BT_LE);
	
	iBranch(pc+4, 1);

	le.setTarget();
	
	iBranch(bpc, 0);
	pc+=4;
}

static void recBGEZ() {
// Branch if Rs >= 0
	u32 bpc = _Imm_ * 4 + pc;
	u32 *b;
	
	if (bpc == pc+4 && psxTestLoadDelay(_Rs_, PSXMu32(bpc)) == 0) {
		return;
	}
	
	if (IsConst(_Rs_)) {
		if ((s32)iRegs[_Rs_].k >= 0) {
			iJump(bpc); return;
		} else {
			iJump(pc+4); return;
		}
	}

	LWPRtoR(r3, &_rRsU_);
	CMPWI(r3, 0);
	BranchTarget ge(BT_GE);
	
	iBranch(pc+4, 1);

	ge.setTarget();

	iBranch(bpc, 0);
	pc+=4;
}
#endif // NEW_REGS
#endif

#if 0
REC_BRANCH(J);
REC_BRANCH(JR);
REC_BRANCH(JAL);
REC_BRANCH(JALR);
#else
#ifdef NEW_REGS
static void recJ() {
// j target

	iJump(_Target_ * 4 + (pc & 0xf0000000));
}

static void recJAL() {
// jal target
	MapConst(31, pc + 4);

	recJ();
}

static void recJR() {
// jr Rs

	STWRtoM((uptr)&target, HWRegs.Get(_Rs_));

	SetBranch();
}

static void recJALR() {
// jalr Rs

	if (_Rd_) {
		MapConst(_Rd_, pc + 4);
	}

	recJR();
}
#else // NEW_REGS
static void recJ() {
// j target

	iJump(_Target_ * 4 + (pc & 0xf0000000));
}

static void recJAL() {
// jal target
	MapConst(31, pc + 4);

	recJ();
}

static void recJR() {
// jr Rs

	if (IsConst(_Rs_)) {
		LIW(r3, iRegs[_Rs_].k);
		STWRtoM((uptr)&target, r3);
	} else {
		LWPRtoR(r3, &_rRsU_);
		STWRtoM((uptr)&target, r3);
	}
	SetBranch();
}

static void recJALR() {
// jalr Rs

	if (_Rd_) {
		MapConst(_Rd_, pc + 4);
	}

	recJR();
}
#endif // NEW_REGS
#endif

#if 0
REC_FUNC(RFE);
#else
static void recRFE() {

	LWPRtoR(r3, &psxRegs.CP0.n.Status);
	RLWINM(r4, r3, 0, 0, 27);
	RLWINM(r3, r3, 30, 28, 31);
	OR(r3, r3, r4);
	STWRtoPR(&psxRegs.CP0.n.Status, r3);

	LIW(r4, (u32)pc);
	iFlushRegs();

	STWRtoPR(&psxRegs.pc, r4);

	CALLFunc((uptr)psxTestSWInts);
	if (branch == 0) {
		branch = 2;
		iRet();
	}
}
#endif

#if 0
REC_FUNC(MFC0);
REC_SYS(MTC0);
REC_FUNC(CFC0);
REC_SYS(CTC0);
#else
#ifdef NEW_REGS
static void recMFC0() {
// Rt = Cop0->Rd
	if (!_Rt_) return;

	LWPRtoR(HWRegs.Put(_Rt_), &_rFsU_);
}

static void recCFC0() {
// Rt = Cop0->Rd

	recMFC0();
}

static void recMTC0() {
// Cop0->Rd = Rt

// TODO: invalidate r3, r4
	if(_Rd_ == 13) {
		RLWINM(r3, HWRegs.Get(_Rt_), 0, 22, 15); // & ~(0xfc00)
		STWRtoPR(&_rFsU_, r3);
	}
	else {
		STWRtoPR(&_rFsU_, HWRegs.Get(_Rt_));
	}

	if (_Rd_ == 12 || _Rd_ == 13) {
		iFlushRegs();
		LIW(r4, (u32)pc);
		STWRtoPR(&psxRegs.pc, r4);
		CALLFunc((u32)psxTestSWInts);

		if(_Rd_ == 12) {
			CALLFunc((uptr)psxTestIntc);
		}

		if(branch == 0) {
			branch = 2;
			iRet();
		}
	}
}

static void recCTC0() {
// Cop0->Rd = Rt

	recMTC0();
}
#else // NEW_REGS
static void recMFC0() {
// Rt = Cop0->Rd
	if (!_Rt_) return;

	iRegs[_Rt_].state = ST_UNK;
	LWPRtoR(r3, &_rFsU_);
	STWRtoPR(&_rRtU_, r3);
}

static void recCFC0() {
// Rt = Cop0->Rd

	recMFC0();
}

static void recMTC0() {
// Cop0->Rd = Rt

	if(IsConst(_Rt_)) {
		LIW(r3, iRegs[_Rt_].k);
	}
	else {
		LWPRtoR(r3, &_rRtU_);
	}
	if(_Rd_ == 13) RLWINM(r3, r3, 0, 22, 15); // & ~(0xfc00)
	STWRtoPR(&_rFsU_, r3);

	if (_Rd_ == 12 || _Rd_ == 13) {
		LIW(r4, (u32)pc);
		iFlushRegs();
		STWRtoPR(&psxRegs.pc, r4);
		CALLFunc((u32)psxTestSWInts);

		if(_Rd_ == 12) {
			CALLFunc((uptr)psxTestIntc);
		}

		if(branch == 0) {
			branch = 2;
			iRet();
		}
	}
}

static void recCTC0() {
// Cop0->Rd = Rt

	recMTC0();
}
#endif // NEW_REGS
#endif

static void recHLE() {
	iFlushRegs();

	CALLFunc((u32)psxHLEt[psxRegs.code & 0x07]);
	branch = 2;
	iRet();
}

static void recNULL() {
//	SysMessage("recUNK: %8.8x\n", psxRegs.code);
}

#endif
 