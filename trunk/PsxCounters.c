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
#include "plugins.h"

psxCounter psxCounters[5];

static int cnts = 4;

static void psxRcntUpd(unsigned long index) {
	psxCounters[index].sCycle = psxGetCycle();
	if (((!(psxCounters[index].mode_st.Disabled)) || (index!=2)) &&
		psxCounters[index].mode & 0x30) {
		//if (psxCounters[index].mode & 0x10) { // Interrupt on target
		if (psxCounters[index].mode_st.Tar) { // Interrupt on target
			psxCounters[index].Cycle = ((psxCounters[index].target - psxCounters[index].count) * psxCounters[index].rate) / BIAS;
		} else { // Interrupt on 0xffff
			psxCounters[index].Cycle = ((0x10000 - psxCounters[index].count) * psxCounters[index].rate) / BIAS;
		}
	} //else psxCounters[index].Cycle = 0xffffffff;
//	if (index == 2) SysPrintf("Cycle %x\n", psxCounters[index].Cycle);
}

static void psxRcntReset(unsigned long index) {
//	SysPrintf("psxRcntReset %x (mode=%x)\n", index, psxCounters[index].mode);
	psxCounters[index].count = 0;
	psxRcntUpd(index);

//	if (index == 2) SysPrintf("rcnt2 %x\n", psxCounters[index].mode);
	psxHu32ref(0x1070)|= SWAPu32(psxCounters[index].interrupt);
	if (!(psxCounters[index].mode & 0x40)) { // Only 1 interrupt
		psxCounters[index].Cycle = 0xffffffff;
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

//	psxRcntUpd(0); psxRcntUpd(1); psxRcntUpd(2); psxRcntUpd(3);
//	psxRcntSet();
int i;
	for (i=0; i<5; i++)
		psxCounters[i].sCycle = psxRegs.cycle;

}

void psxUpdateVSyncRate() {
	if (Config.PsxType) // ntsc - 0 | pal - 1
	     psxCounters[3].rate = (PSXCLK / 50);// / BIAS;
	else psxCounters[3].rate = (PSXCLK / 60);// / BIAS;
	psxCounters[3].rate-= (psxCounters[3].rate / 262) * 22;
	if (Config.VSyncWA) psxCounters[3].rate/= 2;
}

void psxUpdateVSyncRateEnd() {
	if (Config.PsxType) // ntsc - 0 | pal - 1
	     psxCounters[3].rate = (PSXCLK / 50);// / BIAS;
	else psxCounters[3].rate = (PSXCLK / 60);// / BIAS;
	psxCounters[3].rate = (psxCounters[3].rate / 262) * 22;
	if (Config.VSyncWA) psxCounters[3].rate/= 2;
}

void psxRcntUpdate() {
	int i;
	if ((psxGetCycle() - psxCounters[3].sCycle) >= psxCounters[3].Cycle) {
		if (psxCounters[3].mode & 0x10000) { // VSync End (22 hsyncs)
			psxCounters[3].mode&=~0x10000;
			psxUpdateVSyncRate();
			psxRcntUpd(3);
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
			psxRcntUpd(3);
			psxHu32ref(0x1070)|= SWAPu32(1);
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

void psxRcntWcount(u32 index, u32 value) {
//	SysPrintf("writeCcount[%d] = %x\n", index, value);
//	PSXCPU_LOG("writeCcount[%d] = %x\n", index, value);
	psxCounters[index].count = value;
	psxRcntUpd(index);
	psxRcntSet();
}

void psxRcntWmode(u32 index, u32 value)  {
//	SysPrintf("writeCmode[%ld] = %lx\n", index, value);
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
		if(psxCounters[index].mode_st.Div)
			psxCounters[index].rate = 8; // 1/8 speed
		else 
			psxCounters[index].rate = 1; // normal speed
	}
	else
	{
		psxCounters[index].rate = psxCounters[2].rate * 8;

		if (psxCounters[index].mode_st.ClockSource)
		{
			psxCounters[index].rate = (index == 0) ? ((psxCounters[3].rate) / 386) / 262 : (psxCounters[3].rate / 262);
		}
	}

	// Need to set a rate and target
	psxRcntUpd(index);
	psxRcntSet();
}

void psxRcntWtarget(u32 index, u32 value) {
//	SysPrintf("writeCtarget[%ld] = %lx\n", index, value);
	psxCounters[index].target = value & 0xffff;
	psxRcntUpd(index);
	psxRcntSet();
}

u32 psxRcntRcount(u32 index) {
//	SysPrintf("readCcount[%d] = %lx\n", index, psxCounters[index].mode);

	//print_cnt( index );
	u32 ret = psxCounters[index].count;
	if (!psxCounters[index].mode_st.Disabled)
	{
		if(psxCounters[index].mode_st.Tar)
		{
			ret += ((psxGetCycle() - psxCounters[index].sCycle) / psxCounters[index].rate);
		}
		else
		{
			ret += (psxGetCycle() / psxCounters[index].rate);
		}
	}

//	SysPrintf("readCcount[%ld] = %lx (mode %lx, target %lx, cycle %lx)\n", index, ret, psxCounters[index].mode, psxCounters[index].target, psxRegs.cycle);

	return ret;
}

int psxRcntFreeze(gzFile f, int Mode) {
	char Unused[4096 - sizeof(psxCounter)];

	gzfreezel(psxCounters);
	gzfreezel(Unused);

	return 0;
}
