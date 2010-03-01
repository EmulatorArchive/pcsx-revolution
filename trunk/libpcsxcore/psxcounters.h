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

#ifndef __PSXCOUNTERS_H__
#define __PSXCOUNTERS_H__

#include "psxcommon.h"

typedef struct
{
#if defined(__ppc__) || defined(__BIGENDIAN__)
	u32 garbage:22;

	u32 Div:1;

	u32 ClockSource:1;

	u32 unused2:1;

	u32 Repeat:1;

	u32 IRQOVF:1;
	u32 IRQTARGET:1;

	u32 Tar:1;
	
	u32 Reset:1;

	u32 unused:1;

	u32 Disabled:1;
#else
	// General count enable/status.  If 0, no counting happens.
	// This flag is set/unset by the gates.
	u32 Disabled:1;

	u32 unused:1;
	
	u32 Reset:1;

	// 0 Count to $ffff
	// 1 Count to value in target register
	u32 Tar:1;

	//Set both for IRQ on target reached.
	u32 IRQTARGET:1;
	u32 IRQOVF:1;

	u32 Repeat:1;

	u32 unused2:1;

	// 0 - System clock (it seems)
	// 1 - Pixel clock (counter 0)
	//     Horizontal retrace (counter 1)
	u32 ClockSource:1;

	// 0 - System clock (it seems)
	// 1 - 1/8 * System clock (counter 2)
	u32 Div:1;

/*
	When ClockSource of the counters is zero, they all run at the
		same speed. This speed seems to be about 8 times the normal
		speed of root counter 2, which is specified as 1/8 the system
		clock.
*/
#endif
} psxcnt_mode;

typedef struct {
	u32 count, target;
	union {
		u32 mode;
		psxcnt_mode mode_b;
	};
	u32 sCycle, Cycle, rate, interrupt;
	u32 IsCounting:1;
	u32 FutureTarget:1;
} psxCounter;

extern psxCounter psxCounters[3];

void psxRcntUpdate0();
void psxRcntUpdate1();
void psxRcntUpdate2();
void psxRcntUpdate3();
void psxRcntUpdate4();

void psxRcntInit();
void psxRcntWcount(u32 index, u32 value);
void psxRcntWmode(u32 index, u32 value);
void psxRcntWtarget(u32 index, u32 value);
u16 psxRcntRcount(u32 index);
u32 psxRcntRmode(u32 index);
int psxRcntFreeze(gzFile f, int Mode);

void psxUpdateVSyncRate();
void CalcRate(u32 region);

#endif /* __PSXCOUNTERS_H__ */
