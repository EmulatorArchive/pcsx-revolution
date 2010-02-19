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
	u32 frame;
} vSyncRate_t;

static vSyncRate_t vSyncRate;
static void CalcRate(u32 region);

// We need it because of PixelClock rate - 13.5MHz.
static const PsxFixedBits = 12;

static __inline u32 _rcntTarget(u32 index) {
	if(psxCounters[index].mode_b.Tar != 0 ||
		psxCounters[index].mode_b.IRQTARGET != 0)
	{
		return psxCounters[index].target;
	}
	return 0x10000;
}

static void psxRcntUpd(unsigned long index) {
	if (psxCounters[index].mode_b.Disabled != 0) return;

	psxCounters[index].sCycle = psxGetCycle();

	if (psxCounters[index].mode_b.IRQTARGET != 0 || psxCounters[index].mode_b.IRQOVF != 0) {
		s32 count = ((_rcntTarget(index) - psxRcntRcount(index)) * psxCounters[index].rate) / (BIAS << PsxFixedBits);

		if (count > 0) 
			psx_int_add(index, count);
	}
}

void psxRcntInit() {

	memset(psxCounters, 0, sizeof(psxCounters));

	psxCounters[0].rate = 1 << PsxFixedBits; psxCounters[0].interrupt = PsxInt_RTC0;
	psxCounters[1].rate = 1 << PsxFixedBits; psxCounters[1].interrupt = PsxInt_RTC1;
	psxCounters[2].rate = 1 << PsxFixedBits; psxCounters[2].interrupt = PsxInt_RTC2;
	psxCounters[3].interrupt = PsxInt_VBlank;

	CalcRate(Config.PsxType);
	psxUpdateVSyncRate();

	if (SPU_async != NULL) {
		psxCounters[4].rate = (768 * 32);	// Where did it came from?
	}

	int i;
	for (i = 0; i < 4; i++)
		psxCounters[i].sCycle = psxRegs.cycle;
}

static void CalcRate(u32 region) {
	u32 rate;

	if(region & PSX_TYPE_PAL) {
		vSyncRate.frame = ((PSXCLK << PsxFixedBits) / 50);
		vSyncRate.scans = 625;
	}
	else {
		vSyncRate.frame = (((PSXCLK << PsxFixedBits) / 5994) * 100);
		vSyncRate.scans = 525;
	}
	vSyncRate.Blank = (vSyncRate.frame / vSyncRate.scans) * 2;
	
/*
  Region: PAL   Render: 675208;         Blank: 2167;
  Region: NTSC  Render: 562892;         Blank: 2152;
*/

	vSyncRate.Render = vSyncRate.frame - vSyncRate.Blank;
	vSyncRate.region = region;
	SysPrintf("Region: %s\t Render: %d;\t Blank: %d;\n", vSyncRate.region ? "PAL" : "NTSC", vSyncRate.Render >> PsxFixedBits, vSyncRate.Blank >> PsxFixedBits);
}

void psxUpdateVSyncRate() {
	//psxRaiseExtInt(PsxInt_VBlankEnd);		// Should be here, but cause only troubles.
	psxCounters[3].rate = vSyncRate.Render;
	if (Config.VSyncWA) psxCounters[3].rate/= 2;
}

void psxUpdateVSyncRateEnd() {
	psxRaiseExtInt(PsxInt_VBlank);
	psxCounters[3].rate = vSyncRate.Blank;
	if (Config.VSyncWA) psxCounters[3].rate/= 2;
}

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

static __inline void update_cnt(u32 index) {
	psxCounters[index].count = 0;

	if (psxCounters[index].mode_b.IRQTARGET != 0 || psxCounters[index].mode_b.IRQOVF != 0) {
		psxRaiseExtInt(psxCounters[index].interrupt);
	}
	if (psxCounters[index].mode_b.Repeat != 0) {
		psxRcntUpd(index);
	}
}

void psxRcntUpdate0() {
	update_cnt(0);
}

void psxRcntUpdate1() {
	update_cnt(1);
}

void psxRcntUpdate2() {
	update_cnt(2);
}

void psxRcntUpdate4() {
	if (SPU_async) {
		SPU_async(psxCounters[4].rate);		// Peops SPU doesn't really matter what we send.
		psx_int_add(4, psxCounters[4].rate);
	}
}

void psxRcntWcount(u32 index, u32 value) {
	psxCounters[index].count = value;
	psxRcntUpd(index);
}

void psxRcntWmode(u32 index, u32 value)  {
//	SysPrintf("writeCmode[%ld] = %lx\n", index, value);
	psxCounters[index].mode = value;
	if(psxCounters[index].mode_b.Reset != 0)
		psxCounters[index].count = 0;

	psxCounters[index].rate = 1 << PsxFixedBits;

	if(index == 2) {
		if(psxCounters[index].mode_b.Div != 0)
			psxCounters[index].rate = 8 << PsxFixedBits; // 1/8 speed
	}
	else if(psxCounters[index].mode_b.ClockSource != 0) {
		if(index == 0) {
			psxCounters[index].rate = (PSXCLK << PsxFixedBits) / 13500000;	// based on resolution. See http://members.at.infoseek.co.jp/DrHell/ps1/
		}
		else {
			psxCounters[index].rate = (vSyncRate.Render / vSyncRate.scans) * BIAS;
			//SysPrintf("hSync = %d\n", psxCounters[index].rate >> PsxFixedBits);
		}
	}

	psxRcntUpd(index);
}

void psxRcntWtarget(u32 index, u32 value) {
//	SysPrintf("writeCtarget[%ld] = %lx\n", index, value);
	psxCounters[index].target = value;
	psxRcntUpd(index);
}

u16 psxRcntRcount(u32 index) {
	u64 ret = psxCounters[index].count;
	if (psxCounters[index].mode_b.Disabled != 0) return ret;

	if(Config.RCntFix) {
		ret += ((psxGetCycle() - psxCounters[index].sCycle) << PsxFixedBits) / psxCounters[index].rate;
	} 
	else {
		ret += ((psxGetCycle() - psxCounters[index].sCycle) * (BIAS << PsxFixedBits)) / psxCounters[index].rate;
	}

	return ret;
}

int psxRcntFreeze(gzFile f, int Mode) {
	char Unused[4096 - sizeof(psxCounter)];

	gzfreezel(psxCounters);
	gzfreezel(Unused);

	return 0;
}
 
