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
#include "PsxDma.h"
#include "Sio.h"

psxRegisters psxRegs;

typedef struct int_timer {
	u32 time;
	u32 cycle;
	void (*Execute)();
	struct int_timer *next;
} int_timer_st;

static int_timer_st events[PsxEvt_CountAll];
static int_timer_st *event_list;

int psxInit() {
	//SysPrintf(_("Running PCSX Version %s (%s).\n"), PACKAGE_VERSION, __DATE__);

#ifdef PSXREC
	psxCpu = &psxRec;
	if (Config.Cpu) {
#ifndef __GAMECUBE__
		if (Config.Dbg) 
			psxCpu = &psxIntDbg;
		else 
#endif
			psxCpu = &psxInt;
	}
#else
	psxCpu = &psxInt;
#ifndef __GAMECUBE__
	if (Config.Dbg) psxCpu = &psxIntDbg;
#endif
#endif

	Log = 0;

	if (psxMemInit() == -1) return -1;

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

static void _evthandler_Idle()
{
	// Note: the idle event handler should only be invoked at times when the full list of
	// active/pending events is empty:
	event_list = &events[PsxEvt_Idle];
}

static void ResetEvents()
{
	memset(&events, 0, sizeof(int_timer_st) * PsxEvt_CountAll);
	event_list = NULL;

	events[PsxEvt_Exception].Execute	= _evthandler_Exception;
	events[PsxEvt_SIO].Execute			= sioInterrupt;
	events[PsxEvt_Cdrom].Execute		= cdrInterrupt;
	events[PsxEvt_CdromRead].Execute	= cdrReadInterrupt;
	events[PsxEvt_GPU].Execute 			= gpuInterrupt;
	events[PsxEvt_SPU].Execute 			= spuInterrupt;
	
	events[PsxEvt_Idle].Execute 		= _evthandler_Idle;
	events[PsxEvt_Idle].time 		= 0x4000;
	events[PsxEvt_Idle].cycle 		= 0x4000;
	
	event_list = &events[PsxEvt_Idle];

	events[PsxEvt_Counter0].Execute 	= psxRcntUpdate;
	
	//events[PsxEvt_Counter0].Execute 	= psxRcntUpdate0;
	//events[PsxEvt_Counter1].Execute 	= psxRcntUpdate1;
	//events[PsxEvt_Counter2].Execute 	= psxRcntUpdate2;
	//events[PsxEvt_Counter3].Execute 	= psxRcntUpdate3;
	//events[PsxEvt_Counter4].Execute 	= psxRcntUpdate4;
	//psx_int_add(PsxEvt_Counter3, 0);
	psx_int_add(PsxEvt_Counter0, 0);
	//psx_int_add(PsxEvt_Counter1, 0);
	//psx_int_add(PsxEvt_Counter2, 0);
	//psx_int_add(PsxEvt_Counter4, 0);
}

void AddCycles( int amount )
	{
		psxRegs.evtCycleCountdown	-= amount;
		psxRegs.DivUnitCycles		-= amount;
	}

u32 __inline psxGetCycle()
{
	return psxRegs.cycle + (psxRegs.evtCycleDuration - psxRegs.evtCycleCountdown);
}

static s32 GetPendingCycles()
{
	return psxRegs.evtCycleDuration - psxRegs.evtCycleCountdown;
}

void psxReset() {
	psxCpu->Reset();

	psxMemReset();

	memset(&psxRegs, 0, sizeof(psxRegs));

	psxRegs.pc = 0xbfc00000; // Start in bootstrap

	psxRegs.CP0.r[12] = 0x10900000; // COP0 enabled | BEV = 1 | TS = 1
	psxRegs.CP0.r[15] = 0x00000002; // PRevID = Revision ID, same as R3000A

	psxRegs.evtCycleDuration = 32;
	psxRegs.evtCycleCountdown = 32;

	psxHwReset();
	ResetEvents();
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

static __inline void psx_event_add( int n, u32 time )
{
	//events[n].time = time;
	events[n].cycle = time;

	// Find the sorted insertion point into the list of active events:
	int_timer_st* curEvt = event_list;
	int_timer_st* prevEvt = NULL;
	s32 runningDelta = -GetPendingCycles();

	while( 1 )
	{
		// Note: curEvt->next represents the Idle node, which should always be scheduled
		// last .. so the following conditional checks for it and schedules in front of it.
		if( (curEvt == &events[PsxEvt_Idle]) || (runningDelta+curEvt->time > time) )
		{
			events[n].next	= curEvt;
			events[n].time	= time - runningDelta;
			curEvt->time	-= events[n].time;

			if( prevEvt == NULL )
			{
				event_list = &events[n];

				// Node is being inserted at the head of the list, so reschedule the PSX's
				// master counters as needed.

				psxRegs.evtCycleDuration	= events[n].time;
				psxRegs.evtCycleCountdown	= events[n].cycle;
			}
			else
				prevEvt->next = &events[n];
			break;
		}
		runningDelta += curEvt->time;
		prevEvt = curEvt;
		curEvt  = curEvt->next;
	}

	events[PsxEvt_Idle].time = 0x4000;
}

static __inline void psx_event_remove( int n )
{
	if( events[n].next == NULL ) return;		// not even scheduled.
	events[n].next->time += events[n].time;

	// Your Basic List Removal:

	if( event_list == &events[n] )
	{
		event_list = events[n].next;
		
		int psxPending 			= GetPendingCycles();
		psxRegs.evtCycleDuration	= event_list->time;
		psxRegs.evtCycleCountdown	= event_list->time - psxPending;
	}
	else
	{
		int_timer_st* curEvt = event_list;
		while( 1 )
		{
			if( curEvt->next == &events[n] )
			{
				curEvt->next = events[n].next;
				break;
			}
			curEvt = curEvt->next;
		}
	}

	events[n].next = NULL;
	events[PsxEvt_Idle].time = 0x4000;
}

__inline void psx_int_add( int n, s32 ecycle )
{
	// Generally speaking games shouldn't throw ints that haven't been cleared yet.
	// It's usually indicative os something amiss in our emulation, so uncomment this
	// code to help trap those sort of things.
	//if(events[n].next != NULL) \
		printf("Event: %d\t Cycle: %d\tecycle: %d\ttime: %d\told time: %d\n", n, psxRegs.cycle, ecycle, psxRegs.cycle + ecycle, events[n].time);
	psx_event_remove( n );
	psx_event_add( n, ecycle );
}

__inline void psx_int_remove( int n )
{
	psx_event_remove( n );
}

void psxBranchTest() {
	while( 1 ) {
		s32 oldtime = psxRegs.evtCycleCountdown;

		psxRegs.evtCycleCountdown 	= 0;
		psxRegs.cycle 				+= psxRegs.evtCycleDuration;
		psxRegs.evtCycleDuration 	= 0;
		
		int_timer_st* exeEvt	= event_list;
		event_list 				= exeEvt->next;
		exeEvt->next			= NULL;
		exeEvt->Execute();

		psxRegs.evtCycleDuration	 = event_list->time;
		psxRegs.evtCycleCountdown	 = oldtime + event_list->time;
		if( psxRegs.evtCycleCountdown > 0 ) break;
	}

// TODO: Get rid of this
	if (psxHu32(0x1070) & psxHu32(0x1074)) {
		if ((psxRegs.CP0.n.Status & 0x401) == 0x401) {
#ifdef PSXCPU_LOG
			PSXCPU_LOG("Interrupt: %x %x\n", psxHu32(0x1070), psxHu32(0x1074));
#endif
			psxException(0x400, 0);
		}
	}
	
	// Periodic culling of DivStallCycles, prevents it from overflowing in the unlikely event that
	// running code doesn't invoke a DIV or MUL in 2 billion cycles. ;)
	if( psxRegs.DivUnitCycles < 0 )
		psxRegs.DivUnitCycles = 0;
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

