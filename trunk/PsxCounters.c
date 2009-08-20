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

//#include "PsxCommon.h"
#include "PsxCounters.h"
#include "Cheat.h"
#include "R3000A/R3000A.h"
#include "PsxMem.h"
#include "PsxHw.h"
//#include "plugins.h"

static int cnts = 4;
psxCounter psxCounters[5];

//#define _DEBUG_CNT_

#ifdef _DEBUG_CNT_
static u8 cnt[3];
#endif

static u32 vRenderRate, vBlankRate, FrameRate = 0;

static const uint PSXCNT_FUTURE_TARGET = 0x10000000;
static const uint FRAMERATE_NTSC = 2997;	/* frames per second * 100 (29.97)*/
static const uint FRAMERATE_PAL  = 2500;	/* frames per second * 100 (25)*/

static void print_cnt(int index)
{
	SysPrintf("counter[%d]: \
		Mode  = %lx \n\
		Disabled  = %lx \n\
		unused = %lx \n\
		IRQ1 = %lx \n\
		IRQ2  = %lx \n\
		TargetInterrupt  = %lx \n\
		ClockSource  = %lx \n\
		Div  = %lx \n",
		index, 
		psxCounters[index].mode, 
		psxCounters[index].mode_st.Disabled, 
		psxCounters[index].mode_st.unused, 
		psxCounters[index].mode_st.IRQ1, 
		psxCounters[index].mode_st.IRQ2, 
		psxCounters[index].mode_st.Tar, 
		psxCounters[index].mode_st.ClockSource,
		psxCounters[index].mode_st.Div
	);
}

static void _psxRcntSet(int i) {
	if (psxCounters[i].mode_st.Disabled) return;
	if (psxCounters[i].Cycle == 0xffffffff) return;

	s32 count = psxCounters[i].Cycle - (psxGetCycle() - psxCounters[i].sCycle);

	if (count > 0) {
		psx_int_add(0, count);
	}	
}

static void psxRcntUpd(int index) {
	if (psxCounters[index].mode_st.Disabled) return;
	u32 cycle = psxGetCycle();
	u32 count = psxCounters[index].count;
	psxCounters[index].count = psxCounters[index].target;
	psxCounters[index].sCycle = cycle;
	if(!(psxCounters[index].target & PSXCNT_FUTURE_TARGET)) {
		if (psxCounters[index].mode & 0x10) { // Interrupt on target
			if( psxCounters[index].mode & 0x80 )
				psxCounters[index].mode &= ~0x0400; // Interrupt flag
			psxCounters[index].mode |= 0x0800; // Target flag
			//psxHu32ref(0x1070) |= SWAPu32(psxCounters[index].interrupt);
			psxCounters[index].Cycle = ((psxCounters[index].target - count) * psxCounters[index].rate) / BIAS; //*/ - (cycle - psxCounters[index].sCycle);
		} 
		
		if(psxCounters[index].mode & 0x08) {
			psxCounters[index].count = 0;
			if (!(psxCounters[index].mode & 0x40)) { // Only 1 interrupt
				psxCounters[index].Cycle = 0xffffffff;
				psxCounters[index].target |= PSXCNT_FUTURE_TARGET;
			} // else Continuos interrupt mode
		}
		else \
			psxCounters[index].target |= PSXCNT_FUTURE_TARGET;

	} else psxCounters[index].Cycle = 0xffffffff;
	
	if (psxCounters[index].mode & 0x20) { // Interrupt on 0xffff
		// Overflow interrupt
		psxCounters[index].mode |= 0x1000; // Overflow flag
		if(psxCounters[index].mode & 0x80)
			psxCounters[index].mode &= ~0x0400; // Interrupt flag
		//psxHu32ref(0x1070) |= SWAPu32(psxCounters[index].interrupt);
		psxCounters[index].Cycle = ((0x10000 - count) * psxCounters[index].rate) / BIAS; //*/ - (cycle - psxCounters[index].sCycle);
	}
	//_psxRcntSet(index);
}

static void psxRcntReset(int index) {
//	SysPrintf("psxRcntReset %x (mode=%x)\n", index, psxCounters[index].mode);
	
	psxRcntUpd(index);
//	if (index == 2) SysPrintf("rcnt2 %x\n", psxCounters[index].mode);
	psxHu32ref(0x1070) |= SWAPu32(psxCounters[index].interrupt);
	//psxRaiseExtInt( psxCounters[index].interrupt );

}

static void psxRcntSet() {
	int i;

	u32 psxNextCounter = 0x7fffffff;

	for (i=0; i<cnts; i++) {
		s32 count;
		if (psxCounters[i].mode_st.Disabled) continue;
		if (psxCounters[i].Cycle == 0xffffffff) continue;

		count = psxCounters[i].Cycle - (psxGetCycle() - psxCounters[i].sCycle);

		if (count < 0) {
			psxNextCounter = 0; break;
		}

		if (count < (s32)psxNextCounter) {
			psxNextCounter = count;
		}
	}
	psx_int_add(0, psxNextCounter);
}

static void ResetCount(u32 index, u32 value)
{
	psxCounters[index].count = value;
	psxCounters[index].target &= ~PSXCNT_FUTURE_TARGET;
	if(!psxCounters[index].mode_st.Disabled)
	{
		psxRcntUpd(index);
		_psxRcntSet(index);
	}
}

void psxRcntInit() {
	int i;
	memset(psxCounters, 0, sizeof(psxCounters));

	psxCounters[0].rate = 1; psxCounters[0].interrupt = 0x10;
	psxCounters[1].rate = 1; psxCounters[1].interrupt = 0x20;
	psxCounters[2].rate = 1; psxCounters[2].interrupt = 0x40;

	psxCounters[3].interrupt = 1;
	psxCounters[3].mode = 0x58; // The VSync counter mode
	psxCounters[3].target = 1;
	psxUpdateVSyncRate();

	if (SPU_async != NULL) {
		cnts = 5;

		psxCounters[4].rate = 768 * 64;
		psxCounters[4].target = 1;
		psxCounters[4].mode = 0x58;
	} else cnts = 4;

	for (i=0; i<5; i++)
		psxCounters[i].sCycle = psxRegs.cycle;

}

void CalcRate(u32 rate)
{
	FrameRate = rate;
	u64 Frame = ((PSXCLK * 1000000ULL) / FrameRate);
	u64 HalfFrame = Frame / 2;
	u64 Blank = HalfFrame / 2;		// two blanks and renders per frame
	u64 Render = HalfFrame - Blank;	// so use the half-frame value for these...
	vRenderRate = (u32)(Render/10000);
	vBlankRate  = (u32)(Blank/10000);
	// Apply rounding:
	if( ( Render - vRenderRate ) >= 5000 ) vRenderRate++;
	else if( ( Blank - vBlankRate ) >= 5000 ) vBlankRate++;
}

void psxUpdateVSyncRate() {
	if(Config.PsxType)
	{
		if(FrameRate != FRAMERATE_PAL)
			CalcRate(FRAMERATE_PAL);
	}
	else
	{
		if(FrameRate != FRAMERATE_NTSC)
			CalcRate(FRAMERATE_NTSC);
	}
	psxCounters[3].rate = vRenderRate;
}

void psxUpdateVSyncRateEnd() {
	psxCounters[3].rate = vBlankRate;
}

void psxRcntUpdate() {
	int i;
	if ((psxGetCycle() - psxCounters[3].sCycle) >= psxCounters[3].Cycle) {
		if (psxCounters[3].mode & 0x10000) { // VSync End (22 hsyncs)
			psxCounters[3].mode&=~0x10000;
			psxUpdateVSyncRate();
			psxRcntUpd(3);
			GPU_updateLace(); // updateGPU
			SysUpdate();
			ApplyCheats();
#ifdef GTE_LOG
			GTE_LOG("VSync\n");
#endif
#ifdef _DEBUG_CNT_
			printf("VSync\n");
			for(i = 0; i < 3; i++)
			{
				printf("cnt[%d] = %d\n", i, cnt[i]);
				cnt[i] = 0;
			}
#endif
		} else { // VSync Start (240 hsyncs) 
			psxCounters[3].mode|= 0x10000;
			psxUpdateVSyncRateEnd();
			psxRcntUpd(3);
			psxHu32ref(0x1070)|= SWAPu32(1);
#ifdef _DEBUG_CNT_
			for(i = 0; i < 3; i++)
			{
				printf("cnt[%d] = %d\n", i, cnt[i]);
				cnt[i] = 0;
			}
#endif
		}
	}

	for(i = 0; i < 3; i++)
	{
		if ((psxRegs.cycle - psxCounters[i].sCycle) >= psxCounters[i].Cycle)
		{
#ifdef _DEBUG_CNT_
			cnt[i]++;
#endif
			psxRcntReset(i);
		}
	}

	if (cnts >= 5) {
		if ((psxRegs.cycle - psxCounters[4].sCycle) >= psxCounters[4].Cycle) {
			SPU_async((psxGetCycle() - psxCounters[4].sCycle) * BIAS);
			psxRcntReset(4);
		}
	}

	psxRcntSet();
}

void psxRcntWcount(int index, u32 value) {
	ResetCount(index, value);
//	SysPrintf("writeCcount[%d] = %x\n", index, value);
//	PSXCPU_LOG("writeCcount[%d] = %x\n", index, value);
}

void psxRcntWmode(int index, u32 value)  {
	//SysPrintf("writeCmode[%ld] = %lx\n", index, value);
	psxCounters[index].mode = value | 0x0400;
	psxCounters[index].count = 0;

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
#if 1
			if(index == 0)
				psxCounters[index].rate = PSXCLK / 13500000;	// PixelClock
			else
				psxCounters[index].rate = PSXCLK / (Config.PsxType ? (50*625) : (59.94*525));	// Not sure about that... (framerate * scanlines)
#else
			if(index == 0)
				psxCounters[index].rate = (psxCounters[3].rate / 426) / 263;
			else
				psxCounters[index].rate = psxCounters[3].rate / (Config.PsxType ? 314 : 263); // seems ok
#endif
		}
	}

	// Need to set a rate and target
	ResetCount(index, 0);
}

void psxRcntWtarget(int index, u32 value) {
//	SysPrintf("writeCtarget[%ld] = %lx\n", index, value);
	psxCounters[index].target = value;

	s32 cycle = psxGetCycle();
	if(!psxCounters[index].mode_st.Disabled) {
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
		psxCounters[index].target |= PSXCNT_FUTURE_TARGET;

	psxRcntUpd(index);
	_psxRcntSet(index);
}

u32 psxRcntRcount(int index) {
//	SysPrintf("readCcount[%d] = %lx\n", index, psxCounters[index].mode);

	u32 ret = psxCounters[index].count;
	if (!psxCounters[index].mode_st.Disabled)
	{
		ret += ((psxGetCycle() - psxCounters[index].sCycle) / psxCounters[index].rate);
	}

//	SysPrintf("readCcount[%ld] = %lx (mode %lx, target %lx, cycle %lx)\n", index, ret, psxCounters[index].mode, psxCounters[index].target, psxRegs.cycle);

	return ret;
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
