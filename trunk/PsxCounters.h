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

#ifndef __PSXCOUNTERS_H__
#define __PSXCOUNTERS_H__

#include "PsxCommon.h"

typedef struct {
	unsigned long count, mode, target;
	unsigned long sCycle, Cycle, rate, interrupt;
} psxCounter;

extern psxCounter psxCounters[5];

void psxRcntInit();
#ifdef _NEW_COUNTER_
void __inline psxRcntUpdate0();
void __inline psxRcntUpdate1();
void __inline psxRcntUpdate2();
void __inline psxRcntUpdate3();
void __inline psxRcntUpdate4();
#else
void psxRcntUpdate();
#endif
void psxRcntWcount(u32 index, u32 value);
void psxRcntWmode(u32 index, u32 value);
void psxRcntWtarget(u32 index, u32 value);
u32 psxRcntRcount(u32 index);
int psxRcntFreeze(gzFile f, int Mode);

void psxUpdateVSyncRate();

#endif /* __PSXCOUNTERS_H__ */
