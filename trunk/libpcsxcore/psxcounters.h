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

#ifndef __PSXCOUNTERS_H__
#define __PSXCOUNTERS_H__

#include "psxcommon.h"
#include "psxhw.h"
#include "r3000a.h"
#include "psxevents.h"

namespace R3000A {

void psxRcntUpdate0();
void psxRcntUpdate1();
void psxRcntUpdate2();
void psxRcntVSync();
void psxRcntVBlank();

void RcntAdvanceCycles(s32);
void psxRcntInit();

void psxRcntWcount(PsxEventType index, u16 value);
void psxRcntWtarget(PsxEventType index, u16 value);
void psxRcntWmode(PsxEventType index, u32 value);
u16 psxRcntRcount(PsxEventType index);
u32 psxRcntRmode(PsxEventType index);
u16 psxRcntRtarget(PsxEventType index);

int psxRcntFreeze(gzFile f, int Mode);

void CalcRate(u32 region);

}; // namespace

#endif /* __PSXCOUNTERS_H__ */
