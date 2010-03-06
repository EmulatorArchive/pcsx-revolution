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

#ifndef __PPC_H__
#define __PPC_H__

// include basic types
#include "psxcommon.h"
#include "ppc_mnemonics.h"

#define PPCARG1 3
#define PPCARG2 4

void Write32(u32 val);
void Write64(u64 val);
void ReleaseArgs();

void CALLFunc(u32 func);

extern u32 *ppcPtr;
extern u32 *b32Ptr[32];

void ppcInit();
void ppcSetPtr(u32 *ptr);
void ppcShutdown();

void ppcAlign(int bytes);

#ifdef __cplusplus
extern "C" {
#endif


void returnPC();
void recRun(void (*func)());

#ifdef __cplusplus
} // extern "C" 
#endif

u8 dynMemRead8(u32 mem);
u16 dynMemRead16(u32 mem);
u32 dynMemRead32(u32 mem);
void dynMemWrite32(u32 mem, u32 val);

#endif /* __PPC_H__ */
