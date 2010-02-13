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

/*
* Internal PSX counters.
*/

#include "psxcounters.h"
#include "cheat.h"

static int cnts = 4;
psxCounter psxCounters[5];

typedef struct {
	u32 Render;
	u32 Blank;
	u32 region;
	u32 scans;
} vSyncRate_t;

static vSyncRate_t vSyncRate;
static void CalcRate(u32 region);

static const PsxFixedBits = 12;

static void psxRcntUpd(unsigned long index) {
	psxCounters[index].sCycle = psxRegs.cycle;
	if (((!(psxCounters[index].mode & 1)) || (index!=2)) &&
		psxCounters[index].mode & 0x30) {
		if (psxCounters[index].mode & 0x10) { // Interrupt on target
			psxCounters[index].Cycle = ((psxCounters[index].target - psxCounters[index].count) * psxCounters[index].rate) / (BIAS << PsxFixedBits);
		} else { // Interrupt on 0xffff
			psxCounters[index].Cycle = ((0xffff - psxCounters[index].count) * psxCounters[index].rate) / (BIAS << PsxFixedBits);
		}
	} else psxCounters[index].Cycle = 0xffffffff;
//	if (index == 2) SysPrintf("Cycle %x\n", psxCounters[index].Cycle);
}

static void psxRcntReset(unsigned long index) {
//	SysPrintf("psxRcntReset %x (mode=%x)\n", index, psxCounters[index].mode);
	psxCounters[index].count = 0;
	psxRcntUpd(index);

//	if (index == 2) SysPrintf("rcnt2 %x\n", psxCounters[index].mode);
	psxHu32ref(0x1070)|= SWAPu32(psxCounters[index].interrupt);
	psxRegs.interrupt|= 0x80000000;
	if (!(psxCounters[index].mode & 0x40)) { // Only 1 interrupt
		psxCounters[index].Cycle = 0xffffffff;
	} // else Continuos interrupt mode
}

static void psxRcntSet() {
	int i;

	psxNextCounter = 0x7fffffff;
	psxNextsCounter = psxRegs.cycle;

	for (i=0; i<cnts; i++) {
		s32 count;

		if (psxCounters[i].Cycle == 0xffffffff) continue;

		count = psxCounters[i].Cycle - (psxRegs.cycle - psxCounters[i].sCycle);

		if (count < 0) {
			psxNextCounter = 0; break;
		}

		if (count < (s32)psxNextCounter) {
			psxNextCounter = count;
		}
	}
}

void psxRcntInit() {

	memset(psxCounters, 0, sizeof(psxCounters));

	psxCounters[0].rate = 1 << PsxFixedBits; psxCounters[0].interrupt = 0x10;
	psxCounters[1].rate = 1 << PsxFixedBits; psxCounters[1].interrupt = 0x20;
	psxCounters[2].rate = 1 << PsxFixedBits; psxCounters[2].interrupt = 0x40;

	psxCounters[3].interrupt = 1;
	psxCounters[3].mode = 0x58; // The VSync counter mode
	psxCounters[3].target = 1;
	CalcRate(Config.PsxType);
	psxUpdateVSyncRate();

	if (SPU_async != NULL) {
		cnts = 5;

		psxCounters[4].rate = (768 * 64) << PsxFixedBits;
		psxCounters[4].target = 1;
		psxCounters[4].mode = 0x58;
	} else cnts = 4;

	psxRcntUpd(0); psxRcntUpd(1); psxRcntUpd(2); psxRcntUpd(3);
	psxRcntSet();
}

#define NEWRATE

static void CalcRate(u32 region) {
	u32 rate;
#ifndef NEWRATE
	if(region) { // PAL
		rate = (PSXCLK / 50);
	}
	else
	{
		rate = (PSXCLK / 60);
	}
	vSyncRate.Blank = (rate / 262) * 22;
/*
  Region: PAL   Render: 620506;         Blank: 56870;
  Region: NTSC  Render: 517092;         Blank: 47388;
*/
#else
	if(region) { // PAL
		rate = ((PSXCLK << PsxFixedBits) / 50);
		vSyncRate.scans = 625;
	}
	else
	{
		rate = (((PSXCLK << PsxFixedBits) * 100) / 5994);
		vSyncRate.scans = 525;
	}
	vSyncRate.Blank = (rate / vSyncRate.scans) * 2;
	
/*
  Region: PAL   Render: 675210;         Blank: 2166;
  Region: NTSC  Render: 562893;         Blank: 2152;
*/
#endif
	vSyncRate.Render = rate - vSyncRate.Blank;
	vSyncRate.region = region;
	//SysPrintf("Region: %s\t Render: %d;\t Blank: %d;\n", vSyncRate.region ? "PAL" : "NTSC", vSyncRate.Render, vSyncRate.Blank);
}

void psxUpdateVSyncRate() {
	psxCounters[3].rate = vSyncRate.Render;
	if (Config.VSyncWA) psxCounters[3].rate/= 2;
}

void psxUpdateVSyncRateEnd() {
	psxCounters[3].rate = vSyncRate.Blank;
	if (Config.VSyncWA) psxCounters[3].rate/= 2;
}

void psxRcntUpdate() {
	if ((psxRegs.cycle - psxCounters[3].sCycle) >= psxCounters[3].Cycle) {
		if(Config.PsxType != vSyncRate.region) CalcRate(Config.PsxType);
		if (psxCounters[3].mode & 0x10000) { // VSync End (22 hsyncs)
			psxCounters[3].mode&=~0x10000;
			psxUpdateVSyncRate();
			psxRcntUpd(3);
			GPU_updateLace(); // updateGPU
			SysUpdate();
#ifndef GEKKO
			ApplyCheats();
#endif
#ifdef GTE_LOG
			GTE_LOG("VSync\n");
#endif
		} else { // VSync Start (240 hsyncs) 
			psxCounters[3].mode|= 0x10000;
			psxUpdateVSyncRateEnd();
			psxRcntUpd(3);
			psxHu32ref(0x1070)|= SWAPu32(1);
			psxRegs.interrupt|= 0x80000000;
		}
	}

	if ((psxRegs.cycle - psxCounters[0].sCycle) >= psxCounters[0].Cycle) {
		psxRcntReset(0);
	}

	if ((psxRegs.cycle - psxCounters[1].sCycle) >= psxCounters[1].Cycle) {
		psxRcntReset(1);
	}

	if ((psxRegs.cycle - psxCounters[2].sCycle) >= psxCounters[2].Cycle) {
		psxRcntReset(2);
	}

	if (cnts >= 5) {
		if ((psxRegs.cycle - psxCounters[4].sCycle) >= psxCounters[4].Cycle) {
			SPU_async((psxRegs.cycle - psxCounters[4].sCycle) * BIAS);
			psxRcntReset(4);
		}
	}

	psxRcntSet();
#ifndef GEKKO	
	DebugVSync();
#endif
}

void psxRcntWcount(u32 index, u32 value) {
	psxCounters[index].count = value;
	psxRcntUpd(index);
	psxRcntSet();
}

void psxRcntWmode(u32 index, u32 value)  {
//	SysPrintf("writeCmode[%ld] = %lx\n", index, value);
	psxCounters[index].mode = value;
	psxCounters[index].count = 0;
	if(index == 0) {
		switch (value & 0x300) {
			case 0x100:
#ifdef NEWRATE
				psxCounters[index].rate = (PSXCLK << PsxFixedBits) / 135000;
#else
				psxCounters[index].rate = ((psxCounters[3].rate /** BIAS*/) / 386) / 262; // seems ok
#endif
				break;
			default:
				psxCounters[index].rate = 1 << PsxFixedBits;
		}
	}
	else if(index == 1) {
		switch (value & 0x300) {
			case 0x100:
#ifdef NEWRATE
				psxCounters[index].rate = ((vSyncRate.Render) / vSyncRate.scans) * 2;
#else
				psxCounters[index].rate = (psxCounters[3].rate /** BIAS*/) / 262; // seems ok
#endif
				break;
			default:
				psxCounters[index].rate = 1 << PsxFixedBits;
		}
	}
	else if(index == 2) {
		switch (value & 0x300) {
			case 0x200:
				psxCounters[index].rate = 8 << PsxFixedBits; // 1/8 speed
				break;
			default:
				psxCounters[index].rate = 1 << PsxFixedBits; // normal speed
		}
	}

	// Need to set a rate and target
	psxRcntUpd(index);
	psxRcntSet();
}

void psxRcntWtarget(u32 index, u32 value) {
//	SysPrintf("writeCtarget[%ld] = %lx\n", index, value);
	psxCounters[index].target = value;
	psxRcntUpd(index);
	psxRcntSet();
}

u32 psxRcntRcount(u32 index) {
	u32 ret;

//	if ((!(psxCounters[index].mode & 1)) || (index!=2)) {
		if (psxCounters[index].mode & 0x08) { // Wrap at target
			if (Config.RCntFix) { // Parasite Eve 2
				ret = (psxCounters[index].count + /*BIAS **/ (((psxRegs.cycle - psxCounters[index].sCycle) << PsxFixedBits) / psxCounters[index].rate)) & 0xffff;
			} else {
				ret = (psxCounters[index].count + BIAS * (((psxRegs.cycle - psxCounters[index].sCycle) << PsxFixedBits) / psxCounters[index].rate)) & 0xffff;
			}
		} else { // Wrap at 0xffff
			ret = (psxCounters[index].count + BIAS * ((psxRegs.cycle << PsxFixedBits) / psxCounters[index].rate)) & 0xffff;
			if (Config.RCntFix) { // Vandal Hearts 1/2
				ret/= 16;
			}
		}
//		return (psxCounters[index].count + BIAS * ((psxRegs.cycle - psxCounters[index].sCycle) / psxCounters[index].rate)) & 0xffff;
//	} else return 0;

//	SysPrintf("readCcount[%ld] = %lx (mode %lx, target %lx, cycle %lx)\n", index, ret, psxCounters[index].mode, psxCounters[index].target, psxRegs.cycle);

	return ret;
}

int psxRcntFreeze(gzFile f, int Mode) {
	char Unused[4096 - sizeof(psxCounter)];

	gzfreezel(psxCounters);
	gzfreezel(Unused);

	return 0;
}
