// ppc_mnemonics.h

#ifndef _PPC_MNEMONICS_H_
#define _PPC_MNEMONICS_H_

#include "ppc.h"

#define MOD0_INSTR(name, instr) \
	void name(int to) \
	{ \
		write32(instr | (to << 21)); \
	}

#define MOD1_INSTR(name, instr) \
	void name(int to, int from) \
	{ \
		write32(instr | mod1(to, from)); \
	}

#define MOD2_INSTR(name, instr) \
	void name(int to, int from, int offset) \
	{ \
		write32(instr | mod2(to, from, offset)); \
	}

#define MOD3_INSTR(name, instr) \
	void name(int to, int from, int offset) \
	{ \
		write32(instr | mod3(to, from, offset)); \
	}

/* Link register related */
void MFLR(int to);
void MTLR(int to);
void MTCTR(int to);
void BLR();
void BGTLR();

/* Load ops */

// mod1  ((to << 21) | (from & 0xffff))
void LI(int, int);
void LIS(int, int);

// mod2  ((to << 21) | (from << 16) | (offset & 0xffff))
void LWZ(int, int, int);
void LHZ(int, int, int);
void LHA(int, int, int);
void LBZ(int, int, int);
void LMW(int, int, int);

// mod3  ((to << 21) | (from << 16) | (offset << 11))
void LWZX(int, int, int);
void LWBRX(int, int, int);
void LHBRX(int, int, int);

/* Store ops */
// mod2
void STMW(int, int, int);
void STW(int, int, int);
void STH(int, int, int);
void STB(int, int, int);
void STWU(int, int, int);

// mod3
void STHBRX(int, int, int);
void STWBRX(int, int, int);

/* Arithmic ops */
// mod1

// mod2
#if 0
void ADDI  (int, int, int);
void ADDIS (int, int, int);
void ADDIC (int, int, int);
void ADDIC_(int, int, int);
void MULLI (int, int, int);
#else
#define ADDI(REG_DST, REG_SRC, IMM) \
	{int _src = (REG_SRC); int _dst=(REG_DST); \
        write32(0x38000000 | (_dst << 21) | (_src << 16) | ((IMM) & 0xffff));}

#define ADDIS(REG_DST, REG_SRC, IMM) \
	{int _src = (REG_SRC); int _dst=(REG_DST); \
        write32(0x3C000000 | (_dst << 21) | (_src << 16) | ((IMM) & 0xffff));}

#define ADDIC(REG_DST, REG_SRC, IMM) \
	{int _src = (REG_SRC); int _dst=(REG_DST); \
        write32(0x30000000 | (_dst << 21) | (_src << 16) | ((IMM) & 0xffff));}

#define ADDIC_(REG_DST, REG_SRC, IMM) \
	{int _src = (REG_SRC); int _dst=(REG_DST); \
        write32(0x34000000 | (_dst << 21) | (_src << 16) | ((IMM) & 0xffff));}

#define MULLI(REG_DST, REG_SRC, IMM) \
	{int _src = (REG_SRC); int _dst=(REG_DST); \
        write32(0x1C000000 | (_dst << 21) | (_src << 16) | ((IMM) & 0xffff));}
#endif
#define MR(REG_DST, REG_SRC) \
	{int __src = (REG_SRC); int __dst=(REG_DST); \
        if (__src != __dst) {ADDI(__dst, __src, 0);}}

// mod3

#define ADD(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000214 | (_dst << 21) | (_reg1 << 16) |  (_reg2 << 11));}

#define ADDO(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000614 | (_dst << 21) | (_reg1 << 16) |  (_reg2 << 11));}

#define ADDEO(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000514 | (_dst << 21) | (_reg1 << 16) |  (_reg2 << 11));}

#define ADDE(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000114 | (_dst << 21) | (_reg1 << 16) |  (_reg2 << 11));}

#define ADDCO(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000414 | (_dst << 21) | (_reg1 << 16) |  (_reg2 << 11));}

#define ADDZE(REG_DST, REG_SRC) \
	{int _src = (REG_SRC); int _dst=(REG_DST); \
        write32(0x7C000194 | (_dst << 21) | (_src << 16));}

#define SUBF(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000050 | (_dst << 21) | (_reg1 << 16) |  (_reg2 << 11));}

#define SUBFIC(REG_DST, REG1, IMM) \
	{int _reg1 = (REG1); int _dst=(REG_DST); \
        write32(0x20000000 | (_dst << 21) | (_reg1 << 16) |  ((IMM) & 0xffff));}

#define SUBFO(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000450 | (_dst << 21) | (_reg1 << 16) |  (_reg2 << 11));}

#define SUBFC(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000010 | (_dst << 21) | (_reg1 << 16) |  (_reg2 << 11));}

#define SUBFE(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000110 | (_dst << 21) | (_reg1 << 16) |  (_reg2 << 11));}

#define SUBFCO(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000410 | (_dst << 21) | (_reg1 << 16) |  (_reg2 << 11));}

#define SUBFCO_(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000411 | (_dst << 21) | (_reg1 << 16) |  (_reg2 << 11));}

#define SUB(REG_DST, REG1, REG2) \
	{SUBF(REG_DST, REG2, REG1)}

#define SUBO(REG_DST, REG1, REG2) \
	{SUBFO(REG_DST, REG2, REG1)}

#define SUBCO(REG_DST, REG1, REG2) \
	{SUBFCO(REG_DST, REG2, REG1)}

#define SUBCO_(REG_DST, REG1, REG2) \
	{SUBFCO_(REG_DST, REG2, REG1)}
/* Duplicate 
#define SRAWI(REG_DST, REG_SRC, SHIFT) \
	{int _src = (REG_SRC); int _dst=(REG_DST); \
        write32(0x7C000670 | (_src << 21) | (_dst << 16) | (SHIFT << 11));}
*/
#define MULHW(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000096 | (_dst << 21) | (_reg1 << 16) |  (_reg2 << 11));}

#define MULLW(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C0001D6 | (_dst << 21) | (_reg1 << 16) |  (_reg2 << 11));}

#define MULHWU(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000016 | (_dst << 21) | (_reg1 << 16) |  (_reg2 << 11));}

#define DIVW(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C0003D6 | (_dst << 21) | (_reg1 << 16) |  (_reg2 << 11));}

#define DIVWU(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000396 | (_dst << 21) | (_reg1 << 16) |  (_reg2 << 11));}


/* Branch ops */
#define B_FROM(VAR) VAR = ppcPtr
#define B_DST(VAR) *VAR = *VAR | (((s16)((u32)ppcPtr - (u32)VAR)) & 0xfffc)

#define B(DST) \
	{write32(0x48000000 | (((s32)(((DST)+1)<<2)) & 0x3fffffc));}

#define B_L(VAR) \
	{B_FROM(VAR); write32(0x48000001);}

#define BA(DST) \
	{write32(0x48000002 | ((s32)((DST) & 0x3fffffc)));}

#define BLA(DST) \
	{write32(0x48000003 | ((s32)((DST) & 0x3fffffc)));}

#define BNS(DST) \
	{write32(0x40830000 | (((s16)(((DST)+1)<<2)) & 0xfffc));}

#define BNE(DST) \
	{write32(0x40820000 | (((s16)(((DST)+1)<<2)) & 0xfffc));}

#define BNE_L(VAR) \
	{B_FROM(VAR); write32(0x40820001);}

#define BEQ(DST) \
	{write32(0x41820000 | (((s16)(((DST)+1)<<2)) & 0xfffc));}

#define BEQ_L(VAR) \
	{B_FROM(VAR); write32(0x41820001);}

#define BLT(DST) \
	{write32(0x41800000 | (((s16)(((DST)+1)<<2)) & 0xfffc));}

#define BLT_L(VAR) \
	{B_FROM(VAR); write32(0x41800001);}

#define BGT(DST) \
	{write32(0x41810000 | (((s16)(((DST)+1)<<2)) & 0xfffc));}

#define BGT_L(VAR) \
	{B_FROM(VAR); write32(0x41810001);}

#define BGE(DST) \
	{write32(0x40800000 | (((s16)(((DST)+1)<<2)) & 0xfffc));}

#define BGE_L(VAR) \
	{B_FROM(VAR); write32(0x40800001);}

#define BLE(DST) \
	{write32(0x40810000 | (((s16)(((DST)+1)<<2)) & 0xfffc));}

#define BLE_L(VAR) \
	{B_FROM(VAR); write32(0x40810001);}

#define BCTRL() \
	{write32(0x4E800421);}

#define BCTR() \
	{write32(0x4E800420);}


/* compare ops */
// mod1
#define CMPLWI(REG, IMM) \
	{int _reg = (REG); \
        write32(0x28000000 | (_reg << 16) | ((IMM) & 0xffff));}

#define CMPWI(REG, IMM) \
	{int _reg = (REG); \
        write32(0x2C000000 | (_reg << 16) | ((IMM) & 0xffff));}

#define CMPLW(REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); \
        write32(0x7C000040 | (_reg1 << 16) | (_reg2 << 11));}

#define CMPW(REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); \
        write32(0x7C000000 | (_reg1 << 16) | (_reg2 << 11));}

#define MTCRF(MASK, REG) \
	{int _reg = (REG); \
        write32(0x7C000120 | (_reg << 21) | (((MASK)&0xff)<<12));}

#define MFCR(REG) \
	{int _reg = (REG); \
        write32(0x7C000026 | (_reg << 21));}

#define CROR(CR_DST, CR1, CR2) \
	{write32(0x4C000382 | ((CR_DST) << 21) | ((CR1) << 16) |  ((CR2) << 11));}

#define CRXOR(CR_DST, CR1, CR2) \
	{write32(0x4C000182 | ((CR_DST) << 21) | ((CR1) << 16) |  ((CR2) << 11));}

#define CRNAND(CR_DST, CR1, CR2) \
	{write32(0x4C0001C2 | ((CR_DST) << 21) | ((CR1) << 16) |  ((CR2) << 11));}

#define CRANDC(CR_DST, CR1, CR2) \
	{write32(0x4C000102 | ((CR_DST) << 21) | ((CR1) << 16) |  ((CR2) << 11));}


/* shift ops */
#define RLWINM(REG_DST, REG_SRC, SHIFT, START, END) \
	{int _src = (REG_SRC); int _dst = (REG_DST); \
        write32(0x54000000 | (_src << 21) | (_dst << 16) | (SHIFT << 11) | (START << 6) | (END << 1));}

#define RLWINM_(REG_DST, REG_SRC, SHIFT, START, END) \
	{int _src = (REG_SRC); int _dst = (REG_DST); \
        write32(0x54000001 | (_src << 21) | (_dst << 16) | (SHIFT << 11) | (START << 6) | (END << 1));}

#define CLRRWI(REG_DST, REG_SRC, LEN) \
	RLWINM(REG_DST, REG_SRC, 0, 0, 31-LEN)

#define SLWI(REG_DST, REG_SRC, SHIFT) \
	{int _shift = (SHIFT); \
        if (_shift==0) {MR(REG_DST, REG_SRC)} else \
        {RLWINM(REG_DST, REG_SRC, _shift, 0, (31-_shift))}}

#define SRWI(REG_DST, REG_SRC, SHIFT) \
	{int _shift = (SHIFT); \
        if (_shift==0) {MR(REG_DST, REG_SRC)} else \
        RLWINM(REG_DST, REG_SRC, (32-_shift), _shift, 31)}

#define SLW(REG_DST, REG_SRC, REG_SHIFT) \
	{int _src = (REG_SRC), _shift = (REG_SHIFT); int _dst = (REG_DST); \
        write32(0x7C000030 | (_src << 21) | (_dst << 16) | (_shift << 11));}

#define SRW(REG_DST, REG_SRC, REG_SHIFT) \
	{int _src = (REG_SRC), _shift = (REG_SHIFT); int _dst = (REG_DST); \
        write32(0x7C000430 | (_src << 21) | (_dst << 16) | (_shift << 11));}

#define SRAW(REG_DST, REG_SRC, REG_SHIFT) \
	{int _src = (REG_SRC), _shift = (REG_SHIFT); int _dst = (REG_DST); \
        write32(0x7C000630 | (_src << 21) | (_dst << 16) | (_shift << 11));}

#define SRAWI(REG_DST, REG_SRC, SHIFT) \
	{int _src = (REG_SRC); int _dst = (REG_DST); int _shift = (SHIFT); \
        if (_shift==0) {MR(REG_DST, REG_SRC)} else \
        write32(0x7C000670 | (_src << 21) | (_dst << 16) | (_shift << 11));}

#define RLWNM(REG_DST, REG_SRC, REG_SHIFT, START, END) \
	{int _src = (REG_SRC), _shift = (REG_SHIFT); int _dst = (REG_DST); \
        write32(0x5C000000 | (_src << 21) | (_dst << 16) | (_shift << 11) | (START << 6) | (END << 1));}

/* other ops */
#define ORI(REG_DST, REG_SRC, IMM) \
	{int _src = (REG_SRC), _imm = (IMM); int _dst = (REG_DST); \
        if (!((_imm == 0) && ((_src^_dst) == 0))) \
        write32(0x60000000 | (_src << 21) | (_dst << 16) | (_imm & 0xffff));}

#define ORIS(REG_DST, REG_SRC, IMM) \
	{int _src = (REG_SRC), _imm = (IMM); int _dst = (REG_DST); \
        if (!((_imm == 0) && ((_src^_dst) == 0))) \
        write32(0x64000000 | (_src << 21) | (_dst << 16) | (_imm & 0xffff));}

#define OR(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000378 | (_reg1 << 21) | (_dst << 16) | (_reg2 << 11));}

#define OR_(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000379 | (_reg1 << 21) | (_dst << 16) | (_reg2 << 11));}

#define XORI(REG_DST, REG_SRC, IMM) \
	{int _src = (REG_SRC); int _dst=(REG_DST); \
        write32(0x68000000 | (_src << 21) | (_dst << 16) | ((IMM) & 0xffff));}

#define XOR(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000278 | (_reg1 << 21) | (_dst << 16) | (_reg2 << 11));}

#define XOR_(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000279 | (_reg1 << 21) | (_dst << 16) | (_reg2 << 11));}

#define ANDI_(REG_DST, REG_SRC, IMM) \
	{int _src = (REG_SRC); int _dst=(REG_DST); \
        write32(0x70000000 | (_src << 21) | (_dst << 16) | ((IMM) & 0xffff));}

#define AND(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C000038 | (_reg1 << 21) | (_dst << 16) | (_reg2 << 11));}

#define NOR(REG_DST, REG1, REG2) \
	{int _reg1 = (REG1), _reg2 = (REG2); int _dst=(REG_DST); \
        write32(0x7C0000f8 | (_reg1 << 21) | (_dst << 16) | (_reg2 << 11));}

#define NEG(REG_DST, REG_SRC) \
	{int _src = (REG_SRC); int _dst=(REG_DST); \
        write32(0x7C0000D0 | (_dst << 21) | (_src << 16));}

#define NOP() \
	{write320x60000000;}

#define MCRXR(CR_DST) \
	{write32(0x7C000400 | (CR_DST << 23));}

#define EXTSB(REG_DST, REG_SRC) \
	{int _src = (REG_SRC); int _dst=(REG_DST); \
        write32(0x7C000774 | (_src << 21) | (_dst << 16));}

#define EXTSH(REG_DST, REG_SRC) \
	{int _src = (REG_SRC); int _dst=(REG_DST); \
        write32(0x7C000734 | (_src << 21) | (_dst << 16));}


/* floating point ops */
#define FDIVS(FPR_DST, FPR1, FPR2) \
	{write32(0xEC000024 | (FPR_DST << 21) | (FPR1 << 16) | (FPR2 << 11));}

#define FDIV(FPR_DST, FPR1, FPR2) \
	{write32(0xFC000024 | (FPR_DST << 21) | (FPR1 << 16) | (FPR2 << 11));}

#define FMULS(FPR_DST, FPR1, FPR2) \
	{write32(0xEC000032 | (FPR_DST << 21) | (FPR1 << 16) | (FPR2 << 11));}

#define FMUL(FPR_DST, FPR1, FPR2) \
	{write32(0xFC000032 | (FPR_DST << 21) | (FPR1 << 16) | (FPR2 << 11));}

#define FADDS(FPR_DST, FPR1, FPR2) \
	{write32(0xEC00002A | (FPR_DST << 21) | (FPR1 << 16) | (FPR2 << 11));}

#define FADD(FPR_DST, FPR1, FPR2) \
	{write32(0xFC00002A | (FPR_DST << 21) | (FPR1 << 16) | (FPR2 << 11));}

#define FRSP(FPR_DST, FPR_SRC) \
	{write32(0xFC000018 | (FPR_DST << 21) | (FPR_SRC << 11));}

#define FCTIW(FPR_DST, FPR_SRC) \
	{write32(0xFC00001C | (FPR_DST << 21) | (FPR_SRC << 11));}


#define LFS(FPR_DST, OFFSET, REG) \
	{write32(0xC0000000 | (FPR_DST << 21) | (REG << 16) | ((OFFSET) & 0xffff));}

#define STFS(FPR_DST, OFFSET, REG) \
	{write32(0xD0000000 | (FPR_DST << 21) | (REG << 16) | ((OFFSET) & 0xffff));}

#define LFD(FPR_DST, OFFSET, REG) \
	{write32(0xC8000000 | (FPR_DST << 21) | (REG << 16) | ((OFFSET) & 0xffff));}

#define STFD(FPR_DST, OFFSET, REG) \
	{write32(0xD8000000 | (FPR_DST << 21) | (REG << 16) | ((OFFSET) & 0xffff));}



/* extra combined opcodes */
#if 1
#define LIW(REG, IMM) /* Load Immidiate Word */ \
{ \
	int __reg = (REG); u32 __imm = (u32)(IMM); \
	if ((s32)__imm == (s32)((s16)__imm)) \
	{ \
		LI(__reg, (s32)((s16)__imm)); \
	} else if (__reg == 0) { \
		LIS(__reg, (((u32)__imm)>>16)); \
		if ((((u32)__imm) & 0xffff) != 0) \
		{ \
			ORI(__reg, __reg, __imm); \
		} \
	} else { \
		if ((((u32)__imm) & 0xffff) == 0) { \
			LIS(__reg, (((u32)__imm)>>16)); \
		} else { \
			LI(__reg, __imm); \
			if ((__imm & 0x8000) == 0) { \
				ADDIS(__reg, __reg, ((u32)__imm)>>16); \
			} else { \
				ADDIS(__reg, __reg, ((((u32)__imm)>>16) & 0xffff) + 1); \
			} \
		} \
		/*if ((((u32)__imm) & 0xffff) != 0) \
		{ \
			ORI(__reg, __reg, __imm); \
		}*/ \
	} \
}
#else
#define LIW(REG, IMM) /* Load Immidiate Word */ \
{ \
        int __reg = (REG); u32 __imm = (u32)(IMM); \
	if ((s32)__imm == (s32)((s16)__imm)) \
	{ \
		LI(__reg, (s32)((s16)__imm)); \
	} \
	else \
	{ \
		LIS(__reg, (((u32)__imm)>>16)); \
		if ((((u32)__imm) & 0xffff) != 0) \
		{ \
			ORI(__reg, __reg, __imm); \
		} \
	} \
}
#endif

#endif