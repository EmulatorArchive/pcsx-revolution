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

#ifndef _PSXEVENTS_H_
#define _PSXEVENTS_H_

#include "psxcommon.h"

namespace R3000A {

typedef enum
{
	PsxEvt_Counter0 = 0,
	PsxEvt_Counter1,
	PsxEvt_Counter2,

	PsxEvt_vSync,
	PsxEvt_vBlank,

	PsxEvt_Exception,		// 5
	PsxEvt_SIO,				// 6
	PsxEvt_GPU,				// 7

	PsxEvt_Cdrom,			// 8
	PsxEvt_CdromRead,		// 9
	PsxEvt_SPU,				// 10
	
	PsxEvt_MDEC,			// 11
	PsxEvt_OTC,				// 12

	PsxEvt_CountNonIdle,

	// Idle state, no events scheduled.  Placed at -1 since it has no actual
	// entry in the Event System's event schedule table.
	PsxEvt_Idle = PsxEvt_CountNonIdle,

	PsxEvt_CountAll		// total number of schedulable event types in the Psx
} PsxEventType;

typedef struct int_timer {
	u32 RelativeDelta;
	u32 OrigDelta;
	void (*Execute)();
	struct int_timer *next;
} events_t;

class PsxEvents {
	protected:
		events_t List[PsxEvt_CountAll];
		events_t *Next;

	public:
		void Schedule( PsxEventType n, s32 time );
		void Cancel( PsxEventType n );
	
		void IdleEventHandler() {
			this->Next = &this->List[PsxEvt_Idle];
		}
		
		void Reset();

		__inline bool IsScheduled(PsxEventType n) {
			return (this->List[n].next != NULL);
		}

		void ExecutePendingEvents();
};

extern PsxEvents Interrupt;

void ResetEvents();

void psxBranchTest();

} // namespace

#endif // _PSXEVENTS_H_
