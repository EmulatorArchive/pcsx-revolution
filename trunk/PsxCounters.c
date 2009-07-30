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

#include "PsxCommon.h"
#include "PsxCounters.h"
#include "Cheat.h"
#include "R3000A.h"
#include "PsxMem.h"
#include "PsxHw.h"
#include "plugins.h"

psxCounter psxCounters[5];

static int cnts = 4;

static u32 rate1;

static const uint EECNT_FUTURE_TARGET = 0x10000000;

static void print_cnt(int index)
{
	SysPrintf("counter[%d]: \
		Mode  = %lx \n\
		Disabled  = %lx \n\
		unused = %lx \n\
		IRQ1 = %lx \n\
		IRQ2  = %lx \n\
		TargetInterrupt  = %lx \n\
		ClockSource  = %lx \n",
		index, 
		psxCounters[index].mode, 
		psxCounters[index].mode_st.Disabled, 
		psxCounters[index].mode_st.unused, 
		psxCounters[index].mode_st.IRQ1, 
		psxCounters[index].mode_st.IRQ2, 
		psxCounters[index].mode_st.Tar, 
		psxCounters[index].mode_st.ClockSource
	);
}

static void psxRcntUpd(int index) {
	if( psxCounters[index].count > 0x10000 || psxCounters[index].count > psxCounters[index].target )
	{
		psx_int_add(0, 4);
		return;
	}
	if (psxCounters[index].mode_st.Disabled) return;
	u32 cycle = psxGetCycle();
	psxCounters[index].sCycle = cycle;
	if (((!(psxCounters[index].mode_st.Disabled)) || (index!=2)) &&
		psxCounters[index].mode & 0x30) {
		//if (psxCounters[index].mode & 0x10) { // Interrupt on target
		if (psxCounters[index].mode_st.Tar) { // Interrupt on target
			//printf("cnt[%d] Target = %lx\n", index, psxCounters[index].mode);
			psxCounters[index].Cycle = ((psxCounters[index].target - psxCounters[index].count) * psxCounters[index].rate) / BIAS; //*/ - (cycle - psxCounters[index].sCycle);
		} else { // Interrupt on 0xffff
			//printf("cnt[%d] Overflow = %lx\n", index, psxCounters[index].mode);
			psxCounters[index].Cycle = ((0x10000 - psxCounters[index].count) * psxCounters[index].rate) / BIAS; //*/ - (cycle - psxCounters[index].sCycle);
		}
	} //else psxCounters[index].Cycle = 0xffffffff;
	else psxCounters[index].target |= EECNT_FUTURE_TARGET;
//	if (index == 2) SysPrintf("Cycle %x\n", psxCounters[index].Cycle);
}

static void psxRcntReset(int index) {
//	SysPrintf("psxRcntReset %x (mode=%x)\n", index, psxCounters[index].mode);
	psxCounters[index].count = 0;
	psxRcntUpd(index);

//	if (index == 2) SysPrintf("rcnt2 %x\n", psxCounters[index].mode);
	psxHu32ref(0x1070) |= SWAPu32(psxCounters[index].interrupt);
	//psxRaiseExtInt( psxCounters[index].interrupt );
	if (!(psxCounters[index].mode & 0x40)) { // Only 1 interrupt
		//printf("One interrupt\n");
		//psxCounters[index].mode_st.Disabled = 1;
		psxCounters[index].Cycle = 0xffffffff;
		psxCounters[index].target |= EECNT_FUTURE_TARGET;
	} // else Continuos interrupt mode
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

void psxUpdateVSyncRate() {
	//psxHu32ref(0x1070)|= SWAPu32(0x800);
	psxRaiseExtInt( PsxInt_VBlank );
	rate1 = psxCounters[3].rate = (PSXCLK / (Config.PsxType ? 50 : 60));
//printf("rate1 = %d\n", rate1);
}

void psxUpdateVSyncRateEnd() {
	psxHu32ref(0x1070)|= SWAPu32(1);

	psxCounters[3].rate /= (Config.PsxType ? 314 : 263);		// Bad magic =(
//printf("rate2 = %d\n", psxCounters[3].rate);
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
		} else { // VSync Start (240 hsyncs) 
			psxCounters[3].mode|= 0x10000;
			psxUpdateVSyncRateEnd();
			psxRcntUpd(3);
		}
	}

	for(i = 0; i < 3; i++)
	{
		if ((psxRegs.cycle - psxCounters[i].sCycle) >= psxCounters[i].Cycle)
		{
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
//	SysPrintf("writeCcount[%d] = %x\n", index, value);
//	PSXCPU_LOG("writeCcount[%d] = %x\n", index, value);
	psxCounters[index].count = value & 0xffff;
	// reset the target, and make sure we don't get a premature target.
	psxCounters[index].target &= 0xffff;

	if( psxCounters[index].count > psxCounters[index].target ) \
		psxCounters[index].target |= EECNT_FUTURE_TARGET;

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

	psxRcntUpd(index);
	psxRcntSet();
}

void psxRcntWmode(int index, u32 value)  {
	//SysPrintf("writeCmode[%ld] = %lx\n", index, value);
	psxCounters[index].mode = value;
	psxCounters[index].mode |= 0x0400;
	psxCounters[index].count = 0;

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
				psxCounters[index].rate = ((psxCounters[3].rate) / 426) / 263;
			else
				psxCounters[index].rate = rate1 / (Config.PsxType ? 314 : 263); // seems ok
/*
			if(index == 0)
				psxCounters[index].rate = psxCounters[3].rate / (Config.PsxType ? 340 : 341);		// Depends on screen width, but most games set this before write screen width in memory
			else
				psxCounters[index].rate = psxCounters[3].rate / (Config.PsxType ? 314 : 263);	// Not sure about that...
*/
		}
	}

	// Need to set a rate and target
	psxRcntUpd(index);
	psxRcntSet();
}

void psxRcntWtarget(int index, u32 value) {
//	SysPrintf("writeCtarget[%ld] = %lx\n", index, value);
	psxCounters[index].target = value & 0xffff;

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

	psxRcntUpd(index);
	psxRcntSet();
}

u32 psxRcntRcount(int index) {
//	SysPrintf("readCcount[%d] = %lx\n", index, psxCounters[index].mode);

	u32 ret = psxCounters[index].count;
	if (!psxCounters[index].mode_st.Disabled)
	{
		if(psxCounters[index].mode_st.ClockSource != 0x1)
		{
			ret += ((psxGetCycle() - psxCounters[index].sCycle) / psxCounters[index].rate);
		}
	}

//	SysPrintf("readCcount[%ld] = %lx (mode %lx, target %lx, cycle %lx)\n", index, ret, psxCounters[index].mode, psxCounters[index].target, psxRegs.cycle);

	return (ret & 0xffff);
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
