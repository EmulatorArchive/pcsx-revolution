/*  PCSX-Revolution - PS Emulator for Nintendo Wii
 *  Copyright (C) 2009-2010  PCSX-Revolution Dev Team
 *
 *  PCSX-Revolution is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public 
 *  License as published by the Free Software Foundation, either 
 *  version 2 of the License, or (at your option) any later version.
 *
 *  PCSX-Revolution is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with PCSX-Revolution.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "psxevents.h"
#include "psxmem.h"
#include "psxcounters.h"
#include "psxdma.h"
#include "cdrom.h"
#include "sio.h"
#include "plugins.h"

namespace R3000A {

PsxEvents Interrupt;

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

static const u32 SpuRate = (768 * 32);

static void _evthandler_SPU() {
	if (SPU_async) {
		SPU_async(SpuRate);		// Peops SPU doesn't really matter what we send.
		Interrupt.Schedule(PsxEvt_SPU, SpuRate);
	}
}

static void _evthandler_Idle()
{
	// Note: the idle event handler should only be invoked at times when the full List of
	// active/pending events is empty:
	Interrupt.IdleEventHandler();
}

void ResetEvents()
{
// 	if(Events) delete Events;
// 	Events = new PsxEvents;
	memset(&Interrupt, 0, sizeof(Interrupt));

	Interrupt.Reset();
}

void PsxEvents::Reset() {
	this->List[PsxEvt_Exception].Execute	= _evthandler_Exception;
	this->List[PsxEvt_SIO].Execute		= sioInterrupt;
	this->List[PsxEvt_Cdrom].Execute	= cdrInterrupt;
	this->List[PsxEvt_CdromRead].Execute	= cdrReadInterrupt;
	this->List[PsxEvt_GPU].Execute 		= gpuInterrupt;
	this->List[PsxEvt_OTC].Execute 		= otcInterrupt;
	this->List[PsxEvt_SPU].Execute 		= _evthandler_SPU;
	this->List[PsxEvt_Counter0].Execute	= psxRcntUpdate0;
	this->List[PsxEvt_Counter1].Execute	= psxRcntUpdate1;
	this->List[PsxEvt_Counter2].Execute	= psxRcntUpdate2;
	this->List[PsxEvt_vSync].Execute 	= psxRcntVSync;
	this->List[PsxEvt_vBlank].Execute 	= psxRcntVBlank;

	this->List[PsxEvt_Idle].Execute 	= _evthandler_Idle;
	this->List[PsxEvt_Idle].RelativeDelta 	= 0x4000;
	this->List[PsxEvt_Idle].OrigDelta 	= 0x4000;

	this->Next = &this->List[PsxEvt_Idle];
	Interrupt.Schedule(PsxEvt_SPU, 0);
}

void PsxEvents::Schedule( PsxEventType n, s32 time ) {
	// Generally speaking games shouldn't throw ints that haven't been cleared yet.
	// It's usually indicative os something amiss in our emulation.
	if( IsScheduled(n) ) {
// 		return;
		Cancel( n );
	}

	this->List[n].OrigDelta = time;

	// Find the sorted insertion point into the List of active events:
	events_t* curEvt = this->Next;
	events_t* prevEvt = NULL;
	s32 runningDelta = -psxRegs.GetPendingCycles();
	
	while( true )
	{
		// Note: curEvt->next represents the Idle node, which should always be scheduled
		// last .. so the following conditional checks for it and schedules in front of it.
		if( (curEvt == &this->List[PsxEvt_Idle]) || ((runningDelta + curEvt->RelativeDelta) > time) )
		{
			this->List[n].next = curEvt;
			this->List[n].RelativeDelta = time - runningDelta;
			curEvt->RelativeDelta -= this->List[n].RelativeDelta;

			if( prevEvt == NULL )
			{
				this->Next = &this->List[n];

				// Node is being inserted at the head of the List, so reschedule the PSX's
				// master counters as needed.

				psxRegs.evtCycleDuration  = this->List[n].RelativeDelta;
				psxRegs.evtCycleCountdown = this->List[n].OrigDelta;
			}
			else
				prevEvt->next = &this->List[n];
			break;
		}
		runningDelta += curEvt->RelativeDelta;
		prevEvt = curEvt;
		curEvt  = curEvt->next;
	}

	this->List[PsxEvt_Idle].RelativeDelta = 0x4000;
}

void PsxEvents::Cancel( PsxEventType n ) {
// 	events_t *event = &this->List[n];
	if( this->List[n].next == NULL ) return;		// not even scheduled.
	this->List[n].next->RelativeDelta += this->List[n].RelativeDelta;

	if( this->Next == &this->List[n] )
	{
		this->Next = this->List[n].next;
		int psxPending 			= psxRegs.GetPendingCycles();
		psxRegs.evtCycleDuration	= this->Next->RelativeDelta;
		psxRegs.evtCycleCountdown	= this->Next->RelativeDelta - psxPending;
	}
	else
	{
		events_t* curEvt = this->Next;
		while( true )
		{
			if( curEvt->next == &this->List[n] )
			{
				curEvt->next = this->List[n].next;
				break;
			}
			curEvt = curEvt->next;
		}
	}

	this->List[n].next = NULL;
	this->List[PsxEvt_Idle].RelativeDelta = 0x4000;
}

void PsxEvents::ExecutePendingEvents() {
#ifdef PRINT_EVENTS
	SysPrintf("psxBranchTest: \n");
#endif
	while( true ) {

		RcntAdvanceCycles( psxRegs.evtCycleDuration );

		s32 oldtime = psxRegs.evtCycleCountdown;

		psxRegs.evtCycleCountdown	= 0;
		psxRegs.cycle 			+= psxRegs.evtCycleDuration;
		psxRegs.evtCycleDuration	= 0;
		
		events_t* exeEvt = this->Next;
		this->Next 	 = exeEvt->next;
		exeEvt->next	 = NULL;

		exeEvt->Execute();

		psxRegs.evtCycleDuration	 = this->Next->RelativeDelta;
		psxRegs.evtCycleCountdown	 = oldtime + this->Next->RelativeDelta;
		if( psxRegs.evtCycleCountdown > 0 ) break;
	}
#ifdef GTE_TIMING
	// Periodic culling of GteStallCycles, prevents it from overflowing in the unlikely event that
	// running code doesn't invoke a GTE command in 2 billion cycles.
	if( psxRegs.GteUnitCycles < 0 )
		psxRegs.GteUnitCycles = 0;
#endif
}

void psxBranchTest() {
	if (psxRegs.evtCycleCountdown > 0) return;
	Interrupt.ExecutePendingEvents();
}

}
