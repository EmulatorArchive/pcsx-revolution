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
* R3000A CPU functions.
*/

#include "r3000a.h"
#include "cdrom.h"
#include "mdec.h"
#include "gte.h"
#include "cheat.h"
#include "psxcounters.h"

//#define PRINT_EVENTS

psxRegisters psxRegs;

int psxInit() {
	SysPrintf(_("Running PCSX-Revolution Version %s (%s).\n"), PACKAGE_VERSION, __DATE__);

#ifdef PSXREC
	if (Config.Cpu) {
		psxCpu = &psxInt;
	} else psxCpu = &psxRec;
#else
	psxCpu = &psxInt;
#endif

	Log = 0;

	if (psxMemInit() == -1) return -1;

	return psxCpu->Init();
}

#ifdef NEW_EVENTS
static void _evthandler_Exception()
{
	// Note: Re-test conditions here under the assumption that something else might have
	// cleared the condition masks between the time the exception was raised and the time
	// it's being handled here.
	if( (psxHu32(0x1070) & psxHu32(0x1074)) == 0 ) return;
	if ((psxRegs.CP0.n.Status & 0x401) == 0x401) {
#ifdef PSXCPU_LOG
		PSXCPU_LOG("Interrupt: %x %x\n", psxHu32(0x1070), psxHu32(0x1074));
#endif
		psxException(0x400, 0);
	}
}

static void _evthandler_Idle()
{
	// Note: the idle event handler should only be invoked at times when the full list of
	// active/pending events is empty:
	Events.next = &Events.list[PsxEvt_Idle];
}

static void ResetEvents()
{
	memset(&Events.list, 0, sizeof(events_t) * PsxEvt_CountAll);
	Events.next = NULL;

	Events.list[PsxEvt_Exception].Execute	= _evthandler_Exception;
	Events.list[PsxEvt_SIO].Execute			= sioInterrupt;
	Events.list[PsxEvt_Cdrom].Execute		= cdrInterrupt;
	Events.list[PsxEvt_CdromRead].Execute	= cdrReadInterrupt;
	Events.list[PsxEvt_GPU].Execute 		= gpuInterrupt;
	//Events.list[PsxEvt_SPU].Execute 		= spuInterrupt;

	Events.list[PsxEvt_Idle].Execute 		= _evthandler_Idle;
	Events.list[PsxEvt_Idle].time 			= 0x4000;
	Events.list[PsxEvt_Idle].cycle 			= 0x4000;

	Events.next = &Events.list[PsxEvt_Idle];

#ifndef SEPARATE_CNTS
	Events.list[PsxEvt_Counter0].Execute = psxRcntUpdate;
	psx_int_add(PsxEvt_Counter0, 0);
#else
	Events.list[PsxEvt_Counter0].Execute = psxRcntUpdate0;
	Events.list[PsxEvt_Counter1].Execute = psxRcntUpdate1;
	Events.list[PsxEvt_Counter2].Execute = psxRcntUpdate2;
	Events.list[PsxEvt_Counter3].Execute = psxRcntUpdate3;
	Events.list[PsxEvt_Counter4].Execute = psxRcntUpdate4;
	psx_int_add(PsxEvt_Counter3, 0);
	psx_int_add(PsxEvt_Counter0, 0);
	psx_int_add(PsxEvt_Counter1, 0);
	psx_int_add(PsxEvt_Counter2, 0);
	psx_int_add(PsxEvt_Counter4, 0);
#endif
}

u32 __inline psxGetCycle()
{
	return psxRegs.cycle + (psxRegs.evtCycleDuration - psxRegs.evtCycleCountdown);
}

s32 GetPendingCycles()
{
	return psxRegs.evtCycleDuration - psxRegs.evtCycleCountdown;
}

static __inline void psx_event_add( int n, u32 time )
{
	//events[n].time = time;
	Events.list[n].cycle = time;

	// Find the sorted insertion point into the list of active events:
	events_t* curEvt = Events.next;
	events_t* prevEvt = NULL;
	s32 runningDelta = -GetPendingCycles();
	
	while( 1 )
	{
		// Note: curEvt->next represents the Idle node, which should always be scheduled
		// last .. so the following conditional checks for it and schedules in front of it.
		if( (curEvt == &Events.list[PsxEvt_Idle]) || (runningDelta+curEvt->time > time) )
		{
			Events.list[n].next	= curEvt;
			Events.list[n].time	= time - runningDelta;
			curEvt->time	-= Events.list[n].time;

			if( prevEvt == NULL )
			{
				Events.next = &Events.list[n];

				// Node is being inserted at the head of the list, so reschedule the PSX's
				// master counters as needed.

				psxRegs.evtCycleDuration	= Events.list[n].time;
				psxRegs.evtCycleCountdown	= Events.list[n].cycle;
			}
			else
				prevEvt->next = &Events.list[n];
			break;
		}
		runningDelta += curEvt->time;
		prevEvt = curEvt;
		curEvt  = curEvt->next;
	}

	Events.list[PsxEvt_Idle].time = 0x4000;
}

static __inline void psx_event_remove( int n )
{
	if( Events.list[n].next == NULL ) return;		// not even scheduled.
	Events.list[n].next->time += Events.list[n].time;

	if( Events.next == &Events.list[n] )
	{
		Events.next = Events.list[n].next;
		
		int psxPending 				= GetPendingCycles();
		psxRegs.evtCycleDuration	= Events.next->time;
		psxRegs.evtCycleCountdown	= Events.next->time - psxPending;
	}
	else
	{
		events_t* curEvt = Events.next;
		while( 1 )
		{
			if( curEvt->next == &Events.list[n] )
			{
				curEvt->next = Events.list[n].next;
				break;
			}
			curEvt = curEvt->next;
		}
	}

	Events.list[n].next = NULL;
	Events.list[PsxEvt_Idle].time = 0x4000;
}

__inline void psx_int_add( int n, s32 ecycle )
{
	// Generally speaking games shouldn't throw ints that haven't been cleared yet.
	// It's usually indicative os something amiss in our emulation.
	if(Events.list[n].next != NULL)
	{
		psx_event_remove( n );
		//SysPrintf("Event: %d\t Cycle: %d\tecycle: %d\ttime: %d\told time: %d\n", n, psxRegs.cycle, ecycle, psxRegs.cycle + ecycle, Events.list[n].time);
	}
#ifdef PRINT_EVENTS
	SysPrintf("Event: %ld\t Cycle: %ld\tcycles: %ld\n", n, psxRegs.cycle, ecycle);
#endif
	psx_event_add( n, ecycle );
}

__inline void psx_int_remove( int n )
{
	psx_event_remove( n );
}

__inline void psxTestIntc()
{
	if( (psxHu32(0x1070) & psxHu32(0x1074)) == 0 ) return;

	psx_int_add( PsxEvt_Exception, 0 );
}
#endif

__inline void psxRaiseExtInt( uint irq )
{
#ifdef PRINT_EVENTS
	SysPrintf("ExtInt: %ld\n", irq);
#endif
	psxHu32ref(0x1070) |= SWAPu32(1 << irq);
#ifdef NEW_EVENTS
	psxTestIntc();
#else
	psxRegs.interrupt|= 0x80000000;
#endif
}

void psxReset() {
#ifndef GEKKO
	FreeCheatSearchResults();
	FreeCheatSearchMem();
#endif

	psxCpu->Reset();

	psxMemReset();

	memset(&psxRegs, 0, sizeof(psxRegs));

	psxRegs.pc = 0xbfc00000; // Start in bootstrap

	psxRegs.CP0.r[12].d = 0x10900000; // COP0 enabled | BEV = 1 | TS = 1
	psxRegs.CP0.r[15].d = 0x00000002; // PRevID = Revision ID, same as R3000A

#ifdef NEW_EVENTS
	psxRegs.evtCycleDuration = 32;
	psxRegs.evtCycleCountdown = 32;
#endif

	psxHwReset();
#ifdef NEW_EVENTS
	ResetEvents();
#endif
	psxBiosInit();

	if (!Config.HLE)
		psxExecuteBios();

#ifdef EMU_LOG
	EMU_LOG("*BIOS END*\n");
#endif
	Log = 0;
}

void psxShutdown() {
	psxMemShutdown();
	psxBiosShutdown();

	psxCpu->Shutdown();
#ifndef GEKKO
	ClearAllCheats();
	FreeCheatSearchResults();
	FreeCheatSearchMem();
#endif
}

void psxException(u32 code, u32 bd) {
	// Set the Cause
	psxRegs.CP0.n.Cause = code;

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

	if (psxRegs.CP0.n.Status & 0x400000)
		psxRegs.pc = 0xbfc00180;
	else
		psxRegs.pc = 0x80000080;

	// Set the Status
	psxRegs.CP0.n.Status = (psxRegs.CP0.n.Status &~0x3f) |
						  ((psxRegs.CP0.n.Status & 0xf) << 2);

	if (!Config.HLE && (((PSXMu32(psxRegs.CP0.n.EPC) >> 24) & 0xfe) == 0x4a)) {
		// "hokuto no ken" / "Crash Bandicot 2" ... fix
		PSXMu32ref(psxRegs.CP0.n.EPC)&= SWAPu32(~0x02000000);
	}

	if (Config.HLE) psxBiosException();
}

#ifdef NEW_EVENTS
void psxBranchTest() {
	if (psxRegs.evtCycleCountdown > 0) return;
#ifdef PRINT_EVENTS
	SysPrintf("psxBranchTest: \n");
#endif
	while( 1 ) {
		s32 oldtime = psxRegs.evtCycleCountdown;

		psxRegs.evtCycleCountdown	= 0;
		psxRegs.cycle 				+= psxRegs.evtCycleDuration;
		psxRegs.evtCycleDuration	= 0;
		
		events_t* exeEvt = Events.next;
		Events.next 	 = exeEvt->next;
		exeEvt->next	 = NULL;

		exeEvt->Execute();

		psxRegs.evtCycleDuration	 = Events.next->time;
		psxRegs.evtCycleCountdown	 = oldtime + Events.next->time;
		if( psxRegs.evtCycleCountdown > 0 ) break;
	}
#ifdef GTE_TIMING
	// Periodic culling of GteStallCycles, prevents it from overflowing in the unlikely event that
	// running code doesn't invoke a GTE command in 2 billion cycles.
	if( psxRegs.GteUnitCycles < 0 )
		psxRegs.GteUnitCycles = 0;
#endif
}
#else

void psxBranchTest() {
	if ((psxRegs.cycle - psxNextsCounter) >= psxNextCounter)
		psxRcntUpdate();

	if (psxRegs.interrupt) {
		if ((psxRegs.interrupt & 0x80) && (!Config.Sio)) { // sio
			if ((psxRegs.cycle - psxRegs.intCycle[7]) >= psxRegs.intCycle[7+1]) {
				psxRegs.interrupt&=~0x80;
				sioInterrupt();
			}
		}
		if (psxRegs.interrupt & 0x04) { // cdr
			if ((psxRegs.cycle - psxRegs.intCycle[2]) >= psxRegs.intCycle[2+1]) {
				psxRegs.interrupt&=~0x04;
				cdrInterrupt();
			}
		}
		if (psxRegs.interrupt & 0x040000) { // cdr read
			if ((psxRegs.cycle - psxRegs.intCycle[2+16]) >= psxRegs.intCycle[2+16+1]) {
				psxRegs.interrupt&=~0x040000;
				cdrReadInterrupt();
			}
		}
		if (psxRegs.interrupt & 0x01000000) { // gpu dma
			if ((psxRegs.cycle - psxRegs.intCycle[3+24]) >= psxRegs.intCycle[3+24+1]) {
				psxRegs.interrupt&=~0x01000000;
				gpuInterrupt();
			}
		}
		/*if (psxRegs.interrupt & 0x02000000) { // mdec out dma
			if ((psxRegs.cycle - psxRegs.intCycle[5+24]) >= psxRegs.intCycle[5+24+1]) {
				psxRegs.interrupt&=~0x02000000;
				mdec1Interrupt();
			}
		}*/

//		if (psxRegs.interrupt & 0x80000000) {
			psxRegs.interrupt&=~0x80000000;
			psxTestHWInts();
//		}
	}
}

#endif

__inline void psxTestHWInts() {
	if (psxHu32(0x1070) & psxHu32(0x1074)) {
		if ((psxRegs.CP0.n.Status & 0x401) == 0x401) {
#ifdef PSXCPU_LOG
			PSXCPU_LOG("Interrupt: %x %x\n", psxHu32(0x1070), psxHu32(0x1074));
#endif
//			SysPrintf("Interrupt (%x): %x %x\n", psxRegs.cycle, psxHu32(0x1070), psxHu32(0x1074));
			psxException(0x400, 0);
		}
	}
}

void psxJumpTest() {
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
}

void psxExecuteBios() {
	while (psxRegs.pc != 0x80030000)
		psxCpu->ExecuteBlock();
}

