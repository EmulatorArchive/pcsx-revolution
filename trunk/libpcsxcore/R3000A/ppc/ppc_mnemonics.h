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

#ifndef _PPC_MNEMONICS_H_
#define _PPC_MNEMONICS_H_

#include "ppc.h"

#define OFFSET(X,Y) ((u32)(Y)-(u32)(X))

//   0 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
//  |      OPCD      |                                   LI                                  |AA|LK|
#define I_FORM(OPCD, LI, AA, LK) {\
	Write32((OPCD << 26) | (((LI) & 0xffffff) << 2) | (AA << 1) | (LK)); \
}

//   0 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
//  |      OPCD      |      BO      |      BI      |                   BD                    |AA|LK|
#define B_FORM(OPCD, BO, BI, BD, AA, LK) {\
	Write32((OPCD << 26) | (BO << 21) | (BI << 16) | (((BD) & 0x3fff) << 2) | (AA << 1) | (LK)); \
}

//   0 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
//  |      OPCD      |       D      |       A      |                 d/UIMM/SIMM                   |
#define D_FORM(OPCD, RD, RA, IMM) { \
	int _ra = (RA), _rd = (RD); \
	Write32((OPCD << 26) | (_rd << 21) | (_ra << 16) | ((IMM) & 0xffff)); \
}

#define D_FORM_(OPCD, RD, RA, IMM) { \
	int _rd = (RD), _ra = (RA); \
	Write32((OPCD << 26) | (_rd << 21) | (_ra << 16) | ((IMM) & 0xffff)); \
}

//   0 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
//  |      OPCD      |  crfD  |0 |L |       A      |                 d/UIMM/SIMM                   |
#define D_FORM2(OPCD, crfD, RA, IMM) { \
	int _ra = (RA); \
	Write32((OPCD << 26) | (crfD << 23) | (_ra << 16) | ((IMM) & 0xffff)); \
}

//   0 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
//  |      OPCD      |       S      |       A      |      SH      |      MB      |      ME      |Rc|
#define M_FORM(OPCD, RS, RA, SH, MB, ME, Rc) { \
	int rs = (RS), ra = (RA), sh = (SH); \
	Write32((OPCD << 26) | (rs << 21) | (ra << 16) | (sh << 11) | ((MB) << 6) | ((ME) << 1) | (Rc)); \
}

//   0 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
//  |      OPCD      |       D      |       A      |       B      |              XO             |Rc|
#define X_FORM(OPCD, D, A, B, XO, Rc) { \
	int a = (A), b = (B), d = (D); \
	Write32((OPCD << 26) | (d << 21) | (a << 16) | (b << 11) | (((XO) & 0x3ff) << 1) | (Rc)); \
}

#define X_FORM_(OPCD, D, A, B, XO, Rc) { \
	int b = (B), d = (D), a = (A); \
	Write32((OPCD << 26) | (d << 21) | (a << 16) | (b << 11) | (((XO) & 0x3ff) << 1) | (Rc)); \
}

//   0 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
//  |      OPCD      |  crfD  |0 |L |       A      |       B      |              XO             |Rc|
#define X_FORM2(OPCD, D, A, B, XO, Rc) { \
	int a = (A), b = (B), d = (D); \
	Write32((OPCD << 26) | (d << 23) | (a << 16) | (b << 11) | (((XO) & 0x3ff) << 1) | (Rc)); \
}

//   0 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
//  |      OPCD      |       D      |       A      |       B      |OE|            XO            |Rc|
#define XO_FORM(OPCD, D, A, B, OE, XO, Rc) { \
	int a = (A), b = (B), d = (D); \
	Write32((OPCD << 26) | (d << 21) | (a << 16) | (b << 11) | (OE << 10) | (((XO) & 0x1ff) << 1) | (Rc)); \
}

//   0 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
//  |      OPCD      |    BO/crbD   |    BI/crbA   |     crbB     |              XO             |LK|
#define XL_FORM(OPCD, crbD, crbA, crbB, XO, LK) { \
	X_FORM(OPCD, crbD, crbA, crbB, XO, LK); \
}

//   0 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
//  |      OPCD      |       D      |              SPR            |              XO             | 0|
#define XFX_FORM(OPCD, D, SPR, XO) { \
	int d = (D); \
	Write32((OPCD << 26) | (d << 21) | (((SPR) & 0x3ff) << 11) | (((XO) & 0x3ff) << 1)); \
}

//   0 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
//  |      OPCD      |       D      | 0|           CRM         | 0|              XO             | 0|
#define XFX_FORM2(OPCD, D, CRM, XO) { \
	int d = (d); \
	Write32((OPCD << 26) | (d << 21) | (((CRM) & 0xff) << 12) | (((XO) & 0x3ff) << 1)); \
}

//   0 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
//  |      OPCD      |       D      |       A      |      B       |       C      |      XO      |Rc|
#define A_FORM(OPCD, D, A, B, C, XO, Rc) { \
	int a = (A), b = (B), c = (C), d = (D); \
	Write32((OPCD << 26) | (d << 21) | (a << 16) | (b << 11) | (c << 6) | (XO << 1) | (Rc)); \
}

// Less than
#define LT 0
// Greater than
#define GT 1
// Equal
#define EQ 2
// Summary overflow
#define SO 3

// Greater than or equal
#define GE 0
// Less than or equal
#define LE 1
// Not equal
#define NE 2
// Not summary overflow
#define NS 3

// I-Form

#define B(LI) \
	I_FORM(18, LI, 0, 0);

#define BA(LI) \
	I_FORM(18, LI, 1, 0);

#define BL(LI) \
	I_FORM(18, LI, 0, 1);

#define BLA(LI) \
	I_FORM(18, LI, 1, 1);

// B-Form

#define BC(BO, BI, BD) \
	B_FORM(16, BO, BI, BD, 0, 0);

#define BCA(BO, BI, BD) \
	B_FORM(16, BO, BI, BD, 1, 0);

#define BCL(BO, BI, BD) \
	B_FORM(16, BO, BI, BD, 0, 1);

#define BCLA(BO, BI, BD) \
	B_FORM(16, BO, BI, BD, 1, 1);

// D-Form

#define ADDI(REG_DST, REG_SRC, IMM) \
	D_FORM(14, REG_DST, REG_SRC, IMM);

#define ADDIC(REG_DST, REG_SRC, IMM) \
	D_FORM(12, REG_DST, REG_SRC, IMM);

#define ADDIC_(REG_DST, REG_SRC, IMM) \
	D_FORM(13, REG_DST, REG_SRC, IMM);

#define ADDIS(REG_DST, REG_SRC, IMM) \
	D_FORM(15, REG_DST, REG_SRC, IMM);

#define ANDI_(REG_DST, REG_SRC, IMM) \
	D_FORM_(28, REG_SRC, REG_DST, IMM);

#define ANDIS_(REG_DST, REG_SRC, IMM) \
	D_FORM_(29, REG_SRC, REG_DST, IMM);

#define LBZ(REG_DST, IMM, REG_SRC) \
	D_FORM(34, REG_DST, REG_SRC, IMM);

#define LBZU(REG_DST, IMM, REG_SRC) \
	D_FORM(35, REG_DST, REG_SRC, IMM);

#define LFD(REG_DST, IMM, REG_SRC) \
	D_FORM(50, REG_DST, REG_SRC, IMM);

#define LFDU(REG_DST, IMM, REG_SRC) \
	D_FORM(51, REG_DST, REG_SRC, IMM);

#define LFS(REG_DST, IMM, REG_SRC) \
	D_FORM(48, REG_DST, REG_SRC, IMM);

#define LFSU(REG_DST, IMM, REG_SRC) \
	D_FORM(49, REG_DST, REG_SRC, IMM);

#define LHA(REG_DST, IMM, REG_SRC) \
	D_FORM(42, REG_DST, REG_SRC, IMM);

#define LHAU(REG_DST, IMM, REG_SRC) \
	D_FORM(43, REG_DST, REG_SRC, IMM);

#define LHZ(REG_DST, IMM, REG_SRC) \
	D_FORM(40, REG_DST, REG_SRC, IMM);

#define LHZU(REG_DST, IMM, REG_SRC) \
	D_FORM(41, REG_DST, REG_SRC, IMM);

#define LMW(REG_DST, IMM, REG_SRC) \
	D_FORM(46, REG_DST, REG_SRC, IMM);

#define LWZ(REG_DST, IMM, REG_SRC) \
	D_FORM(32, REG_DST, REG_SRC, IMM);

#define LWZU(REG_DST, IMM, REG_SRC) \
	D_FORM(33, REG_DST, REG_SRC, IMM);

#define MULLI(REG_DST, REG_SRC, IMM) \
	D_FORM( 7, REG_DST, REG_SRC, IMM);

#define ORI(REG_DST, REG_SRC, IMM) \
	D_FORM_(24, REG_SRC, REG_DST, IMM);

#define ORIS(REG_DST, REG_SRC, IMM) \
	D_FORM_(25, REG_SRC, REG_DST, IMM);

#define STB(REG_DST, IMM, REG_SRC) \
	D_FORM(38, REG_DST, REG_SRC, IMM);

#define STBU(REG_DST, IMM, REG_SRC) \
	D_FORM(39, REG_DST, REG_SRC, IMM);

#define STFD(REG_DST, IMM, REG_SRC) \
	D_FORM(54, REG_DST, REG_SRC, IMM);

#define STFDU(REG_DST, IMM, REG_SRC) \
	D_FORM(55, REG_DST, REG_SRC, IMM);

#define STFS(REG_DST, IMM, REG_SRC) \
	D_FORM(52, REG_DST, REG_SRC, IMM);

#define STFSU(REG_DST, IMM, REG_SRC) \
	D_FORM(53, REG_DST, REG_SRC, IMM);

#define STH(REG_DST, IMM, REG_SRC) \
	D_FORM(44, REG_DST, REG_SRC, IMM);

#define STHU(REG_DST, IMM, REG_SRC) \
	D_FORM(45, REG_DST, REG_SRC, IMM);

#define STMW(REG_DST, IMM, REG_SRC) \
	D_FORM(47, REG_DST, REG_SRC, IMM);

#define STW(REG_DST, IMM, REG_SRC) \
	D_FORM(36, REG_DST, REG_SRC, IMM);

#define STWU(REG_DST, IMM, REG_SRC) \
	D_FORM(37, REG_DST, REG_SRC, IMM);

#define SUBFIC(REG_DST, REG_SRC, IMM) \
	D_FORM( 8, REG_DST, REG_SRC, IMM);

#define TWI(REG_DST, REG_SRC, IMM) \
	D_FORM( 3, REG_DST, REG_SRC, IMM);

#define XORI(REG_DST, REG_SRC, IMM) \
	D_FORM_(26, REG_SRC, REG_DST, IMM);

#define XORIS(REG_DST, REG_SRC, IMM) \
	D_FORM_(27, REG_SRC, REG_DST, IMM);

// D-Form2

#define CMPI(BF, REG_SRC, IMM) \
	D_FORM2(11, BF, REG_SRC, IMM);

#define CMPLI(BF, REG_SRC, IMM) \
	D_FORM2(10, BF, REG_SRC, IMM);

// X-Form

#define AND(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 28, 0);

#define AND_(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 28, 1);

#define ANDC(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 60, 0);

#define ANDC_(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 60, 1);

#define CNTLZW(REG_DST, REG1) \
	X_FORM_(31, REG1, REG_DST, 0, 26, 0);

#define CNTLZW_(REG_DST, REG1) \
	X_FORM_(31, REG1, REG_DST, 0, 26, 1);

#define DCBF(REG1, REG2) \
	X_FORM(31, 0, REG1, REG2, 86, 0);

#define DCBI(REG1, REG2) \
	X_FORM(31, 0, REG1, REG2, 470, 0);

#define DCBST(REG1, REG2) \
	X_FORM(31, 0, REG1, REG2, 54, 0);

#define DCBT(REG1, REG2) \
	X_FORM(31, 0, REG1, REG2, 278, 0);

#define DCBTST(REG1, REG2) \
	X_FORM(31, 0, REG1, REG2, 246, 0);

#define DCBZ(REG1, REG2) \
	X_FORM(31, 0, REG1, REG2, 1014, 0);

#define EQV(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 284, 0);

#define EQV_(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 284, 1);

#define EXTSB(REG_DST, REG1) \
	X_FORM_(31, REG1, REG_DST, 0, 954, 0);

#define EXTSB_(REG_DST, REG1) \
	X_FORM_(31, REG1, REG_DST, 0, 954, 1);

#define EXTSH(REG_DST, REG1) \
	X_FORM_(31, REG1, REG_DST, 0, 922, 0);

#define EXTSH_(REG_DST, REG1) \
	X_FORM_(31, REG1, REG_DST, 0, 922, 1);

#define FABS(REG_DST, REG2) \
	X_FORM(63, REG_DST, 0, REG2, 264, 0);

#define FABS_(REG_DST, REG2) \
	X_FORM(63, REG_DST, 0, REG2, 264, 1);

#define FCTIW(REG_DST, REG2) \
	X_FORM(63, REG_DST, 0, REG2, 14, 0);

#define FCTIW_(REG_DST, REG2) \
	X_FORM(63, REG_DST, 0, REG2, 14, 1);

#define FCTIWZ(REG_DST, REG2) \
	X_FORM(63, REG_DST, 0, REG2, 15, 0);

#define FCTIWZ_(REG_DST, REG2) \
	X_FORM(63, REG_DST, 0, REG2, 15, 1);

#define FMR(REG_DST, REG2) \
	X_FORM(63, REG_DST, 0, REG2, 72, 0);

#define FMR_(REG_DST, REG2) \
	X_FORM(63, REG_DST, 0, REG2, 72, 1);

#define FNABS(REG_DST, REG2) \
	X_FORM(63, REG_DST, 0, REG2, 136, 0);

#define FNABS_(REG_DST, REG2) \
	X_FORM(63, REG_DST, 0, REG2, 136, 1);

#define FNEG(REG_DST, REG2) \
	X_FORM(63, REG_DST, 0, REG2, 40, 0);

#define FNEG_(REG_DST, REG2) \
	X_FORM(63, REG_DST, 0, REG2, 40, 1);

#define FRSP(REG_DST, REG2) \
	X_FORM(63, REG_DST, 0, REG2, 12, 0);

#define FRSP_(REG_DST, REG2) \
	X_FORM(63, REG_DST, 0, REG2, 12, 1);

#define ICBI(REG1, REG2) \
	X_FORM(31, 0, REG1, REG2, 982, 0);

#define LBZUX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 119, 0);

#define LBZX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 87, 0);

#define LFDUX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 631, 0);

#define LFDX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 599, 0);

#define LFSUX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 567, 0);

#define LFSX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 535, 0);

#define LHAUX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 375, 0);

#define LHAX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 343, 0);

#define LHBRX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 790, 0);

#define LHZUX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 311, 0);

#define LHZX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 279, 0);

#define LSWI(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 597, 0);

#define LSWX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 533, 0);

#define LWARX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 20, 0);

#define LWBRX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 534, 0);

#define LWZUX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 55, 0);

#define LWZX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 23, 0);

#define MFCR(REG_DST) \
	X_FORM(31, REG_DST, 0, 0, 19, 0);

#define MFFS(REG_DST) \
	X_FORM(63, REG_DST, 0, 0, 583, 0);

#define MFFS_(REG_DST) \
	X_FORM(63, REG_DST, 0, 0, 583, 1);

#define MFMSR(REG_DST) \
	X_FORM(31, REG_DST, 0, 0, 83, 0);

#define MTFSB0(REG_DST) \
	X_FORM(63, REG_DST, 0, 0, 70, 0);

#define MTFSB0_(REG_DST) \
	X_FORM(63, REG_DST, 0, 0, 70, 1);

#define MTFSB1(REG_DST) \
	X_FORM(63, REG_DST, 0, 0, 38, 0);

#define MTFSB1_(REG_DST) \
	X_FORM(63, REG_DST, 0, 0, 38, 1);

#define NAND(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 476, 0);

#define NAND_(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 476, 1);

#define NOR(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 124, 0);

#define NOR_(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 124, 1);

#define OR(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 444, 0);

#define OR_(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 444, 1);

#define ORC(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 412, 0);

#define ORC_(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 412, 1);

#define SLW(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 24, 0);

#define SLW_(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 24, 1);

#define SRAW(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 792, 0);

#define SRAW_(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 792, 1);

#define SRAWI(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 824, 0);

#define SRAWI_(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 824, 1);

#define SRW(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 536, 0);

#define SRW_(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 536, 1);

#define STBUX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 247, 0);

#define STBX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 215, 0);

#define STFDUX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 759, 0);

#define SRFDX(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 727, 0);

#define STFIWX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 983, 0);

#define STFSUX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 695, 0);

#define STFSX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 663, 0);

#define STHBRX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 918, 0);

#define STHUX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 439, 0);

#define STHX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 407, 0);

#define STSWI(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 725, 0);

#define STSWX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 661, 0);

#define STWBRX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 662, 0);

#define STWCX_(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 150, 1);

#define STWUX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 183, 0);

#define STWX(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 151, 0);

#define SYNC() \
	X_FORM(31, 0, 0, 0, 598, 0);

#define TW(REG_DST, REG1, REG2) \
	X_FORM(31, REG_DST, REG1, REG2, 4, 0);

#define XOR(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 316, 0);

#define XOR_(REG_DST, REG1, REG2) \
	X_FORM_(31, REG1, REG_DST, REG2, 316, 1);

// X-Form2

#define CMP(BF, REG1, REG2) \
	X_FORM2(31, BF, REG1, REG2, 0, 0);

#define CMPL(BF, REG1, REG2) \
	X_FORM2(31, BF, REG1, REG2, 32, 0);

#define FCMPO(BF, REG1, REG2) \
	X_FORM2(63, BF, REG1, REG2, 32, 0);

#define FCMPU(BF, REG1, REG2) \
	X_FORM2(63, BF, REG1, REG2, 0, 0);

#define MCRXR(REG_DST) \
	X_FORM2(31, REG_DST, 0, 0, 512, 0);

// XFX-Form

#define MFSPR(REG_DST, SPR) \
	XFX_FORM(31, REG_DST, SPR, 339);

#define MTSPR(SPR, REG_SRC) \
	XFX_FORM(31, REG_SRC, SPR, 467);

// XFX-Form2

#define MTCRF(REG_DST, CRM) \
	XFX_FORM2(31, REG_DST, CRM, 144);

// XO-Form

#define ADD(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 266, 0);

#define ADD_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 266, 1);

#define ADDO(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 266, 0);

#define ADDO_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 266, 1);

#define ADDC(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 10, 0);

#define ADDC_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 10, 1);

#define ADDCO(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 10, 0);

#define ADDCO_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 10, 1);

#define ADDE(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 138, 0);

#define ADDE_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 138, 1);

#define ADDEO(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 138, 0);

#define ADDEO_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 138, 1);

#define ADDME(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 0, 234, 0);

#define ADDME_(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 0, 234, 1);

#define ADDMEO(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 1, 234, 0);

#define ADDMEO_(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 1, 234, 1);

#define ADDZE(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 0, 202, 0);

#define ADDZE_(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 0, 202, 1);

#define ADDZEO(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 1, 202, 0);

#define ADDZEO_(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 1, 202, 1);

#define DIVW(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 491, 0);

#define DIVW_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 491, 1);

#define DIVWO(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 491, 0);

#define DIVWO_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 491, 1);

#define DIVWU(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 459, 0);

#define DIVWU_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 459, 1);

#define DIVWUO(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 459, 0);

#define DIVWUO_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 459, 1);

#define MULHW(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 75, 0);

#define MULHW_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 75, 1);

#define MULHWU(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 11, 0);

#define MULHWU_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 11, 1);

#define MULLW(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 235, 0);

#define MULLW_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 235, 1);

#define MULLWO(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 235, 0);

#define MULLWO_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 235, 1);

#define NEG(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 0, 104, 0);

#define NEG_(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 0, 104, 1);

#define NEGO(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 1, 104, 0);

#define NEGO_(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 1, 104, 1);

#define SUBF(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 40, 0);

#define SUBF_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 40, 1);

#define SUBFO(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 40, 0);

#define SUBFO_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 40, 1);

#define SUBFC(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 8, 0);

#define SUBFC_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 8, 1);

#define SUBFCO(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 8, 0);

#define SUBFCO_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 8, 1);

#define SUBFE(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 136, 0);

#define SUBFE_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 0, 136, 1);

#define SUBFEO(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 136, 0);

#define SUBFEO_(REG_DST, REG1, REG2) \
	XO_FORM(31, REG_DST, REG1, REG2, 1, 136, 1);

#define SUBFME(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 0, 232, 0);

#define SUBFME_(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 0, 232, 1);

#define SUBFMEO(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 1, 232, 0);

#define SUBFMEO_(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 1, 232, 1);

#define SUBFZE(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 0, 200, 0);

#define SUBFZE_(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 0, 200, 1);

#define SUBFZEO(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 1, 200, 0);

#define SUBFZEO_(REG_DST, REG1) \
	XO_FORM(31, REG_DST, REG1, 0, 1, 200, 1);

// XL-Form

#define BCCTR(BO, BI) \
	XL_FORM(19, BO, BI, 0, 528, 0);

#define BCCTRL(BO, BI) \
	XL_FORM(19, BO, BI, 0, 528, 1);

#define BCLR(BO, BI) \
	XL_FORM(19, BO, BI, 0, 16, 0);

#define BCLRL(BO, BI) \
	XL_FORM(19, BO, BI, 0, 16, 1);

#define CRAND(REG_DST, REG1, REG2) \
	XL_FORM(19, REG_DST, REG1, REG2, 257, 0);

#define CRANDC(REG_DST, REG1, REG2) \
	XL_FORM(19, REG_DST, REG1, REG2, 129, 0);

#define CREQV(REG_DST, REG1, REG2) \
	XL_FORM(19, REG_DST, REG1, REG2, 289, 0);

#define CRNAND(REG_DST, REG1, REG2) \
	XL_FORM(19, REG_DST, REG1, REG2, 225, 0);

#define CRNOR(REG_DST, REG1, REG2) \
	XL_FORM(19, REG_DST, REG1, REG2, 33, 0);

#define CROR(REG_DST, REG1, REG2) \
	XL_FORM(19, REG_DST, REG1, REG2, 449, 0);

#define CRORC(REG_DST, REG1, REG2) \
	XL_FORM(19, REG_DST, REG1, REG2, 417, 0);

#define CRXOR(REG_DST, REG1, REG2) \
	XL_FORM(19, REG_DST, REG1, REG2, 193, 0);

#define ISYNC() \
	XL_FORM(19, 0, 0, 0, 150, 0);

// A-Form

#define FADD(REG_DST, REG1, REG2) \
	A_FORM(63, REG_DST, REG1, REG2, 0, 21, 0);

#define FADD_(REG_DST, REG1, REG2) \
	A_FORM(63, REG_DST, REG1, REG2, 0, 21, 1);

#define FADDS(REG_DST, REG1, REG2) \
	A_FORM(59, REG_DST, REG1, REG2, 0, 21, 0);

#define FADDS_(REG_DST, REG1, REG2) \
	A_FORM(59, REG_DST, REG1, REG2, 0, 21, 1);

#define FDIV(REG_DST, REG1, REG2) \
	A_FORM(63, REG_DST, REG1, REG2, 0, 18, 0);

#define FDIV_(REG_DST, REG1, REG2) \
	A_FORM(63, REG_DST, REG1, REG2, 0, 18, 1);

#define FDIVS(REG_DST, REG1, REG2) \
	A_FORM(59, REG_DST, REG1, REG2, 0, 18, 0);

#define FDIVS_(REG_DST, REG1, REG2) \
	A_FORM(59, REG_DST, REG1, REG2, 0, 18, 1);

#define FMADD(REG_DST, REG1, REG2, REG3) \
	A_FORM(63, REG_DST, REG1, REG2, REG3, 29, 0);

#define FMADD_(REG_DST, REG1, REG2, REG3) \
	A_FORM(63, REG_DST, REG1, REG2, REG3, 29, 1);

#define FMADDS(REG_DST, REG1, REG2, REG3) \
	A_FORM(59, REG_DST, REG1, REG2, REG3, 29, 0);

#define FMADDS_(REG_DST, REG1, REG2, REG3) \
	A_FORM(59, REG_DST, REG1, REG2, REG3, 29, 1);

#define FMSUB(REG_DST, REG1, REG2, REG3) \
	A_FORM(63, REG_DST, REG1, REG2, REG3, 28, 0);

#define FMSUB_(REG_DST, REG1, REG2, REG3) \
	A_FORM(63, REG_DST, REG1, REG2, REG3, 28, 1);

#define FMSUBS(REG_DST, REG1, REG2, REG3) \
	A_FORM(59, REG_DST, REG1, REG2, REG3, 28, 0);

#define FMSUBS_(REG_DST, REG1, REG2, REG3) \
	A_FORM(59, REG_DST, REG1, REG2, REG3, 28, 1);

#define FMUL(REG_DST, REG1, REG3) \
	A_FORM(63, REG_DST, REG1, 0, REG3, 25, 0);

#define FMUL_(REG_DST, REG1, REG3) \
	A_FORM(63, REG_DST, REG1, 0, REG3, 25, 1);

#define FMULS(REG_DST, REG1, REG3) \
	A_FORM(59, REG_DST, REG1, 0, REG3, 25, 0);

#define FMULS_(REG_DST, REG1, REG3) \
	A_FORM(59, REG_DST, REG1, 0, REG3, 25, 1);

#define FNMADD(REG_DST, REG1, REG2, REG3) \
	A_FORM(63, REG_DST, REG1, REG2, REG3, 31, 0);

#define FNMADD_(REG_DST, REG1, REG2, REG3) \
	A_FORM(63, REG_DST, REG1, REG2, REG3, 31, 1);

#define FNMADDS(REG_DST, REG1, REG2, REG3) \
	A_FORM(59, REG_DST, REG1, REG2, REG3, 31, 0);

#define FNMADDS_(REG_DST, REG1, REG2, REG3) \
	A_FORM(59, REG_DST, REG1, REG2, REG3, 31, 1);

#define FNMSUB(REG_DST, REG1, REG2, REG3) \
	A_FORM(63, REG_DST, REG1, REG2, REG3, 30, 0);

#define FNMSUB_(REG_DST, REG1, REG2, REG3) \
	A_FORM(63, REG_DST, REG1, REG2, REG3, 30, 1);

#define FNMSUBS(REG_DST, REG1, REG2, REG3) \
	A_FORM(59, REG_DST, REG1, REG2, REG3, 30, 0);

#define FNMSUBS_(REG_DST, REG1, REG2, REG3) \
	A_FORM(59, REG_DST, REG1, REG2, REG3, 30, 1);

#define FRES(REG_DST, REG2) \
	A_FORM(59, REG_DST, 0, REG2, 0, 24, 0);

#define FRES_(REG_DST, REG2) \
	A_FORM(59, REG_DST, 0, REG2, 0, 24, 1);

#define FRSQRTE(REG_DST, REG2) \
	A_FORM(63, REG_DST, 0, REG2, 0, 26, 0);

#define FRSQRTE_(REG_DST, REG2) \
	A_FORM(63, REG_DST, 0, REG2, 0, 26, 1);

#define FSEL(REG_DST, REG1, REG2, REG3) \
	A_FORM(63, REG_DST, REG1, REG2, REG3, 23, 0);

#define FSEL_(REG_DST, REG1, REG2, REG3) \
	A_FORM(63, REG_DST, REG1, REG2, REG3, 23, 1);

#define FSUB(REG_DST, REG1, REG2) \
	A_FORM(63, REG_DST, REG1, REG2, 0, 20, 0);

#define FSUB_(REG_DST, REG1, REG2) \
	A_FORM(63, REG_DST, REG1, REG2, 0, 20, 1);

#define FSUBS(REG_DST, REG1, REG2) \
	A_FORM(59, REG_DST, REG1, REG2, 0, 20, 0);

#define FSUBS_(REG_DST, REG1, REG2) \
	A_FORM(59, REG_DST, REG1, REG2, 0, 20, 1);

// M-Form

#define RLWIMI(REG_DST, REG_SRC, SHIFT, START, END) \
	M_FORM(20, REG_SRC, REG_DST, SHIFT, START, END, 0);

#define RLWIMI_(REG_DST, REG_SRC, SHIFT, START, END) \
	M_FORM(20, REG_SRC, REG_DST, SHIFT, START, END, 1);

#define RLWINM(REG_DST, REG_SRC, SHIFT, START, END) \
	M_FORM(21, REG_SRC, REG_DST, SHIFT, START, END, 0);

#define RLWINM_(REG_DST, REG_SRC, SHIFT, START, END) \
	M_FORM(21, REG_SRC, REG_DST, SHIFT, START, END, 1);

#define RLWNM(REG_DST, REG_SRC, SHIFT, START, END) \
	M_FORM(23, REG_SRC, REG_DST, SHIFT, START, END, 0);

#define RLWNM_(REG_DST, REG_SRC, SHIFT, START, END) \
	M_FORM(23, REG_SRC, REG_DST, SHIFT, START, END, 1);
	
	
/* Extemded mnemonics */

#define LI(REG_DST, IMM) \
	ADDI(REG_DST, 0, IMM);

#define LIS(REG_DST, IMM) \
	ADDIS(REG_DST, 0, IMM);

#define MTLR(REG_SRC) \
	MTSPR((8 << 5), REG_SRC);

#define MTCTR(REG_SRC) \
	MTSPR((9 << 5), REG_SRC);

#define MFLR(REG_DST) \
	MFSPR(REG_DST, (8 << 5));

#define SUBI(REG_DST, REG1, IMM) \
	{ADDI((REG_DST), (REG1), -(IMM))}

#define SUBIC(REG_DST, REG1, IMM) \
	{ADDIC((REG_DST), (REG1), -(IMM))}

#define SUB(REG_DST, REG1, REG2) \
	{SUBF(REG_DST, REG2, REG1)}

#define SUBC(REG_DST, REG1, REG2) \
	{SUBFC(REG_DST, REG2, REG1)}

#define MR(REG_DST, REG_SRC) \
	{int __src = (REG_SRC); int __dst=(REG_DST); \
        OR(__dst, __src, __src);}

#define CMPLWI(REG, IMM) \
	CMPLI(0, REG, IMM);

#define CMPWI(REG, IMM) \
	CMPI(0, REG, IMM);

#define CMPLW(REG1, REG2) \
	CMPL(0, REG1, REG2);

#define CMPW(REG1, REG2) \
	CMP(0, REG1, REG2);
        
/* shift ops */

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

/* other ops */

#define NOT(REG_DST, REG1) \
	NOR(REG_DST, REG1, REG1);

#define NOP() \
	ORI(0, 0, 0);

/* branch extended mnemonics */

#define BCTR() \
	BCCTR(20, 0);

#define BCTRL() \
	BCCTRL(20, 0);

#define BLR() \
	BCLR(20, 0);

#define B_TRUE 12
#define B_FALSE 4

// Less than
#define LT 0
// Greater than
#define GT 1
// Equal
#define EQ 2
// Summary overflow
#define SO 3

// Greater than or equal
#define GE 0
// Less than or equal
#define LE 1
// Not equal
#define NE 2
// Not summary overflow
#define NS 3

// true
#define BT(BI, DST) \
	BC(B_TRUE, BI, (DST)+1);

// false
#define BF(BI, DST) \
	BC(B_FALSE, BI, (DST)+1);

#define BLT(DST) \
	BT(LT, DST);

#define BGT(DST) \
	BT(GT, DST);

#define BEQ(DST) \
	BT(EQ, DST);

#define BLE(DST) \
	BF(LE, DST);

#define BGE(DST) \
	BF(GE, DST);

#define BNE(DST) \
	BF(NE, DST);

// Not Less than = Greater than or equal
#define BNL BGE
// Not greater than = Less than or equal
#define BNG BLE

/* end of branch extended mnemonics list */

/* JIT helpers */

// Load PsxReg to HWReg
#define LWPRtoR(REG, PSXREG) { \
	LWZ((REG), OFFSET(&psxRegs, (PSXREG)), r31); \
}

#define LHPRtoR(REG, PSXREG) { \
	LHZ((REG), OFFSET(&psxRegs, (PSXREG)), r31); \
}

#define LBPRtoR(REG, PSXREG) { \
	LBZ((REG), OFFSET(&psxRegs, (PSXREG)), r31); \
}

// Store HWReg to PsxReg
#define STWRtoPR(PSXREG, REG) { \
	STW((REG), OFFSET(&psxRegs, (PSXREG)), r31); \
}

#define STHRtoPR(PSXREG, REG) { \
	STH((REG), OFFSET(&psxRegs, (PSXREG)), r31); \
}

#define STBRtoPR(PSXREG, REG) { \
	STB((REG), OFFSET(&psxRegs, (PSXREG)), r31); \
}

// Load PsxMem to HWReg
#define LWMtoR(REG, MEM) { \
	LIW((REG), (MEM)); \
	LWBRX((REG), 0, (REG)); \
}

// Store HWReg to PsxMem
#define STWRtoM(MEM, REG) { \
	LIW(r7, (MEM)); \
	STWBRX((REG), 0, r7); \
}

#define STHRtoM(MEM, REG) { \
	LIW(r7, (MEM)); \
	STHBRX((REG), 0, r7); \
}

#define STBRtoM(MEM, REG) { \
	LIW(r7, (MEM)); \
	STB((REG), 0, r7); \
}

/* extra combined opcodes */
#if 0
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
			if ((__imm & 0x8000) == 0) { \
				ADDIS(__reg, __reg, ((u32)__imm)>>16); \
			} else { \
				ADDIS(__reg, __reg, ((((u32)__imm)>>16) & 0xffff) + 1); \
			} \
			ORI(__reg, __reg, __imm); \
		} \
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