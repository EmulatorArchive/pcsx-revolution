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

#include "psxhw.h"
#include "r3000a.h"
#include "psxmem.h"
#include "psxcounters.h"
#include "plugins.h"
#include "cheat.h"

static int cnts = 4;
psxCounter psxCounters[3];

typedef struct {
	u32 Render;
	u32 Blank;
	u32 region;
	u32 scans;
	u32 frame;
} vSyncRate_t;

typedef struct {
	u32 rate;
	u32 mode:1;
} vSyncCounter_t;

// I just love bit fields =)
typedef union {
	u32 value;
	struct {
		u32 unused: 16;
		u32 width1: 1;
		u32 width0: 2;
	} width;
} width_t;

static const u32 SpuRate = (768 * 32);

static vSyncRate_t vSyncRate;
static vSyncCounter_t vSyncCounter;

// We need it because of PixelClock rate - 13.5MHz.
static const PsxFixedBits = 12;

static u32 _rcntRate(u32 index) {
	if(index == 2) {
		if(psxCounters[index].mode_b.Div != 0)
			psxCounters[index].rate = 8 << PsxFixedBits; // 1/8 speed
	}
	else if(psxCounters[index].mode_b.ClockSource != 0) {
		if(index == 0) {
			// based on resolution. See http://members.at.infoseek.co.jp/DrHell/ps1/ 
			// something like (vSyncRate.frame / (width * ((vSyncRate.scans / 3) * 2)))
			width_t screen;
			u32 width;
			screen.value = psxHu32(0x1814);
			if(screen.width.width1 != 0) width = 384;
			else {
				switch(screen.width.width0) {
					case 0: width = 256;
					case 1: width = 320;
					case 2: width = 512;
					case 3: width = 640;
				}
			}
			psxCounters[index].rate = (vSyncRate.frame / (width * ((vSyncRate.scans / 3) * 2)));
		}
		else {
			psxCounters[index].rate = (vSyncRate.frame / vSyncRate.scans) * BIAS;
			//SysPrintf("hSync = %d\n", psxCounters[index].rate >> PsxFixedBits);
		}
	}
	return psxCounters[index].rate;
}

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
		s32 count = ((_rcntTarget(index) - psxCounters[index].count) * _rcntRate(index)) / (BIAS << PsxFixedBits);
		if (count > 0)
			psx_int_add(index, count);
	}
}

void psxRcntInit() {

	memset(psxCounters, 0, sizeof(psxCounters));

	CalcRate(Config.PsxType);
	psxUpdateVSyncRate();

	int i;
	for (i = 0; i < 3; i++) {
		psxCounters[i].sCycle = psxRegs.cycle;
		psxCounters[i].rate = 1 << PsxFixedBits;
		psxCounters[i].interrupt = i + 4;
	}
}

void CalcRate(u32 region) {
	if(region & PSX_TYPE_PAL) {
		vSyncRate.frame = ((PSXCLK << PsxFixedBits) / 50);
		vSyncRate.scans = 625;
	}
	else {
		vSyncRate.frame = (((PSXCLK << PsxFixedBits) / 5994) * 100);
		vSyncRate.scans = 525;
	}
	vSyncRate.Blank = (vSyncRate.frame / vSyncRate.scans) * BIAS;
	
/*
  Region: PAL   Render: 675208;         Blank: 2167;
  Region: NTSC  Render: 562892;         Blank: 2152;
*/

	vSyncRate.Render = (vSyncRate.frame - vSyncRate.Blank) / (BIAS << PsxFixedBits);
	vSyncRate.Blank /= (BIAS << PsxFixedBits);
	vSyncRate.region = region;
// 	SysPrintf("Region: %s\t frame: %d\t Render: %d;\t Blank: %d;\n", vSyncRate.region ? "PAL" : "NTSC", vSyncRate.frame >> PsxFixedBits, vSyncRate.Render, vSyncRate.Blank);
}

void psxUpdateVSyncRate() {
// 	psxRaiseExtInt(PsxInt_VBlankEnd);		// Should be here, but cause only troubles.
	vSyncCounter.rate = vSyncRate.Render;
	if (Config.VSyncWA) vSyncCounter.rate/= 2;
}

void psxUpdateVSyncRateEnd() {
	psxRaiseExtInt(PsxInt_VBlank);
	vSyncCounter.rate = vSyncRate.Blank;
	if (Config.VSyncWA) vSyncCounter.rate/= 2;
}

void psxRcntUpdate3() {
	if(Config.PsxType != vSyncRate.region)
		CalcRate(Config.PsxType);

	if (vSyncCounter.mode) { // VSync End (22 hsyncs)
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
		psxUpdateVSyncRateEnd();
	}
	psx_int_add(3, vSyncCounter.rate);
	vSyncCounter.mode ^= 1;
}

static __inline void update_cnt(u32 index) {
	if (psxCounters[index].mode_b.IRQTARGET != 0) {
		psxCounters[index].count = psxCounters[index].target;
		psxRaiseExtInt(psxCounters[index].interrupt);

		if(psxCounters[index].mode_b.Tar != 0) {
			// Reset on target
			psxCounters[index].count = 0;
		}
	}

	if (psxCounters[index].mode_b.IRQOVF != 0) {
		psxCounters[index].count = 0;
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
		SPU_async(SpuRate);		// Peops SPU doesn't really matter what we send.
		psx_int_add(4, SpuRate);
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

	psxRcntUpd(index);
}

void psxRcntWtarget(u32 index, u32 value) {
//	SysPrintf("writeCtarget[%ld] = %lx\n", index, value);
	psxCounters[index].target = value;
#if 1
	if(psxIsActiveEvent(index)) \
		psxCounters[index].count = psxRcntRcount(index);
#endif
	psxRcntUpd(index);
}

u16 psxRcntRcount(u32 index) {
	if (psxCounters[index].mode_b.Disabled != 0) return psxCounters[index].count;

	u64 ret = psxCounters[index].count;

	if(Config.RCntFix) {
		ret += ((psxGetCycle() - psxCounters[index].sCycle) << PsxFixedBits) / _rcntRate(index);
	} 
	else {
		ret += ((psxGetCycle() - psxCounters[index].sCycle) * (BIAS << PsxFixedBits)) / _rcntRate(index);
	}

	return ret;
}

int psxRcntFreeze(gzFile f, int Mode) {
	char Unused[4096 - sizeof(psxCounter)];

	gzfreezel(psxCounters);
	gzfreezel(Unused);

	return 0;
}
 
