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

//#define PRINT_EVENTS

typedef struct int_timer {
	u32 RelativeDelta;
	u32 OrigDelta;
	void (*Execute)();
	struct int_timer *next;
} events_t;

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
		psx_int_add(PsxEvt_SPU, SpuRate);
	}
}
static void _evthandler_Idle();

class PsxEvents {
	protected:
		events_t List[PsxEvt_CountAll];
		events_t *Next;

	public:
		void Add( PsxEventType n, s32 time );
		void Remove( PsxEventType n );
	
		void IdleEventHandler() {
			Next = &List[PsxEvt_Idle];
		}
		
		void Reset() {
			List[PsxEvt_Exception].Execute	= _evthandler_Exception;
			List[PsxEvt_SIO].Execute		= sioInterrupt;
			List[PsxEvt_Cdrom].Execute		= cdrInterrupt;
			List[PsxEvt_CdromRead].Execute	= cdrReadInterrupt;
			List[PsxEvt_GPU].Execute 		= gpuInterrupt;
			List[PsxEvt_OTC].Execute 		= otcInterrupt;
			List[PsxEvt_SPU].Execute 		= _evthandler_SPU;
			List[PsxEvt_Counter0].Execute	= psxRcntUpdate0;
			List[PsxEvt_Counter1].Execute	= psxRcntUpdate1;
			List[PsxEvt_Counter2].Execute	= psxRcntUpdate2;
			List[PsxEvt_vSync].Execute 		= psxRcntVSync;
			List[PsxEvt_vBlank].Execute 	= psxRcntVBlank;

			List[PsxEvt_Idle].Execute 		= _evthandler_Idle;
			List[PsxEvt_Idle].RelativeDelta 			= 0x4000;
			List[PsxEvt_Idle].OrigDelta 		= 0x4000;
	
			Next = &List[PsxEvt_Idle];
		}
		
		bool IsActiveEvent(PsxEventType n) {
			return (List[n].next != NULL);
		}
		
		void ExecutePendingEvents();
};

static PsxEvents Events;

static void _evthandler_Idle()
{
	// Note: the idle event handler should only be invoked at times when the full List of
	// active/pending events is empty:
	Events.IdleEventHandler();
}

void ResetEvents()
{
// 	if(Events) delete Events;
// 	Events = new PsxEvents;
	memset(&Events, 0, sizeof(Events));

	Events.Reset();
}

void PsxEvents::Add( PsxEventType n, s32 time ) {
// 	events_t *event = &List[n];
	List[n].OrigDelta = time;

	// Find the sorted insertion point into the List of active events:
	events_t* curEvt = Next;
	events_t* prevEvt = NULL;
	s32 runningDelta = -GetPendingCycles();
	
	while( true )
	{
		// Note: curEvt->next represents the Idle node, which should always be scheduled
		// last .. so the following conditional checks for it and schedules in front of it.
		if( (curEvt == &List[PsxEvt_Idle]) || ((runningDelta + curEvt->RelativeDelta) > time) )
		{
			List[n].next = curEvt;
			List[n].RelativeDelta = time - runningDelta;
			curEvt->RelativeDelta -= List[n].RelativeDelta;

			if( prevEvt == NULL )
			{
				Next = &List[n];

				// Node is being inserted at the head of the List, so reschedule the PSX's
				// master counters as needed.

				psxRegs.evtCycleDuration  = List[n].RelativeDelta;
				psxRegs.evtCycleCountdown = List[n].OrigDelta;
			}
			else
				prevEvt->next = &List[n];
			break;
		}
		runningDelta += curEvt->RelativeDelta;
		prevEvt = curEvt;
		curEvt  = curEvt->next;
	}

	List[PsxEvt_Idle].RelativeDelta = 0x4000;
}

void PsxEvents::Remove( PsxEventType n ) {
// 	events_t *event = &List[n];
	if( List[n].next == NULL ) return;		// not even scheduled.
	List[n].next->RelativeDelta += List[n].RelativeDelta;

	if( Next == &List[n] )
	{
		Next = List[n].next;
		int psxPending 				= GetPendingCycles();
		psxRegs.evtCycleDuration	= Next->RelativeDelta;
		psxRegs.evtCycleCountdown	= Next->RelativeDelta - psxPending;
	}
	else
	{
		events_t* curEvt = Next;
		while( true )
		{
			if( curEvt->next == &List[n] )
			{
				curEvt->next = List[n].next;
				break;
			}
			curEvt = curEvt->next;
		}
	}

	List[n].next = NULL;
	List[PsxEvt_Idle].RelativeDelta = 0x4000;
}

void PsxEvents::ExecutePendingEvents() {
#ifdef PRINT_EVENTS
	SysPrintf("psxBranchTest: \n");
#endif
	while( true ) {

		RcntAdvanceCycles( psxRegs.evtCycleDuration );

		s32 oldtime = psxRegs.evtCycleCountdown;

		psxRegs.evtCycleCountdown	= 0;
		psxRegs.cycle 				+= psxRegs.evtCycleDuration;
		psxRegs.evtCycleDuration	= 0;
		
		events_t* exeEvt = Next;
		Next 	 = exeEvt->next;
		exeEvt->next	 = NULL;

		exeEvt->Execute();

		psxRegs.evtCycleDuration	 = Next->RelativeDelta;
		psxRegs.evtCycleCountdown	 = oldtime + Next->RelativeDelta;
		if( psxRegs.evtCycleCountdown > 0 ) break;
	}
#ifdef GTE_TIMING
	// Periodic culling of GteStallCycles, prevents it from overflowing in the unlikely event that
	// running code doesn't invoke a GTE command in 2 billion cycles.
	if( psxRegs.GteUnitCycles < 0 )
		psxRegs.GteUnitCycles = 0;
#endif
}

void psx_int_add( PsxEventType n, s32 ecycle )
{
	// Generally speaking games shouldn't throw ints that haven't been cleared yet.
	// It's usually indicative os something amiss in our emulation.
	if(Events.IsActiveEvent(n))
	{
// 		SysPrintf("Event: %d\t Cycle: %d\tecycle: %d\ttime: %d\told time: %d\n", n, psxRegs.OrigDelta, ecycle, psxRegs.OrigDelta + ecycle, Events.List[n].RelativeDelta);
		return;
		Events.Remove( n );
	}
#ifdef PRINT_EVENTS
	SysPrintf("Event: %ld\t Cycle: %ld\tcycles: %ld\n", n, psxRegs.OrigDelta, ecycle);
#endif
	Events.Add( n, ecycle );
}

void psx_int_remove( PsxEventType n )
{
	Events.Remove( n );
}

bool psxIsActiveEvent(PsxEventType n) {
	return (Events.IsActiveEvent(n));
}

void psxBranchTest() {
	if (psxRegs.evtCycleCountdown > 0) return;
	Events.ExecutePendingEvents();
}

