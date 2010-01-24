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

//#include <stdio.h>
//#include <stdlib.h>
//#include <math.h>
#include "PsxCommon.h"
#include "Gte.h"
#include "R3000A.h"

#undef _Op_
#define _Op_     _fOp_(psxRegs.code)
#undef _Funct_
#define _Funct_  _fFunct_(psxRegs.code)
#undef _Rd_
#define _Rd_     _fRd_(psxRegs.code)
#undef _Rt_
#define _Rt_     _fRt_(psxRegs.code)
#undef _Rs_
#define _Rs_     _fRs_(psxRegs.code)
#undef _Sa_
#define _Sa_     _fSa_(psxRegs.code)
#undef _Im_
#define _Im_     _fIm_(psxRegs.code)
#undef _Target_
#define _Target_ _fTarget_(psxRegs.code)

#undef _Imm_
#define _Imm_	 _fImm_(psxRegs.code)
#undef _ImmU_
#define _ImmU_	 _fImmU_(psxRegs.code)

#ifdef _MSC_VER_
#pragma warning(disable:4244)
#pragma warning(disable:4761)
#endif

#ifdef __GAMECUBE__ //__MACOSX__
#define SEL16(n) ((n)^1)
#define SEL8(n) ((n)^3)
#else
#define SEL16(n) (n)
#define SEL8(n) (n)
#endif

#define VX( n ) ( n < 3 ? ((s16*)psxRegs.CP2D.r)[SEL16( (n << 2) )] : gteIR1 )
#define VY( n ) ( n < 3 ? ((s16*)psxRegs.CP2D.r)[SEL16( (n << 2) + 1 )] : gteIR2 )
#define VZ( n ) ( n < 3 ? ((s16*)psxRegs.CP2D.r)[SEL16( (n << 2) + 2 )] : gteIR3 )
#define MX11( n ) ( n < 3 ? ((s16*)psxRegs.CP2C.r)[ SEL16( n << 4 ) ] : 0 )
#define MX12( n ) ( n < 3 ? ((s16*)psxRegs.CP2C.r)[ SEL16(( n << 4 ) + 1) ] : 0 )
#define MX13( n ) ( n < 3 ? ((s16*)psxRegs.CP2C.r)[ SEL16(( n << 4 ) + 2) ] : 0 )
#define MX21( n ) ( n < 3 ? ((s16*)psxRegs.CP2C.r)[ SEL16(( n << 4 ) + 3) ] : 0 )
#define MX22( n ) ( n < 3 ? ((s16*)psxRegs.CP2C.r)[ SEL16(( n << 4 ) + 4) ] : 0 )
#define MX23( n ) ( n < 3 ? ((s16*)psxRegs.CP2C.r)[ SEL16(( n << 4 ) + 5) ] : 0 )
#define MX31( n ) ( n < 3 ? ((s16*)psxRegs.CP2C.r)[ SEL16(( n << 4 ) + 6) ] : 0 )
#define MX32( n ) ( n < 3 ? ((s16*)psxRegs.CP2C.r)[ SEL16(( n << 4 ) + 7) ] : 0 )
#define MX33( n ) ( n < 3 ? ((s16*)psxRegs.CP2C.r)[ SEL16(( n << 4 ) + 8) ] : 0 )

#define CV1( n ) ( n < 3 ? ((s32*)psxRegs.CP2C.r)[ ( n << 3 ) + 5 ] : 0 )
#define CV2( n ) ( n < 3 ? ((s32*)psxRegs.CP2C.r)[ ( n << 3 ) + 6 ] : 0 )
#define CV3( n ) ( n < 3 ? ((s32*)psxRegs.CP2C.r)[ ( n << 3 ) + 7 ] : 0 )

#define fSX( n ) ((u16*)psxRegs.CP2D.r)[ SEL16( (n + 12) * 2 )]
#define fSY( n ) ((u16*)psxRegs.CP2D.r)[ SEL16( ((n + 12) * 2) + 1 )]
#define fSZ( n ) ((u16*)psxRegs.CP2D.r)[ SEL16( (n + 17) * 2 )]

#define gteVX0     ((s16*)psxRegs.CP2D.r)[SEL16(0)]
#define gteVY0     ((s16*)psxRegs.CP2D.r)[SEL16(1)]
#define gteVZ0     ((s16*)psxRegs.CP2D.r)[SEL16(2)]
#define gteVX1     ((s16*)psxRegs.CP2D.r)[SEL16(4)]
#define gteVY1     ((s16*)psxRegs.CP2D.r)[SEL16(5)]
#define gteVZ1     ((s16*)psxRegs.CP2D.r)[SEL16(6)]
#define gteVX2     ((s16*)psxRegs.CP2D.r)[SEL16(8)]
#define gteVY2     ((s16*)psxRegs.CP2D.r)[SEL16(9)]
#define gteVZ2     ((s16*)psxRegs.CP2D.r)[SEL16(10)]
#define gteRGB     psxRegs.CP2D.r[6]
#define gteOTZ     ((s16*)psxRegs.CP2D.r)[SEL16(7*2)]
#define gteIR0     ((s32*)psxRegs.CP2D.r)[8]
#define gteIR1     ((s32*)psxRegs.CP2D.r)[9]
#define gteIR2     ((s32*)psxRegs.CP2D.r)[10]
#define gteIR3     ((s32*)psxRegs.CP2D.r)[11]
#define gteSXY0    ((s32*)psxRegs.CP2D.r)[12]
#define gteSXY1    ((s32*)psxRegs.CP2D.r)[13]
#define gteSXY2    ((s32*)psxRegs.CP2D.r)[14]
#define gteSXYP    ((s32*)psxRegs.CP2D.r)[15]
#define gteSX0     ((s16*)psxRegs.CP2D.r)[SEL16(12*2)]
#define gteSY0     ((s16*)psxRegs.CP2D.r)[SEL16(12*2+1)]
#define gteSX1     ((s16*)psxRegs.CP2D.r)[SEL16(13*2)]
#define gteSY1     ((s16*)psxRegs.CP2D.r)[SEL16(13*2+1)]
#define gteSX2     ((s16*)psxRegs.CP2D.r)[SEL16(14*2)]
#define gteSY2     ((s16*)psxRegs.CP2D.r)[SEL16(14*2+1)]
#define gteSXP     ((s16*)psxRegs.CP2D.r)[SEL16(15*2)]
#define gteSYP     ((s16*)psxRegs.CP2D.r)[SEL16(15*2+1)]
#define gteSZ0     ((u16*)psxRegs.CP2D.r)[SEL16(16*2)]
#define gteSZ1     ((u16*)psxRegs.CP2D.r)[SEL16(17*2)]
#define gteSZ2     ((u16*)psxRegs.CP2D.r)[SEL16(18*2)]
#define gteSZ3     ((u16*)psxRegs.CP2D.r)[SEL16(19*2)]
#define gteRGB0    psxRegs.CP2D.r[20]
#define gteRGB1    psxRegs.CP2D.r[21]
#define gteRGB2    psxRegs.CP2D.r[22]
#define gteMAC0    psxRegs.CP2D.r[24]
#define gteMAC1    ((s32*)psxRegs.CP2D.r)[25]
#define gteMAC2    ((s32*)psxRegs.CP2D.r)[26]
#define gteMAC3    ((s32*)psxRegs.CP2D.r)[27]
#define gteIRGB    psxRegs.CP2D.r[28]
#define gteORGB    psxRegs.CP2D.r[29]
#define gteLZCS    psxRegs.CP2D.r[30]
#define gteLZCR    psxRegs.CP2D.r[31]

#define gteR       ((u8 *)psxRegs.CP2D.r)[SEL8(6*4)]
#define gteG       ((u8 *)psxRegs.CP2D.r)[SEL8(6*4+1)]
#define gteB       ((u8 *)psxRegs.CP2D.r)[SEL8(6*4+2)]
#define gteCODE    ((u8 *)psxRegs.CP2D.r)[SEL8(6*4+3)]
#define gteC       gteCODE

#define gteR0      ((u8 *)psxRegs.CP2D.r)[SEL8(20*4)]
#define gteG0      ((u8 *)psxRegs.CP2D.r)[SEL8(20*4+1)]
#define gteB0      ((u8 *)psxRegs.CP2D.r)[SEL8(20*4+2)]
#define gteCODE0   ((u8 *)psxRegs.CP2D.r)[SEL8(20*4+3)]
#define gteC0      gteCODE0

#define gteR1      ((u8 *)psxRegs.CP2D.r)[SEL8(21*4)]
#define gteG1      ((u8 *)psxRegs.CP2D.r)[SEL8(21*4+1)]
#define gteB1      ((u8 *)psxRegs.CP2D.r)[SEL8(21*4+2)]
#define gteCODE1   ((u8 *)psxRegs.CP2D.r)[SEL8(21*4+3)]
#define gteC1      gteCODE1

#define gteR2      ((u8 *)psxRegs.CP2D.r)[SEL8(22*4)]
#define gteG2      ((u8 *)psxRegs.CP2D.r)[SEL8(22*4+1)]
#define gteB2      ((u8 *)psxRegs.CP2D.r)[SEL8(22*4+2)]
#define gteCODE2   ((u8 *)psxRegs.CP2D.r)[SEL8(22*4+3)]
#define gteC2      gteCODE2



#define gteR11  ((s16*)psxRegs.CP2C.r)[SEL16(0)]
#define gteR12  ((s16*)psxRegs.CP2C.r)[SEL16(1)]
#define gteR13  ((s16*)psxRegs.CP2C.r)[SEL16(2)]
#define gteR21  ((s16*)psxRegs.CP2C.r)[SEL16(3)]
#define gteR22  ((s16*)psxRegs.CP2C.r)[SEL16(4)]
#define gteR23  ((s16*)psxRegs.CP2C.r)[SEL16(5)]
#define gteR31  ((s16*)psxRegs.CP2C.r)[SEL16(6)]
#define gteR32  ((s16*)psxRegs.CP2C.r)[SEL16(7)]
#define gteR33  ((s16*)psxRegs.CP2C.r)[SEL16(8)]
#define gteTRX  ((s32*)psxRegs.CP2C.r)[5]
#define gteTRY  ((s32*)psxRegs.CP2C.r)[6]
#define gteTRZ  ((s32*)psxRegs.CP2C.r)[7]
#define gteL11  ((s16*)psxRegs.CP2C.r)[SEL16(16)]
#define gteL12  ((s16*)psxRegs.CP2C.r)[SEL16(17)]
#define gteL13  ((s16*)psxRegs.CP2C.r)[SEL16(18)]
#define gteL21  ((s16*)psxRegs.CP2C.r)[SEL16(19)]
#define gteL22  ((s16*)psxRegs.CP2C.r)[SEL16(20)]
#define gteL23  ((s16*)psxRegs.CP2C.r)[SEL16(21)]
#define gteL31  ((s16*)psxRegs.CP2C.r)[SEL16(22)]
#define gteL32  ((s16*)psxRegs.CP2C.r)[SEL16(23)]
#define gteL33  ((s16*)psxRegs.CP2C.r)[SEL16(24)]
#define gteRBK  ((s32*)psxRegs.CP2C.r)[13]
#define gteGBK  ((s32*)psxRegs.CP2C.r)[14]
#define gteBBK  ((s32*)psxRegs.CP2C.r)[15]
#define gteLR1  ((s16*)psxRegs.CP2C.r)[SEL16(32)]
#define gteLR2  ((s16*)psxRegs.CP2C.r)[SEL16(33)]
#define gteLR3  ((s16*)psxRegs.CP2C.r)[SEL16(34)]
#define gteLG1  ((s16*)psxRegs.CP2C.r)[SEL16(35)]
#define gteLG2  ((s16*)psxRegs.CP2C.r)[SEL16(36)]
#define gteLG3  ((s16*)psxRegs.CP2C.r)[SEL16(37)]
#define gteLB1  ((s16*)psxRegs.CP2C.r)[SEL16(38)]
#define gteLB2  ((s16*)psxRegs.CP2C.r)[SEL16(39)]
#define gteLB3  ((s16*)psxRegs.CP2C.r)[SEL16(40)]
#define gteRFC  ((s32*)psxRegs.CP2C.r)[21]
#define gteGFC  ((s32*)psxRegs.CP2C.r)[22]
#define gteBFC  ((s32*)psxRegs.CP2C.r)[23]
#define gteOFX  ((s32*)psxRegs.CP2C.r)[24]
#define gteOFY  ((s32*)psxRegs.CP2C.r)[25]
#define gteH    ((u16*)psxRegs.CP2C.r)[SEL16(52)]
#define gteDQA  ((s16*)psxRegs.CP2C.r)[SEL16(54)]
#define gteDQB  ((s32*)psxRegs.CP2C.r)[28]
#define gteZSF3 ((s16*)psxRegs.CP2C.r)[SEL16(58)]
#define gteZSF4 ((s16*)psxRegs.CP2C.r)[SEL16(60)]
#define gteFLAG psxRegs.CP2C.r[31]

#define GTE_OP( op ) ( ( op >> 20 ) & 31 )
#define GTE_SF( op ) ( ( op >> 19 ) & 1 )
#define GTE_MX( op ) ( ( op >> 17 ) & 3 )
#define GTE_V( op ) ( ( op >> 15 ) & 3 )
#define GTE_CV( op ) ( ( op >> 13 ) & 3 )
#define GTE_CD( op ) ( ( op >> 11 ) & 3 ) /* not used */
#define GTE_LM( op ) ( ( op >> 10 ) & 1 )
#define GTE_CT( op ) ( ( op >> 6 ) & 15 ) /* not used */
#define GTE_FUNCT( op ) ( op & 63 )
#define INS_COFUN( op ) ( op & 0x1ffffff )

#define gteop ( INS_COFUN( psxRegs.code ) )

static inline s64 BOUNDS( s64 n_value, s64 n_max, int n_maxflag, s64 n_min, int n_minflag )
{
	if( n_value > n_max )
	{
		gteFLAG |= n_maxflag;
	}
	else if( n_value < n_min )
	{
		gteFLAG |= n_minflag;
	}
	return n_value;
}

static inline s32 LIM( s32 value, s32 max, s32 min, u32 flag )
{
	s32 ret = value;
	if( value > max )
	{
		gteFLAG |= flag;
		ret = max;
	}
	else if( value < min )
	{
		gteFLAG |= flag;
		ret = min;
	}
	return ret;
}

#define A1( a ) BOUNDS( ( a ), 0x7fffffff, ( 1 << 30 ), -(s64)0x80000000, ( 1 << 31 ) | ( 1 << 27 ) )
#define A2( a ) BOUNDS( ( a ), 0x7fffffff, ( 1 << 29 ), -(s64)0x80000000, ( 1 << 31 ) | ( 1 << 26 ) )
#define A3( a ) BOUNDS( ( a ), 0x7fffffff, ( 1 << 28 ), -(s64)0x80000000, ( 1 << 31 ) | ( 1 << 25 ) )
#define Lm_B1( a, l ) LIM( ( a ), 0x7fff, -0x8000 * !l, ( 1 << 31 ) | ( 1 << 24 ) )
#define Lm_B2( a, l ) LIM( ( a ), 0x7fff, -0x8000 * !l, ( 1 << 31 ) | ( 1 << 23 ) )
#define Lm_B3( a, l ) LIM( ( a ), 0x7fff, -0x8000 * !l, ( 1 << 22 ) )
#define Lm_C1( a ) LIM( ( a ), 0x00ff, 0x0000, ( 1 << 21 ) )
#define Lm_C2( a ) LIM( ( a ), 0x00ff, 0x0000, ( 1 << 20 ) )
#define Lm_C3( a ) LIM( ( a ), 0x00ff, 0x0000, ( 1 << 19 ) )
#define Lm_D( a ) LIM( ( a ), 0xffff, 0x0000, ( 1 << 31 ) | ( 1 << 18 ) )

static inline u32 Lm_E( u32 result )
{
	if( result > 0x1ffff )
	{
		gteFLAG |= ( 1 << 31 ) | ( 1 << 17 );
		return 0x1ffff;
	}

	return result;
}

#define F( a ) BOUNDS( ( a ), 0x7fffffff, ( 1 << 31 ) | ( 1 << 16 ), -(s64)0x80000000, ( 1 << 31 ) | ( 1 << 15 ) )
#define Lm_G1( a ) LIM( ( a ), 0x3ff, -0x400, ( 1 << 31 ) | ( 1 << 14 ) )
#define Lm_G2( a ) LIM( ( a ), 0x3ff, -0x400, ( 1 << 31 ) | ( 1 << 13 ) )
#define Lm_H( a ) LIM( ( a ), 0xfff, 0x000, ( 1 << 12 ) )

__inline u32 MFC2(int reg) {
	switch(reg) {
		case 1:
		case 3:
		case 5:
		case 8:
		case 9:
		case 10:
		case 11:
			psxRegs.CP2D.r[ reg ] = (s32)(s16)psxRegs.CP2D.r[ reg ];
			break;

		case 7:
		case 16:
		case 17:
		case 18:
		case 19:
			psxRegs.CP2D.r[ reg ] = (u32)(u16)psxRegs.CP2D.r[ reg ];
			break;

		case 15:
			psxRegs.CP2D.r[ reg ] = gteSXY2;
			break;

		case 28: case 30:
			SysPrintf("MFC2: psxRegs.CP2D.r[%d] cannot be read\n");
			return 0;

		case 29:
			psxRegs.CP2D.r[ reg ] = LIM( gteIR1 >> 7, 0x1f, 0, 0 ) | 
									( LIM( gteIR2 >> 7, 0x1f, 0, 0 ) << 5 ) | 
									( LIM( gteIR3 >> 7, 0x1f, 0, 0 ) << 10 );
			break;
	}
	return psxRegs.CP2D.r[reg];
}

__inline void MTC2(unsigned long value, int reg) {
	switch(reg) {

		case 8: case 9: case 10: case 11:
			psxRegs.CP2D.r[reg] = (short)value;
			break;

		case 15:
			gteSXY0 = gteSXY1;
			gteSXY1 = gteSXY2;
			gteSXY2 = value;
			gteSXYP = value;
			break;

		case 16: case 17: case 18: case 19:
			psxRegs.CP2D.r[reg] = (value & 0xffff);
			break;
			
		case 28:
			gteIRGB = value;
			gteIR1 = ( value & 0x1f ) << 7;
			gteIR2 = ( value & 0x3e0 ) << 2;
			gteIR3 = ( value & 0x7c00 ) >> 3;
			break;

		case 30:
			gteLZCS = value;

			int a = gteLZCS;
#if defined(_MSC_VER_)
			if (a > 0) {
				__asm {
					mov eax, a;
					bsr eax, eax;
					mov a, eax;
				}
				gteLZCR = 31 - a;
			} else if (a < 0) {
				__asm {
					mov eax, a;
					xor eax, 0xffffffff;
					bsr eax, eax;
					mov a, eax;
				}
				gteLZCR = 31 - a;
			} else {
				gteLZCR = 32;
			}
#elif defined(__LINUX__) || defined(__MINGW32__)
			if (a > 0) {
				__asm__ ("bsrl %1, %0\n" : "=r"(a) : "r"(a) );
				gteLZCR = 31 - a;
			} else if (a < 0) {
				a^= 0xffffffff;
				__asm__ ("bsrl %1, %0\n" : "=r"(a) : "r"(a) );
				gteLZCR = 31 - a;
			} else {
				gteLZCR = 32;
			}
#elif defined(__ppc__)
			__asm__ ("cntlzw %0, %1" : "=r" (a) : "r"(a));
			gteLZCR = a;
#else
			if (a > 0) {
				int i;
				for (i=31; (a & (1 << i)) == 0 && i >= 0; i--);
				gteLZCR = 31 - i;
			} else if (a < 0) {
				int i;
				a^= 0xffffffff;
				for (i=31; (a & (1 << i)) == 0 && i >= 0; i--);
				gteLZCR = 31 - i;
			} else {
				gteLZCR = 32;
			}
#endif
			break;
			
		case 7: case 29: case 31:
			SysPrintf("MTC2: psxRegs.CP2D.r[%d] cannot be write\n", reg);
			return;
		
		default:
			psxRegs.CP2D.r[reg] = value;
	}
}

static void setcp2cr( int reg, u32 value )
{
	switch( reg )
	{
	case 4:
	case 12:
	case 20:
	case 26:
	case 27:
	case 29:
	case 30:
		value = (s32)(s16) value;
		break;

	case 31:
		SysPrintf("setcp2cr: reg=%d\n", reg);
		value = value & 0x7ffff000;
		if( ( value & 0x7f87e000 ) != 0 )
		{
			value |= 0x80000000;
		}
		break;
	}

	psxRegs.CP2C.r[ reg ] = value;
}

void gteMFC2() {
	if (!_Rt_) return;
	_rRt_ = MFC2(_Rd_);
}

void gteCFC2() {
	if (!_Rt_) return;
	_rRt_ = psxRegs.CP2C.r[_Rd_];
}

void gteMTC2() {
	MTC2(_rRt_, _Rd_);
}

void gteCTC2() {
	setcp2cr( _Rd_, _rRt_ );
}

#define _oB_ (psxRegs.GPR.r[_Rs_] + _Imm_)

void gteLWC2() {
	MTC2(psxMemRead32(_oB_), _Rt_);        
}

void gteSWC2() {
	psxMemWrite32(_oB_, MFC2(_Rt_));
}

void gteRTPS() {
	
if( gteop != 0x0180001 )
{
	SysPrintf("RTPS - error\n");
}

#ifdef GTE_LOG
	GTE_LOG("GTE_RTPS\n");
#endif

	gteFLAG = 0;

	s32 h_over_sz3;

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
/*  // old method - Ridge Racer 4 broken.
	gteMAC0 = F( ((s64) gteDQB + (((s64) ((s64) gteDQA << 8) * h_over_sz3 ) >> 8)) );
	gteIR0 = Lm_H( gteMAC0 );
*/
	gteMAC0 = F( ((s64) gteDQB + (((s64) ((s64) gteDQA << 8) * h_over_sz3 ) >> 8) ) >> 4 );
	gteIR0 = Lm_H( gteMAC0 );
}

void gteRTPT() {


#ifdef GTE_LOG
	GTE_LOG("GTE_RTPT\n");
#endif

	gteFLAG = 0;

	s32 h_over_sz3;
	int v;

	gteSZ0 = gteSZ3;
	s32 vx, vy, vz;
	for( v = 0; v < 3; v++ )
	{
		vx = VX( v );
		vy = VY( v );
		vz = VZ( v );
		gteMAC1 = A1( ( ( (s64) gteTRX << 12 ) + ( gteR11 * vx ) + ( gteR12 * vy ) + ( gteR13 * vz ) ) >> 12 );
		gteMAC2 = A2( ( ( (s64) gteTRY << 12 ) + ( gteR21 * vx ) + ( gteR22 * vy ) + ( gteR23 * vz ) ) >> 12 );
		gteMAC3 = A3( ( ( (s64) gteTRZ << 12 ) + ( gteR31 * vx ) + ( gteR32 * vy ) + ( gteR33 * vz ) ) >> 12 );
		gteIR1 = Lm_B1( gteMAC1, 0 );
		gteIR2 = Lm_B2( gteMAC2, 0 );
		gteIR3 = Lm_B3( gteMAC3, 0 );
		fSZ( v ) = Lm_D( gteMAC3 );
		h_over_sz3 = Lm_E(gteH * 65536 / (fSZ( v ) + 0.5));
		fSX( v ) = Lm_G1( F( ( (s64) gteOFX + ( (s64) gteIR1 * h_over_sz3 ) ) >> 16 ) );
		fSY( v ) = Lm_G2( F( ( (s64) gteOFY + ( (s64) gteIR2 * h_over_sz3 ) ) >> 16 ) );
	}
/*  // old method - Ridge Racer 4 broken.
	gteMAC0 = F( ((s64) gteDQB + (((s64) ((s64) gteDQA << 8) * h_over_sz3 ) >> 8)) );
	gteIR0 = Lm_H( gteMAC0 );
*/
	gteMAC0 = F( ((s64) gteDQB + (((s64) ((s64) gteDQA << 8) * h_over_sz3 ) >> 8) ) >> 4 ); // Seems ok, but need more games to test.
	gteIR0 = Lm_H( gteMAC0 );
}

void gteMVMVA() {

#ifdef GTE_LOG
	GTE_LOG("GTE_MVMVA %lx\n", psxRegs.code & 0x1ffffff);
#endif

	if( GTE_OP( gteop ) != 0x04 )
	{
		printf("MVMVA - Error\n");
		//return;
	}

	int shift = 12 * GTE_SF( gteop );
	int mx = GTE_MX( gteop );
	int v = GTE_V( gteop );
	int cv = GTE_CV( gteop );
	int lm = GTE_LM( gteop );
	
	s32 vx = VX( v );
	s32 vy = VY( v );
	s32 vz = VZ( v );

	gteMAC1 = A1(( ( (s64) CV1( cv ) << 12 ) + ( MX11( mx ) * vx ) + ( MX12( mx ) * vy ) + ( MX13( mx ) * vz ) ) >> shift);
	gteMAC2 = A2(( ( (s64) CV2( cv ) << 12 ) + ( MX21( mx ) * vx ) + ( MX22( mx ) * vy ) + ( MX23( mx ) * vz ) ) >> shift);
	gteMAC3 = A3(( ( (s64) CV3( cv ) << 12 ) + ( MX31( mx ) * vx ) + ( MX32( mx ) * vy ) + ( MX33( mx ) * vz ) ) >> shift);

	gteFLAG = 0;

	gteIR1 = Lm_B1( gteMAC1, lm );
	gteIR2 = Lm_B2( gteMAC2, lm );
	gteIR3 = Lm_B3( gteMAC3, lm );
}

void gteNCLIP() {

#ifdef GTE_LOG
	GTE_LOG("GTE_NCLIP\n");
#endif

	gteFLAG = 0;
	
	gteMAC0 = F( (s64)gteSX0 * (gteSY1 - gteSY2) +
				gteSX1 * (gteSY2 - gteSY0) +
				gteSX2 * (gteSY0 - gteSY1) );
}

void gteAVSZ3() {

#ifdef GTE_LOG
	GTE_LOG("GTE_AVSZ3\n");
#endif
	gteFLAG = 0;

	gteMAC0 = F(((gteSZ1 + gteSZ2 + gteSZ3) * (gteZSF3)) >> 12);
	gteOTZ = Lm_D(gteMAC0);
}

void gteAVSZ4() {

#ifdef GTE_LOG
	GTE_LOG("GTE_AVSZ4\n");
#endif
	gteFLAG = 0;

	gteMAC0 = F(((gteSZ0 + gteSZ1 + gteSZ2 + gteSZ3) * (gteZSF4)) >> 12);
	gteOTZ = Lm_D(gteMAC0);
}

void gteSQR() {

#ifdef GTE_LOG
	GTE_LOG("GTE_SQR %lx\n", psxRegs.code & 0x1ffffff);
#endif

	gteFLAG = 0;

	int shift = 12 * GTE_SF( gteop );
	int lm = GTE_LM( gteop );

	gteMAC1 = A1( ( gteIR1 * gteIR1 ) >> shift );
	gteMAC2 = A2( ( gteIR2 * gteIR2 ) >> shift );
	gteMAC3 = A3( ( gteIR3 * gteIR3 ) >> shift );
	gteIR1 = Lm_B1( gteMAC1, lm );
	gteIR2 = Lm_B2( gteMAC2, lm );
	gteIR3 = Lm_B3( gteMAC3, lm );
}

void gteNCCS() {

#ifdef GTE_LOG
	GTE_LOG("GTE_NCCS\n");
#endif

	gteFLAG = 0;

	gteMAC1 = A1( ( ( (s64) gteL11 * gteVX0 ) + ( gteL12 * gteVY0 ) + ( gteL13 * gteVZ0 ) ) >> 12 );
	gteMAC2 = A2( ( ( (s64) gteL21 * gteVX0 ) + ( gteL22 * gteVY0 ) + ( gteL23 * gteVZ0 ) ) >> 12 );
	gteMAC3 = A3( ( ( (s64) gteL31 * gteVX0 ) + ( gteL32 * gteVY0 ) + ( gteL33 * gteVZ0 ) ) >> 12 );
	gteIR1 = Lm_B1( gteMAC1, 1 );
	gteIR2 = Lm_B2( gteMAC2, 1 );
	gteIR3 = Lm_B3( gteMAC3, 1 );
	gteMAC1 = A1( ( ( (s64) gteRBK << 12 ) + ( gteLR1 * gteIR1 ) + ( gteLR2 * gteIR2 ) + ( gteLR3 * gteIR3 ) ) >> 12 );
	gteMAC2 = A2( ( ( (s64) gteGBK << 12 ) + ( gteLG1 * gteIR1 ) + ( gteLG2 * gteIR2 ) + ( gteLG3 * gteIR3 ) ) >> 12 );
	gteMAC3 = A3( ( ( (s64) gteBBK << 12 ) + ( gteLB1 * gteIR1 ) + ( gteLB2 * gteIR2 ) + ( gteLB3 * gteIR3 ) ) >> 12 );
	gteIR1 = Lm_B1( gteMAC1, 1 );
	gteIR2 = Lm_B2( gteMAC2, 1 );
	gteIR3 = Lm_B3( gteMAC3, 1 );
	gteMAC1 = A1( ( (s64) gteR * gteIR1 ) >> 8 );
	gteMAC2 = A2( ( (s64) gteG * gteIR2 ) >> 8 );
	gteMAC3 = A3( ( (s64) gteB * gteIR3 ) >> 8 );
	gteIR1 = Lm_B1( gteMAC1, 1 );
	gteIR2 = Lm_B2( gteMAC2, 1 );
	gteIR3 = Lm_B3( gteMAC3, 1 );
	gteCODE0 = gteCODE1;
	gteCODE1 = gteCODE2;
	gteCODE2 = gteCODE;
	gteRGB0 = gteRGB1;
	gteRGB1 = gteRGB2;
	gteR2 = Lm_C1( gteMAC1 >> 4 );
	gteG2 = Lm_C2( gteMAC2 >> 4 );
	gteB2 = Lm_C3( gteMAC3 >> 4 );
}

void gteNCCT() {

#ifdef GTE_LOG
	GTE_LOG("GTE_NCCT\n");
#endif

    gteFLAG = 0;
	int v;
	s32 vx, vy, vz;
	gteCODE0 = gteCODE;

	for( v = 0; v < 3; v++ )
	{
		vx = VX( v );
		vy = VY( v );
		vz = VZ( v );
		gteMAC1 = A1( ( ( (s64) gteL11 * vx ) + ( gteL12 * vy ) + ( gteL13 * vz ) ) >> 12 );
		gteMAC2 = A2( ( ( (s64) gteL21 * vx ) + ( gteL22 * vy ) + ( gteL23 * vz ) ) >> 12 );
		gteMAC3 = A3( ( ( (s64) gteL31 * vx ) + ( gteL32 * vy ) + ( gteL33 * vz ) ) >> 12 );
		gteIR1 = Lm_B1( gteMAC1, 1 );
		gteIR2 = Lm_B2( gteMAC2, 1 );
		gteIR3 = Lm_B3( gteMAC3, 1 );
		gteMAC1 = A1( ( ( (s64) gteRBK << 12 ) + ( gteLR1 * gteIR1 ) + ( gteLR2 * gteIR2 ) + ( gteLR3 * gteIR3 ) ) >> 12 );
		gteMAC2 = A2( ( ( (s64) gteGBK << 12 ) + ( gteLG1 * gteIR1 ) + ( gteLG2 * gteIR2 ) + ( gteLG3 * gteIR3 ) ) >> 12 );
		gteMAC3 = A3( ( ( (s64) gteBBK << 12 ) + ( gteLB1 * gteIR1 ) + ( gteLB2 * gteIR2 ) + ( gteLB3 * gteIR3 ) ) >> 12 );
		gteIR1 = Lm_B1( gteMAC1, 1 );
		gteIR2 = Lm_B2( gteMAC2, 1 );
		gteIR3 = Lm_B3( gteMAC3, 1 );
		gteMAC1 = A1( ( (s64) gteR * gteIR1 ) >> 8 );
		gteMAC2 = A2( ( (s64) gteG * gteIR2 ) >> 8 );
		gteMAC3 = A3( ( (s64) gteB * gteIR3 ) >> 8 );
		gteCODE1 = gteCODE2;
		gteCODE2 = gteCODE;
		gteRGB0 = gteRGB1;
		gteRGB1 = gteRGB2;
		gteR2 = Lm_C1( gteMAC1 >> 4 );
		gteG2 = Lm_C2( gteMAC2 >> 4 );
		gteB2 = Lm_C3( gteMAC3 >> 4 );
	}

	gteIR1 = Lm_B1( gteMAC1, 1 );
	gteIR2 = Lm_B2( gteMAC2, 1 );
	gteIR3 = Lm_B3( gteMAC3, 1 );

}

void gteNCDS() {

if( gteop != 0x0e80413 )
{
	SysPrintf("NCDS - error\n");
}

#ifdef GTE_LOG
	GTE_LOG("GTE_NCDS\n");
#endif

    gteFLAG = 0;

	gteMAC1 = A1( ( ( (s64) gteL11 * gteVX0 ) + ( gteL12 * gteVY0 ) + ( gteL13 * gteVZ0 ) ) >> 12 );
	gteMAC2 = A2( ( ( (s64) gteL21 * gteVX0 ) + ( gteL22 * gteVY0 ) + ( gteL23 * gteVZ0 ) ) >> 12 );
	gteMAC3 = A3( ( ( (s64) gteL31 * gteVX0 ) + ( gteL32 * gteVY0 ) + ( gteL33 * gteVZ0 ) ) >> 12 );
	gteIR1 = Lm_B1( gteMAC1, 1 );
	gteIR2 = Lm_B2( gteMAC2, 1 );
	gteIR3 = Lm_B3( gteMAC3, 1 );
	gteMAC1 = A1( ( ( (s64) gteRBK << 12 ) + ( gteLR1 * gteIR1 ) + ( gteLR2 * gteIR2 ) + ( gteLR3 * gteIR3 ) ) >> 12 );
	gteMAC2 = A2( ( ( (s64) gteGBK << 12 ) + ( gteLG1 * gteIR1 ) + ( gteLG2 * gteIR2 ) + ( gteLG3 * gteIR3 ) ) >> 12 );
	gteMAC3 = A3( ( ( (s64) gteBBK << 12 ) + ( gteLB1 * gteIR1 ) + ( gteLB2 * gteIR2 ) + ( gteLB3 * gteIR3 ) ) >> 12 );
	gteIR1 = Lm_B1( gteMAC1, 1 );
	gteIR2 = Lm_B2( gteMAC2, 1 );
	gteIR3 = Lm_B3( gteMAC3, 1 );
	gteMAC1 = A1( ( ( ( (s64) gteR << 4 ) * gteIR1 ) + ( gteIR0 * Lm_B1( gteRFC - ( ( gteR * gteIR1 ) >> 8 ), 0 ) ) ) >> 12 );
	gteMAC2 = A2( ( ( ( (s64) gteG << 4 ) * gteIR2 ) + ( gteIR0 * Lm_B2( gteGFC - ( ( gteG * gteIR2 ) >> 8 ), 0 ) ) ) >> 12 );
	gteMAC3 = A3( ( ( ( (s64) gteB << 4 ) * gteIR3 ) + ( gteIR0 * Lm_B3( gteBFC - ( ( gteB * gteIR3 ) >> 8 ), 0 ) ) ) >> 12 );
	gteIR1 = Lm_B1( gteMAC1, 1 );
	gteIR2 = Lm_B2( gteMAC2, 1 );
	gteIR3 = Lm_B3( gteMAC3, 1 );
	gteCODE0 = gteCODE1;
	gteCODE1 = gteCODE2;
	gteCODE2 = gteCODE;
	gteRGB0 = gteRGB1;
	gteRGB1 = gteRGB2;
	gteR2 = Lm_C1( gteMAC1 >> 4 );
	gteG2 = Lm_C2( gteMAC2 >> 4 );
	gteB2 = Lm_C3( gteMAC3 >> 4 );
}

void gteNCDT() {

#ifdef GTE_LOG
	GTE_LOG("GTE_NCDT\n");
#endif

    gteFLAG = 0;
	
	gteCODE0 = gteCODE;

	int v;
	s32 vx, vy, vz;
	for( v = 0; v < 3; v++ )
	{

		vx = VX( v );
		vy = VY( v );
		vz = VZ( v );
		gteMAC1 = A1( ( ( (s64) gteL11 * vx ) + ( gteL12 * vy ) + ( gteL13 * vz ) ) >> 12 );
		gteMAC2 = A2( ( ( (s64) gteL21 * vx ) + ( gteL22 * vy ) + ( gteL23 * vz ) ) >> 12 );
		gteMAC3 = A3( ( ( (s64) gteL31 * vx ) + ( gteL32 * vy ) + ( gteL33 * vz ) ) >> 12 );
		gteIR1 = Lm_B1( gteMAC1, 1 );
		gteIR2 = Lm_B2( gteMAC2, 1 );
		gteIR3 = Lm_B3( gteMAC3, 1 );
		gteMAC1 = A1( ( ( (s64) gteRBK << 12 ) + ( gteLR1 * gteIR1 ) + ( gteLR2 * gteIR2 ) + ( gteLR3 * gteIR3 ) ) >> 12 );
		gteMAC2 = A2( ( ( (s64) gteGBK << 12 ) + ( gteLG1 * gteIR1 ) + ( gteLG2 * gteIR2 ) + ( gteLG3 * gteIR3 ) ) >> 12 );
		gteMAC3 = A3( ( ( (s64) gteBBK << 12 ) + ( gteLB1 * gteIR1 ) + ( gteLB2 * gteIR2 ) + ( gteLB3 * gteIR3 ) ) >> 12 );
		gteIR1 = Lm_B1( gteMAC1, 1 );
		gteIR2 = Lm_B2( gteMAC2, 1 );
		gteIR3 = Lm_B3( gteMAC3, 1 );
		gteMAC1 = A1( ( ( ( (s64) gteR << 4 ) * gteIR1 ) + ( gteIR0 * Lm_B1( gteRFC - ( ( gteR * gteIR1 ) >> 8 ), 0 ) ) ) >> 12 );
		gteMAC2 = A2( ( ( ( (s64) gteG << 4 ) * gteIR2 ) + ( gteIR0 * Lm_B2( gteGFC - ( ( gteG * gteIR2 ) >> 8 ), 0 ) ) ) >> 12 );
		gteMAC3 = A3( ( ( ( (s64) gteB << 4 ) * gteIR3 ) + ( gteIR0 * Lm_B3( gteBFC - ( ( gteB * gteIR3 ) >> 8 ), 0 ) ) ) >> 12 );
		gteCODE1 = gteCODE2;
		gteCODE2 = gteCODE;
		gteRGB0 = gteRGB1;
		gteRGB1 = gteRGB2;
		gteR2 = Lm_C1( gteMAC1 >> 4 );
		gteG2 = Lm_C2( gteMAC2 >> 4 );
		gteB2 = Lm_C3( gteMAC3 >> 4 );
	}

	gteIR1 = Lm_B1( gteMAC1, 1 );
	gteIR2 = Lm_B2( gteMAC2, 1 );
	gteIR3 = Lm_B3( gteMAC3, 1 );
} 

void gteOP() {

#ifdef GTE_LOG
	GTE_LOG("GTE_OP %lx\n", psxRegs.code & 0x1ffffff);
#endif

	gteFLAG = 0;
	
	int shift = 12 * GTE_SF( gteop );
	int lm = GTE_LM( gteop );

	gteMAC1 = A1( ( (s64) ( gteR22 * gteIR3 ) - ( gteR33 * gteIR2 ) ) >> shift );
	gteMAC2 = A2( ( (s64) ( gteR33 * gteIR1 ) - ( gteR11 * gteIR3 ) ) >> shift );
	gteMAC3 = A3( ( (s64) ( gteR11 * gteIR2 ) - ( gteR22 * gteIR1 ) ) >> shift );
	gteIR1 = Lm_B1( gteMAC1, lm );
	gteIR2 = Lm_B2( gteMAC2, lm );
	gteIR3 = Lm_B3( gteMAC3, lm );
}

void gteDCPL() {

#ifdef GTE_LOG
	GTE_LOG("GTE_DCPL\n");
#endif

	gteFLAG=0;
	int shift = 12 * GTE_SF( gteop );
	int lm = GTE_LM( gteop );

	s64 iR = (gteR << 4) * gteIR1;
	s64 iG = (gteG << 4) * gteIR2;
	s64 iB = (gteB << 4) * gteIR3;

	gteMAC1 = A1( iR + gteIR0 * Lm_B1( gteRFC - iR, 0 ) ) >> shift;
	gteMAC2 = A2( iG + gteIR0 * Lm_B1( gteGFC - iG, 0 ) ) >> shift;
	gteMAC3 = A3( iB + gteIR0 * Lm_B1( gteBFC - iB, 0 ) ) >> shift;

	gteIR1 = Lm_B1( gteMAC1 >> 4, lm );
	gteIR2 = Lm_B2( gteMAC2 >> 4, lm );
	gteIR3 = Lm_B3( gteMAC3 >> 4, lm );
	gteRGB0 = gteRGB1;
	gteRGB1 = gteRGB2;
	gteCODE2 = gteCODE;
	gteR2 = Lm_C1( gteMAC1 >> 4 );
	gteG2 = Lm_C2( gteMAC2 >> 4 );
	gteB2 = Lm_C3( gteMAC3 >> 4 );
}

void gteGPF() {

#ifdef GTE_LOG
	GTE_LOG("GTE_GPF %lx\n", psxRegs.code & 0x1ffffff);
#endif

	gteFLAG = 0;
	int shift = 12 * GTE_SF( gteop );

	gteMAC1 = A1( ( (s64) gteIR0 * gteIR1 ) >> shift );
	gteMAC2 = A2( ( (s64) gteIR0 * gteIR2 ) >> shift );
	gteMAC3 = A3( ( (s64) gteIR0 * gteIR3 ) >> shift );
	gteIR1 = Lm_B1( gteMAC1, 0 );
	gteIR2 = Lm_B2( gteMAC2, 0 );
	gteIR3 = Lm_B3( gteMAC3, 0 );
	gteCODE0 = gteCODE1;
	gteCODE1 = gteCODE2;
	gteCODE2 = gteCODE;
	gteRGB0 = gteRGB1;
	gteRGB1 = gteRGB2;
	gteR2 = Lm_C1( gteMAC1 >> 4 );
	gteG2 = Lm_C2( gteMAC2 >> 4 );
	gteB2 = Lm_C3( gteMAC3 >> 4 );
}

void gteGPL() {

#ifdef GTE_LOG
	GTE_LOG("GTE_GPL %lx\n", psxRegs.code & 0x1ffffff);
#endif

	gteFLAG = 0;

	int shift = 12 * GTE_SF( gteop );

	gteMAC1 = A1( ( ( (s64) gteMAC1 << shift ) + ( gteIR0 * gteIR1 ) ) >> shift );
	gteMAC2 = A2( ( ( (s64) gteMAC2 << shift ) + ( gteIR0 * gteIR2 ) ) >> shift );
	gteMAC3 = A3( ( ( (s64) gteMAC3 << shift ) + ( gteIR0 * gteIR3 ) ) >> shift );
	gteIR1 = Lm_B1( gteMAC1, 0 );
	gteIR2 = Lm_B2( gteMAC2, 0 );
	gteIR3 = Lm_B3( gteMAC3, 0 );
	gteCODE0 = gteCODE1;
	gteCODE1 = gteCODE2;
	gteCODE2 = gteCODE;
	gteRGB0 = gteRGB1;
	gteRGB1 = gteRGB2;
	gteR2 = Lm_C1( gteMAC1 >> 4 );
	gteG2 = Lm_C2( gteMAC2 >> 4 );
	gteB2 = Lm_C3( gteMAC3 >> 4 );
}

void gteDPCS() {

#ifdef GTE_LOG
	GTE_LOG("GTE_DPCS\n");
#endif

	gteFLAG = 0;
	int shift = 12 * GTE_SF( gteop );
	int lm = GTE_LM( gteop );

	gteMAC1 = A1(( ( gteR << 16 ) + ( gteIR0 * Lm_B1( A1( (s64) gteRFC - ( gteR << 4 ) ) << ( 12 - shift ), lm ) ) ) >> shift);
	gteMAC2 = A2(( ( gteG << 16 ) + ( gteIR0 * Lm_B2( A2( (s64) gteGFC - ( gteG << 4 ) ) << ( 12 - shift ), lm ) ) ) >> shift);
	gteMAC3 = A3(( ( gteB << 16 ) + ( gteIR0 * Lm_B3( A3( (s64) gteBFC - ( gteB << 4 ) ) << ( 12 - shift ), lm ) ) ) >> shift);

	gteIR1 = Lm_B1( gteMAC1, lm );
	gteIR2 = Lm_B2( gteMAC2, lm );
	gteIR3 = Lm_B3( gteMAC3, lm );
	gteRGB0 = gteRGB1;
	gteRGB1 = gteRGB2;
	gteCODE0 = gteCODE1;
	gteCODE1 = gteCODE2;
	gteCODE2 = gteCODE;
	gteR2 = Lm_C1( gteMAC1 >> 4 );
	gteG2 = Lm_C2( gteMAC2 >> 4 );
	gteB2 = Lm_C3( gteMAC3 >> 4 );
}

void gteDPCT() {

#ifdef GTE_LOG
	GTE_LOG("GTE_DPCT\n");
#endif

	gteFLAG = 0;
	int shift = 12 * GTE_SF( gteop );
	int lm = GTE_LM( gteop );

	int v;
	gteCODE0 = gteCODE;
	for( v = 0; v < 3; v++ )
	{
		gteMAC1 = A1( ( ( (s64) gteR0 << 16 ) + ( (s64) gteIR0 * ( Lm_B1( gteRFC - ( gteR0 << 4 ), lm ) ) ) ) >> shift );
		gteMAC2 = A2( ( ( (s64) gteG0 << 16 ) + ( (s64) gteIR0 * ( Lm_B1( gteGFC - ( gteG0 << 4 ), lm ) ) ) ) >> shift );
		gteMAC3 = A3( ( ( (s64) gteB0 << 16 ) + ( (s64) gteIR0 * ( Lm_B1( gteBFC - ( gteB0 << 4 ), lm ) ) ) ) >> shift );
		gteCODE1 = gteCODE2;
		gteCODE2 = gteCODE;
		gteRGB0 = gteRGB1;
		gteRGB1 = gteRGB2;
		gteR2 = Lm_C1( gteMAC1 >> 4 );
		gteG2 = Lm_C2( gteMAC2 >> 4 );
		gteB2 = Lm_C3( gteMAC3 >> 4 );
	}
	gteIR1 = Lm_B1( gteMAC1, lm );
	gteIR2 = Lm_B2( gteMAC2, lm );
	gteIR3 = Lm_B3( gteMAC3, lm );
}

void gteNCS() {

#ifdef GTE_LOG
	GTE_LOG("GTE_NCS\n");
#endif

	gteFLAG = 0;

	gteMAC1 = A1( ( ( (s64) gteL11 * gteVX0 ) + ( gteL12 * gteVY0 ) + ( gteL13 * gteVZ0 ) ) >> 12 );
	gteMAC2 = A2( ( ( (s64) gteL21 * gteVX0 ) + ( gteL22 * gteVY0 ) + ( gteL23 * gteVZ0 ) ) >> 12 );
	gteMAC3 = A3( ( ( (s64) gteL31 * gteVX0 ) + ( gteL32 * gteVY0 ) + ( gteL33 * gteVZ0 ) ) >> 12 );
	gteIR1 = Lm_B1( gteMAC1, 1 );
	gteIR2 = Lm_B2( gteMAC2, 1 );
	gteIR3 = Lm_B3( gteMAC3, 1 );
	gteMAC1 = A1( ( ( (s64) gteRBK << 12 ) + ( gteLR1 * gteIR1 ) + ( gteLR2 * gteIR2 ) + ( gteLR3 * gteIR3 ) ) >> 12 );
	gteMAC2 = A2( ( ( (s64) gteGBK << 12 ) + ( gteLG1 * gteIR1 ) + ( gteLG2 * gteIR2 ) + ( gteLG3 * gteIR3 ) ) >> 12 );
	gteMAC3 = A3( ( ( (s64) gteBBK << 12 ) + ( gteLB1 * gteIR1 ) + ( gteLB2 * gteIR2 ) + ( gteLB3 * gteIR3 ) ) >> 12 );
	gteIR1 = Lm_B1( gteMAC1, 1 );
	gteIR2 = Lm_B2( gteMAC2, 1 );
	gteIR3 = Lm_B3( gteMAC3, 1 );
	gteCODE0 = gteCODE1;
	gteCODE1 = gteCODE2;
	gteCODE2 = gteCODE;
	gteRGB0 = gteRGB1;
	gteRGB1 = gteRGB2;
	gteR2 = Lm_C1( gteMAC1 >> 4 );
	gteG2 = Lm_C2( gteMAC2 >> 4 );
	gteB2 = Lm_C3( gteMAC3 >> 4 );
}

void gteNCT() {

#ifdef GTE_LOG
GTE_LOG("GTE_NCT\n");
#endif

	gteFLAG = 0;

	int v;
	s32 vx, vy, vz;
	gteCODE0 = gteCODE;
	for( v = 0; v < 3; v++ )
	{
		vx = VX( v );
		vy = VY( v );
		vz = VZ( v );
		gteMAC1 = A1( ( ( (s64) gteL11 * vx ) + ( gteL12 * vy ) + ( gteL13 * vz ) ) >> 12 );
		gteMAC2 = A2( ( ( (s64) gteL21 * vx ) + ( gteL22 * vy ) + ( gteL23 * vz ) ) >> 12 );
		gteMAC3 = A3( ( ( (s64) gteL31 * vx ) + ( gteL32 * vy ) + ( gteL33 * vz ) ) >> 12 );
		gteIR1 = Lm_B1( gteMAC1, 1 );
		gteIR2 = Lm_B2( gteMAC2, 1 );
		gteIR3 = Lm_B3( gteMAC3, 1 );
		gteMAC1 = A1( ( ( (s64) gteRBK << 12 ) + ( gteLR1 * gteIR1 ) + ( gteLR2 * gteIR2 ) + ( gteLR3 * gteIR3 ) ) >> 12 );
		gteMAC2 = A2( ( ( (s64) gteGBK << 12 ) + ( gteLG1 * gteIR1 ) + ( gteLG2 * gteIR2 ) + ( gteLG3 * gteIR3 ) ) >> 12 );
		gteMAC3 = A3( ( ( (s64) gteBBK << 12 ) + ( gteLB1 * gteIR1 ) + ( gteLB2 * gteIR2 ) + ( gteLB3 * gteIR3 ) ) >> 12 );
		gteCODE1 = gteCODE2;
		gteCODE2 = gteCODE;
		gteRGB0 = gteRGB1;
		gteRGB1 = gteRGB2;
		gteR2 = Lm_C1( gteMAC1 >> 4 );
		gteG2 = Lm_C2( gteMAC2 >> 4 );
		gteB2 = Lm_C3( gteMAC3 >> 4 );
	}
	gteIR1 = Lm_B1( gteMAC1, 1 );
	gteIR2 = Lm_B2( gteMAC2, 1 );
	gteIR3 = Lm_B3( gteMAC3, 1 );
}

void gteCC() {
	gteFLAG = 0;

	gteMAC1 = A1( ( ( (s64) gteRBK << 12 ) + ( gteLR1 * gteIR1 ) + ( gteLR2 * gteIR2 ) + ( gteLR3 * gteIR3 ) ) >> 12 );
	gteMAC2 = A2( ( ( (s64) gteGBK << 12 ) + ( gteLG1 * gteIR1 ) + ( gteLG2 * gteIR2 ) + ( gteLG3 * gteIR3 ) ) >> 12 );
	gteMAC3 = A3( ( ( (s64) gteBBK << 12 ) + ( gteLB1 * gteIR1 ) + ( gteLB2 * gteIR2 ) + ( gteLB3 * gteIR3 ) ) >> 12 );
	gteIR1 = Lm_B1( gteMAC1, 1 );
	gteIR2 = Lm_B2( gteMAC2, 1 );
	gteIR3 = Lm_B3( gteMAC3, 1 );
	gteMAC1 = A1( ( (s64) gteR * gteIR1 ) >> 8 );
	gteMAC2 = A2( ( (s64) gteG * gteIR2 ) >> 8 );
	gteMAC3 = A3( ( (s64) gteB * gteIR3 ) >> 8 );
	gteIR1 = Lm_B1( gteMAC1, 1 );
	gteIR2 = Lm_B2( gteMAC2, 1 );
	gteIR3 = Lm_B3( gteMAC3, 1 );
	gteCODE0 = gteCODE1;
	gteCODE1 = gteCODE2;
	gteCODE2 = gteCODE;
	gteRGB0 = gteRGB1;
	gteRGB1 = gteRGB2;
	gteR2 = Lm_C1( gteMAC1 >> 4 );
	gteG2 = Lm_C2( gteMAC2 >> 4 );
	gteB2 = Lm_C3( gteMAC3 >> 4 );
}

void gteINTPL() {

#ifdef GTE_LOG
	GTE_LOG("GTE_INTP\n");
#endif

	gteFLAG = 0;

	int shift = 12 * GTE_SF( gteop );
	int lm = GTE_LM( gteop );

	gteMAC1 = ( ( gteIR1 << 12 ) + ( gteIR0 * Lm_B1( A1( (s64) gteRFC - gteIR1 ) << ( 12 - shift ), 0 ) ) ) >> shift;
	gteMAC2 = ( ( gteIR2 << 12 ) + ( gteIR0 * Lm_B2( A2( (s64) gteGFC - gteIR2 ) << ( 12 - shift ), 0 ) ) ) >> shift;
	gteMAC3 = ( ( gteIR3 << 12 ) + ( gteIR0 * Lm_B3( A3( (s64) gteBFC - gteIR3 ) << ( 12 - shift ), 0 ) ) ) >> shift;
	gteIR1 = Lm_B1( gteMAC1, lm );
	gteIR2 = Lm_B2( gteMAC2, lm );
	gteIR3 = Lm_B3( gteMAC3, lm );
	gteRGB0 = gteRGB1;
	gteRGB1 = gteRGB2;
	gteCODE2 = gteCODE;
	gteR2 = Lm_C1( gteMAC1 >> 4 );
	gteG2 = Lm_C2( gteMAC2 >> 4 );
	gteB2 = Lm_C3( gteMAC3 >> 4 );

}

void gteCDP() {

if( gteop == 0x1280414 )
{
	SysPrintf("CDP - error\n");
}

#ifdef GTE_LOG
	GTE_LOG("GTE_CDP\n");
#endif

	gteFLAG = 0;

	gteMAC1 = A1( ( ( (s64) gteRBK << 12 ) + ( gteLR1 * gteIR1 ) + ( gteLR2 * gteIR2 ) + ( gteLR3 * gteIR3 ) ) >> 12 );
	gteMAC2 = A2( ( ( (s64) gteGBK << 12 ) + ( gteLG1 * gteIR1 ) + ( gteLG2 * gteIR2 ) + ( gteLG3 * gteIR3 ) ) >> 12 );
	gteMAC3 = A3( ( ( (s64) gteBBK << 12 ) + ( gteLB1 * gteIR1 ) + ( gteLB2 * gteIR2 ) + ( gteLB3 * gteIR3 ) ) >> 12 );
	gteIR1 = Lm_B1( gteMAC1, 1 );
	gteIR2 = Lm_B2( gteMAC2, 1 );
	gteIR3 = Lm_B3( gteMAC3, 1 );
	gteMAC1 = A1( ( ( ( (s64) gteR << 4 ) * gteIR1 ) + ( gteIR0 * Lm_B1( gteRFC - ( ( gteR * gteIR1 ) >> 8 ), 0 ) ) ) >> 12 );
	gteMAC2 = A2( ( ( ( (s64) gteG << 4 ) * gteIR2 ) + ( gteIR0 * Lm_B2( gteGFC - ( ( gteG * gteIR2 ) >> 8 ), 0 ) ) ) >> 12 );
	gteMAC3 = A3( ( ( ( (s64) gteB << 4 ) * gteIR3 ) + ( gteIR0 * Lm_B3( gteBFC - ( ( gteB * gteIR3 ) >> 8 ), 0 ) ) ) >> 12 );
	gteIR1 = Lm_B1( gteMAC1, 1 );
	gteIR2 = Lm_B2( gteMAC2, 1 );
	gteIR3 = Lm_B3( gteMAC3, 1 );
	gteCODE0 = gteCODE1;
	gteCODE1 = gteCODE2;
	gteCODE2 = gteCODE;
	gteRGB0 = gteRGB1;
	gteRGB1 = gteRGB2;
	gteR2 = Lm_C1( gteMAC1 >> 4 );
	gteG2 = Lm_C2( gteMAC2 >> 4 );
	gteB2 = Lm_C3( gteMAC3 >> 4 );
}
