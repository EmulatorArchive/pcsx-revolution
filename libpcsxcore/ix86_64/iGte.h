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

#ifndef __IGTE_H__
#define __IGTE_H__

#include "../r3000a.h"
#include "../psxmem.h"
#include "../gte.h"

#define CP2_FUNC(f) \
void gte##f(); \
static void rec##f() { \
	iFlushRegs(); \
	MOV32ItoM((uptr)&psxRegs.code, (u32)psxRegs.code); \
	CALLFunc((uptr)gte##f); \
/*	branch = 2; */\
}

#define CP2_FUNCNC(f) \
void gte##f(); \
static void rec##f() { \
	iFlushRegs(); \
	CALLFunc((uptr)gte##f); \
/*	branch = 2; */\
}

#define BOUNDS( reg, max, max_flag, min, min_flag ) { \
	CMP32ItoR(reg, min); \
	j8Ptr[0] = JL8(0); \
 	CMP32ItoR(reg, max); \
	j8Ptr[1] = JG8(0); \
 \
	j8Ptr[2] = JMP8(0); \
 \
 	x86SetJ8(j8Ptr[0]); \
	OR32ItoM((uptr)&gteFLAG, min_flag); \
	j8Ptr[3] = JMP8(0); \
 \
 	x86SetJ8(j8Ptr[1]); \
	OR32ItoM((uptr)&gteFLAG, max_flag); \
 \
	x86SetJ8(j8Ptr[3]); \
	x86SetJ8(j8Ptr[2]); \
}

#define LIM(reg, gteout, max, min, flag) {\
 	CMP32ItoR(reg, min); \
	j8Ptr[0] = JL8(0); \
 	CMP32ItoR(reg, max); \
	j8Ptr[1] = JG8(0); \
 \
	MOV32RtoM((uptr)&gteout, reg); \
	j8Ptr[2] = JMP8(0); \
 \
 	x86SetJ8(j8Ptr[0]); \
	MOV32ItoM((uptr)&gteout, min); \
	j8Ptr[3] = JMP8(0); \
 \
 	x86SetJ8(j8Ptr[1]); \
	MOV32ItoM((uptr)&gteout, max); \
 \
	x86SetJ8(j8Ptr[3]); \
	OR32ItoM((uptr)&gteFLAG, flag); \
 \
	x86SetJ8(j8Ptr[2]); \
}

#define A1( reg ) BOUNDS( ( reg ), 0x7fffffff, ( 1 << 30 ), -(s64)0x80000000, ( 1 << 31 ) | ( 1 << 27 ) )
#define A2( reg ) BOUNDS( ( reg ), 0x7fffffff, ( 1 << 29 ), -(s64)0x80000000, ( 1 << 31 ) | ( 1 << 26 ) )
#define A3( reg ) BOUNDS( ( reg ), 0x7fffffff, ( 1 << 28 ), -(s64)0x80000000, ( 1 << 31 ) | ( 1 << 25 ) )
#define Lm_B1( reg, out, l ) LIM( ( reg ), ( out ), 0x7fff, -0x8000 * !l, ( 1 << 31 ) | ( 1 << 24 ) )
#define Lm_B2( reg, out, l ) LIM( ( reg ), ( out ), 0x7fff, -0x8000 * !l, ( 1 << 31 ) | ( 1 << 23 ) )
#define Lm_B3( reg, out, l ) LIM( ( reg ), ( out ), 0x7fff, -0x8000 * !l, ( 1 << 22 ) )
#define Lm_C1( reg, out ) LIM( ( reg ), ( out ), 0x00ff, 0x0000, ( 1 << 21 ) )
#define Lm_C2( reg, out ) LIM( ( reg ), ( out ), 0x00ff, 0x0000, ( 1 << 20 ) )
#define Lm_C3( reg, out ) LIM( ( reg ), ( out ), 0x00ff, 0x0000, ( 1 << 19 ) )
#define Lm_D( reg, out ) LIM( ( reg ), ( out ), 0xffff, 0x0000, ( 1 << 31 ) | ( 1 << 18 ) )

#define Lm_E( reg ) { \
	CMP32ItoR(reg, 0x1ffff); \
	j8Ptr[0] = JL8(0); \
	\
	MOV32ItoR(reg, 0x1ffff); \
	OR32ItoM((uptr)&gteFLAG, (( 1 << 31 ) | ( 1 << 17 ))); \
	\
	x86SetJ8(j8Ptr[0]); \
}

#define F( reg ) BOUNDS( ( reg ), 0x7fffffff, ( 1 << 31 ) | ( 1 << 16 ), -(s64)0x80000000, ( 1 << 31 ) | ( 1 << 15 ) )
#define Lm_G1( reg, out ) LIM( ( reg ), ( out ), 0x3ff, -0x400, ( 1 << 31 ) | ( 1 << 14 ) )
#define Lm_G2( reg, out ) LIM( ( reg ), ( out ), 0x3ff, -0x400, ( 1 << 31 ) | ( 1 << 13 ) )
#define Lm_H( reg, out ) LIM( ( reg ), ( out ), 0xfff, 0x000, ( 1 << 12 ) )

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
			MOVSX32M16toR(EAX, (uptr)&psxRegs.cp2d[ _Rd_ ].sw.l);
			MOV32RtoM((uptr)&psxRegs.cp2d[ _Rd_ ].d, EAX);
			MOV32RtoM((uptr)&psxRegs.GPR.r[_Rt_], EAX);
			break;

		case 7: case 16: case 17:
		case 18: case 19:
			MOVZX32M16toR(EAX, (uptr)&psxRegs.cp2d[ _Rd_ ].w.l);
			MOV32RtoM((uptr)&psxRegs.cp2d[ _Rd_ ].d, EAX);
			MOV32RtoM((uptr)&psxRegs.GPR.r[_Rt_], EAX);
			break;

		case 15: 
			MOV32MtoR(EAX, (uptr)&gteSXY2);
			MOV32RtoM((uptr)&psxRegs.cp2d[ _Rd_ ].d, EAX);
			MOV32RtoM((uptr)&psxRegs.GPR.r[_Rt_], EAX);
			break;

		case 29:
			MOV32ItoM((uptr)&psxRegs.code, (u32)psxRegs.code);
			CALLFunc((uptr)gteMFC2);
			/*
				psxRegs.cp2d[ reg ] = LIM( gteIR1 >> 7, 0x1f, 0, 0 ) | 
									( LIM( gteIR2 >> 7, 0x1f, 0, 0 ) << 5 ) | 
									( LIM( gteIR3 >> 7, 0x1f, 0, 0 ) << 10 );
			*/
			break;

		default:
			MOV32MtoR(EAX, (uptr)&psxRegs.cp2d[_Rd_].d);
			MOV32RtoM((uptr)&psxRegs.GPR.r[_Rt_], EAX);
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

		case 15:
			MOV32MtoR(EAX, (uptr)&gteSXY1);
			MOV32RtoM((uptr)&gteSXY0, EAX);
			MOV32MtoR(EAX, (uptr)&gteSXY2);
			MOV32RtoM((uptr)&gteSXY1, EAX);
			
			if (IsConst(_Rt_)) {
				MOV32ItoR(EAX, iRegs[_Rt_].k);
			}
			else {
				MOV32MtoR(EAX, (uptr)&psxRegs.GPR.r[_Rt_]);
			}

			MOV32RtoM((uptr)&gteSXY2, EAX);
			MOV32RtoM((uptr)&gteSXYP, EAX);
			break;

		case 28:
			if (IsConst(_Rt_)) {
				MOV32ItoR(EAX, iRegs[_Rt_].k);
			}
			else {
				MOV32MtoR(EAX, (uptr)&psxRegs.GPR.r[_Rt_]);
			}
			MOV32RtoM((uptr)&gteIRGB, EAX);
			MOV32RtoR(EDX, EAX);
			AND32ItoR(EDX, 0x1f);
			SHL32ItoR(EDX, 7);
			MOV32RtoM((uptr)&gteIR1, EDX);
			MOV32RtoR(EDX, EAX);
			AND32ItoR(EDX, 0x3e0);
			SHL32ItoR(EDX, 2);
			MOV32RtoM((uptr)&gteIR2, EDX);
			MOV32RtoR(EDX, EAX);
			AND32ItoR(EDX, 0x7c00);
			SHR32ItoR(EDX, 3);
			MOV32RtoM((uptr)&gteIR3, EDX);
			break;
			
		case 30:
			MOV32ItoM((uptr)&psxRegs.code, (u32)psxRegs.code);
			CALLFunc((uptr)gteMTC2);
			break;
			
		default:
			if (IsConst(_Rt_)) {
				MOV32ItoM((uptr)&psxRegs.cp2d[_Rd_].d, iRegs[_Rt_].k);
			} else {
				MOV32MtoR(EAX, (uptr)&psxRegs.GPR.r[_Rt_]);
				MOV32RtoM((uptr)&psxRegs.cp2d[_Rd_].d, EAX);
			}
			break;
	}
}
#endif

#if 0
CP2_FUNC(CFC2);
#else
static void recCFC2() {
// Rt = Cop2C->Rd
	if (!_Rt_) return;

	iRegs[_Rt_].state = ST_UNK;
	MOV32MtoR(EAX, (uptr)&psxRegs.cp2c[_Rd_].d);
	MOV32RtoM((uptr)&psxRegs.GPR.r[_Rt_], EAX);
}
#endif

#if 0
CP2_FUNC(CTC2);
#else
static void recCTC2() {
// Cop2C->Rd = Rt

	if (IsConst(_Rt_)) {
		MOV32ItoM((uptr)&psxRegs.cp2c[_Rd_], iRegs[_Rt_].k);
	} else {
		MOV32MtoR(EAX, (uptr)&psxRegs.GPR.r[_Rt_]);
		MOV32RtoM((uptr)&psxRegs.cp2c[_Rd_], EAX);
	}
}
#endif

#if 0
CP2_FUNC(LWC2);
CP2_FUNC(SWC2);
#else
static void recLWC2() {
// Cop2D->Rt = mem[Rs + Im] (unsigned)

if(_Rt_ != 30) { // TODO
	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;

		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			MOV32MtoR(EAX, (uptr)&psxM[addr & 0x1fffff]);
			//MOV32RtoM((uptr)&psxRegs.cp2d[_Rt_].d, EAX);
			j8Ptr[0] = JMP8(0);
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			MOV32MtoR(EAX, (uptr)&psxH[addr & 0xfff]);
			//MOV32RtoM((uptr)&psxRegs.cp2d[_Rt_].d, EAX);
			j8Ptr[0] = JMP8(0);
		}
	}

	SetArg_OfB(X86ARG1);
	CALLFunc((uptr)psxMemRead32);
	//MOV32RtoM((uptr)&psxRegs.cp2d[_Rt_].d, EAX);

	if (IsConst(_Rs_)) {
		x86SetJ8(j8Ptr[0]);
	}
}
	switch (_Rt_) {

		case 15:
			MOV32MtoR(EDX, (uptr)&gteSXY1);
			MOV32RtoM((uptr)&gteSXY0, EDX);
			MOV32MtoR(EDX, (uptr)&gteSXY2);
			MOV32RtoM((uptr)&gteSXY1, EDX);
			MOV32RtoM((uptr)&gteSXY2, EAX);
			MOV32RtoM((uptr)&gteSXYP, EAX);
			break;

		case 28:
			MOV32RtoM((uptr)&gteIRGB, EAX);
			MOV32RtoR(EDX, EAX);
			AND32ItoR(EDX, 0x1f);
			SHL32ItoR(EDX, 7);
			MOV32RtoM((uptr)&gteIR1, EDX);
			MOV32RtoR(EDX, EAX);
			AND32ItoR(EDX, 0x3e0);
			SHL32ItoR(EDX, 2);
			MOV32RtoM((uptr)&gteIR2, EDX);
			MOV32RtoR(EDX, EAX);
			AND32ItoR(EDX, 0x7c00);
			SHR32ItoR(EDX, 3);
			MOV32RtoM((uptr)&gteIR3, EDX);
			break;
		
		case 30:
			iFlushRegs();
			MOV32ItoM((uptr)&psxRegs.code, (u32)psxRegs.code);
			CALLFunc((uptr)gteLWC2);
			return;
			
		default: 
			MOV32RtoM((uptr)&psxRegs.cp2d[_Rt_].d, EAX);
	}
	// All GTE memory access takes time of 2 instructions
	resp += 8;
}

static void recSWC2() {
// mem[Rs + Im] = Rt

	switch (_Rt_) {
		case 1: case 3: case 5:
		case 8: case 9: case 10:
		case 11:
			MOVSX32M16toR(EAX, (uptr)&psxRegs.cp2d[ _Rt_ ].sw.l);
			MOV32RtoM((uptr)&psxRegs.cp2d[ _Rt_ ].d, EAX);
			break;

		case 7: case 16: case 17:
		case 18: case 19:
			MOVZX32M16toR(EAX, (uptr)&psxRegs.cp2d[ _Rt_ ].w.l);
			MOV32RtoM((uptr)&psxRegs.cp2d[ _Rt_ ].d, EAX);
			break;

		case 15: 
			MOV32MtoR(EAX, (uptr)&gteSXY2);
			MOV32RtoM((uptr)&psxRegs.cp2d[ _Rt_ ].d, EAX);
			break;

		case 29:
			iFlushRegs();
			MOV32ItoM((uptr)&psxRegs.code, (u32)psxRegs.code);
			CALLFunc((uptr)gteSWC2);
			return;
	}

	if (IsConst(_Rs_)) {
		u32 addr = iRegs[_Rs_].k + _Imm_;
		int t = addr >> 16;

		if ((t & 0x1fe0) == 0 && (t & 0x1fff) != 0) {
			MOV32MtoR(EAX, (uptr)&psxRegs.cp2d[_Rt_].d);
			MOV32RtoM((uptr)&psxM[addr & 0x1fffff], EAX);
			resp += 8;
			return;
		}
		if (t == 0x1f80 && addr < 0x1f801000) {
			MOV32MtoR(EAX, (uptr)&psxRegs.cp2d[_Rt_].d);
			MOV32RtoM((uptr)&psxH[addr & 0xfff], EAX);
			resp += 8;
			return;
		}
	}

	MOV32MtoR(X86ARG2, (uptr)&psxRegs.cp2d[_Rt_].d);
	SetArg_OfB(X86ARG1);
	CALLFunc((uptr)psxMemWrite32);
	// All GTE memory access takes time of 2 instructions
	resp += 8;
}

#endif
void print_mac(){
		SysPrintf("MAC1 = %d\n", gteMAC1);
}
#if 1
CP2_FUNCNC(RTPS);
#else
void recRTPS() {
	MOV32ItoM((uptr)&gteFLAG, 0);
/* 0x0000000000423550 <gteRTPS+0>: sub    $0x8,%rsp 
0x0000000000423554 <gteRTPS+4>: movswl 0x330dad(%rip),%edx        # EDX = gteVX0
0x000000000042355b <gteRTPS+11>:        movswl 0x330d26(%rip),%eax        # EAX = gteR11
0x0000000000423562 <gteRTPS+18>:        movl   $0x0,0x330e18(%rip)        # gteFLAG = 0
0x000000000042356c <gteRTPS+28>:        imul   %edx,%eax                  # gteVX0 * gteR11                        
0x000000000042356f <gteRTPS+31>:        movswl 0x330d94(%rip),%edx        # EDX = gteVY0
0x0000000000423576 <gteRTPS+38>:        movslq %eax,%rsi                                          
0x0000000000423579 <gteRTPS+41>:        movslq 0x330d9c(%rip),%rax        # RAX = gteTRX
0x0000000000423580 <gteRTPS+48>:        shl    $0xc,%rax                  # gteTRX << 12                       
0x0000000000423584 <gteRTPS+52>:        add    %rax,%rsi                                          
0x0000000000423587 <gteRTPS+55>:        movswl 0x330cfc(%rip),%eax        # 0x75428a <psxRegs+266>
0x000000000042358e <gteRTPS+62>:        imul   %edx,%eax                                          
0x0000000000423591 <gteRTPS+65>:        movslq %eax,%rdx                                          
0x0000000000423594 <gteRTPS+68>:        movswl 0x330cf1(%rip),%eax        # 0x75428c <psxRegs+268>
0x000000000042359b <gteRTPS+75>:        add    %rdx,%rsi                                          
0x000000000042359e <gteRTPS+78>:        movswl 0x330d67(%rip),%edx        # 0x75430c <psxRegs+396>
0x00000000004235a5 <gteRTPS+85>:        imul   %edx,%eax                                          
0x00000000004235a8 <gteRTPS+88>:        cltq                                                      
0x00000000004235aa <gteRTPS+90>:        add    %rax,%rsi                                          
0x00000000004235ad <gteRTPS+93>:        sar    $0xc,%rsi                                          
0x00000000004235b1 <gteRTPS+97>:        cmp    $0x7fffffff,%rsi                                   
0x00000000004235b8 <gteRTPS+104>:       jle    0x423a50 <gteRTPS+1280>                            
0x00000000004235be <gteRTPS+110>:       movl   $0x40000000,0x330dbc(%rip)        # 0x754384 <psxRegs+516>           
0x00000000004235c8 <gteRTPS+120>:       xor    %eax,%eax                                          
0x00000000004235ca <gteRTPS+122>:       mov    $0x457572,%edi                                     
0x00000000004235cf <gteRTPS+127>:       mov    %esi,0x330d17(%rip)        # 0x7542ec <psxRegs+364>

0x00000000004235d5 <gteRTPS+133>:       callq  0x405a60 <SysPrintf>
0x00000000004235da <gteRTPS+138>:       movswl 0x330d2d(%rip),%eax        # 0x75430e <psxRegs+398>
0x00000000004235e1 <gteRTPS+145>:       movswl 0x330ca0(%rip),%edi        # 0x754288 <psxRegs+264>
0x00000000004235e8 <gteRTPS+152>:       movswl 0x330c9b(%rip),%esi        # 0x75428a <psxRegs+266>
0x00000000004235ef <gteRTPS+159>:       movswl 0x330d1a(%rip),%edx        # 0x754310 <psxRegs+400>
0x00000000004235f6 <gteRTPS+166>:       movswl 0x330c8f(%rip),%ecx        # 0x75428c <psxRegs+268>
0x00000000004235fd <gteRTPS+173>:       movswl 0x330d0d(%rip),%r8d        # 0x754312 <psxRegs+402>
0x0000000000423605 <gteRTPS+181>:       imul   %edi,%eax                                          
0x0000000000423608 <gteRTPS+184>:       imul   %esi,%edx                                          
0x000000000042360b <gteRTPS+187>:       cltq                                                      
0x000000000042360d <gteRTPS+189>:       movslq %edx,%rdx                                          
0x0000000000423610 <gteRTPS+192>:       lea    (%rax,%rdx,1),%rdx                                 
0x0000000000423614 <gteRTPS+196>:       movslq 0x330d05(%rip),%rax        # 0x754320 <psxRegs+416>
0x000000000042361b <gteRTPS+203>:       imul   %ecx,%r8d                                          
0x000000000042361f <gteRTPS+207>:       shl    $0xc,%rax                                          
0x0000000000423623 <gteRTPS+211>:       movslq %r8d,%r8                                           
0x0000000000423626 <gteRTPS+214>:       lea    (%rdx,%rax,1),%rax                                 
0x000000000042362a <gteRTPS+218>:       lea    (%rax,%r8,1),%rdx                                  
0x000000000042362e <gteRTPS+222>:       sar    $0xc,%rdx                                          
0x0000000000423632 <gteRTPS+226>:       cmp    $0x7fffffff,%rdx                                   
0x0000000000423639 <gteRTPS+233>:       jle    0x423a30 <gteRTPS+1248>                            
0x000000000042363f <gteRTPS+239>:       orl    $0x20000000,0x330d3b(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x0000000000423649 <gteRTPS+249>:       movswl 0x330cc6(%rip),%eax        # 0x754316 <psxRegs+406>
0x0000000000423650 <gteRTPS+256>:       mov    %edx,0x330c9a(%rip)        # 0x7542f0 <psxRegs+368>

0x0000000000423656 <gteRTPS+262>:       movswl 0x330cb7(%rip),%edx        # 0x754314 <psxRegs+404>
0x000000000042365d <gteRTPS+269>:       imul   %eax,%esi                                          
0x0000000000423660 <gteRTPS+272>:       imul   %edi,%edx                                          
0x0000000000423663 <gteRTPS+275>:       movslq %esi,%rsi                                          
0x0000000000423666 <gteRTPS+278>:       movslq %edx,%rdx                                          
0x0000000000423669 <gteRTPS+281>:       lea    (%rdx,%rsi,1),%rax                                 
0x000000000042366d <gteRTPS+285>:       movslq 0x330cb0(%rip),%rdx        # 0x754324 <psxRegs+420>
0x0000000000423674 <gteRTPS+292>:       shl    $0xc,%rdx                                          
0x0000000000423678 <gteRTPS+296>:       lea    (%rax,%rdx,1),%rdx                                 
0x000000000042367c <gteRTPS+300>:       movswl 0x330c95(%rip),%eax        # 0x754318 <psxRegs+408>
0x0000000000423683 <gteRTPS+307>:       imul   %eax,%ecx                                          
0x0000000000423686 <gteRTPS+310>:       movslq %ecx,%rcx                                          
0x0000000000423689 <gteRTPS+313>:       lea    (%rdx,%rcx,1),%rax                                 
0x000000000042368d <gteRTPS+317>:       sar    $0xc,%rax                                          
0x0000000000423691 <gteRTPS+321>:       cmp    $0x7fffffff,%rax                                   
0x0000000000423697 <gteRTPS+327>:       jle    0x423a10 <gteRTPS+1216>                            
0x000000000042369d <gteRTPS+333>:       orl    $0x10000000,0x330cdd(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x00000000004236a7 <gteRTPS+343>:       mov    %eax,0x330c47(%rip)        # 0x7542f4 <psxRegs+372>
0x00000000004236ad <gteRTPS+349>:       mov    0x330c39(%rip),%eax        # 0x7542ec <psxRegs+364>
0x00000000004236b3 <gteRTPS+355>:       cmp    $0x7fff,%eax                                       
0x00000000004236b8 <gteRTPS+360>:       jle    0x4239f0 <gteRTPS+1184>                            
0x00000000004236be <gteRTPS+366>:       orl    $0x81000000,0x330cbc(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x00000000004236c8 <gteRTPS+376>:       mov    $0x7fff,%eax                                       
0x00000000004236cd <gteRTPS+381>:       mov    %eax,0x330bd9(%rip)        # 0x7542ac <psxRegs+300>
0x00000000004236d3 <gteRTPS+387>:       mov    0x330c17(%rip),%eax        # 0x7542f0 <psxRegs+368>
0x00000000004236d9 <gteRTPS+393>:       cmp    $0x7fff,%eax                                       

0x00000000004236de <gteRTPS+398>:       jle    0x4239d0 <gteRTPS+1152>                            
0x00000000004236e4 <gteRTPS+404>:       orl    $0x80800000,0x330c96(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x00000000004236ee <gteRTPS+414>:       mov    $0x7fff,%eax                                       
0x00000000004236f3 <gteRTPS+419>:       mov    %eax,0x330bb7(%rip)        # 0x7542b0 <psxRegs+304>
0x00000000004236f9 <gteRTPS+425>:       mov    0x330bf5(%rip),%eax        # 0x7542f4 <psxRegs+372>
0x00000000004236ff <gteRTPS+431>:       cmp    $0x7fff,%eax                                       
0x0000000000423704 <gteRTPS+436>:       mov    %eax,%edx                                          
0x0000000000423706 <gteRTPS+438>:       jle    0x4239b0 <gteRTPS+1120>                            
0x000000000042370c <gteRTPS+444>:       orl    $0x400000,0x330c6e(%rip)        # 0x754384 <psxRegs+516>                                                                                             
0x0000000000423716 <gteRTPS+454>:       mov    $0x7fff,%edx                                       
0x000000000042371b <gteRTPS+459>:       mov    %edx,0x330b93(%rip)        # 0x7542b4 <psxRegs+308>
0x0000000000423721 <gteRTPS+465>:       movzwl 0x330ba4(%rip),%edx        # 0x7542cc <psxRegs+332>
0x0000000000423728 <gteRTPS+472>:       cmp    $0xffff,%eax                                       
0x000000000042372d <gteRTPS+477>:       mov    %dx,0x330b94(%rip)        # 0x7542c8 <psxRegs+328> 
0x0000000000423734 <gteRTPS+484>:       movzwl 0x330b95(%rip),%edx        # 0x7542d0 <psxRegs+336>
0x000000000042373b <gteRTPS+491>:       mov    %dx,0x330b8a(%rip)        # 0x7542cc <psxRegs+332> 
0x0000000000423742 <gteRTPS+498>:       movzwl 0x330b8b(%rip),%edx        # 0x7542d4 <psxRegs+340>
0x0000000000423749 <gteRTPS+505>:       mov    %dx,0x330b80(%rip)        # 0x7542d0 <psxRegs+336> 
0x0000000000423750 <gteRTPS+512>:       jle    0x423990 <gteRTPS+1088>                            
0x0000000000423756 <gteRTPS+518>:       orl    $0x80040000,0x330c24(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x0000000000423760 <gteRTPS+528>:       mov    $0xffff,%ecx                                       
0x0000000000423765 <gteRTPS+533>:       mov    $0xffffffff,%edx                                   
0x000000000042376a <gteRTPS+538>:       cvtsi2sd %ecx,%xmm1                                       
0x000000000042376e <gteRTPS+542>:       movzwl 0x330bfb(%rip),%eax        # 0x754370 <psxRegs+496>
0x0000000000423775 <gteRTPS+549>:       mov    %dx,0x330b58(%rip)        # 0x7542d4 <psxRegs+340> 

0x000000000042377c <gteRTPS+556>:       shl    $0x10,%eax                                         
0x000000000042377f <gteRTPS+559>:       cvtsi2sd %eax,%xmm0                                       
0x0000000000423783 <gteRTPS+563>:       addsd  0x343ad(%rip),%xmm1        # 0x457b38              
0x000000000042378b <gteRTPS+571>:       divsd  %xmm1,%xmm0                                        
0x000000000042378f <gteRTPS+575>:       cvttsd2si %xmm0,%rdx                                      
0x0000000000423794 <gteRTPS+580>:       cmp    $0x1ffff,%edx                                      
0x000000000042379a <gteRTPS+586>:       movslq %edx,%rax                                          
0x000000000042379d <gteRTPS+589>:       jbe    0x4237ae <gteRTPS+606>                             
0x000000000042379f <gteRTPS+591>:       orl    $0x80020000,0x330bdb(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x00000000004237a9 <gteRTPS+601>:       mov    $0x1ffff,%eax                                      
0x00000000004237ae <gteRTPS+606>:       mov    0x330b08(%rip),%edx        # 0x7542bc <psxRegs+316>
0x00000000004237b4 <gteRTPS+612>:       movslq 0x330bad(%rip),%rcx        # 0x754368 <psxRegs+488>
0x00000000004237bb <gteRTPS+619>:       mov    %edx,0x330af7(%rip)        # 0x7542b8 <psxRegs+312>
0x00000000004237c1 <gteRTPS+625>:       mov    0x330af9(%rip),%edx        # 0x7542c0 <psxRegs+320>
0x00000000004237c7 <gteRTPS+631>:       mov    %edx,0x330aef(%rip)        # 0x7542bc <psxRegs+316>
0x00000000004237cd <gteRTPS+637>:       movslq 0x330ad8(%rip),%rdx        # 0x7542ac <psxRegs+300>
0x00000000004237d4 <gteRTPS+644>:       imul   %rax,%rdx                                          
0x00000000004237d8 <gteRTPS+648>:       add    %rcx,%rdx                                          
0x00000000004237db <gteRTPS+651>:       cmp    $0x7fffffff,%rdx                                   
0x00000000004237e2 <gteRTPS+658>:       jle    0x423970 <gteRTPS+1056>                            
0x00000000004237e8 <gteRTPS+664>:       orl    $0x80010000,0x330b92(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x00000000004237f2 <gteRTPS+674>:       shr    $0x10,%rdx                                         
0x00000000004237f6 <gteRTPS+678>:       cmp    $0x3ff,%edx                                        
0x00000000004237fc <gteRTPS+684>:       jle    0x423948 <gteRTPS+1016>                            
0x0000000000423802 <gteRTPS+690>:       orl    $0x80004000,0x330b78(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x000000000042380c <gteRTPS+700>:       mov    $0x3ff,%ecx                                        

0x0000000000423811 <gteRTPS+705>:       movslq 0x330a98(%rip),%rdx        # 0x7542b0 <psxRegs+304>
0x0000000000423818 <gteRTPS+712>:       mov    %cx,0x330aa1(%rip)        # 0x7542c0 <psxRegs+320> 
0x000000000042381f <gteRTPS+719>:       movslq 0x330b46(%rip),%rcx        # 0x75436c <psxRegs+492>
0x0000000000423826 <gteRTPS+726>:       imul   %rax,%rdx                                          
0x000000000042382a <gteRTPS+730>:       add    %rcx,%rdx                                          
0x000000000042382d <gteRTPS+733>:       cmp    $0x7fffffff,%rdx                                   
0x0000000000423834 <gteRTPS+740>:       jle    0x423928 <gteRTPS+984>                             
0x000000000042383a <gteRTPS+746>:       orl    $0x80010000,0x330b40(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x0000000000423844 <gteRTPS+756>:       shr    $0x10,%rdx                                         
0x0000000000423848 <gteRTPS+760>:       cmp    $0x3ff,%edx                                        
0x000000000042384e <gteRTPS+766>:       jle    0x423900 <gteRTPS+944>                             
0x0000000000423854 <gteRTPS+772>:       orl    $0x80002000,0x330b26(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x000000000042385e <gteRTPS+782>:       mov    $0x3ff,%ecx                                        
0x0000000000423863 <gteRTPS+787>:       movswq 0x330b09(%rip),%rdx        # 0x754374 <psxRegs+500>
0x000000000042386b <gteRTPS+795>:       mov    %cx,0x330a50(%rip)        # 0x7542c2 <psxRegs+322> 
0x0000000000423872 <gteRTPS+802>:       shl    $0x8,%rdx                                          
0x0000000000423876 <gteRTPS+806>:       imul   %rdx,%rax                                          
0x000000000042387a <gteRTPS+810>:       movslq 0x330af7(%rip),%rdx        # 0x754378 <psxRegs+504>
0x0000000000423881 <gteRTPS+817>:       sar    $0x8,%rax                                          
0x0000000000423885 <gteRTPS+821>:       add    %rdx,%rax                                          
0x0000000000423888 <gteRTPS+824>:       sar    $0x4,%rax                                          
0x000000000042388c <gteRTPS+828>:       cmp    $0x7fffffff,%rax                                   
0x0000000000423892 <gteRTPS+834>:       jle    0x4238c0 <gteRTPS+880>                             
0x0000000000423894 <gteRTPS+836>:       orl    $0x80010000,0x330ae6(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x000000000042389e <gteRTPS+846>:       cmp    $0xfff,%eax                                        
0x00000000004238a3 <gteRTPS+851>:       mov    %eax,0x330a3f(%rip)        # 0x7542e8 <psxRegs+360>
0x00000000004238a9 <gteRTPS+857>:       jg     0x4238df <gteRTPS+911>                             

0x00000000004238ab <gteRTPS+859>:       test   %eax,%eax                                          
0x00000000004238ad <gteRTPS+861>:       js     0x423a70 <gteRTPS+1312>                            
0x00000000004238b3 <gteRTPS+867>:       mov    %eax,0x3309ef(%rip)        # 0x7542a8 <psxRegs+296>
0x00000000004238b9 <gteRTPS+873>:       add    $0x8,%rsp                                          
0x00000000004238bd <gteRTPS+877>:       retq                                                      
0x00000000004238be <gteRTPS+878>:       xchg   %ax,%ax                                            
0x00000000004238c0 <gteRTPS+880>:       cmp    $0xffffffff80000000,%rax                           
0x00000000004238c6 <gteRTPS+886>:       jge    0x42389e <gteRTPS+846>                             
0x00000000004238c8 <gteRTPS+888>:       orl    $0x80008000,0x330ab2(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x00000000004238d2 <gteRTPS+898>:       cmp    $0xfff,%eax                                        
0x00000000004238d7 <gteRTPS+903>:       mov    %eax,0x330a0b(%rip)        # 0x7542e8 <psxRegs+360>
0x00000000004238dd <gteRTPS+909>:       jle    0x4238ab <gteRTPS+859>                             
0x00000000004238df <gteRTPS+911>:       mov    $0xfff,%eax                                        
0x00000000004238e4 <gteRTPS+916>:       orl    $0x1000,0x330a96(%rip)        # 0x754384 <psxRegs+516>                                                                                               
0x00000000004238ee <gteRTPS+926>:       mov    %eax,0x3309b4(%rip)        # 0x7542a8 <psxRegs+296>
0x00000000004238f4 <gteRTPS+932>:       add    $0x8,%rsp                                          
0x00000000004238f8 <gteRTPS+936>:       retq                                                      
0x00000000004238f9 <gteRTPS+937>:       nopl   0x0(%rax)                                          
0x0000000000423900 <gteRTPS+944>:       cmp    $0xfffffc00,%edx                                   
0x0000000000423906 <gteRTPS+950>:       mov    %edx,%ecx                                          
0x0000000000423908 <gteRTPS+952>:       jge    0x423863 <gteRTPS+787>                             
0x000000000042390e <gteRTPS+958>:       orl    $0x80002000,0x330a6c(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x0000000000423918 <gteRTPS+968>:       mov    $0xfffffc00,%ecx                                   
0x000000000042391d <gteRTPS+973>:       jmpq   0x423863 <gteRTPS+787>                             
0x0000000000423922 <gteRTPS+978>:       nopw   0x0(%rax,%rax,1)                                   
0x0000000000423928 <gteRTPS+984>:       cmp    $0xffffffff80000000,%rdx                           
0x000000000042392f <gteRTPS+991>:       jge    0x423844 <gteRTPS+756>                             
0x0000000000423935 <gteRTPS+997>:       orl    $0x80008000,0x330a45(%rip)        # 0x754384 <psxRegs+516>                                                                                           

0x000000000042393f <gteRTPS+1007>:      jmpq   0x423844 <gteRTPS+756>                             
0x0000000000423944 <gteRTPS+1012>:      nopl   0x0(%rax)                                          
0x0000000000423948 <gteRTPS+1016>:      cmp    $0xfffffc00,%edx                                   
0x000000000042394e <gteRTPS+1022>:      mov    %edx,%ecx                                          
0x0000000000423950 <gteRTPS+1024>:      jge    0x423811 <gteRTPS+705>                             
0x0000000000423956 <gteRTPS+1030>:      orl    $0x80004000,0x330a24(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x0000000000423960 <gteRTPS+1040>:      mov    $0xfffffc00,%ecx                                   
0x0000000000423965 <gteRTPS+1045>:      jmpq   0x423811 <gteRTPS+705>                             
0x000000000042396a <gteRTPS+1050>:      nopw   0x0(%rax,%rax,1)                                   
0x0000000000423970 <gteRTPS+1056>:      cmp    $0xffffffff80000000,%rdx                           
0x0000000000423977 <gteRTPS+1063>:      jge    0x4237f2 <gteRTPS+674>                             
0x000000000042397d <gteRTPS+1069>:      orl    $0x80008000,0x3309fd(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x0000000000423987 <gteRTPS+1079>:      jmpq   0x4237f2 <gteRTPS+674>                             
0x000000000042398c <gteRTPS+1084>:      nopl   0x0(%rax)                                          
0x0000000000423990 <gteRTPS+1088>:      test   %eax,%eax                                          
0x0000000000423992 <gteRTPS+1090>:      mov    %eax,%edx                                          
0x0000000000423994 <gteRTPS+1092>:      movzwl %ax,%ecx                                           
0x0000000000423997 <gteRTPS+1095>:      jns    0x42376a <gteRTPS+538>                             
0x000000000042399d <gteRTPS+1101>:      orl    $0x80040000,0x3309dd(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x00000000004239a7 <gteRTPS+1111>:      xor    %ecx,%ecx                                          
0x00000000004239a9 <gteRTPS+1113>:      xor    %edx,%edx                                          
0x00000000004239ab <gteRTPS+1115>:      jmpq   0x42376a <gteRTPS+538>                             
0x00000000004239b0 <gteRTPS+1120>:      cmp    $0xffff8000,%eax                                   
0x00000000004239b5 <gteRTPS+1125>:      jge    0x42371b <gteRTPS+459>                             
0x00000000004239bb <gteRTPS+1131>:      orl    $0x400000,0x3309bf(%rip)        # 0x754384 <psxRegs+516>                                                                                             
0x00000000004239c5 <gteRTPS+1141>:      mov    $0xffff8000,%edx                                   
0x00000000004239ca <gteRTPS+1146>:      jmpq   0x42371b <gteRTPS+459>                             
0x00000000004239cf <gteRTPS+1151>:      nop                                                       
0x00000000004239d0 <gteRTPS+1152>:      cmp    $0xffff8000,%eax                                   
0x00000000004239d5 <gteRTPS+1157>:      jge    0x4236f3 <gteRTPS+419>                             
0x00000000004239db <gteRTPS+1163>:      orl    $0x80800000,0x33099f(%rip)        # 0x754384 <psxRegs+516>                                                                                           
0x00000000004239e5 <gteRTPS+1173>:      mov    $0xffff8000,%eax                                   
0x00000000004239ea <gteRTPS+1178>:      jmpq   0x4236f3 <gteRTPS+419>                             
0x00000000004239ef <gteRTPS+1183>:      nop
0x00000000004239f0 <gteRTPS+1184>:      cmp    $0xffff8000,%eax
0x00000000004239f5 <gteRTPS+1189>:      jge    0x4236cd <gteRTPS+381>
0x00000000004239fb <gteRTPS+1195>:      orl    $0x81000000,0x33097f(%rip)        # 0x754384 <psxRegs+516>
0x0000000000423a05 <gteRTPS+1205>:      mov    $0xffff8000,%eax
0x0000000000423a0a <gteRTPS+1210>:      jmpq   0x4236cd <gteRTPS+381>
0x0000000000423a0f <gteRTPS+1215>:      nop
0x0000000000423a10 <gteRTPS+1216>:      cmp    $0xffffffff80000000,%rax
0x0000000000423a16 <gteRTPS+1222>:      jge    0x4236a7 <gteRTPS+343>
0x0000000000423a1c <gteRTPS+1228>:      orl    $0x82000000,0x33095e(%rip)        # 0x754384 <psxRegs+516>
0x0000000000423a26 <gteRTPS+1238>:      jmpq   0x4236a7 <gteRTPS+343>
0x0000000000423a2b <gteRTPS+1243>:      nopl   0x0(%rax,%rax,1)
0x0000000000423a30 <gteRTPS+1248>:      cmp    $0xffffffff80000000,%rdx
0x0000000000423a37 <gteRTPS+1255>:      jge    0x423649 <gteRTPS+249>
0x0000000000423a3d <gteRTPS+1261>:      orl    $0x84000000,0x33093d(%rip)        # 0x754384 <psxRegs+516>
0x0000000000423a47 <gteRTPS+1271>:      jmpq   0x423649 <gteRTPS+249>
0x0000000000423a4c <gteRTPS+1276>:      nopl   0x0(%rax)
0x0000000000423a50 <gteRTPS+1280>:      cmp    $0xffffffff80000000,%rsi
0x0000000000423a57 <gteRTPS+1287>:      jge    0x4235c8 <gteRTPS+120>
0x0000000000423a5d <gteRTPS+1293>:      movl   $0x88000000,0x33091d(%rip)        # 0x754384 <psxRegs+516>
0x0000000000423a67 <gteRTPS+1303>:      jmpq   0x4235c8 <gteRTPS+120>
0x0000000000423a6c <gteRTPS+1308>:      nopl   0x0(%rax)
0x0000000000423a70 <gteRTPS+1312>:      xor    %eax,%eax
0x0000000000423a72 <gteRTPS+1314>:      orl    $0x1000,0x330908(%rip)        # 0x754384 <psxRegs+516>
0x0000000000423a7c <gteRTPS+1324>:      mov    %eax,0x330826(%rip)        # 0x7542a8 <psxRegs+296>
0x0000000000423a82 <gteRTPS+1330>:      add    $0x8,%rsp

0x0000000000423a86 <gteRTPS+1334>:      retq
     */
	// MAC1
	MOV32MtoR(EAX, (uptr)&gteR11);
	IMUL32M((uptr)&gteVX0);
	MOV64RtoR(RSI, EAX);

	MOV32MtoR(EAX, (uptr)&gteR12);
	IMUL32M((uptr)&gteVY0);
	ADD64RtoR(RSI, EAX);

	MOV32MtoR(EAX, (uptr)&gteR13);
	IMUL32M((uptr)&gteVZ0);
	ADD64RtoR(RSI, EAX);

	MOV32MtoR(RAX, (uptr)&gteTRX);
	SHL64ItoR(RAX, 0xc);
	ADD64RtoR(RSI, RAX);
	//MOV64RtoR(EAX, ECX);
	SAR64ItoR(RSI, 0xc);
	A1(RSI);
	MOV32RtoM((uptr)&gteMAC1, EAX);
CALLFunc( (uptr) print_mac );
	/*  * MAC1 = -1207                                                                                   
 * MAC1 = 2249978                                                                                 
 * MAC1 = 2522827                                                                                 
 * MAC1 = 2189735                                                                                 
 * MAC1 = 1620546                                                                                 
 * MAC1 = 1803601                                                                                 
 * MAC1 = 1158610                                                                                 
 * MAC1 = 410453                                                                                  
 * MAC1 = 890                                                                                     
 * MAC1 = 1586666                                                                                 
 * MAC1 = 2776329                                                                                 
 * MAC1 = 1177447                                                                                 
 * MAC1 = 1010324                                                                                 
 * MAC1 = 654861                                                                                  
 * MAC1 = 590284                                                                                  
 * MAC1 = -1197                                                                                   
 * MAC1 = 2267077                                                                                 
 * MAC1 = 2634011                                                                                 
 * MAC1 = 2348694                                                                                 
 * MAC1 = 1626756                                                                                 
 * MAC1 = 1672397                                                                                 
 * MAC1 = 1457560                                                                                 
 * MAC1 = 401094          */
	// MAC2
	MOV32MtoR(EAX, (uptr)&gteR21);
	MUL32M((uptr)&gteVX1);
	MOV32RtoR(ECX, EAX);
	MOV32MtoR(EAX, (uptr)&gteR22);
	MUL32M((uptr)&gteVY1);
	ADD32RtoR(ECX, EAX);
	MOV32MtoR(EAX, (uptr)&gteR23);
	MUL32M((uptr)&gteVZ1);
	ADD32RtoR(ECX, EAX);
	MOV32MtoR(EAX, (uptr)&gteTRY);
	SHL32ItoR(EAX, 12);
	ADD32RtoR(ECX, EAX);
	MOV32RtoR(EAX, ECX);
	SHR32ItoR(EAX, 12);
	A2(EAX);
	MOV32RtoM((uptr)&gteMAC2, EAX);
	
	// MAC3
	MOV32MtoR(EAX, (uptr)&gteR31);
	MUL32M((uptr)&gteVX2);
	MOV32RtoR(ECX, EAX);
	MOV32MtoR(EAX, (uptr)&gteR32);
	MUL32M((uptr)&gteVY2);
	ADD32RtoR(ECX, EAX);
	MOV32MtoR(EAX, (uptr)&gteR33);
	MUL32M((uptr)&gteVZ2);
	ADD32RtoR(ECX, EAX);
	MOV32MtoR(EAX, (uptr)&gteTRZ);
	SHL32ItoR(EAX, 12);
	ADD32RtoR(ECX, EAX);
	SHR32ItoR(ECX, 12);
	A3(ECX);
	MOV32RtoM((uptr)&gteMAC3, ECX);
	
	MOV32MtoR(EAX, (uptr)&gteMAC1);
	Lm_B1(EAX, gteIR1, 0);
	
	MOV32MtoR(EAX, (uptr)&gteMAC2);
	Lm_B2(EAX, gteIR2, 0);
	
	MOV32MtoR(EAX, (uptr)&gteMAC3);
	Lm_B3(EAX, gteIR3, 0);
	
	MOV32MtoR(EAX, (uptr)&gteSZ1);
	MOV32RtoM((uptr)&gteSZ0, EAX);
	MOV32MtoR(EAX, (uptr)&gteSZ2);
	MOV32RtoM((uptr)&gteSZ1, EAX);
	MOV32MtoR(EAX, (uptr)&gteSZ3);
	MOV32RtoM((uptr)&gteSZ2, EAX);
	
	MOV32MtoR(EAX, (uptr)&gteSXY1);
	MOV32RtoM((uptr)&gteSXY0, EAX);
	MOV32MtoR(EAX, (uptr)&gteSXY2);
	MOV32RtoM((uptr)&gteSXY1, EAX);

	Lm_D(ECX, gteSZ3);

	MOV32MtoR(EAX, (uptr)&gteH);
	MOV32ItoR(ECX, 65536);
	MUL32R(ECX);
	MOV32MtoR(ECX, (uptr)&gteSZ3);
	ADD32ItoR(ECX, 0.5);
	DIV32R(ECX);
	Lm_E(EAX);
	
	MOV32RtoR(ECX, EAX);
	
	MOV32MtoR(EBX, (uptr)&gteIR1);
	MUL32R(EBX);
	MOV32MtoR(EBX, (uptr)&gteOFX);
	ADD32RtoR(EAX, EBX);
	SHR32ItoR(EAX, 16);
	F(EAX);
	Lm_G1(EAX, gteSX2);
	
	MOV32RtoR(EAX, ECX);

	MOV32MtoR(EBX, (uptr)&gteIR2);
	MUL32R(EBX);
	MOV32MtoR(EBX, (uptr)&gteOFY);
	ADD32RtoR(EAX, EBX);
	SHR32ItoR(EAX, 16);
	F(EAX);
	Lm_G2(EAX, gteSY2);
	
	MOV32RtoR(EAX, ECX);
	MOV32MtoR(EBX, (uptr)&gteDQA);
	SHL32ItoR(EBX, 8);
	MUL32R(EBX);
	SHR32ItoR(EAX, 8);
	MOV32MtoR(EBX, (uptr)&gteDQB);
	ADD32RtoR(EAX, EBX);
	SHR32ItoR(EAX, 4);
	F(EAX);

	Lm_H(EAX, gteIR0);
	MOV32RtoM((uptr)&gteMAC0, EAX);
/*
	gteMAC1 = A1( ( ( (s64) gteTRX << 12 ) + ( gteR11 * gteVX0 ) + ( gteR12 * gteVY0 ) + ( gteR13 * gteVZ0 ) ) >> 12 );
	gteMAC2 = A2( ( ( (s64) gteTRY << 12 ) + ( gteR21 * gteVX0 ) + ( gteR22 * gteVY0 ) + ( gteR23 * gteVZ0 ) ) >> 12 );
	gteMAC3 = A3( ( ( (s64) gteTRZ << 12 ) + ( gteR31 * gteVX0 ) + ( gteR32 * gteVY0 ) + ( gteR33 * gteVZ0 ) ) >> 12 );
	gteIR1 = Lm_B1( gteMAC1, 0 );
	gteIR2 = Lm_B2( gteMAC2, 0 );
	gteIR3 = Lm_B3( gteMAC3, 0 );
	gteSZ0 = gteSZ1;
	gteSZ1 = gteSZ2;
	gteSZ2 = gteSZ3;
	gteSZ3 = Lm_D( gteMAC3 );
	h_over_sz3 = Lm_E((gteH * 65536) / (gteSZ3 + 0.5));
	gteSXY0 = gteSXY1;
	gteSXY1 = gteSXY2;
	gteSX2 = Lm_G1( F( (s64) gteOFX + ( (s64) gteIR1 * h_over_sz3 ) ) >> 16 );
	gteSY2 = Lm_G2( F( (s64) gteOFY + ( (s64) gteIR2 * h_over_sz3 ) ) >> 16 );
	gteMAC0 = F( ((s64) gteDQB + (((s64) ((s64) gteDQA << 8) * h_over_sz3 ) >> 8) ) >> 4 );
	gteIR0 = Lm_H( gteMAC0 );
*/
}
#endif

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
	TEST32ItoM((uptr)&gteFLAG, 0x7F87E000); \
	j8Ptr[0] = JZ8(0); \
	OR32ItoM((uptr)&gteFLAG, 0x80000000); \
 \
 	x86SetJ8(j8Ptr[0]); \
}

#define LIM32X8(reg, gteout, negv, posv, flagb) { \
 	CMP32ItoR(reg, negv); \
	j8Ptr[0] = JL8(0); \
 	CMP32ItoR(reg, posv); \
	j8Ptr[1] = JG8(0); \
 \
	MOV8RtoM((uptr)&gteout, reg); \
	j8Ptr[2] = JMP8(0); \
 \
 	x86SetJ8(j8Ptr[0]); \
	MOV8ItoM((uptr)&gteout, negv); \
	j8Ptr[3] = JMP8(0); \
 \
 	x86SetJ8(j8Ptr[1]); \
	MOV8ItoM((uptr)&gteout, posv); \
 \
	x86SetJ8(j8Ptr[3]); \
	OR32ItoM((uptr)&gteFLAG, 1<<flagb); \
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
	MOV32RtoM((uptr)&ir, reg); \
/*	j8Ptr[2] = JMP8(0);*/ \
 \
/*	x86SetJ8(j8Ptr[0]);*/ \
/*	MOV32ItoM((uptr)&ir, negv);*/ \
/*	j8Ptr[3] = JMP8(0);*/ \
 \
/*	x86SetJ8(j8Ptr[1]);*/ \
/*	MOV32ItoM((uptr)&ir, posv);*/ \
 \
/*	x86SetJ8(j8Ptr[3]);*/ \
/*	OR32ItoR((uptr)&gteFLAG, 1<<flagb);*/ \
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
	MOVSX32M16toR(EAX, (uptr)&mx##vn##1); \
	IMUL32R(EBX); \
/*	j8Ptr[0] = JO8(0);*/ \
	MOV32RtoR(ECX, EAX); \
 \
	MOVSX32M16toR(EAX, (uptr)&mx##vn##2); \
	IMUL32R(EDI); \
/*	j8Ptr[1] = JO8(0);*/ \
	ADD32RtoR(ECX, EAX); \
/*	j8Ptr[2] = JO8(0);*/ \
 \
	MOVSX32M16toR(EAX, (uptr)&mx##vn##3); \
	IMUL32R(ESI); \
/*	j8Ptr[3] = JO8(0);*/ \
	ADD32RtoR(ECX, EAX); \
/*	j8Ptr[4] = JO8(0);*/ \
}

/*	SSX = (_v0) * mx##11 + (_v1) * mx##12 + (_v2) * mx##13; 
    SSY = (_v0) * mx##21 + (_v1) * mx##22 + (_v2) * mx##23; 
    SSZ = (_v0) * mx##31 + (_v1) * mx##32 + (_v2) * mx##33; */

#define _MVMVA_ADD(_vx, jn) { \
	ADD32MtoR(ECX, (uptr)&_vx); \
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
	MOVSX32M16toR(EBX, (uptr)&_v0); \
	MOVSX32M16toR(EDI, (uptr)&_v1); \
	MOVSX32M16toR(ESI, (uptr)&_v2); \
}

static void recMVMVA() {
	int i;

//	SysPrintf("GTE_MVMVA %lx\n", psxRegs.code & 0x1ffffff);

/*	PUSH32R(ESI);
	PUSH32R(EDI);
	PUSH32R(EBX);
*/
	XOR32RtoR(EAX, EAX); /* gteFLAG = 0 */
	MOV32RtoM((uptr)&gteFLAG, EAX);

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
	OR32ItoM((uptr)&gteFLAG, 1<<29);
	x86SetJ8(j8Ptr[9]);*/
	MOV32RtoM((uptr)&gteMAC1, ECX);

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
	MOV32RtoM((uptr)&gteMAC2, ECX);

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
	MOV32RtoM((uptr)&gteMAC3, ECX);

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
	MOV32MtoR(EAX, (uptr)&gteIR##vn); \
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
	MOV32MtoR(ECX, (uptr)&gteIR0);
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
	MOV32MtoR(EDX, (uptr)&gteRGB1);
	MOV32MtoR(ECX, (uptr)&gteRGB2);
	MOV32RtoM((uptr)&gteRGB0, EDX);
	MOV32RtoM((uptr)&gteRGB1, ECX);

	POP32R(EDX);
	POP32R(ECX);
	SAR32ItoR(ECX, 4);
	SAR32ItoR(EDX, 4);
	SAR32ItoR(EAX, 4);

	_LIM_B1(ECX, gteR2);
	_LIM_B2(EDX, gteG2);
	_LIM_B3(EAX, gteB2);
	MOV8MtoR(EAX, (uptr)&gteCODE);
	MOV8RtoM((uptr)&gteCODE2, EAX);

/*	gteR2 = limB1(gteMAC1 / 16.0f);
	gteG2 = limB2(gteMAC2 / 16.0f);
	gteB2 = limB3(gteMAC3 / 16.0f); gteCODE2 = gteCODE;*/

	SUM_FLAG();
	MOV32RtoM((uptr)&gteFLAG, EBX);

//	POP32R(EBX);
}
#endif
#endif


#endif /* __IGTE_H__ */
