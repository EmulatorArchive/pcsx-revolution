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

#include "CdRom.h"
#include "Mdec.h"
#include "Gte.h"
#include "PsxMem.h"
#include "PsxCounters.h"
#include "PsxBios.h"
#include "Cheat.h"
#include "R3000A.h"

#include "PsxHw.h"

// TODO Add all events to list.
typedef struct {
	u32 time;
	void (*Execute)();
	u8 Enabled;
} int_timer;

int_timer events[PsxEvt_CountAll];

psxRegisters psxRegs;

int psxInit() {

	psxCpu = &psxInt;
#if defined(__i386__) || defined(__sh__) || defined(__ppc__)
	if (!Config.Cpu) psxCpu = &psxRec;
#endif
	Log = 0; // GP

	if (psxMemInit() == -1) {
		SysMessage("Failed to allocate memory for emulation\n Please restart the emulator and try again.");
		return -1;
	}
	
	return psxCpu->Init();
}

static void _evthandler_Exception()
{
	// Note: Re-test conditions here under the assumption that something else might have
	// cleared the condition masks between the time the exception was raised and the time
	// it's being handled here.

	if( psxHu32(0x1078) == 0 ) return;
	if( (psxHu32(0x1070) & psxHu32(0x1074)) == 0 ) return;

	if ((psxRegs.CP0.n.Status & 0xFE01) >= 0x401)
	{
		psxException( 0, 0 );
		psxRegs.pc += 4;
	}
}

static void ResetEvents()
{
	memset(&events, 0, sizeof(int_timer) * PsxEvt_CountAll);

	events[PsxEvt_Exception].Execute	= _evthandler_Exception;
	events[PsxEvt_SIO].Execute			= sioInterrupt;
	events[PsxEvt_Cdrom].Execute		= cdrInterrupt;
	events[PsxEvt_CdromRead].Execute	= cdrReadInterrupt;
	events[PsxEvt_MDEC].Execute 		= mdec1Interrupt; 
	events[PsxEvt_GPU].Execute 			= gpuInterrupt;
	events[PsxEvt_SPU].Execute 			= spuInterrupt;

	// Set counters
#ifdef _NEW_COUNTER_
	events[PsxEvt_Counter0].Execute 	= psxRcntUpdate0;
	events[PsxEvt_Counter1].Execute 	= psxRcntUpdate1;
	events[PsxEvt_Counter2].Execute 	= psxRcntUpdate2;
	events[PsxEvt_Counter3].Execute 	= psxRcntUpdate3;
	events[PsxEvt_Counter4].Execute 	= psxRcntUpdate4;
	psx_int_add(PsxEvt_Counter0, 3);
	psx_int_add(PsxEvt_Counter1, 3);
	psx_int_add(PsxEvt_Counter2, 3);
	psx_int_add(PsxEvt_Counter3, 1);
	psx_int_add(PsxEvt_Counter4, 3);
#else
	events[PsxEvt_Counter0].Execute 	= psxRcntUpdate;
	psx_int_add(PsxEvt_Counter0, 1);
#endif
}

void psxReset() 
{
	psxCpu->Reset();
	psxMemReset();
	memset(&psxRegs, 0, sizeof(psxRegs));
	psxRegs.pc = 0xbfc00000; // Start in bootstrap
	psxRegs.CP0.r[12] = 0x10900000; // COP0 enabled | BEV = 1 | TS = 1
	psxRegs.CP0.r[15] = 0x00000002; // PRevID = Revision ID, same as R3000A
	
	psxRegs.NextBranchCycle = psxRegs.cycle + 4;
	
	psxHwReset();
	ResetEvents();
	psxBiosInit();
	if (!Config.HLE)
		psxExecuteBios();

#ifdef EMU_LOG
	EMU_LOG("*BIOS END*\n");
#endif
	Log=0; // GP
}

void psxShutdown() 
{
	psxMemShutdown();
	psxBiosShutdown();
	psxSIOShutdown();
	psxCpu->Shutdown();
	ClearAllCheats();
}

void psxException(u32 code, u32 bd) {
	// Set the Cause
	psxRegs.CP0.n.Cause &= ~0x7f;
	psxRegs.CP0.n.Cause |= code;

	// Set the EPC & PC
	if (bd) {
#ifdef PSXCPU_LOG
		PSXCPU_LOG("bd set!!!\n");
#endif
		SysPrintf("bd set!!!\n");
		psxRegs.CP0.n.Cause|= 0x80000000;
		psxRegs.CP0.n.EPC = (psxRegs.pc - 4);
	} else
		psxRegs.CP0.n.EPC = (psxRegs.pc);

	psxRegs.pc = (psxRegs.CP0.n.Status & 0x400000) ? 0xbfc00180 : 0x80000080;

	// Set the Status
	psxRegs.CP0.n.Status = (psxRegs.CP0.n.Status &~0x3f) |
						  ((psxRegs.CP0.n.Status & 0xf) << 2);

	if (Config.HLE) psxBiosException();
}

__inline int psxSetNextBranch( u32 startCycle, s32 delta )
{
	// typecast the conditional to signed so that things don't explode
	// if the startCycle is ahead of our current cpu cycle.

	if( (int)(psxRegs.NextBranchCycle - startCycle) > delta )
	{
		psxRegs.NextBranchCycle = startCycle + delta;
	}
}

__inline void psx_int_add( int n, s32 ecycle )
{
	// Generally speaking games shouldn't throw ints that haven't been cleared yet.
	// It's usually indicative os something amiss in our emulation, so uncomment this
	// code to help trap those sort of things.
	//if(events[n].Enabled)
	//	printf("Event: %d\t Cycle: %d\n", n, psxRegs.cycle);

	events[n].Enabled = 1;
	events[n].time = psxRegs.cycle + ecycle;

	psxSetNextBranch( psxRegs.cycle, ecycle );
}

__inline void psx_int_remove( int n )
{
	events[n].Enabled = 0;
}

void psxBranchTest() {
	psxRegs.NextBranchCycle = psxRegs.cycle + 0x7fffffff;
	
	int i;
	for(i = 0; i < PsxEvt_CountAll; i++)
	{
		if(!events[i].Enabled) continue;
		if(psxRegs.cycle >= events[i].time)
		{
			events[i].Enabled = 0;
			events[i].Execute();
		}
		else psxSetNextBranch( psxRegs.cycle, events[i].time - psxRegs.cycle );
	}
	
	if (psxHu32(0x1070) & psxHu32(0x1074)) {
		if ((psxRegs.CP0.n.Status & 0x401) == 0x401) {
#ifdef PSXCPU_LOG
			PSXCPU_LOG("Interrupt: %x %x\n", psxHu32(0x1070), psxHu32(0x1074));
#endif
			psxException(0x400, 0);
		}
	}
}

__inline void psxTestIntc()
{
	if( psxHu32(0x1078) == 0 ) return;
	if( (psxHu32(0x1070) & psxHu32(0x1074)) == 0 ) return;

	psx_int_add( PsxEvt_Exception, 0 );
}

__inline void psxRaiseExtInt( uint irq )
{
	psxHu32ref(0x1070) |= SWAPu32(1 << irq);
	psxTestIntc();
}


void psxJumpTest() {
#ifndef __GAMECUBE__
	if (!Config.HLE && Config.PsxOut) {
		u32 call = psxRegs.GPR.n.t1 & 0xff;
		switch (psxRegs.pc & 0x1fffff) {
			case 0xa0:
#ifdef PSXBIOS_LOG
				if (call != 0x28 && call != 0xe) {
					PSXBIOS_LOG("Bios call a0: %s (%x) %x,%x,%x,%x\n", biosA0n[call], call, psxRegs.GPR.n.a0, psxRegs.GPR.n.a1, psxRegs.GPR.n.a2, psxRegs.GPR.n.a3); }
#endif
				if (biosA0[call])
					biosA0[call]();
				break;
			case 0xb0:
#ifdef PSXBIOS_LOG
				if (call != 0x17 && call != 0xb) {
					PSXBIOS_LOG("Bios call b0: %s (%x) %x,%x,%x,%x\n", biosB0n[call], call, psxRegs.GPR.n.a0, psxRegs.GPR.n.a1, psxRegs.GPR.n.a2, psxRegs.GPR.n.a3); }
#endif
				if (biosB0[call])
					biosB0[call]();
				break;
			case 0xc0:
#ifdef PSXBIOS_LOG
				PSXBIOS_LOG("Bios call c0: %s (%x) %x,%x,%x,%x\n", biosC0n[call], call, psxRegs.GPR.n.a0, psxRegs.GPR.n.a1, psxRegs.GPR.n.a2, psxRegs.GPR.n.a3);
#endif
				if (biosC0[call])
					biosC0[call]();
				break;
		}
	}
#endif
}

void psxExecuteBios() {
	while (psxRegs.pc != 0x80030000)
		psxCpu->ExecuteBlock();
}

