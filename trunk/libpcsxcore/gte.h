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

#ifndef __GTE_H__
#define __GTE_H__

#include "psxcommon.h"
#include "r3000a.h"

#ifdef GTE_TIMING
u32 GteStall;
#endif

#define VX( n ) ( n < 3 ? psxRegs.cp2d[ n << 1 ].sw.l : psxRegs.cp2d[ 9 ].sw.l )
#define VY( n ) ( n < 3 ? psxRegs.cp2d[ n << 1 ].sw.h : psxRegs.cp2d[ 10 ].sw.l )
#define VZ( n ) ( n < 3 ? psxRegs.cp2d[ ( n << 1 ) + 1 ].sw.l : psxRegs.cp2d[ 11 ].sw.l )
#define MX11( n ) ( n < 3 ? psxRegs.cp2c[ ( n << 3 ) ].sw.l : 0 )
#define MX12( n ) ( n < 3 ? psxRegs.cp2c[ ( n << 3 ) ].sw.h : 0 )
#define MX13( n ) ( n < 3 ? psxRegs.cp2c[ ( n << 3 ) + 1 ].sw.l : 0 )
#define MX21( n ) ( n < 3 ? psxRegs.cp2c[ ( n << 3 ) + 1 ].sw.h : 0 )
#define MX22( n ) ( n < 3 ? psxRegs.cp2c[ ( n << 3 ) + 2 ].sw.l : 0 )
#define MX23( n ) ( n < 3 ? psxRegs.cp2c[ ( n << 3 ) + 2 ].sw.h : 0 )
#define MX31( n ) ( n < 3 ? psxRegs.cp2c[ ( n << 3 ) + 3 ].sw.l : 0 )
#define MX32( n ) ( n < 3 ? psxRegs.cp2c[ ( n << 3 ) + 3 ].sw.h : 0 )
#define MX33( n ) ( n < 3 ? psxRegs.cp2c[ ( n << 3 ) + 4 ].sw.l : 0 )
#define CV1( n ) ( n < 3 ? psxRegs.cp2c[ ( n << 3 ) + 5 ].sd : 0 )
#define CV2( n ) ( n < 3 ? psxRegs.cp2c[ ( n << 3 ) + 6 ].sd : 0 )
#define CV3( n ) ( n < 3 ? psxRegs.cp2c[ ( n << 3 ) + 7 ].sd : 0 )

#define fSX( n ) ( (psxRegs.cp2d)[ ((n) + 12) ].sw.l )
#define fSY( n ) ( (psxRegs.cp2d)[ ((n) + 12) ].sw.h )
#define fSZ( n ) ( (psxRegs.cp2d)[ ((n) + 17) ].w.l ) /* (n == 0) => SZ1; */

#define gteVXY0 ( psxRegs.cp2d[ 0 ].d )
#define gteVX0  ( psxRegs.cp2d[ 0 ].sw.l )
#define gteVY0  ( psxRegs.cp2d[ 0 ].sw.h )
#define gteVZ0  ( psxRegs.cp2d[ 1 ].sw.l )
#define gteVXY1 ( psxRegs.cp2d[ 2 ].d )
#define gteVX1  ( psxRegs.cp2d[ 2 ].sw.l )
#define gteVY1  ( psxRegs.cp2d[ 2 ].sw.h )
#define gteVZ1  ( psxRegs.cp2d[ 3 ].sw.l )
#define gteVXY2 ( psxRegs.cp2d[ 4 ].d )
#define gteVX2  ( psxRegs.cp2d[ 4 ].sw.l )
#define gteVY2  ( psxRegs.cp2d[ 4 ].sw.h )
#define gteVZ2  ( psxRegs.cp2d[ 5 ].sw.l )
#define gteRGB  ( psxRegs.cp2d[ 6 ].d )
#define gteR    ( psxRegs.cp2d[ 6 ].b.l )
#define gteG    ( psxRegs.cp2d[ 6 ].b.h )
#define gteB    ( psxRegs.cp2d[ 6 ].b.h2 )
#define gteCODE ( psxRegs.cp2d[ 6 ].b.h3 )
#define gteOTZ  ( psxRegs.cp2d[ 7 ].w.l )
#define gteIR0  ( psxRegs.cp2d[ 8 ].sw.l )
#define gteIR1  ( psxRegs.cp2d[ 9 ].sw.l )
#define gteIR2  ( psxRegs.cp2d[ 10 ].sw.l )
#define gteIR3  ( psxRegs.cp2d[ 11 ].sw.l )
#define gteSXY0 ( psxRegs.cp2d[ 12 ].d )
#define gteSX0  ( psxRegs.cp2d[ 12 ].sw.l )
#define gteSY0  ( psxRegs.cp2d[ 12 ].sw.h )
#define gteSXY1 ( psxRegs.cp2d[ 13 ].d )
#define gteSX1  ( psxRegs.cp2d[ 13 ].sw.l )
#define gteSY1  ( psxRegs.cp2d[ 13 ].sw.h )
#define gteSXY2 ( psxRegs.cp2d[ 14 ].d )
#define gteSX2  ( psxRegs.cp2d[ 14 ].sw.l )
#define gteSY2  ( psxRegs.cp2d[ 14 ].sw.h )
#define gteSXYP ( psxRegs.cp2d[ 15 ].d )
#define gteSXP  ( psxRegs.cp2d[ 15 ].sw.l )
#define gteSYP  ( psxRegs.cp2d[ 15 ].sw.h )
#define gteSZ0  ( psxRegs.cp2d[ 16 ].w.l )
#define gteSZ1  ( psxRegs.cp2d[ 17 ].w.l )
#define gteSZ2  ( psxRegs.cp2d[ 18 ].w.l )
#define gteSZ3  ( psxRegs.cp2d[ 19 ].w.l )
#define gteRGB0  ( psxRegs.cp2d[ 20 ].d )
#define gteR0    ( psxRegs.cp2d[ 20 ].b.l )
#define gteG0    ( psxRegs.cp2d[ 20 ].b.h )
#define gteB0    ( psxRegs.cp2d[ 20 ].b.h2 )
#define gteCODE0 ( psxRegs.cp2d[ 20 ].b.h3 )
#define gteRGB1  ( psxRegs.cp2d[ 21 ].d )
#define gteR1    ( psxRegs.cp2d[ 21 ].b.l )
#define gteG1    ( psxRegs.cp2d[ 21 ].b.h )
#define gteB1    ( psxRegs.cp2d[ 21 ].b.h2 )
#define gteCODE1 ( psxRegs.cp2d[ 21 ].b.h3 )
#define gteRGB2  ( psxRegs.cp2d[ 22 ].d )
#define gteR2    ( psxRegs.cp2d[ 22 ].b.l )
#define gteG2    ( psxRegs.cp2d[ 22 ].b.h )
#define gteB2    ( psxRegs.cp2d[ 22 ].b.h2 )
#define gteCODE2 ( psxRegs.cp2d[ 22 ].b.h3 )
#define gteRES1  ( psxRegs.cp2d[ 23 ].d )
#define gteMAC0  ( psxRegs.cp2d[ 24 ].sd )
#define gteMAC1  ( psxRegs.cp2d[ 25 ].sd )
#define gteMAC2  ( psxRegs.cp2d[ 26 ].sd )
#define gteMAC3  ( psxRegs.cp2d[ 27 ].sd )
#define gteIRGB  ( psxRegs.cp2d[ 28 ].d )
#define gteORGB  ( psxRegs.cp2d[ 29 ].d )
#define gteLZCS  ( psxRegs.cp2d[ 30 ].d )
#define gteLZCR  ( psxRegs.cp2d[ 31 ].d )

#define gteR11R12 ( psxRegs.cp2c[ 0 ].sd )
#define gteR22R23 ( psxRegs.cp2c[ 2 ].sd )
#define gteR11 ( psxRegs.cp2c[ 0 ].sw.l )
#define gteR12 ( psxRegs.cp2c[ 0 ].sw.h )
#define gteR13 ( psxRegs.cp2c[ 1 ].sw.l )
#define gteR21 ( psxRegs.cp2c[ 1 ].sw.h )
#define gteR22 ( psxRegs.cp2c[ 2 ].sw.l )
#define gteR23 ( psxRegs.cp2c[ 2 ].sw.h )
#define gteR31 ( psxRegs.cp2c[ 3 ].sw.l )
#define gteR32 ( psxRegs.cp2c[ 3 ].sw.h )
#define gteR33 ( psxRegs.cp2c[ 4 ].sw.l )
#define gteTRX ( psxRegs.cp2c[ 5 ].sd )
#define gteTRY ( psxRegs.cp2c[ 6 ].sd )
#define gteTRZ ( psxRegs.cp2c[ 7 ].sd )
#define gteL11 ( psxRegs.cp2c[ 8 ].sw.l )
#define gteL12 ( psxRegs.cp2c[ 8 ].sw.h )
#define gteL13 ( psxRegs.cp2c[ 9 ].sw.l )
#define gteL21 ( psxRegs.cp2c[ 9 ].sw.h )
#define gteL22 ( psxRegs.cp2c[ 10 ].sw.l )
#define gteL23 ( psxRegs.cp2c[ 10 ].sw.h )
#define gteL31 ( psxRegs.cp2c[ 11 ].sw.l )
#define gteL32 ( psxRegs.cp2c[ 11 ].sw.h )
#define gteL33 ( psxRegs.cp2c[ 12 ].sw.l )
#define gteRBK ( psxRegs.cp2c[ 13 ].sd )
#define gteGBK ( psxRegs.cp2c[ 14 ].sd )
#define gteBBK ( psxRegs.cp2c[ 15 ].sd )
#define gteLR1 ( psxRegs.cp2c[ 16 ].sw.l )
#define gteLR2 ( psxRegs.cp2c[ 16 ].sw.h )
#define gteLR3 ( psxRegs.cp2c[ 17 ].sw.l )
#define gteLG1 ( psxRegs.cp2c[ 17 ].sw.h )
#define gteLG2 ( psxRegs.cp2c[ 18 ].sw.l )
#define gteLG3 ( psxRegs.cp2c[ 18 ].sw.h )
#define gteLB1 ( psxRegs.cp2c[ 19 ].sw.l )
#define gteLB2 ( psxRegs.cp2c[ 19 ].sw.h )
#define gteLB3 ( psxRegs.cp2c[ 20 ].sw.l )
#define gteRFC ( psxRegs.cp2c[ 21 ].sd )
#define gteGFC ( psxRegs.cp2c[ 22 ].sd )
#define gteBFC ( psxRegs.cp2c[ 23 ].sd )
#define gteOFX ( psxRegs.cp2c[ 24 ].sd )
#define gteOFY ( psxRegs.cp2c[ 25 ].sd )
#define gteH   ( psxRegs.cp2c[ 26 ].sw.l )
#define gteDQA ( psxRegs.cp2c[ 27 ].sw.l )
#define gteDQB ( psxRegs.cp2c[ 28 ].sd )
#define gteZSF3 ( psxRegs.cp2c[ 29 ].sw.l )
#define gteZSF4 ( psxRegs.cp2c[ 30 ].sw.l )
#define gteFLAG ( psxRegs.cp2c[ 31 ].d )

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

void gteMFC2();
void gteCFC2();
void gteMTC2();
void gteCTC2();
void gteLWC2();
void gteSWC2();

void gteRTPS();
void gteOP();
void gteNCLIP();
void gteDPCS();
void gteINTPL();
void gteMVMVA();
void gteNCDS();
void gteNCDT();
void gteCDP();
void gteNCCS();
void gteCC();
void gteNCS();
void gteNCT();
void gteSQR();
void gteDCPL();
void gteDPCT();
void gteAVSZ3();
void gteAVSZ4();
void gteRTPT();
void gteGPF();
void gteGPL();
void gteNCCT();

#endif /* __GTE_H__ */
