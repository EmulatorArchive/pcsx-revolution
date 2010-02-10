/*  Pcsx - Pc Psx Emulator
 *  Copyright (C) 1999-2003  Pcsx Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __PGTE_H__
#define __PGTE_H__

#ifdef GEKKO
#	define _LANGUAGE_ASSEMBLY
#	include <ogc/machine/asm.h>
#endif

#include "gte.h"
#include "ppc.h"

int psxCP2time[64] = {
        2,  16, 1,  1,  1,  1, 8,  1,  // 00
        1,  1,  1,  1,  6,  1, 1,  1,  // 08
        8,  8,  8,  19, 13, 1, 44, 1,  // 10
        1,  1,  1,  17, 11, 1, 14, 1,  // 18
        30, 1,  1,  1,  1,  1, 1,  1,  // 20
        5,  8,  17, 1,  1,  5, 6,  1,  // 28
        23, 1,  1,  1,  1,  1, 1,  1,  // 30
        1,  1,  1,  1,  1,  6, 5,  39  // 38
};

#define CP2_FUNC(f) \
static void rec##f() { \
	if (pc < cop2readypc) idlecyclecount += (cop2readypc - pc)>>2; \
	iFlushRegs(); \
	LIW(r9, (u32)psxRegs.code); \
	STWRtoPR(&psxRegs.code, r9); \
	CALLFunc ((u32)gte##f); \
	cop2readypc = pc + (psxCP2time[_fFunct_(psxRegs.code)]<<2); \
}

#define CP2_FUNCNC(f) \
static void rec##f() { \
	if (pc < cop2readypc) idlecyclecount += ((cop2readypc - pc)>>2); \
	iFlushRegs(); \
	CALLFunc ((u32)gte##f); \
/*	branch = 2; */\
	cop2readypc = pc + psxCP2time[_fFunct_(psxRegs.code)]; \
}

#if 0
CP2_FUNC(LWC2);
#else
static void recLWC2() {
// Cop2D->Rt = mem[Rs + Im] (unsigned)

if(_Rt_ != 30) { //TODO
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;

		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			LWMtoR(r3, (uptr)&psxM[addr & 0x1fffff]);
			B_L(b32Ptr[0]);
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			LWMtoR(r3, (uptr)&psxH[addr & 0xfff]);
			B_L(b32Ptr[0]);
		}
#if 1
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
					LWMtoR(r3, (uptr)&psxH[addr & 0xffff]);
					//STWRtoPR(&psxRegs.GPR.r[_Rt_], r3);
					B_L(b32Ptr[0]);

				case 0x1f801810:
					CALLFunc((uptr)GPU_readData);
					//STWRtoPR(&psxRegs.GPR.r[_Rt_], r3);
					B_L(b32Ptr[0]);

				case 0x1f801814:
					CALLFunc((uptr)GPU_readStatus);
					//STWRtoPR(&psxRegs.GPR.r[_Rt_], r3);
					B_L(b32Ptr[0]);
			}
		}
#endif
	}

	SetArg_OfB(PPCARG1);
	CALLFunc((uptr)psxMemRead32);

	if(IsConst(_Rs_)) {
		B_DST(b32Ptr[0]);
	}
}
	switch (_Rt_) {
		case 7: case 29: case 31:	// Readonly regs
			return;

		case 15:
			LWPRtoR(r9, &gteSXY1);
			LWPRtoR(r11, &gteSXY2);
			STWRtoPR(&gteSXY0, r9);
			STWRtoPR(&gteSXY1, r11);
			STWRtoPR(&gteSXY2, r3);
			STWRtoPR(&gteSXYP, r3);
			break;

		case 28:
			RLWINM(r9, r3, 7, 20, 24);	// IR1
			RLWINM(r11, r3, 2, 20, 24);	// IR2
			RLWINM(r12, r3, 29, 20, 24);	// IR3
			STWRtoPR(&gteIRGB, r3);
			STWRtoPR(&gteIR1, r9);
			STWRtoPR(&gteIR2, r11);
			STWRtoPR(&gteIR3, r12);
			break;

		case 30:
			iFlushRegs();
			LIW(r3, (u32)psxRegs.code);
			STWRtoPR(&psxRegs.code, r3);
			CALLFunc ((uptr)gteLWC2);
			break;
			
		default:
			STWRtoPR(&psxRegs.cp2d[_Rt_].d, r3);
			break;
	}
	resp += 16;
}
#endif

#if 0
CP2_FUNC(CFC2);
#else
static void recCFC2() {
// Rt = Cop2C->Rd
	if (!_Rt_) return;

	iRegs[_Rt_].state = ST_UNK;
	LWPRtoR(r3, &psxRegs.cp2c[_Rd_].d);
	STWRtoPR(&psxRegs.GPR.r[_Rt_], r3);
}
#endif

#if 0
CP2_FUNC(CTC2);
#else
static void recCTC2() {
// Cop2C->Rd = Rt
	if (IsConst(_Rt_)) {
		LIW(r3, iRegs[_Rt_].k);
	} else {
		LWPRtoR(r3, &psxRegs.GPR.r[_Rt_]);
	}
	STWRtoPR(&psxRegs.cp2c[_Rd_].d, r3);
}
#endif

#if 0
CP2_FUNC(MFC2);
#else
static void recMFC2() {
// Rt = Cop2D->Rd
	if (!_Rt_) return;

	iRegs[_Rt_].state = ST_UNK;

	switch (_Rd_) {
		case 1: case 3: case 5:
		case 8: case 9: case 10:
		case 11:
			LHPRtoR(r3, &psxRegs.cp2d[_Rd_].sw.l);
			EXTSH(r3, r3);
			STWRtoPR(&psxRegs.cp2d[_Rd_].d, r3);
			STWRtoPR(&psxRegs.GPR.r[_Rt_], r3);
			break;


		case 7: case 16: case 17:
		case 18: case 19:
			LHPRtoR(r3, &psxRegs.cp2d[_Rd_].w.l);
			STWRtoPR(&psxRegs.cp2d[_Rd_].d, r3);
			STWRtoPR(&psxRegs.GPR.r[_Rt_], r3);
			break;
			
		case 15:
			LWPRtoR(r3, (uptr)&gteSXY2);
			STWRtoPR(&psxRegs.cp2d[_Rd_].d, r3);
			STWRtoPR(&psxRegs.GPR.r[_Rt_], r3);
			break;

		case 29: 
			iFlushRegs();
			LIW(r3, (u32)psxRegs.code);
			STWRtoPR(&psxRegs.code, r3);
			CALLFunc ((uptr)gteMFC2);
			break;

		default:
			LWPRtoR(r3, &psxRegs.cp2d[_Rd_].d);
			STWRtoPR(&psxRegs.GPR.r[_Rt_], r3);
			break;
	}
}
#endif
#if 0
CP2_FUNC(MTC2);
#else
static void recMTC2() {
// Cop2D->Rd = Rt
	int fixt = 0;

//	iFlushRegs();

	switch (_Rd_) {
		case 7: case 29: case 31:	// Readonly regs
			return;

		case 15:
			if (IsConst(_Rt_)) {
				LIW(r3, iRegs[_Rt_].k);
			}
			else {
				LWPRtoR(r3, &psxRegs.GPR.r[_Rt_]);
			}
			LWPRtoR(r11, (uptr)&gteSXY1);
			LWPRtoR(r9, (uptr)&gteSXY2);
			STWRtoPR(&gteSXY0, r11);
			STWRtoPR(&gteSXY1, r9);
			STWRtoPR(&gteSXY2, r3);
			STWRtoPR(&gteSXYP, r3);
			break;

		case 28:
			if (IsConst(_Rt_)) {
				LIW(r3, iRegs[_Rt_].k);
			}
			else {
				LWPRtoR(r3, &psxRegs.GPR.r[_Rt_]);
			}
			RLWINM(r9, r3, 7, 20, 24);	// IR1
			RLWINM(r11, r3, 2, 20, 24);	// IR2
			RLWINM(r12, r3, 29, 20, 24);	// IR3
			STHRtoPR(&gteIR1, r9);
			STHRtoPR(&gteIR2, r11);
			STHRtoPR(&gteIR3, r12);
			STWRtoPR(&gteIRGB, r3);
			break;
			
		case 30:
			iFlushRegs();
			LIW(r3, (u32)psxRegs.code);
			STWRtoPR(&psxRegs.code, r3);
			CALLFunc ((uptr)gteMTC2);
			break;
			
		default:
			if (IsConst(_Rt_)) {
				LIW(r3, iRegs[_Rt_].k);
			}
			else {
				LWPRtoR(r3, &psxRegs.GPR.r[_Rt_]);
			}
			STWRtoPR(&psxRegs.cp2d[_Rd_].d, r3);
			break;
	}
}
#endif

#if 0
CP2_FUNC(SWC2);
#else
static void recSWC2() {
// mem[Rs + Im] = Rt

	switch (_Rt_) {
		case 1: case 3: case 5:
		case 8: case 9: case 10:
		case 11:
			LHPRtoR(r3, &psxRegs.cp2d[_Rt_].sw.l);
			EXTSH(r3, r3);
			STWRtoPR(&psxRegs.cp2d[_Rt_].d, r3);
			break;

		case 7: case 16: case 17:
		case 18: case 19:
			LHPRtoR(r3, &psxRegs.cp2d[_Rt_].w.l);
			STWRtoPR(&psxRegs.cp2d[_Rt_].d, r3);
			break;
			
		case 15:
			LWPRtoR(r3, (uptr)&gteSXY2);
			STWRtoPR(&psxRegs.cp2d[_Rt_].d, r3);
			break;

		case 29: 
			iFlushRegs();
			LIW(r3, (u32)psxRegs.code);
			STWRtoPR(&psxRegs.code, r3);
			CALLFunc ((uptr)gteSWC2);
			return;
	}


	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;

		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			LWPRtoR(r3, &psxRegs.cp2d[_Rt_].d);
			STWRtoM((uptr)&psxM[addr & 0x1fffff], r3);
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			LWPRtoR(r3, &psxRegs.cp2d[_Rt_].d);
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
					LWPRtoR(r3, &psxRegs.cp2d[_Rt_].d);
					STWRtoM((uptr)&psxH[addr & 0xffff], r3);
					return;

				case 0x1f801810:
					LWPRtoR(r3, &psxRegs.cp2d[_Rt_].d);
					CALLFunc((uptr)&GPU_writeData);
					return;

				case 0x1f801814:
					LWPRtoR(r3, &psxRegs.cp2d[_Rt_].d);
					CALLFunc((uptr)&GPU_writeStatus);
					return;
					
				case 0x1f801820:
					LWPRtoR(r3, &psxRegs.cp2d[_Rt_].d);
					CALLFunc((uptr)&mdecWrite0);
					return;

				case 0x1f801824:
					LWPRtoR(r3, &psxRegs.cp2d[_Rt_].d);
					CALLFunc((uptr)&mdecWrite1);
					return;
			}
		}
	}

	SetArg_OfB(PPCARG1);
	LWPRtoR(PPCARG2, &psxRegs.cp2d[_Rt_].d);
	CALLFunc((uptr)psxMemWrite32);
}
#endif

CP2_FUNCNC(RTPS);
CP2_FUNC(OP);
CP2_FUNCNC(NCLIP);
CP2_FUNC(DPCS);
CP2_FUNC(INTPL);
CP2_FUNC(MVMVA);
CP2_FUNCNC(NCDS);
CP2_FUNCNC(NCDT);
CP2_FUNCNC(CDP);
CP2_FUNCNC(NCCS);
CP2_FUNCNC(CC);
CP2_FUNCNC(NCS);
CP2_FUNCNC(NCT);
CP2_FUNC(SQR);
CP2_FUNC(DCPL);
CP2_FUNC(DPCT);
CP2_FUNCNC(AVSZ3);
CP2_FUNCNC(AVSZ4);
CP2_FUNC(RTPT);
CP2_FUNC(GPF);
CP2_FUNC(GPL);
CP2_FUNCNC(NCCT);

#if 0

#define SUM_FLAG() { \
	TEST32ItoM((u32)&gteFLAG, 0x7F87E000); \
	j8Ptr[0] = JZ8(0); \
	OR32ItoM((u32)&gteFLAG, 0x80000000); \
 \
 	x86SetJ8(j8Ptr[0]); \
}

#define LIM32X8(reg, gteout, negv, posv, flagb) { \
 	CMP32ItoR(reg, negv); \
	j8Ptr[0] = JL8(0); \
 	CMP32ItoR(reg, posv); \
	j8Ptr[1] = JG8(0); \
 \
	MOV8RtoM((u32)&gteout, reg); \
	j8Ptr[2] = JMP8(0); \
 \
 	x86SetJ8(j8Ptr[0]); \
	MOV8ItoM((u32)&gteout, negv); \
	j8Ptr[3] = JMP8(0); \
 \
 	x86SetJ8(j8Ptr[1]); \
	MOV8ItoM((u32)&gteout, posv); \
 \
	x86SetJ8(j8Ptr[3]); \
	OR32ItoM((u32)&gteFLAG, 1<<flagb); \
 \
	x86SetJ8(j8Ptr[2]); \
}

#define _LIM_B1(reg, gteout) LIM32X8(reg, gteout, 0, 255, 21);
#define _LIM_B2(reg, gteout) LIM32X8(reg, gteout, 0, 255, 20);
#define _LIM_B3(reg, gteout) LIM32X8(reg, gteout, 0, 255, 19);

#define MAC2IRn(reg, ir, flagb, negv, posv) { \
/* 	CMP32ItoR(reg, negv);*/ \
/*	j8Ptr[0] = JL8(0); */\
/* 	CMP32ItoR(reg, posv);*/ \
/*	j8Ptr[1] = JG8(0);*/ \
 \
	MOV32RtoM((u32)&ir, reg); \
/*	j8Ptr[2] = JMP8(0);*/ \
 \
/*	x86SetJ8(j8Ptr[0]);*/ \
/*	MOV32ItoM((u32)&ir, negv);*/ \
/*	j8Ptr[3] = JMP8(0);*/ \
 \
/*	x86SetJ8(j8Ptr[1]);*/ \
/*	MOV32ItoM((u32)&ir, posv);*/ \
 \
/*	x86SetJ8(j8Ptr[3]);*/ \
/*	OR32ItoR((u32)&gteFLAG, 1<<flagb);*/ \
 \
/*	x86SetJ8(j8Ptr[2]);*/ \
}



#define gte_C11 gteLR1
#define gte_C12 gteLR2
#define gte_C13 gteLR3
#define gte_C21 gteLG1
#define gte_C22 gteLG2
#define gte_C23 gteLG3
#define gte_C31 gteLB1
#define gte_C32 gteLB2
#define gte_C33 gteLB3


#define _MVMVA_FUNC(vn, mx) { \
	MOVSX32M16toR(EAX, (u32)&mx##vn##1); \
	IMUL32R(EBX); \
/*	j8Ptr[0] = JO8(0);*/ \
	MOV32RtoR(ECX, EAX); \
 \
	MOVSX32M16toR(EAX, (u32)&mx##vn##2); \
	IMUL32R(EDI); \
/*	j8Ptr[1] = JO8(0);*/ \
	ADD32RtoR(ECX, EAX); \
/*	j8Ptr[2] = JO8(0);*/ \
 \
	MOVSX32M16toR(EAX, (u32)&mx##vn##3); \
	IMUL32R(ESI); \
/*	j8Ptr[3] = JO8(0);*/ \
	ADD32RtoR(ECX, EAX); \
/*	j8Ptr[4] = JO8(0);*/ \
}

/*	SSX = (_v0) * mx##11 + (_v1) * mx##12 + (_v2) * mx##13; 
    SSY = (_v0) * mx##21 + (_v1) * mx##22 + (_v2) * mx##23; 
    SSZ = (_v0) * mx##31 + (_v1) * mx##32 + (_v2) * mx##33; */

#define _MVMVA_ADD(_vx, jn) { \
	ADD32MtoR(ECX, (u32)&_vx); \
/*	j8Ptr[jn] = JO8(0);*/ \
}
/*			SSX+= gteRFC;
			SSY+= gteGFC;
			SSZ+= gteBFC;*/

#define _MVMVA1(vn) { \
	switch (psxRegs.code & 0x60000) { \
		case 0x00000: /* R */ \
			_MVMVA_FUNC(vn, gteR); break; \
		case 0x20000: /* L */ \
			_MVMVA_FUNC(vn, gteL); break; \
		case 0x40000: /* C */ \
			_MVMVA_FUNC(vn, gte_C); break; \
		default: \
			return; \
	} \
}

#define _MVMVA_LOAD(_v0, _v1, _v2) { \
	MOVSX32M16toR(EBX, (u32)&_v0); \
	MOVSX32M16toR(EDI, (u32)&_v1); \
	MOVSX32M16toR(ESI, (u32)&_v2); \
}

static void recMVMVA() {
	int i;

//	SysPrintf("GTE_MVMVA %lx\n", psxRegs.code & 0x1ffffff);

/*	PUSH32R(ESI);
	PUSH32R(EDI);
	PUSH32R(EBX);
*/
	XOR32RtoR(EAX, EAX); /* gteFLAG = 0 */
	MOV32RtoM((u32)&gteFLAG, EAX);

	switch (psxRegs.code & 0x18000) {
		case 0x00000: /* V0 */
			_MVMVA_LOAD(gteVX0, gteVY0, gteVZ0); break;
		case 0x08000: /* V1 */
			_MVMVA_LOAD(gteVX1, gteVY1, gteVZ1); break;
		case 0x10000: /* V2 */
			_MVMVA_LOAD(gteVX2, gteVY2, gteVZ2); break;
		case 0x18000: /* IR */
			_MVMVA_LOAD(gteIR1, gteIR2, gteIR3); break;
	}

// MAC1
	for (i=5; i<8; i++) j8Ptr[i] = 0;
	_MVMVA1(1);

	if (psxRegs.code & 0x80000) {
		SAR32ItoR(ECX, 12);
//		SSX /= 4096.0; SSY /= 4096.0; SSZ /= 4096.0;
	}

	switch (psxRegs.code & 0x6000) {
		case 0x0000: // Add TR
			_MVMVA_ADD(gteTRX, 5); break;
		case 0x2000: // Add BK
			_MVMVA_ADD(gteRBK, 6); break;
		case 0x4000: // Add FC
			_MVMVA_ADD(gteRFC, 7); break;
	}
/*
	j8Ptr[9] = JMP8(0);
	for (i=0; i<5; i++) x86SetJ8(j8Ptr[i]);
	for (i=5; i<8; i++) if (j8Ptr[i]) x86SetJ8(j8Ptr[i]);

//	TEST32ItoR(EDX, 0x80000000);
	OR32ItoM((u32)&gteFLAG, 1<<29);
	x86SetJ8(j8Ptr[9]);*/
	MOV32RtoM((u32)&gteMAC1, ECX);

	if (psxRegs.code & 0x400) {
	 	MAC2IRn(ECX, gteIR1, 24, 0, 32767);
	} else {
	 	MAC2IRn(ECX, gteIR1, 24, -32768, 32767);
	}

// MAC2
	_MVMVA1(2);

	if (psxRegs.code & 0x80000) {
		SAR32ItoR(ECX, 12);
//		SSX /= 4096.0; SSY /= 4096.0; SSZ /= 4096.0;
	}

	switch (psxRegs.code & 0x6000) {
		case 0x0000: // Add TR
			_MVMVA_ADD(gteTRY, 5); break;
		case 0x2000: // Add BK
			_MVMVA_ADD(gteGBK, 6); break;
		case 0x4000: // Add FC
			_MVMVA_ADD(gteGFC, 7); break;
	}

/*	for (i=0; i<5; i++) x86SetJ8(j8Ptr[i]);
	for (i=5; i<8; i++) if (j8Ptr[i]) x86SetJ8(j8Ptr[i]);*/
	MOV32RtoM((u32)&gteMAC2, ECX);

	if (psxRegs.code & 0x400) {
	 	MAC2IRn(ECX, gteIR2, 23, 0, 32767);
	} else {
	 	MAC2IRn(ECX, gteIR2, 23, -32768, 32767);
	}

// MAC3
	_MVMVA1(3);

	if (psxRegs.code & 0x80000) {
		SAR32ItoR(ECX, 12);
//		SSX /= 4096.0; SSY /= 4096.0; SSZ /= 4096.0;
	}

	switch (psxRegs.code & 0x6000) {
		case 0x0000: // Add TR
			_MVMVA_ADD(gteTRZ, 5); break;
		case 0x2000: // Add BK
			_MVMVA_ADD(gteBBK, 6); break;
		case 0x4000: // Add FC
			_MVMVA_ADD(gteBFC, 7); break;
	}

/*	for (i=0; i<5; i++) x86SetJ8(j8Ptr[i]);
	for (i=5; i<8; i++) if (j8Ptr[i]) x86SetJ8(j8Ptr[i]);*/
	MOV32RtoM((u32)&gteMAC3, ECX);

	if (psxRegs.code & 0x400) {
	 	MAC2IRn(ECX, gteIR3, 22, 0, 32767);
	} else {
	 	MAC2IRn(ECX, gteIR3, 22, -32768, 32767);
	}
/*		MAC2IR1()
	else MAC2IR()*/

//	SUM_FLAG();

/*	POP32R(EBX);
	POP32R(EDI);
	POP32R(ESI);*/
}

#if 0

#define _GPF1(vn) { \
	MOV32MtoR(EAX, (u32)&gteIR##vn); \
	IMUL32R(ECX); \
/*	MOV32RtoR(ECX, EAX); */\
}

static void recGPF() {
//	SysPrintf("GTE_GPF %lx\n", psxRegs.code & 0x1ffffff);

	PUSH32R(EBX);

	XOR32RtoR(EBX, EBX); /* gteFLAG = 0 */

/*		gteMAC1 = NC_OVERFLOW1(gteIR0 * gteIR1);
		gteMAC2 = NC_OVERFLOW2(gteIR0 * gteIR2);
        gteMAC3 = NC_OVERFLOW3(gteIR0 * gteIR3);*/
	MOV32MtoR(ECX, (u32)&gteIR0);
// MAC1
	_GPF1(1);

	if (psxRegs.code & 0x80000) {
		SAR32ItoR(EAX, 12);
	}
	MAC2IRn(EAX, gteIR1, 24, -32768, 32767);
	PUSH32R(EAX);

// MAC2
	_GPF1(2);

	if (psxRegs.code & 0x80000) {
		SAR32ItoR(EAX, 12);
	}
	MAC2IRn(EAX, gteIR2, 23, -32768, 32767);
	PUSH32R(EAX);

// MAC3
	_GPF1(3);

	if (psxRegs.code & 0x80000) {
		SAR32ItoR(EAX, 12);
	}
	MAC2IRn(EAX, gteIR3, 22, -32768, 32767);
//	MAC2IR();

//	gteRGB0 = gteRGB1;
//	gteRGB1 = gteRGB2;
	MOV32MtoR(EDX, (u32)&gteRGB1);
	MOV32MtoR(ECX, (u32)&gteRGB2);
	MOV32RtoM((u32)&gteRGB0, EDX);
	MOV32RtoM((u32)&gteRGB1, ECX);

	POP32R(EDX);
	POP32R(ECX);
	SAR32ItoR(ECX, 4);
	SAR32ItoR(EDX, 4);
	SAR32ItoR(EAX, 4);

	_LIM_B1(ECX, gteR2);
	_LIM_B2(EDX, gteG2);
	_LIM_B3(EAX, gteB2);
	MOV8MtoR(EAX, (u32)&gteCODE);
	MOV8RtoM((u32)&gteCODE2, EAX);

/*	gteR2 = limB1(gteMAC1 / 16.0f);
	gteG2 = limB2(gteMAC2 / 16.0f);
	gteB2 = limB3(gteMAC3 / 16.0f); gteCODE2 = gteCODE;*/

	SUM_FLAG();
	MOV32RtoM((u32)&gteFLAG, EBX);

//	POP32R(EBX);
}
#endif
#endif

#endif // __PGTE_H__

