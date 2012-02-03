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

namespace R3000A {

#ifdef GTE_TIMING
u32 GteStall;
#endif

#define VX( n ) ( n < 3 ? psxRegs.CP2D.r[ n << 1 ].sw.l : psxRegs.CP2D.r[ 9 ].sw.l )
#define VY( n ) ( n < 3 ? psxRegs.CP2D.r[ n << 1 ].sw.h : psxRegs.CP2D.r[ 10 ].sw.l )
#define VZ( n ) ( n < 3 ? psxRegs.CP2D.r[ ( n << 1 ) + 1 ].sw.l : psxRegs.CP2D.r[ 11 ].sw.l )
#define MX11( n ) ( n < 3 ? psxRegs.CP2C.r[ ( n << 3 ) ].sw.l : 0 )
#define MX12( n ) ( n < 3 ? psxRegs.CP2C.r[ ( n << 3 ) ].sw.h : 0 )
#define MX13( n ) ( n < 3 ? psxRegs.CP2C.r[ ( n << 3 ) + 1 ].sw.l : 0 )
#define MX21( n ) ( n < 3 ? psxRegs.CP2C.r[ ( n << 3 ) + 1 ].sw.h : 0 )
#define MX22( n ) ( n < 3 ? psxRegs.CP2C.r[ ( n << 3 ) + 2 ].sw.l : 0 )
#define MX23( n ) ( n < 3 ? psxRegs.CP2C.r[ ( n << 3 ) + 2 ].sw.h : 0 )
#define MX31( n ) ( n < 3 ? psxRegs.CP2C.r[ ( n << 3 ) + 3 ].sw.l : 0 )
#define MX32( n ) ( n < 3 ? psxRegs.CP2C.r[ ( n << 3 ) + 3 ].sw.h : 0 )
#define MX33( n ) ( n < 3 ? psxRegs.CP2C.r[ ( n << 3 ) + 4 ].sw.l : 0 )
#define CV1( n ) ( n < 3 ? psxRegs.CP2C.r[ ( n << 3 ) + 5 ].sd : 0 )
#define CV2( n ) ( n < 3 ? psxRegs.CP2C.r[ ( n << 3 ) + 6 ].sd : 0 )
#define CV3( n ) ( n < 3 ? psxRegs.CP2C.r[ ( n << 3 ) + 7 ].sd : 0 )

#define fSX( n ) ( (psxRegs.CP2D).r[ ((n) + 12) ].sw.l )
#define fSY( n ) ( (psxRegs.CP2D).r[ ((n) + 12) ].sw.h )
#define fSZ( n ) ( (psxRegs.CP2D).r[ ((n) + 17) ].w.l ) /* (n == 0) => SZ1; */

#define gteVXY0 ( psxRegs.CP2D.r[ 0 ].d )
#define gteVX0  ( psxRegs.CP2D.r[ 0 ].sw.l )
#define gteVY0  ( psxRegs.CP2D.r[ 0 ].sw.h )
#define gteVZ0  ( psxRegs.CP2D.r[ 1 ].sw.l )
#define gteVXY1 ( psxRegs.CP2D.r[ 2 ].d )
#define gteVX1  ( psxRegs.CP2D.r[ 2 ].sw.l )
#define gteVY1  ( psxRegs.CP2D.r[ 2 ].sw.h )
#define gteVZ1  ( psxRegs.CP2D.r[ 3 ].sw.l )
#define gteVXY2 ( psxRegs.CP2D.r[ 4 ].d )
#define gteVX2  ( psxRegs.CP2D.r[ 4 ].sw.l )
#define gteVY2  ( psxRegs.CP2D.r[ 4 ].sw.h )
#define gteVZ2  ( psxRegs.CP2D.r[ 5 ].sw.l )
#define gteRGB  ( psxRegs.CP2D.r[ 6 ].d )
#define gteR    ( psxRegs.CP2D.r[ 6 ].b.l )
#define gteG    ( psxRegs.CP2D.r[ 6 ].b.h )
#define gteB    ( psxRegs.CP2D.r[ 6 ].b.h2 )
#define gteCODE ( psxRegs.CP2D.r[ 6 ].b.h3 )
#define gteOTZ  ( psxRegs.CP2D.r[ 7 ].w.l )
#define gteIR0  ( psxRegs.CP2D.r[ 8 ].sw.l )
#define gteIR1  ( psxRegs.CP2D.r[ 9 ].sw.l )
#define gteIR2  ( psxRegs.CP2D.r[ 10 ].sw.l )
#define gteIR3  ( psxRegs.CP2D.r[ 11 ].sw.l )
#define gteSXY0 ( psxRegs.CP2D.r[ 12 ].d )
#define gteSX0  ( psxRegs.CP2D.r[ 12 ].sw.l )
#define gteSY0  ( psxRegs.CP2D.r[ 12 ].sw.h )
#define gteSXY1 ( psxRegs.CP2D.r[ 13 ].d )
#define gteSX1  ( psxRegs.CP2D.r[ 13 ].sw.l )
#define gteSY1  ( psxRegs.CP2D.r[ 13 ].sw.h )
#define gteSXY2 ( psxRegs.CP2D.r[ 14 ].d )
#define gteSX2  ( psxRegs.CP2D.r[ 14 ].sw.l )
#define gteSY2  ( psxRegs.CP2D.r[ 14 ].sw.h )
#define gteSXYP ( psxRegs.CP2D.r[ 15 ].d )
#define gteSXP  ( psxRegs.CP2D.r[ 15 ].sw.l )
#define gteSYP  ( psxRegs.CP2D.r[ 15 ].sw.h )
#define gteSZ0  ( psxRegs.CP2D.r[ 16 ].w.l )
#define gteSZ1  ( psxRegs.CP2D.r[ 17 ].w.l )
#define gteSZ2  ( psxRegs.CP2D.r[ 18 ].w.l )
#define gteSZ3  ( psxRegs.CP2D.r[ 19 ].w.l )
#define gteRGB0  ( psxRegs.CP2D.r[ 20 ].d )
#define gteR0    ( psxRegs.CP2D.r[ 20 ].b.l )
#define gteG0    ( psxRegs.CP2D.r[ 20 ].b.h )
#define gteB0    ( psxRegs.CP2D.r[ 20 ].b.h2 )
#define gteCODE0 ( psxRegs.CP2D.r[ 20 ].b.h3 )
#define gteRGB1  ( psxRegs.CP2D.r[ 21 ].d )
#define gteR1    ( psxRegs.CP2D.r[ 21 ].b.l )
#define gteG1    ( psxRegs.CP2D.r[ 21 ].b.h )
#define gteB1    ( psxRegs.CP2D.r[ 21 ].b.h2 )
#define gteCODE1 ( psxRegs.CP2D.r[ 21 ].b.h3 )
#define gteRGB2  ( psxRegs.CP2D.r[ 22 ].d )
#define gteR2    ( psxRegs.CP2D.r[ 22 ].b.l )
#define gteG2    ( psxRegs.CP2D.r[ 22 ].b.h )
#define gteB2    ( psxRegs.CP2D.r[ 22 ].b.h2 )
#define gteCODE2 ( psxRegs.CP2D.r[ 22 ].b.h3 )
#define gteRES1  ( psxRegs.CP2D.r[ 23 ].d )
#define gteMAC0  ( psxRegs.CP2D.r[ 24 ].sd )
#define gteMAC1  ( psxRegs.CP2D.r[ 25 ].sd )
#define gteMAC2  ( psxRegs.CP2D.r[ 26 ].sd )
#define gteMAC3  ( psxRegs.CP2D.r[ 27 ].sd )
#define gteIRGB  ( psxRegs.CP2D.r[ 28 ].d )
#define gteORGB  ( psxRegs.CP2D.r[ 29 ].d )
#define gteLZCS  ( psxRegs.CP2D.r[ 30 ].d )
#define gteLZCR  ( psxRegs.CP2D.r[ 31 ].d )

#define gteR11R12 ( psxRegs.CP2C.r[ 0 ].sd )
#define gteR22R23 ( psxRegs.CP2C.r[ 2 ].sd )
#define gteR11 ( psxRegs.CP2C.r[ 0 ].sw.l )
#define gteR12 ( psxRegs.CP2C.r[ 0 ].sw.h )
#define gteR13 ( psxRegs.CP2C.r[ 1 ].sw.l )
#define gteR21 ( psxRegs.CP2C.r[ 1 ].sw.h )
#define gteR22 ( psxRegs.CP2C.r[ 2 ].sw.l )
#define gteR23 ( psxRegs.CP2C.r[ 2 ].sw.h )
#define gteR31 ( psxRegs.CP2C.r[ 3 ].sw.l )
#define gteR32 ( psxRegs.CP2C.r[ 3 ].sw.h )
#define gteR33 ( psxRegs.CP2C.r[ 4 ].sw.l )
#define gteTRX ( psxRegs.CP2C.r[ 5 ].sd )
#define gteTRY ( psxRegs.CP2C.r[ 6 ].sd )
#define gteTRZ ( psxRegs.CP2C.r[ 7 ].sd )
#define gteL11 ( psxRegs.CP2C.r[ 8 ].sw.l )
#define gteL12 ( psxRegs.CP2C.r[ 8 ].sw.h )
#define gteL13 ( psxRegs.CP2C.r[ 9 ].sw.l )
#define gteL21 ( psxRegs.CP2C.r[ 9 ].sw.h )
#define gteL22 ( psxRegs.CP2C.r[ 10 ].sw.l )
#define gteL23 ( psxRegs.CP2C.r[ 10 ].sw.h )
#define gteL31 ( psxRegs.CP2C.r[ 11 ].sw.l )
#define gteL32 ( psxRegs.CP2C.r[ 11 ].sw.h )
#define gteL33 ( psxRegs.CP2C.r[ 12 ].sw.l )
#define gteRBK ( psxRegs.CP2C.r[ 13 ].sd )
#define gteGBK ( psxRegs.CP2C.r[ 14 ].sd )
#define gteBBK ( psxRegs.CP2C.r[ 15 ].sd )
#define gteLR1 ( psxRegs.CP2C.r[ 16 ].sw.l )
#define gteLR2 ( psxRegs.CP2C.r[ 16 ].sw.h )
#define gteLR3 ( psxRegs.CP2C.r[ 17 ].sw.l )
#define gteLG1 ( psxRegs.CP2C.r[ 17 ].sw.h )
#define gteLG2 ( psxRegs.CP2C.r[ 18 ].sw.l )
#define gteLG3 ( psxRegs.CP2C.r[ 18 ].sw.h )
#define gteLB1 ( psxRegs.CP2C.r[ 19 ].sw.l )
#define gteLB2 ( psxRegs.CP2C.r[ 19 ].sw.h )
#define gteLB3 ( psxRegs.CP2C.r[ 20 ].sw.l )
#define gteRFC ( psxRegs.CP2C.r[ 21 ].sd )
#define gteGFC ( psxRegs.CP2C.r[ 22 ].sd )
#define gteBFC ( psxRegs.CP2C.r[ 23 ].sd )
#define gteOFX ( psxRegs.CP2C.r[ 24 ].sd )
#define gteOFY ( psxRegs.CP2C.r[ 25 ].sd )
#define gteH   ( psxRegs.CP2C.r[ 26 ].sw.l )
#define gteDQA ( psxRegs.CP2C.r[ 27 ].sw.l )
#define gteDQB ( psxRegs.CP2C.r[ 28 ].sd )
#define gteZSF3 ( psxRegs.CP2C.r[ 29 ].sw.l )
#define gteZSF4 ( psxRegs.CP2C.r[ 30 ].sw.l )
#define gteFLAG ( psxRegs.CP2C.r[ 31 ].d )

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

}

#endif /* __GTE_H__ */
