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

#include "PsxCounters.h"
#include "Cheat.h"
#include "R3000A/R3000A.h"
#include "PsxMem.h"
#include "PsxHw.h"

psxCounter psxCounters[5];

static u32 vRenderRate, vBlankRate;
static float FrameRate = 0.0f;

static const u32 PSXCNT_ENABLE_GATE = (1<<0);	// enables gate-based counters
/* frames per second * 100 */
static const uint FRAMERATE_NTSC = 5994;
static const uint FRAMERATE_PAL  = 5000;
/* scanlines */
static const uint SCANLINES_NTSC = 525;
static const uint SCANLINES_PAL  = 625;

static const uint PSXSOUNDCLK = 48000;

typedef struct
{
	u32 hBlank0:1;		// counter 0 is counting HBLANKs (16bit)
	u32 vBlank1:1;		// counter 1 is counting VBLANKs (16bit)
	u32 vBlank3:1;		// Counter 3 is counting VBLANKs in 32 bit style.
}  PsxGateFlags_t;

static PsxGateFlags_t psxGateFlags;

static void CheckEndGate(int index);
static void CheckStartGate(int index);

static void _psxRcntSet(int i) {
	if( psxCounters[i].IsFutureTarget ) return;

	s32 count = psxCounters[i].Cycle - (psxGetCycle() - psxCounters[i].sCycle);

	if (count > 0)
		psx_int_add(i, count);
}

static void psxRcntUpd(int index) {
	if ( !psxCounters[index].IsCounting ) return;

	u32 count = psxCounters[index].count;
	psxCounters[index].count = psxCounters[index].target;

	if( !psxCounters[index].IsFutureTarget )
	{
		if (psxCounters[index].mode & 0x10) {	// Interrupt on target
			if( psxCounters[index].mode & 0x80 )
				psxCounters[index].mode &= ~0x0400;	// Interrupt flag
			psxCounters[index].mode |= 0x0800;	// Target flag
			psxHu32ref(0x1070) |= SWAPu32(psxCounters[index].interrupt);
			psxCounters[index].Cycle = (psxCounters[index].target - count) * psxCounters[index].rate;
		} 

		if(psxCounters[index].mode & 0x08) {	// Reset
			psxCounters[index].count = 0;
			if (!(psxCounters[index].mode & 0x40)) { // Only 1 interrupt
				psxCounters[index].IsFutureTarget = 1;
			}
		}
		else
			psxCounters[index].IsFutureTarget = 1;
	}

	if (psxCounters[index].mode & 0x20) {	// Interrupt on overflow
		psxCounters[index].mode |= 0x1000;	// Overflow flag
		if(psxCounters[index].mode & 0x80)
			psxCounters[index].mode &= ~0x0400;	// Interrupt flag
		psxHu32ref(0x1070) |= SWAPu32(psxCounters[index].interrupt);
		psxCounters[index].Cycle = (0x10000 - count) * psxCounters[index].rate;
	}
	_psxRcntSet(index);
}

static void ResetCount(u32 index, u32 value)
{
	psxCounters[index].count = value;
	psxCounters[index].IsFutureTarget = 0;
	if (psxCounters[index].IsCounting)
	{
		psxRcntUpd(index);
	}
}

void psxRcntInit() {
	int i;
	memset(psxCounters, 0, sizeof(psxCounters));

	psxCounters[0].rate = 1; psxCounters[0].interrupt = 0x10;
	psxCounters[1].rate = 1; psxCounters[1].interrupt = 0x20;
	psxCounters[2].rate = 1; psxCounters[2].interrupt = 0x40;

	psxUpdateVSyncRate();

	if (SPU_async) {
		psxCounters[4].rate = PSXSOUNDCLK;
	}

	for (i=0; i<3; i++)
	{
		psxCounters[i].IsCounting = 1;
		psxCounters[i].sCycle = psxRegs.cycle;
	}

}

void CalcRate(u32 frames, u32 scans)
{
	FrameRate = frames;

	float frame = (PSXCLK / FrameRate) * 100;

	// frame
	vBlankRate = PSXCLK / scans;
	vRenderRate = frame - vBlankRate;

	// half frame
	vRenderRate /= 2;
	vBlankRate /= 2;
}

void psxUpdateVSyncRate() {
	psxCounters[3].rate = vRenderRate;
	if( psxGateFlags.hBlank0 ) CheckStartGate(0);
	if( psxGateFlags.vBlank1 ) CheckStartGate(1);
	if( psxGateFlags.vBlank3 ) CheckStartGate(3);

}

void psxUpdateVSyncRateEnd() {
	psxCounters[3].rate = vBlankRate;
	if( psxGateFlags.hBlank0 ) CheckEndGate(0);
	if( psxGateFlags.vBlank1 ) CheckEndGate(1);
	if( psxGateFlags.vBlank3 ) CheckEndGate(3);
}

static inline void cnt_upd(int i)
{
	psxCounters[i].sCycle = psxRegs.cycle;
	psxRcntUpd(i);
}

void psxRcntUpdate0() {
	cnt_upd(0);
}

void psxRcntUpdate1() {
	cnt_upd(1);
}

void psxRcntUpdate2() {
	cnt_upd(2);
}

void psxRcntUpdate3() {
	if(Config.PsxType)
	{
		if(FrameRate != FRAMERATE_PAL)
			CalcRate(FRAMERATE_PAL, SCANLINES_PAL);
	}
	else
	{
		if(FrameRate != FRAMERATE_NTSC)
			CalcRate(FRAMERATE_NTSC, SCANLINES_NTSC);
	}
	if (psxCounters[3].mode & 0x10000) { // VSync End (22 hsyncs)
		psxCounters[3].mode&=~0x10000;
		psxUpdateVSyncRate();
		psxHu32ref(0x1070)|= SWAPu32(0x800);
		GPU_updateLace(); // updateGPU
		SysUpdate();
		ApplyCheats();
#ifdef GTE_LOG
		GTE_LOG("VSync\n");
#endif
	} else { // VSync Start (240 hsyncs) 
		psxCounters[3].mode|= 0x10000;
		psxUpdateVSyncRateEnd();
		psxHu32ref(0x1070)|= SWAPu32(1);
	}
	psx_int_add(3, psxCounters[3].rate/* / BIAS*/);
}


void psxRcntUpdate4() {
	if (SPU_async) {
		psxCounters[4].sCycle = psxRegs.cycle;
		SPU_async(psxCounters[4].rate);
		psx_int_add(4, psxCounters[4].rate);
	}
}

void psxRcntWcount(int index, u32 value) {
	ResetCount(index, value);
//	SysPrintf("writeCcount[%d] = %x\n", index, value);
//	PSXCPU_LOG("writeCcount[%d] = %x\n", index, value);
}

static int PixelClock()
{
	return (PSXCLK / 13500000);
}

static u32 hSync()
{
	u32 scanlines;
	u32 framerate;
	if(Config.PsxType)
	{
		scanlines = SCANLINES_PAL;
		framerate = FRAMERATE_PAL;
	}
	else
	{
		scanlines = SCANLINES_NTSC;
		framerate = FRAMERATE_NTSC;
	}
	return ((PSXCLK * 100) / (scanlines * framerate));
}

void psxRcntWmode(int index, u32 value)  {
	//SysPrintf("writeCmode[%ld] = %lx\n", index, value);
	psxCounters[index].mode = value | 0x0400;
	psxCounters[index].count = 0;
	psxCounters[index].IsCounting = 1;

	if(index == 2) {
		psxCounters[index].rate = 1; // normal speed

		if(psxCounters[index].mode_st.Div)
			psxCounters[index].rate = 8; // 1/8 speed
	}
	else
	{
		psxCounters[index].rate = 1;

		if (psxCounters[index].mode_st.ClockSource)
		{
			if(index == 0)
				psxCounters[index].rate = PixelClock();
			else
				psxCounters[index].rate = hSync();
		}
	}
	
	if( psxCounters[index].mode & PSXCNT_ENABLE_GATE )
	{
		psxCounters[index].IsCounting = 0;
		if( index == 0 )
			psxGateFlags.hBlank0 = 1;
		else
			psxGateFlags.vBlank1 = 1;
	}
	else
	{
		if( index == 0 )
			psxGateFlags.hBlank0 = 0;
		else
			psxGateFlags.vBlank1 = 0;
	}

	// Need to set a rate and target
	ResetCount(index, 0);
}

void psxRcntWtarget(int index, u32 value) {
//	SysPrintf("writeCtarget[%ld] = %lx\n", index, value);
	psxCounters[index].target = value;

	s32 cycle = psxGetCycle();
	if (psxCounters[index].IsCounting)
	{
		u32 change = cycle - psxCounters[index].sCycle;
		if( change > 0 )
		{
			psxCounters[index].count += change / psxCounters[index].rate;
			change -= (change / psxCounters[index].rate) * psxCounters[index].rate;
			psxCounters[index].sCycle = cycle - change;
		}
	}
	else psxCounters[index].sCycle = cycle;
	
	if( psxCounters[index].count >= psxCounters[index].target ) \
		psxCounters[index].IsFutureTarget = 1;

	psxRcntUpd(index);
}

u32 psxRcntRcount(int index) {
//	SysPrintf("readCcount[%d] = %lx\n", index, psxCounters[index].mode);

	u32 ret = psxCounters[index].count;
	if(psxCounters[index].IsCounting)
	{
		s32 delta = ((psxGetCycle() - psxCounters[index].sCycle) / psxCounters[index].rate);
		ret += delta;
	}

//	SysPrintf("readCcount[%ld] = %lx (mode %lx, target %lx, cycle %lx)\n", index, ret, psxCounters[index].mode, psxCounters[index].target, psxRegs.cycle);

	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////
// The PSX Does Sci-Fi: How?  Gate Travel!

/*
Gate:
   TM_NO_GATE                   000
   TM_GATE_ON_Count             001
   TM_GATE_ON_ClearStart        011
   TM_GATE_ON_Clear_OFF_Start   101
   TM_GATE_ON_Start             111

   V-blank  ----+    +----------------------------+    +------
                |    |                            |    |
                |    |                            |    |
                +----+                            +----+
 TM_NO_GATE:

                0================================>============

 TM_GATE_ON_Count:

                <---->0==========================><---->0=====

 TM_GATE_ON_ClearStart:

                0====>0================================>0=====

 TM_GATE_ON_Clear_OFF_Start:

                0====><-------------------------->0====><-----

 TM_GATE_ON_Start:

                <---->0==========================>============
*/

// ------------------------------------------------------------------------
void CheckStartGate(int index)
{
	if(!(psxCounters[index].mode & PSXCNT_ENABLE_GATE)) return;	// not enabled? nothing to do!

	switch((psxCounters[index].mode & 0x6) >> 1)
	{
		case 0x0: // GATE_ON_count - stop count on gate start:
			//PSXCPU_LOG( "PSX Counter[%d] stopped (Gate 0).", index );
			psxCounters[index].IsCounting = 0;
		return;

		case 0x1: //GATE_ON_ClearStart - count normally with resets after every end gate
			// do nothing - All counting will be done on a need-to-count basis.
		return;

		case 0x2: //GATE_ON_Clear_OFF_Start - start counting on gate start, stop on gate end
			//PSXCPU_LOG( "PSX Counter[%d] started (Gate 2).", index );
			psxCounters[index].IsCounting = 1;
			ResetCount(index, 0);
		break;

		case 0x3: //GATE_ON_Start - start and count normally on gate end (no restarts or stops or clears)
			// do nothing!
		return;
	}
}

// ------------------------------------------------------------------------
void CheckEndGate(int index)
{
	if(!(psxCounters[index].mode & PSXCNT_ENABLE_GATE)) return; //Ignore Gate
	switch((psxCounters[index].mode & 0x6) >> 1)
	{
		case 0x0: //GATE_ON_count - reset and start counting
		case 0x1: //GATE_ON_ClearStart - count normally with resets after every end gate
			//PSXCPU_LOG( "PSX Counter[%d] started (Gate 0 and 1).", index );
			psxCounters[index].IsCounting = 1;
			ResetCount(index, 0);
		break;

		case 0x2: //GATE_ON_Clear_OFF_Start - start counting on gate start, stop on gate end
			//PSXCPU_LOG( "PSX Counter[%d] stopped (Gate 2).", index );
			psxCounters[index].IsCounting = 0;
		return;	// do not set the counter

		case 0x3: //GATE_ON_Start - start and count normally (no restarts or stops or clears)
			if( !psxCounters[index].IsCounting )
			{
				//PSXCPU_LOG( "PSX Counter[%d] started (Gate 3).", index );
				psxCounters[index].IsCounting = 1;
				ResetCount(index, 0);
			}
		break;
	}
}

u32 psxRcntRmode(int index)
{
	u32 ret = psxCounters[index].mode;
	psxCounters[index].mode &= ~0x1800;
	psxCounters[index].mode |= 0x400;
	return ret;
}

int psxRcntFreeze(gzFile f, int Mode) {
	char Unused[4096 - sizeof(psxCounter)];

	gzfreezel(psxCounters);
	gzfreezel(Unused);

	return 0;
}
