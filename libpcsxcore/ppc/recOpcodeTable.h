#ifndef _recR3000ATABLE_H_
#define _recR3000ATABLE_H_

/* - Arithmetic with immediate operand - */
/*********************************************************
* Arithmetic with immediate operand                      *
* Format:  OP rt, rs, immediate                          *
*********************************************************/

#if 0
REC_FUNC(SLTI);
REC_FUNC(SLTIU);
#else
//CR0:	SIGN      | POSITIVE | ZERO  | SOVERFLOW | SOVERFLOW | OVERFLOW | CARRY
static void recSLTI() {
// Rt = Rs < Im (signed)
	if (!_Rt_) return;

	if (IsConst(_Rs_)) {
		MapConst(_Rt_, (s32)iRegs[_Rs_].k < _Imm_);
	} else {
		iRegs[_Rt_].state = ST_UNK;

		if (_Imm_ == 0) {
			LWPRtoR(r3, &_rRsS_);
			SRWI(r3, r3, 31);
			STWRtoPR(&_rRtS_, r3);
		} else {
			LWPRtoR(r3, &_rRsS_);
			CMPWI(r3, _Imm_);
			LI(r3, 1);
			STWRtoPR(&_rRtS_, r3);
			BLT(1);
			LI(r3, 0);
			STWRtoPR(&_rRtS_, r3);
		}
	}
}

static void recSLTIU() {
// Rt = Rs < Im (unsigned)
	if (!_Rt_) return;

	if (IsConst(_Rs_)) {
		MapConst(_Rt_, iRegs[_Rs_].k < _ImmU_);
	} else {
		iRegs[_Rt_].state = ST_UNK;
		LWPRtoR(r3, &_rRsU_);
		CMPLWI(r3, (u32)_ImmU_);
		LI(r3, 1);
		STWRtoPR(&_rRtU_, r3);
		BLT(1);
		LI(r3, 0);
		STWRtoPR(&_rRtU_, r3);
	}
}
#endif
#if 1
REC_FUNC(ADDI);
REC_FUNC(ADDIU);
REC_FUNC(ANDI);
REC_FUNC(ORI);
REC_FUNC(XORI)
#else
static void recADDIU()  {
// Rt = Rs + Im
	if (!_Rt_) return;

	if (IsConst(_Rs_)) {
		MapConst(_Rt_, iRegs[_Rs_].k + _Imm_);
	} else {
		iRegs[_Rt_].state = ST_UNK;

		LWPRtoR(r3, &_rRsS_);
		LIW(r9, _Imm_);
		ADD(r3, r3, r9)
		STWRtoPR(&_rRtS_, r3);
	}
}

static void recADDI()  {
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
		LIW(r9, _ImmU_);
		AND(r3, r3, r9);
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

		LWPRtoR(r9, &_rRsU_);
		LIW(r3, _ImmU_);
		OR(r3, r3, r9);
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
		LWPRtoR(r9, &_rRsU_);
		LIW(r3, _ImmU_);
		XOR(r3, r3, r9);
		STWRtoPR(&_rRtU_, r3);
	}
}
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
#else
static void recADDU() {
// Rd = Rs + Rt 
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, (s32)iRegs[_Rs_].k + (s32)iRegs[_Rt_].k);
		return;
	} else if (IsConst(_Rs_)) {
		iRegs[_Rd_].state = ST_UNK;

		LWPRtoR(r9, &_rRtS_);
		LIW(r3, (s32)iRegs[_Rs_].k);
		ADD(r3, r3, r9);
		STWRtoPR(&_rRdS_, r3);
	} else if (IsConst(_Rt_)) {
		iRegs[_Rd_].state = ST_UNK;

		LWPRtoR(r3, &_rRsS_);
		LIW(r9, (s32)iRegs[_Rt_].k);
		ADD(r3, r3, r9);
		STWRtoPR(&_rRdS_, r3);
	}
	else {
		iRegs[_Rd_].state = ST_UNK;
		
		LWPRtoR(r3, &_rRsS_);
		LWPRtoR(r9, &_rRtS_);
		ADD(r3, r3, r9);
		STWRtoPR(&_rRdS_, r3);
	}
}

static void recADD() {
// Rd = Rs + Rt
	recADDU();
}
#endif

#if 0
REC_FUNC(SUB);
REC_FUNC(SUBU);
#else
static void recSUBU() {
// Rd = Rs - Rt
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, (s32)iRegs[_Rs_].k - (s32)iRegs[_Rt_].k);
		return;
	} else if (IsConst(_Rs_)) {
		iRegs[_Rd_].state = ST_UNK;

		LIW(r3, (s32)iRegs[_Rs_].k);
		LWPRtoR(r9, &_rRtS_);
		SUBF(r3, r9, r3);
		STWRtoPR(&_rRdS_, r3);
	} else if (IsConst(_Rt_)) {
		iRegs[_Rd_].state = ST_UNK;

		LWPRtoR(r3, &_rRsS_);
		LIW(r9, (s32)iRegs[_Rt_].k);
		SUBF(r3, r9, r3);
		STWRtoPR(&_rRdS_, r3);
	}
	else {
		iRegs[_Rd_].state = ST_UNK;

		LWPRtoR(r3, &_rRsS_);
		LWPRtoR(r9, &_rRtS_);
		SUBF(r3, r9, r3);
		STWRtoPR(&_rRdS_, r3);
	}
}   

static void recSUB() {
// Rd = Rs - Rt
	recSUBU();
}
#endif

#if 0
REC_FUNC(AND);
REC_FUNC(OR);
REC_FUNC(XOR);
REC_FUNC(NOR);
#else
static void recAND() {
// Rd = Rs And Rt
	if (!_Rd_) return;
    
	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rs_].k & iRegs[_Rt_].k);
	} else if (IsConst(_Rs_)) {
		iRegs[_Rd_].state = ST_UNK;

		LIW(r9, iRegs[_Rs_].k);
		LWPRtoR(r3, &_rRtU_);
		AND(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	} else if (IsConst(_Rt_)) {
		iRegs[_Rd_].state = ST_UNK;

		LIW(r3, iRegs[_Rt_].k);
		LWPRtoR(r9, &_rRsU_);
		AND(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	} else {
		iRegs[_Rd_].state = ST_UNK;
		
		LWPRtoR(r3, &_rRtU_);
		LWPRtoR(r9, &_rRsU_);
		AND(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	}
}   

static void recOR() {
// Rd = Rs Or Rt
	if (!_Rd_) return;
    
	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rs_].k | iRegs[_Rt_].k);
	} else if (IsConst(_Rs_)) {
		iRegs[_Rd_].state = ST_UNK;

		LIW(r9, iRegs[_Rs_].k);
		LWPRtoR(r3, &_rRtU_);
		OR(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	} else if (IsConst(_Rt_)) {
		iRegs[_Rd_].state = ST_UNK;

		LIW(r3, iRegs[_Rt_].k);
		LWPRtoR(r9, &_rRsU_);
		OR(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	} else {
		iRegs[_Rd_].state = ST_UNK;
		
		LWPRtoR(r3, &_rRtU_);
		LWPRtoR(r9, &_rRsU_);
		OR(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	}
}

static void recXOR() {
// Rd = Rs Xor Rt
	if (!_Rd_) return;
    
	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rs_].k | iRegs[_Rt_].k);
	} else if (IsConst(_Rs_)) {
		iRegs[_Rd_].state = ST_UNK;

		LIW(r9, iRegs[_Rs_].k);
		LWPRtoR(r3, &_rRtU_);
		XOR(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	} else if (IsConst(_Rt_)) {
		iRegs[_Rd_].state = ST_UNK;

		LIW(r3, iRegs[_Rt_].k);
		LWPRtoR(r9, &_rRsU_);
		XOR(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	} else {
		iRegs[_Rd_].state = ST_UNK;
		
		LWPRtoR(r3, &_rRtU_);
		LWPRtoR(r9, &_rRsU_);
		XOR(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	}
}

static void recNOR() {
// Rd = Rs Nor Rt
	if (!_Rd_) return;
    
	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rs_].k | iRegs[_Rt_].k);
	} else if (IsConst(_Rs_)) {
		iRegs[_Rd_].state = ST_UNK;

		LIW(r9, iRegs[_Rs_].k);
		LWPRtoR(r3, &_rRtU_);
		NOR(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	} else if (IsConst(_Rt_)) {
		iRegs[_Rd_].state = ST_UNK;

		LIW(r3, iRegs[_Rt_].k);
		LWPRtoR(r9, &_rRsU_);
		NOR(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	} else {
		iRegs[_Rd_].state = ST_UNK;
		
		LWPRtoR(r3, &_rRtU_);
		LWPRtoR(r9, &_rRsU_);
		NOR(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	}
}

#endif

#if 0
REC_FUNC(SLT);
#else
static void recSLT() {
// Rd = Rs < Rt (signed)
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, (s32)iRegs[_Rs_].k < (s32)iRegs[_Rt_].k);
	} else if(IsConst(_Rs_)) {
		iRegs[_Rd_].state = ST_UNK;
		
		LWPRtoR(r3, &_rRtS_);
		LIW(r9, (s32)iRegs[_Rs_].k);
		CMPW(r9, r3);
		LI(r3, 1);
		STWRtoPR(&_rRdS_, r3);
		BLT(1);
		LI(r3, 0);
		STWRtoPR(&_rRdS_, r3);
	} else if (IsConst(_Rt_)) {
		iRegs[_Rd_].state = ST_UNK;
		
		LWPRtoR(r9, &_rRsS_);
		LIW(r3, (s32)iRegs[_Rt_].k);
		CMPW(r9, r3);
		LI(r3, 1);
		STWRtoPR(&_rRdS_, r3);
		BLT(1);
		LI(r3, 0);
		STWRtoPR(&_rRdS_, r3);
	} else {
		iRegs[_Rd_].state = ST_UNK;
		
		LWPRtoR(r3, &_rRtS_);
		LWPRtoR(r9, &_rRsS_);
		CMPW(r9, r3);
		LI(r3, 1);
		STWRtoPR(&_rRdS_, r3);
		BLT(1);
		LI(r3, 0);
		STWRtoPR(&_rRdS_, r3);
	}
}
#endif

#if 1
REC_FUNC(SLTU);
#else
static void recSLTU() { 
// Rd = Rs < Rt (unsigned)
	if (!_Rd_) return;

	if (IsConst(_Rs_) && IsConst(_Rt_)) {
		MapConst(_Rd_, iRegs[_Rs_].k < iRegs[_Rt_].k);
	} else if(IsConst(_Rs_)) {
		iRegs[_Rd_].state = ST_UNK;

		LWPRtoR(r3, &_rRtU_);
		LIW(r9, iRegs[_Rs_].k);
		CMPW(r9, r3);
		LI(r3, 1);
		STWRtoPR(&_rRdU_, r3);
		BLT(1);
		LI(r3, 0);
		STWRtoPR(&_rRdU_, r3);
	} else if (IsConst(_Rt_)) {
		iRegs[_Rd_].state = ST_UNK;
		
		LWPRtoR(r9, &_rRsU_);
		LIW(r3, iRegs[_Rt_].k);
		CMPW(r9, r3);
		LI(r3, 1);
		STWRtoPR(&_rRdU_, r3);
		BLT(1);
		LI(r3, 0);
		STWRtoPR(&_rRdU_, r3);
	} else {
		iRegs[_Rd_].state = ST_UNK;
		
		LWPRtoR(r3, &_rRtU_);
		LWPRtoR(r9, &_rRsU_);
		CMPW(r9, r3);
		LI(r3, 1);
		STWRtoPR(&_rRdU_, r3);
		BLT(1);
		LI(r3, 0);
		STWRtoPR(&_rRdU_, r3);
	}
}
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
	if(IsConst(_Rs_)) {
		LIW(r3, iRegs[_Rs_].k);
		LWPRtoR(r9, &_rRtS_);
	}
	else if(IsConst(_Rt_)) {
		LWPRtoR(r3, &_rRsS_);
		LIW(r9, iRegs[_Rt_].k);
	}
	else {
		LWPRtoR(r3, &_rRsS_);
		LWPRtoR(r9, &_rRtS_);
	}
	MULHW(r6, r3, r9);
	MULLW(r8, r3, r9);
	STWRtoPR(&_rHiU_, r6);
	STWRtoPR(&_rLoU_, r8);
}

static void recMULTU() {
// Lo/Hi = Rs * Rt (unsigned)
	if(IsConst(_Rs_)) {
		LIW(r3, iRegs[_Rs_].k);
		LWPRtoR(r9, &_rRtU_);
	}
	else if(IsConst(_Rt_)) {
		LWPRtoR(r3, &_rRsU_);
		LIW(r9, iRegs[_Rt_].k);
	}
	else {
		LWPRtoR(r3, &_rRsU_);
		LWPRtoR(r9, &_rRtU_);
	}
	MULHWU(r6, r3, r9);
	MULLW(r8, r3, r9);
	STWRtoPR(&_rHiU_, r6);
	STWRtoPR(&_rLoU_, r8);
}
#endif

#if 0
REC_FUNC(DIV);
REC_FUNC(DIVU);
#else
static void recDIV() {
// Lo/Hi = Rs / Rt (signed)

	if(IsConst(_Rs_) && IsConst(_Rt_)) {
		if( iRegs[_Rt_].k == 0 )
		{
			LIW(r3, iRegs[_Rs_].k);
			LIW(r9, ((iRegs[_Rs_].k >= 0) ? -1 : 1));
			STWRtoPR(&_rHiS_, r3);
			STWRtoPR(&_rLoS_, r9);
			return;
		}
		if( iRegs[_Rs_].k == 0x80000000 && iRegs[_Rt_].k == 0xffffffff )
		{
			LIW(r9, iRegs[_Rs_].k);
			LI(r3, 0);
			STWRtoPR(&_rHiS_, r3);
			STWRtoPR(&_rLoS_, r9);
			return;
		}

		LIW(r3, iRegs[_Rs_].k % iRegs[_Rt_].k);
		LIW(r9, iRegs[_Rs_].k / iRegs[_Rt_].k);
		STWRtoPR(&_rHiS_, r3);
		STWRtoPR(&_rLoS_, r9);
		return;
	}
	else if(IsConst(_Rs_)) {
		LIW(r3, iRegs[_Rs_].k);
		LWPRtoR(r9, &_rRtS_);
	}
	else if(IsConst(_Rt_)) {
		LWPRtoR(r3, &_rRsS_);
		LIW(r9, iRegs[_Rt_].k);
	}
	else {
		LWPRtoR(r3, &_rRsS_);
		LWPRtoR(r9, &_rRtS_);
	}
	
	CMPWI(r9, 0);
	BNE_L(b32Ptr[0]);	// if(Rt == 0)

	CMPWI(r3, 0);
	BLT_L(b32Ptr[1]);	// Rs >= 0 ? ...
	LI(r9, -1);		// then -1 ...
	STWRtoPR(&_rHiS_, r3);
	STWRtoPR(&_rLoS_, r9);
	B_L(b32Ptr[2]);

	B_DST(b32Ptr[1]);	// else 1.
	LI(r9, 1);
	STWRtoPR(&_rHiS_, r3);
	STWRtoPR(&_rLoS_, r9);
	B_L(b32Ptr[2]);

	B_DST(b32Ptr[0]);
	LIS(r10, -32768);
	CMPW(r3, r10);
	BEQ_L(b32Ptr[3]);	// if(( Rs == 0x80000000 )
	B_FROM(b32Ptr[4]);
	DIVW(r10, r3, r9);	// rLo = Rs / Rt 
	MULLW(r9, r10, r9);
	SUBF(r9, r9, r3);	// rHi = Rs % Rt
	STWRtoPR(&_rLoS_, r10);
	STWRtoPR(&_rHiS_, r9);
	B_L(b32Ptr[2]);
	
	B_DST(b32Ptr[3]);
	CMPWI(r9, -1);
	BNE(*b32Ptr[4]);	// && Rt == 0xffffffff)
	STWRtoPR(&_rLoS_, r3);
	LI(r3, 0);
	STWRtoPR(&_rHiS_, r3);
	
	B_DST(b32Ptr[2]);
}

static void recDIVU() {
// Lo/Hi = Rs / Rt (unsigned)

	if(IsConst(_Rs_) && IsConst(_Rt_)) {
		if( iRegs[_Rt_].k == 0 )
		{
			LIW(r3, iRegs[_Rs_].k);
			LIW(r9, (u32)(-1));
			STWRtoPR(&_rHiU_, r3);
			STWRtoPR(&_rLoU_, r9);
			return;
		}
		LIW(r3, iRegs[_Rs_].k % iRegs[_Rt_].k);
		LIW(r9, iRegs[_Rs_].k / iRegs[_Rt_].k);
		STWRtoPR(&_rHiU_, r3);
		STWRtoPR(&_rLoU_, r9);
		return;
	}
	else if(IsConst(_Rs_)) {
		LIW(r3, iRegs[_Rs_].k);
		LWPRtoR(r9, &_rRtU_);
	}
	else if(IsConst(_Rt_)) {
		LWPRtoR(r3, &_rRsU_);
		LIW(r9, iRegs[_Rt_].k);
	}
	else {
		LWPRtoR(r3, &_rRsU_);
		LWPRtoR(r9, &_rRtU_);
	}

	CMPWI(r9, 0);
	BNE_L(b32Ptr[0]);

	LI(r9, -1);
	STWRtoPR(&_rHiU_, r3);
	STWRtoPR(&_rLoU_, r9);
	B_L(b32Ptr[2]);

	B_DST(b32Ptr[0]);
	DIVWU(r10, r3, r9);	// rLo = Rs / Rt 
	MULLW(r9, r10, r9);
	SUBF(r9, r9, r3);	// rHi = Rs % Rt
	STWRtoPR(&_rLoU_, r10);
	STWRtoPR(&_rHiU_, r9);

	B_DST(b32Ptr[2]);
}
#endif
//End of * Register mult/div & Register trap logic  

/* - memory access - */

#if 0
REC_FUNC(SWL);
#else
static void recSWL() {

	if (IsConst(_Rs_)) {
#if 1
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;
		const u32 shift = (addr & 3) << 3;

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
#endif
	}
	
	SetArg_OfB(r12);
	RLWINM(r10, r12, 3, 27, 28);	// (addr & 3) << 3
	RLWINM(r12, r12, 0, 0, 29);		// (addr & ~3)
	MR(PPCARG1, r12);
	CALLFunc((u32)psxMemRead32);
	if (IsConst(_Rt_)) {
		LIW(r11, iRegs[_Rt_].k);
	}
	else {
		LWPRtoR(r11, &_rRtU_);
	}
	LI(r4, -256);
	SLW(r4, r4, r10);
	SUBFIC(r10, r10, 24);
	AND(r4, r3, r4);
	MR(PPCARG1, r12);
	SRW(r10, r11, r10);

	OR(PPCARG2, r4, r10);
	CALLFunc((uptr)psxMemWrite32);
}
#endif
#if 0
REC_FUNC(SWR);
#else
static void recSWR() {

	if (IsConst(_Rs_)) {
#if 1
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;
		const u32 shift = (addr & 3) << 3;

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
#endif
	}

	SetArg_OfB(r12);
	RLWINM(r10, r12, 3, 27, 28);	// (addr & 3) << 3
	RLWINM(r12, r12, 0, 0, 29);		// (addr & ~3)
	MR(PPCARG1, r12);
	CALLFunc((u32)psxMemRead32);
	if (IsConst(_Rt_)) {
		LIW(r11, iRegs[_Rt_].k);
	}
	else {
		LWPRtoR(r11, &_rRtU_);
	}

	LIS(r9, 255);
	ORI(r9, r9, 65535);
	SLW(r11, r11, r10);
	SUBFIC(r10, r10, 24);
	SRAW(r9, r9, r10);

	AND(r3, r3, r9);
	OR(PPCARG2, r11, r3);

	MR(PPCARG1, r12);
	CALLFunc((uptr)psxMemWrite32);
}
#endif
#if 0
REC_FUNC(LWL);
#else
static void recLWL() {

	if (IsConst(_Rs_)) {
#if 1
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;
		const u32 shift = (addr & 3) << 3;

		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (!_Rt_) return;
			iRegs[_Rt_].state = ST_UNK;

			LWMtoR(r10, (uptr)&psxM[addr & 0x1ffffc]);
			LIS(r9, 255);
			ORI(r9, r9, 65535);
			SLWI(r10, r10, (24-shift));
			SRAWI(r12, r9, shift);
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
			LIS(r9, 255);
			ORI(r9, r9, 65535);
			SLWI(r10, r10, (24-shift));
			SRAWI(r12, r9, shift);
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
#endif
	}

	SetArg_OfB(r12);
	RLWINM(PPCARG1, r12, 0, 0, 29);		// (add & ~3)
	CALLFunc((u32)psxMemRead32);
	if(_Rt_) {
		iRegs[_Rt_].state = ST_UNK;

		RLWINM(r12, r12, 3, 27, 28);	// (addr & 3) << 3
		LIS(r9, 255);
		ORI(r9, r9, 65535);
		SRAW(r9, r9, r12);
		SUBFIC(r12, r12, 24);
		SLW(r3, r3, r12);
		if (IsConst(_Rt_)) {
			LIW(r11, iRegs[_Rt_].k);
		}
		else {
			LWPRtoR(r11, &_rRtU_);
		}
		AND(r9, r9, r11);
		OR(r3, r3, r9);

		STWRtoPR(&_rRtU_, r3);
	}
}
#endif
#if 0
REC_FUNC(LWR);
#else
static void recLWR() {

	if (IsConst(_Rs_)) {
#if 1
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;
		const u32 shift = (addr & 3) << 3;

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
#endif
	}

	SetArg_OfB(r12);
	RLWINM(PPCARG1, r12, 0, 0, 29);		// (addr & ~3)
	CALLFunc((u32)psxMemRead32);
	if(_Rt_) {
		iRegs[_Rt_].state = ST_UNK;

		RLWINM(r12, r12, 3, 27, 28);	// shift = (addr & 3) << 3
		SUBFIC(r9, r12, 24);
		LI(r10, -256);					// 0xffffff00
		SRW(r3, r3, r12);
		SLW(r9, r10, r9);
		if (IsConst(_Rt_)) {
			LIW(r11, iRegs[_Rt_].k);
		}
		else {
			LWPRtoR(r11, &_rRtU_);
		}
		AND(r9, r9, r11);
		OR(r3, r3, r9);
		STWRtoPR(&_rRtU_, r3);
	}
}
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
    
		if ((t & 0xfff0)  == 0xbfc0) {
			if (!_Rt_) return;
			// since bios is readonly it won't change
			MapConst(_Rt_, psxRs8(addr));
			return;
		}
		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (!_Rt_) return;
			iRegs[_Rt_].state = ST_UNK;

			LWMtoR(r3, (uptr)&psxM[addr & 0x1fffff]);
			EXTSB(r3, r3);
			STWRtoPR(&_rRtS_, r3);
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (!_Rt_) return;
			iRegs[_Rt_].state = ST_UNK;

			LWMtoR(r3, (uptr)&psxH[addr & 0xfff]);
			EXTSB(r3, r3);
			STWRtoPR(&_rRtS_, r3);
			return;
		}
	//	SysPrintf("unhandled r8 %x\n", addr);
	}
#endif
	SetArg_OfB(PPCARG1);
	CALLFunc((u32)psxMemRead8);
	if (_Rt_) {
		iRegs[_Rt_].state = ST_UNK;
		EXTSB(r3, r3);
		STWRtoPR(&_rRtS_, r3);
	}
	resp += 16;
}

static void recLBU() {
// Rt = mem[Rs + Im] (unsigned)
#if 1
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;

		if ((t & 0xfff0)  == 0xbfc0) {
			if (!_Rt_) return;
			// since bios is readonly it won't change
			MapConst(_Rt_, psxRu8(addr));
			return;
		}
		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (!_Rt_) return;
			iRegs[_Rt_].state = ST_UNK;

			LWMtoR(r3, (uptr)&psxM[addr & 0x1fffff]);
			RLWINM(r3, r3, 0, 24, 31);
			STWRtoPR(&_rRtU_, r3);
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (!_Rt_) return;
			iRegs[_Rt_].state = ST_UNK;

			LWMtoR(r3, (uptr)&psxH[addr & 0xfff]);
			RLWINM(r3, r3, 0, 24, 31);
			STWRtoPR(&_rRtU_, r3);
			return;
		}
	}
#endif
	SetArg_OfB(PPCARG1);
	CALLFunc((u32)psxMemRead8);

	if (_Rt_) {
		iRegs[_Rt_].state = ST_UNK;
		RLWINM(r3, r3, 0, 24, 31);
		STWRtoPR(&_rRtU_, r3);
	}
	resp += 16;
}

static void recLH() {
// Rt = mem[Rs + Im] (signed)
#if 1
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;
	
		if ((t & 0xfff0)  == 0xbfc0) {
			if (!_Rt_) return;
			// since bios is readonly it won't change
			MapConst(_Rt_, psxRs16(addr));
			return;
		}
		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (!_Rt_) return;
			iRegs[_Rt_].state = ST_UNK;

			LWMtoR(r3, (uptr)&psxM[addr & 0x1fffff]);
			EXTSH(r3, r3);
			STWRtoPR(&_rRtS_, r3);
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (!_Rt_) return;
			iRegs[_Rt_].state = ST_UNK;

			LWMtoR(r3, (uptr)&psxH[addr & 0xfff]);
			EXTSH(r3, r3);
			STWRtoPR(&_rRtS_, r3);
			return;
		}
	//	SysPrintf("unhandled r16 %x\n", addr);
	}
#endif
	SetArg_OfB(PPCARG1);
	CALLFunc((u32)psxMemRead16);
	if (_Rt_) {
		iRegs[_Rt_].state = ST_UNK;
		EXTSH(r3, r3);
		STWRtoPR(&_rRtS_, r3);
	}
	resp += 16;
}

static void recLHU() {
// Rt = mem[Rs + Im] (unsigned)
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
			iRegs[_Rt_].state = ST_UNK;

			LWMtoR(r3, (uptr)&psxM[addr & 0x1fffff]);
			RLWINM(r3, r3, 0, 16, 31);
			STWRtoPR(&_rRtU_, r3);
			resp += 8;
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (!_Rt_) return;
			iRegs[_Rt_].state = ST_UNK;

			LWMtoR(r3, (uptr)&psxH[addr & 0xfff]);
			RLWINM(r3, r3, 0, 16, 31);
			STWRtoPR(&_rRtU_, r3);
			resp += 8;
			return;
		}

		if (t == 0x1f80) {
			if (addr >= 0x1f801c00 && addr < 0x1f801e00) {
					if (!_Rt_) return;
					iRegs[_Rt_].state = ST_UNK;

					LIW(PPCARG1, addr);
					CALLFunc((uptr)SPU_readRegister);
					RLWINM(r3, r3, 0, 16, 31);
					STWRtoPR(&_rRtU_, r3);
					resp += 8;
					return;
			}
			switch (addr) {
				case 0x1f801100: case 0x1f801110: case 0x1f801120:
					if (!_Rt_) return;
					iRegs[_Rt_].state = ST_UNK;

					LIW(PPCARG1, (addr >> 4) & 0x3);
					CALLFunc((uptr)psxRcntRcount);
					RLWINM(r3, r3, 0, 16, 31);
					STWRtoPR(&_rRtU_, r3);
					resp += 16;
					return;

				case 0x1f801104: case 0x1f801114: case 0x1f801124:
					if (!_Rt_) return;
					iRegs[_Rt_].state = ST_UNK;

					LIW(r3, (uptr)&psxCounters);
					LWZ(r3, r3, OFFSET(&psxCounters, (&psxCounters[(addr >> 4) & 0x3].mode)));
					RLWINM(r3, r3, 0, 16, 31);
					STWRtoPR(&_rRtU_, r3);
					resp += 8;
					return;
	
				case 0x1f801108: case 0x1f801118: case 0x1f801128:
					if (!_Rt_) return;
					iRegs[_Rt_].state = ST_UNK;

					LIW(r3, (uptr)&psxCounters);
					LWZ(r3, r3, OFFSET(&psxCounters, (&psxCounters[(addr >> 4) & 0x3].target)));
					RLWINM(r3, r3, 0, 16, 31);
					STWRtoPR(&_rRtU_, r3);
					resp += 8;
					return;
			}
		}

	//	SysPrintf("unhandled r16u %x\n", addr);
	}

	SetArg_OfB(PPCARG1);
	CALLFunc((u32)psxMemRead16);
	if (_Rt_) {
		iRegs[_Rt_].state = ST_UNK;
		//RLWINM(r3, r3, 0, 16, 31);
		STWRtoPR(&_rRtU_, r3);
	}
	resp += 16;
}
#endif

#if 0
REC_FUNC(LW);
#else
static void recLW() {
// Rt = mem[Rs + Im] (unsigned)

if(!Settings.Lw) {
	iFlushRegs();
	LIW(r3, (u32)psxRegs.code);
	STWRtoPR(&psxRegs.code, r3);
	LIW(r3, (u32)pc);
	STWRtoPR(&psxRegs.pc, r3);
	CALLFunc((u32)psxLW);
} 
else {
#if 1
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;

		/*if ((t & 0xfff0) == 0xbfc0) {
			if (!_Rt_) return;
			// since bios is readonly it won't change
			MapConst(_Rt_, psxRu32(addr));
			return;
		}*/
		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			if (!_Rt_) return;
			iRegs[_Rt_].state = ST_UNK;

			LWMtoR(r3, (uptr)&psxM[addr & 0x1fffff]);
			STWRtoPR(&_rRtS_, r3);
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			if (!_Rt_) return;
			iRegs[_Rt_].state = ST_UNK;

			LWMtoR(r3, (uptr)&psxH[addr & 0xfff]);
			STWRtoPR(&_rRtS_, r3);
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
					iRegs[_Rt_].state = ST_UNK;
					
					LWMtoR(r3, (uptr)&psxH[addr & 0xffff]);
					STWRtoPR(&_rRtS_, r3);
					return;

				case 0x1f801810:
					if (!_Rt_) return;
					iRegs[_Rt_].state = ST_UNK;

					CALLFunc((uptr)GPU_readData);
					STWRtoPR(&_rRtS_, r3);
					return;

				case 0x1f801814:
					if (!_Rt_) return;
					iRegs[_Rt_].state = ST_UNK;

					CALLFunc((uptr)GPU_readStatus);
					STWRtoPR(&_rRtS_, r3);
					return;
			}
		}

//		SysPrintf("unhandled r32 %x\n", addr);
	}
#endif
	SetArg_OfB(PPCARG1);
	CALLFunc((uptr)psxMemRead32);
	if (_Rt_) {
		iRegs[_Rt_].state = ST_UNK;
		STWRtoPR(&_rRtS_, r3);
	}
	//ADDI(r1, r1, 16);
	resp += 16;
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
	if (IsConst(_Rt_)) {
		LIW(PPCARG2, (u8)iRegs[_Rt_].k);
	} else {
		LBPRtoR(PPCARG2, &psxRegs.GPR.r[_Rt_].b.l);
	}
	CALLFunc((u32)psxMemWrite8);
	resp += 8;
}

static void recSH() {
// mem[Rs + Im] = Rt

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
				resp+= 8;
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

	SetArg_OfB(PPCARG1);
	if (IsConst(_Rt_)) {
		LIW(PPCARG2, (u16)iRegs[_Rt_].k);
	} else {
		LHPRtoR(PPCARG2, &psxRegs.GPR.r[_Rt_].w.l);
	}
	CALLFunc((u32)psxMemWrite16);
	resp += 8;
}

static void recSW() {
// mem[Rs + Im] = Rt
	//u32 *b1, *b2;

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
					//resp+= 4;
					return;

				case 0x1f801814:
					if (IsConst(_Rt_)) {
						LIW(PPCARG1, iRegs[_Rt_].k);
					} else {
						LWPRtoR(PPCARG1, &_rRtU_);
					}
					CALLFunc((u32)&GPU_writeStatus);
					//resp+= 4;
					return;
					
				case 0x1f801820:
					if (IsConst(_Rt_)) {
						LIW(PPCARG1, iRegs[_Rt_].k);
					} else {
						LWPRtoR(PPCARG1, &_rRtU_);
					}
					CALLFunc((u32)&mdecWrite0);
					//resp+= 4;
					return;

				case 0x1f801824:
					if (IsConst(_Rt_)) {
						LIW(PPCARG1, iRegs[_Rt_].k);
					} else {
						LWPRtoR(PPCARG1, &_rRtU_);
					}
					CALLFunc((u32)&mdecWrite1);
					//resp+= 4;
					return;
			}
		}

	}

	SetArg_OfB(PPCARG1);
	if (IsConst(_Rt_)) {
		LIW(PPCARG2, iRegs[_Rt_].k);
	} else {
		LWPRtoR(PPCARG2, &_rRtU_);
	}
	CALLFunc((u32)psxMemWrite32);
	resp += 8;
}
#endif

#if 0
REC_FUNC(SLL);
REC_FUNC(SRL);
REC_FUNC(SRA);
#else
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
#endif

/* - shift ops - */

#if 0
REC_FUNC(SLLV);
REC_FUNC(SRLV);
REC_FUNC(SRAV);
#else
static void recSLLV() {
// Rd = Rt << Rs
	if (!_Rd_) return;

	if (IsConst(_Rt_) && IsConst(_Rs_)) {
		MapConst(_Rd_, iRegs[_Rt_].k << (iRegs[_Rs_].k & 0x1f));
	}
	else if (IsConst(_Rs_)) {
		iRegs[_Rd_].state = ST_UNK;
		LWPRtoR(r3, &_rRtU_);
		if(iRegs[_Rs_].k & 0x1f) SLWI(r3, r3, (iRegs[_Rs_].k & 0x1f));
		STWRtoPR(&_rRdU_, r3);
	}
	else if (IsConst(_Rt_)) {
		iRegs[_Rd_].state = ST_UNK;
		LIW(r3, iRegs[_Rt_].k);
		LWPRtoR(r9, &_rRsU_);
		RLWINM(r9, r9, 0, 27, 31); // &0x1f
		SLW(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	}
	else {
		iRegs[_Rd_].state = ST_UNK;
		LWPRtoR(r3, &_rRtU_);
		LWPRtoR(r9, &_rRsU_);
		RLWINM(r9, r9, 0, 27, 31); // &0x1f
		SLW(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	}
}

static void recSRLV() {
// Rd = Rt >> Rs
	if (!_Rd_) return;

	if (IsConst(_Rt_) && IsConst(_Rs_)) {
		MapConst(_Rd_, iRegs[_Rt_].k >> (iRegs[_Rs_].k & 0x1f));
	}
	else if (IsConst(_Rs_)) {
		iRegs[_Rd_].state = ST_UNK;
		LWPRtoR(r3, &_rRtU_);
		if(iRegs[_Rs_].k & 0x1f) SRWI(r3, r3, (iRegs[_Rs_].k & 0x1f));
		STWRtoPR(&_rRdU_, r3);
	}
	else if (IsConst(_Rt_)) {
		iRegs[_Rd_].state = ST_UNK;
		LIW(r3, iRegs[_Rt_].k);
		LWPRtoR(r9, &_rRsU_);
		RLWINM(r9, r9, 0, 27, 31); // &0x1f
		SRW(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	}
	else {
		iRegs[_Rd_].state = ST_UNK;
		LWPRtoR(r3, &_rRtU_);
		LWPRtoR(r9, &_rRsU_);
		RLWINM(r9, r9, 0, 27, 31); // &0x1f
		SRW(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	}
}

static void recSRAV() {
// Rd = Rt >> Rs
	if (!_Rd_) return;

	if (IsConst(_Rt_) && IsConst(_Rs_)) {
		MapConst(_Rd_, (s32)iRegs[_Rt_].k >> (iRegs[_Rs_].k & 0x1f));
	}
	else if (IsConst(_Rs_)) {
		iRegs[_Rd_].state = ST_UNK;

		LWPRtoR(r3, &_rRtU_);
		if(iRegs[_Rs_].k & 0x1f) SRAWI(r3, r3, (iRegs[_Rs_].k & 0x1f));
		STWRtoPR(&_rRdU_, r3);
	}
	else if (IsConst(_Rt_)) {
		iRegs[_Rd_].state = ST_UNK;

		LIW(r3, iRegs[_Rt_].k);
		LWPRtoR(r9, &_rRsU_);
		RLWINM(r9, r9, 0, 27, 31); // &0x1f
		SRAW(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	}
	else {
		iRegs[_Rd_].state = ST_UNK;

		LWPRtoR(r3, &_rRtU_);
		LWPRtoR(r9, &_rRsU_);
		RLWINM(r9, r9, 0, 27, 31); // &0x1f
		SRAW(r3, r3, r9);
		STWRtoPR(&_rRdU_, r3);
	}
}
#endif

#if 0
REC_SYS(SYSCALL);
REC_SYS(BREAK);
#else
static void recSYSCALL() {
	iFlushRegs();

	LIW(r3, pc - 4);
	STWRtoPR(&psxRegs.pc, r3);
	LIW(PPCARG1, 0x20);
	LIW(PPCARG2, (branch == 1 ? 1 : 0));
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
static void recMFHI() {
// Rd = Hi
	if (!_Rd_) return;

	iRegs[_Rd_].state = ST_UNK;
	LWPRtoR(r9, &_rHiU_);
	STWRtoPR(&_rRdU_, r9);
}

static void recMTHI() {
// Hi = Rs

	if (IsConst(_Rs_)) {
		LIW(r9, iRegs[_Rs_].k);
		STWRtoPR(&_rHiU_, r9);
	} else {
		LWPRtoR(r9, &_rRsU_);
		STWRtoPR(&_rHiU_, r9);
	}
}

static void recMFLO() {
// Rd = Lo
	if (!_Rd_) return;
	
	iRegs[_Rd_].state = ST_UNK;
	LWPRtoR(r9, &_rLoU_);
	STWRtoPR(&_rRdU_, r9);
}

static void recMTLO() {
// Lo = Rs

	if (IsConst(_Rs_)) {
		LIW(r9, iRegs[_Rs_].k);
		STWRtoPR(&_rLoU_, r9);
	} else {
		LWPRtoR(r9, &_rRsU_);
		STWRtoPR(&_rLoU_, r9);
	}
}
#endif
/* - branch ops - */

#if 0
REC_BRANCH(BEQ);     // *FIXME
#else
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
			if ((iRegs[_Rs_].k & 0xffff) == iRegs[_Rs_].k) {
				CMPLWI(r3, iRegs[_Rs_].k);
			}
			else if ((s32)(s16)iRegs[_Rs_].k == (s32)iRegs[_Rs_].k) {
				CMPWI(r3, iRegs[_Rs_].k);
			}
			else {
				LWPRtoR(r9, &_rRsU_);
				CMPLW(r9, r3);
			}
		}
		else if (IsConst(_Rt_)) {
			LWPRtoR(r9, &_rRsU_);
			if ((iRegs[_Rt_].k & 0xffff) == iRegs[_Rt_].k) {
				CMPLWI(r9, iRegs[_Rt_].k);
			}
			else if ((s32)(s16)iRegs[_Rt_].k == (s32)iRegs[_Rt_].k) {
				CMPWI(r9, iRegs[_Rt_].k);
			}
			else {
				LWPRtoR(r3, &_rRtU_);
				CMPLW(r9, r3);
			}
		}
		else {
			LWPRtoR(r3, &_rRtU_);
			LWPRtoR(r9, &_rRsU_);
			CMPLW(r9, r3);
		}
		BEQ_L( b32Ptr[4] );

		iBranch(pc+4, 1);

		B_DST( b32Ptr[4] );

		iBranch(bpc, 0);
		pc+=4;
	}
}
#endif

#if 0
REC_BRANCH(BNE);     // *FIXME
#else
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
		if ((iRegs[_Rs_].k & 0xffff) == iRegs[_Rs_].k) {
			CMPLWI(r3, iRegs[_Rs_].k);
		}
		else if ((s32)(s16)iRegs[_Rs_].k == (s32)iRegs[_Rs_].k) {
			CMPWI(r3, iRegs[_Rs_].k);
		}
		else {
			LWPRtoR(r9, &_rRsU_);
			CMPLW(r9, r3);
		}
	}
	else if (IsConst(_Rt_)) {
		LWPRtoR(r9, &_rRsU_);
		if ((iRegs[_Rt_].k & 0xffff) == iRegs[_Rt_].k) {
			CMPLWI(r9, iRegs[_Rt_].k);
		}
		else if ((s32)(s16)iRegs[_Rt_].k == (s32)iRegs[_Rt_].k) {
			CMPWI(r9, iRegs[_Rt_].k);
		}
		else {
			LWPRtoR(r3, &_rRtU_);
			CMPLW(r9, r3);
		}
	}
	else {
		LWPRtoR(r3, &_rRtU_);
		LWPRtoR(r9, &_rRsU_);
		CMPLW(r9, r3);
	}
	BNE_L(b32Ptr[4]);

	iBranch(pc+4, 1);

	B_DST(b32Ptr[4]);

	iBranch(bpc, 0);
	pc+=4;
}
#endif

#if 0
REC_BRANCH(BLTZ);
REC_BRANCH(BGTZ);
REC_BRANCH(BLTZAL);
REC_BRANCH(BGEZAL);

REC_BRANCH(BLEZ);
REC_BRANCH(BGEZ);
#else
static void recBLTZ() {
// Branch if Rs < 0
	u32 bpc = _Imm_ * 4 + pc;
	//u32 *b;
	
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
	BLT_L(b32Ptr[4]);
	
	iBranch(pc+4, 1);

	B_DST(b32Ptr[4]);
	
	iBranch(bpc, 0);
	pc+=4;
}

static void recBGTZ() {
// Branch if Rs > 0
    u32 bpc = _Imm_ * 4 + pc;
    u32 *b;
	
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
    BGT_L(b32Ptr[4]);
    
    iBranch(pc+4, 1);

    B_DST(b32Ptr[4]);

    iBranch(bpc, 0);
    pc+=4;
}

static void recBLTZAL() {
// Branch if Rs < 0
    u32 bpc = _Imm_ * 4 + pc;
    u32 *b;
	
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
    BLT_L(b32Ptr[4]);
    
    iBranch(pc+4, 1);

    B_DST(b32Ptr[4]);
    
    MapConst(31, pc + 4);

    iBranch(bpc, 0);
    pc+=4;
}

static void recBGEZAL() {
// Branch if Rs >= 0
    u32 bpc = _Imm_ * 4 + pc;
    u32 *b;
	
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
    BGE_L(b32Ptr[4]);
    
    iBranch(pc+4, 1);

    B_DST(b32Ptr[4]);
    
    MapConst(31, pc + 4);

    iBranch(bpc, 0);
    pc+=4;
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
	BLE_L(b32Ptr[4]);
	
	iBranch(pc+4, 1);

	B_DST(b32Ptr[4]);
	
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
	BGE_L(b32Ptr[4]);
	
	iBranch(pc+4, 1);

	B_DST(b32Ptr[4]);
	
	iBranch(bpc, 0);
	pc+=4;
}
#endif

#if 0
REC_BRANCH(J);
REC_BRANCH(JR);
REC_BRANCH(JAL);
REC_BRANCH(JALR);
#else
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
		//LIW(r9, (uptr)&target);
		STWRtoM((uptr)&target, r3);
	} else {
		LWPRtoR(r3, &_rRsU_);
		//LIW(r9, (uptr)&target);
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
#endif

#if 0
REC_FUNC(RFE);
#else
static void recRFE() {

	LWPRtoR(r3, &psxRegs.CP0.n.Status);
	RLWINM(r9, r3, 0, 0, 27);
	RLWINM(r3, r3, 30, 28, 31);
	OR(r3, r3, r9);
	STWRtoPR(&psxRegs.CP0.n.Status, r3);

	iFlushRegs();
	LIW(r3, (u32)pc);
	STWRtoPR(&psxRegs.pc, r3);
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
		iFlushRegs();
		LIW(r3, (u32)pc);
		STWRtoPR(&psxRegs.pc, r3);
		CALLFunc((u32)psxTestSWInts);

		if(_Rd_ == 12) {
#ifndef NEW_EVENTS
			LWPRtoR(r3, &psxRegs.interrupt);
			ORIS(r3, r3, 0x8000);
			STWRtoPR(&psxRegs.interrupt, r3);
#else
			CALLFunc((uptr)psxTestIntc);
#endif
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
#endif

static void recHLE() {
	iFlushRegs();

	CALLFunc((u32)psxHLEt[psxRegs.code & 0xffff]);
	branch = 2;
	iRet();
}

#endif
 