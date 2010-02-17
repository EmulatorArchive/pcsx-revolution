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
#include "psxhw.h"

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

// We need it because of PixelClock rate - 13.5MHz.
static const PsxFixedBits = 12;

static void psxRcntUpd(unsigned long index) {
#ifndef NEW_EVENTS
u32 cycle = psxRegs.cycle;
#else
u32 cycle = psxGetCycle();
#endif
	psxCounters[index].sCycle = cycle;
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
	if (psxCounters[index].mode & 0x30) {
#ifndef NEW_EVENTS
		psxHu32ref(0x1070)|= SWAPu32(psxCounters[index].interrupt);
		psxRegs.interrupt|= 0x80000000;
#else
		psxRaiseExtInt(psxCounters[index].interrupt);
#endif
}
	if (!(psxCounters[index].mode & 0x40)) { // Only 1 interrupt
		psxCounters[index].Cycle = 0xffffffff;
	} // else Continuos interrupt mode
}

#ifdef SEPARATE_CNTS

static void psxRcntSet(u32 index) {
	if (psxCounters[index].Cycle == 0xffffffff) return;

	s32 count = psxCounters[index].Cycle - (psxGetCycle() - psxCounters[index].sCycle);

	if (count < 0) return;
	psx_int_add(index, count);

}

#else

static void psxRcntSet() {
	int i;
#ifndef NEW_EVENTS
u32 cycle = psxRegs.cycle;
#else
u32 cycle = psxGetCycle();
#endif
	psxNextCounter = 0x7fffffff;
	psxNextsCounter = cycle;

	for (i=0; i<cnts; i++) {
		s32 count;

		if (psxCounters[i].Cycle == 0xffffffff) continue;

		count = psxCounters[i].Cycle - (cycle - psxCounters[i].sCycle);

		if (count < 0) {
			psxNextCounter = 0; break;
		}

		if (count < (s32)psxNextCounter) {
			psxNextCounter = count;
		}
	}
#ifdef NEW_EVENTS
	psx_int_add(0, psxNextCounter);
#endif
}

#endif

void psxRcntInit() {

	memset(psxCounters, 0, sizeof(psxCounters));
#ifndef NEW_EVENTS
	psxCounters[0].rate = 1 << PsxFixedBits; psxCounters[0].interrupt = 0x10;
	psxCounters[1].rate = 1 << PsxFixedBits; psxCounters[1].interrupt = 0x20;
	psxCounters[2].rate = 1 << PsxFixedBits; psxCounters[2].interrupt = 0x40;
	psxCounters[3].interrupt = 1;
#else
	psxCounters[0].rate = 1 << PsxFixedBits; psxCounters[0].interrupt = 4;
	psxCounters[1].rate = 1 << PsxFixedBits; psxCounters[1].interrupt = 5;
	psxCounters[2].rate = 1 << PsxFixedBits; psxCounters[2].interrupt = 6;
	psxCounters[3].interrupt = 0;
#endif
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

// 	psxRcntUpd(0); psxRcntUpd(1); psxRcntUpd(2); psxRcntUpd(3);
// 	psxRcntSet();
int i;
	for (i=0; i<5; i++)
		psxCounters[i].sCycle = psxRegs.cycle;
}

#define NEWRATE

static void CalcRate(u32 region) {
	u32 rate;
#ifndef NEWRATE
	if(region) { // PAL
		rate = (PSXCLK << PsxFixedBits / 50);
	}
	else
	{
		rate = (PSXCLK << PsxFixedBits / 60);
	}
	vSyncRate.Blank = (rate / 262) * 22;
/*
  Region: PAL   Render: 620506;         Blank: 56870;
  Region: NTSC  Render: 517092;         Blank: 47388;
*/
#else
	if(region & PSX_TYPE_PAL) {
		rate = ((PSXCLK << PsxFixedBits) / 50);
		vSyncRate.scans = 625;
	}
	else {
		rate = (((PSXCLK << PsxFixedBits) / 5994) * 100);
		vSyncRate.scans = 525;
	}
	vSyncRate.Blank = (rate / vSyncRate.scans) * BIAS;
	
/*
  Region: PAL   Render: 675208;         Blank: 2167;
  Region: NTSC  Render: 562892;         Blank: 2152;
*/
#endif
	vSyncRate.Render = rate - vSyncRate.Blank;
	vSyncRate.region = region;
	//SysPrintf("Region: %s\t Render: %d;\t Blank: %d;\n", vSyncRate.region ? "PAL" : "NTSC", vSyncRate.Render >> PsxFixedBits, vSyncRate.Blank >> PsxFixedBits);
}

void psxUpdateVSyncRate() {
	psxCounters[3].rate = vSyncRate.Render;
	if (Config.VSyncWA) psxCounters[3].rate/= 2;
}

void psxUpdateVSyncRateEnd() {
	psxRaiseExtInt(PsxInt_VBlank);
	psxCounters[3].rate = vSyncRate.Blank;
	if (Config.VSyncWA) psxCounters[3].rate/= 2;
}

#ifdef SEPARATE_CNTS
void psxRcntUpdate3() {
	if(Config.PsxType != vSyncRate.region) CalcRate(Config.PsxType);
	if (psxCounters[3].mode & 0x10000) { // VSync End (22 hsyncs)
		psxCounters[3].mode&=~0x10000;
		psxUpdateVSyncRate();
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
		
	}
	psx_int_add(3, psxCounters[3].rate / (BIAS << PsxFixedBits));
}

void psxRcntUpdate0() {
	psxRcntReset(0);
	psxRcntSet(0);
}

void psxRcntUpdate1() {
	psxRcntReset(1);
	psxRcntSet(1);
}

void psxRcntUpdate2() {
	psxRcntReset(2);
	psxRcntSet(2);
}

void psxRcntUpdate4() {
	if (SPU_async) {
		SPU_async((psxRegs.cycle - psxCounters[4].sCycle) * BIAS);
	}
	psx_int_add(4, psxCounters[4].rate / (BIAS << PsxFixedBits));
}
#else

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
			psxHu32ref(0x1070)|= SWAPu32(psxCounters[3].interrupt);
#ifndef NEW_EVENTS
			psxRegs.interrupt|= 0x80000000;
#else
			psxTestIntc();
#endif
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
#endif

void psxRcntWcount(u32 index, u32 value) {
	psxCounters[index].count = value;
	psxRcntUpd(index);
#ifdef SEPARATE_CNTS
	psxRcntSet(index);
#else
	psxRcntSet();
#endif
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
				psxCounters[index].rate = ((vSyncRate.Render) / vSyncRate.scans) * BIAS;
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
#ifdef SEPARATE_CNTS
	psxRcntSet(index);
#else
	psxRcntSet();
#endif
}

void psxRcntWtarget(u32 index, u32 value) {
//	SysPrintf("writeCtarget[%ld] = %lx\n", index, value);
	psxCounters[index].target = value;
	psxRcntUpd(index);
#ifdef SEPARATE_CNTS
	psxRcntSet(index);
#else
	psxRcntSet();
#endif
}

u32 psxRcntRcount(u32 index) {
	u32 ret = psxCounters[index].count;
#ifndef NEW_EVENTS
u32 cycle = psxRegs.cycle;
#else
u32 cycle = psxGetCycle();
#endif
	if (psxCounters[index].mode & 0x08) { // Wrap at target
		if (Config.RCntFix) { // Parasite Eve 2
			ret += (/*BIAS **/ (((cycle - psxCounters[index].sCycle) << PsxFixedBits) / psxCounters[index].rate)) & 0xffff;
		} else {
			ret += (BIAS * (((cycle - psxCounters[index].sCycle) << PsxFixedBits) / psxCounters[index].rate)) & 0xffff;
		}
	} else { // Wrap at 0xffff
		ret += (BIAS * ((cycle << PsxFixedBits) / psxCounters[index].rate)) & 0xffff;
		if (Config.RCntFix) { // Vandal Hearts 1/2
			ret/= 16;
		}
	}

	//SysPrintf("readCcount[%ld] = %lx (mode %lx, target %lx, cycle %lx)\n", index, ret, psxCounters[index].mode, psxCounters[index].target, psxRegs.cycle);

	return ret;
}

int psxRcntFreeze(gzFile f, int Mode) {
	char Unused[4096 - sizeof(psxCounter)];

	gzfreezel(psxCounters);
	gzfreezel(Unused);

	return 0;
}
