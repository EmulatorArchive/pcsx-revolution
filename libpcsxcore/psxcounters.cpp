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

/*
* Internal PSX counters.
*/

#include "psxhw.h"
#include "r3000a.h"
#include "psxmem.h"
#include "psxcounters.h"
#include "plugins.h"
#include "cheat.h"
#include "system.h"

namespace R3000A {

// We need it because of PixelClock rate - 13.5MHz.
static const u32 PsxFixedBits = 12;

// I just love bit fields =)
typedef union {
	u32 value;
	struct {
		u32 unused: 16;
		u32 width1: 1;
		u32 width0: 2;
	} width;
} width_t;

typedef struct {
	u32 Render;
	u32 Blank;
	u32 region;
	u32 scans;
	u32 frame;
} vSyncRate_t;

static vSyncRate_t vSyncRate;

typedef struct
{
#if defined(GEKKO) || defined(__BIGENDIAN__)
	u32 garbage:22;
	u32 Div:1;
	u32 ClockSource:1;
	u32 unused2:1;
	u32 Repeat:1;
	u32 IRQOVF:1;
	u32 IRQTARGET:1;
	u32 Tar:1;
	u32 Reset:1;
	u32 unused:1;
	u32 Stop:1;
#else
	// General count enable/status.  If 0, no counting happens.
	// This flag is set/unset by the gates.
	u32 Stop:1;
	u32 unused:1;
	u32 Reset:1;
	// 0 Count to $ffff
	// 1 Count to value in target register
	u32 Tar:1;
	//Set both for IRQ on target reached.
	u32 IRQTARGET:1;
	u32 IRQOVF:1;
	u32 Repeat:1;
	u32 unused2:1;

	// 0 - System clock (it seems)
	// 1 - Pixel clock (counter 0)
	//     Horizontal retrace (counter 1)
	u32 ClockSource:1;

	// 0 - System clock (it seems)
	// 1 - 1/8 * System clock (counter 2)
	u32 Div:1;
/*
	When ClockSource of the counters is zero, they all run at the
		same speed. This speed seems to be about 8 times the normal
		speed of root counter 2, which is specified as 1/8 the system
		clock.
*/
#endif
} psxcnt_mode;

class psxCounter {
	protected:
		PsxEventType index;
		u16 Count, Target;
		union {
			u32 mode;
			psxcnt_mode mode_b;
		};
		u32 Rate;
		s32 cyclepass;
		u32 Interrupt;
		bool IsCounting;
		bool IsFutureTarget;

	public:
		psxCounter() :
			index( PsxEvt_Idle )			// some kinda..
		,	Interrupt( 0xcdcd )		// ... invalid object state!
		{
		}
		
		psxCounter( PsxEventType cntidx ) : 
			index( cntidx ),
			IsFutureTarget( true ),
			IsCounting( true ),
			cyclepass( 0 ),
			Rate( 1 << PsxFixedBits ),
			Interrupt( PsxInt_RTC0 + cntidx ),
			Count( 0 ),
			Target( 0 ),
			mode( 0 )
		{
		}
		
		void print_cnt() {
			SysPrintf("Stop: %d\n", mode_b.Stop);
			SysPrintf("Stop: %d\n", mode_b.unused);
			SysPrintf("Stop: %d\n", mode_b.Reset);
			SysPrintf("Stop: %d\n", mode_b.Tar);
			SysPrintf("Stop: %d\n", mode_b.IRQTARGET);
			SysPrintf("Stop: %d\n", mode_b.IRQOVF);
			SysPrintf("Stop: %d\n", mode_b.Repeat);
			SysPrintf("Stop: %d\n", mode_b.unused2);
			SysPrintf("Stop: %d\n", mode_b.ClockSource);
			SysPrintf("Stop: %d\n", mode_b.Div);
			SysPrintf("Count: %d\n", Count);
			SysPrintf("target: %d\n", Target);
			SysPrintf("rate: %d\n", Rate >> PsxFixedBits);
			SysPrintf("rate: %b\n", IsFutureTarget);
			SysPrintf("===============\n");
		}
		
		void AdvanceCycle(s32 delta) {
			cyclepass += delta;
		}
		
		s32 ScaleByRate( s32 delta ) const
		{
			return (delta * Rate) >> PsxFixedBits;
		}
		
		u16 ReadCount() {
			u16 ret = Count;
			if (mode_b.Stop == 0) {
				if(Config.RCntFix) {
					s32 pendingCycles = cyclepass + GetPendingCycles();
					s32 delta = (pendingCycles << PsxFixedBits) / (Rate * BIAS);
					ret += delta;
				}
				else {
					s32 pendingCycles = cyclepass + GetPendingCycles();
					s32 delta = (pendingCycles << PsxFixedBits) / Rate;
					ret += delta;
				}
			}
			//if(index == 2 && mode_b.Div != 0) SysPrintf("Rcount[%d] = 0x%x rate = 0x%x\n", index, ret, Rate);
			return ret;
		}
		
		u16 ReadTarget() {
			return Target;
		}
		
		u16 ReadMode() {
			return mode;
		}
		
		void Update();
		
		void Schedule() {
			if (mode_b.Stop == 0) {

				if (mode_b.IRQTARGET != 0 || mode_b.IRQOVF != 0) {
					s32 count;
					if(mode_b.Tar != 0)
					{
						count = ScaleByRate(Target - Count);
					}
					else {
						count = ScaleByRate(0x10000 - Count);
					}

					if (count > 0)
						R3000A::Interrupt.Schedule(index, count);
				}
			}
			cyclepass = 0;
		}
		
		void WriteMode(u32 newmode);
		
		void WriteCount(u32 newcount) {
			Count = newcount;
			IsFutureTarget = false;
			Schedule();
		}
		
		void WriteTargeg(u16 newtarget) {
			//	SysPrintf("writeCtarget[%ld] = %lx\n", index, value);
			Target = newtarget;
			_update_counted_timepass();
			IsFutureTarget = (Target <= Count);
			Schedule();
		}
		
		void _update_counted_timepass( )
		{
			if( R3000A::Interrupt.IsScheduled( index ) )
			{
				s32 pendingCycles = cyclepass + GetPendingCycles();
				s32 delta = (pendingCycles << PsxFixedBits) / Rate;
				Count += delta;
			}
		}
};

void psxCounter::WriteMode(u32 newmode) {
	//	SysPrintf("writeCmode[%ld] = %lx\n", index, value);
	mode = newmode;
	if(mode_b.Reset != 0)
		Count = 0;
	
	Rate = (1 << PsxFixedBits) / BIAS;
	if(index == 2) {
		if(mode_b.Div != 0)
			Rate = (8 << PsxFixedBits) / BIAS; // 1/8 speed
	}
	else if(mode_b.ClockSource != 0) {
		if(index == 0) {
			// based on resolution. See http://members.at.infoseek.co.jp/DrHell/ps1/ 
			// something like (vSyncRate.frame / (width * ((vSyncRate.scans / 3) * 2)))
			width_t screen;
			u32 width;
			screen.value = psxHu32(0x1814);
			if(screen.width.width1 != 0) width = 384;
			else {
				switch(screen.width.width0) {
					case 0: width = 256;
					case 1: width = 320;
					case 2: width = 512;
					case 3: width = 640;
				}
			}
			Rate = (vSyncRate.frame / (width * ((vSyncRate.scans / 3) * 2)));
		}
		else {
			Rate = (vSyncRate.frame / vSyncRate.scans);
			//SysPrintf("hSync = %d\n", psxCounters[index].rate >> PsxFixedBits);
		}
	}
	IsFutureTarget = false;
	Schedule();
}

void psxCounter::Update() {
	if (mode_b.IRQTARGET != 0 || mode_b.Tar != 0) {
		Count = Target;

		if(IsFutureTarget == false) {
			if(mode_b.IRQTARGET != 0) {
				psxRaiseExtInt((PsxIrq)Interrupt);
			}

			if(mode_b.Tar != 0) {
				// Reset on target
				Count = 0;
				if (mode_b.Repeat == 0) {
					IsFutureTarget = true;
				}
			}
			else
				IsFutureTarget = true;
		}
	}

	if (mode_b.IRQOVF != 0) {
		Count = 0;
		IsFutureTarget = false;
		psxRaiseExtInt((PsxIrq)Interrupt);
	}
	if (mode_b.Repeat != 0) {
		Schedule();
	}
}

static psxCounter *psxCounters[3] = {NULL, NULL, NULL};

void psxRcntInit() {
	for( int i=0; i<3; ++i ) {
		if(psxCounters[i] != NULL) delete psxCounters[i];
		psxCounters[i] = new psxCounter( ((PsxEventType)i) );
		psxCounters[i]->Schedule();
	}

	CalcRate(Config.PsxType);
// 	psx_int_add(PsxEvt_vBlank, 0);
	psxRcntVBlank();
}

void CalcRate(u32 region) {
	if(region & PSX_TYPE_PAL) {
		vSyncRate.frame = ((PSXCLK << PsxFixedBits) / 50);
		vSyncRate.scans = 625;
	}
	else {
		vSyncRate.frame = (((PSXCLK << PsxFixedBits) / 5994) * 100);
		vSyncRate.scans = 525;
	}
	vSyncRate.Blank = (vSyncRate.frame / vSyncRate.scans) * BIAS;
	
/*
  Region: PAL   Render: 675208;         Blank: 2167;
  Region: NTSC  Render: 562892;         Blank: 2152;
*/

	vSyncRate.Render = (vSyncRate.frame - vSyncRate.Blank) / (BIAS << PsxFixedBits);
	vSyncRate.Blank /= (BIAS << PsxFixedBits);
	vSyncRate.region = region;
// 	SysPrintf("Region: %s\t frame: %d\t Render: %d;\t Blank: %d;\n", vSyncRate.region ? "PAL" : "NTSC", vSyncRate.frame >> PsxFixedBits, vSyncRate.Render, vSyncRate.Blank);
}

void psxRcntVSync() {
	if(Config.PsxType != vSyncRate.region)
		CalcRate(Config.PsxType);

	GPU_updateLace(); // updateGPU
	SysUpdate();
#ifndef GEKKO
	ApplyCheats();
#endif
#ifdef GTE_LOG
	GTE_LOG("VSync\n");
#endif
	Interrupt.Schedule(PsxEvt_vBlank, vSyncRate.Render);
}

void psxRcntVBlank() {
	psxRaiseExtInt(PsxInt_VBlank);

	Interrupt.Schedule(PsxEvt_vSync, vSyncRate.Blank);
}

void psxRcntUpdate0() {
	psxCounters[0]->Update();
}

void psxRcntUpdate1() {
	psxCounters[1]->Update();
}

void psxRcntUpdate2() {
	psxCounters[2]->Update();
}

void psxRcntWcount(PsxEventType index, u16 value) {
	psxCounters[index]->WriteCount(value);
}

void psxRcntWmode(PsxEventType index, u32 value)  {
	psxCounters[index]->WriteMode(value);
}

void psxRcntWtarget(PsxEventType index, u16 value) {
	psxCounters[index]->WriteTargeg(value);
}

u16 psxRcntRcount(PsxEventType index) {
	return psxCounters[index]->ReadCount();
}

u32 psxRcntRmode(PsxEventType index) {
	return psxCounters[index]->ReadMode();
}

u16 psxRcntRtarget(PsxEventType index) {
	return psxCounters[index]->ReadTarget();
}

void RcntAdvanceCycles(s32 delta) {
	for(int i = 0; i < 3; i++)
		psxCounters[i]->AdvanceCycle(delta);
}

int psxRcntFreeze(gzFile f, int Mode) {
	char Unused[4096 - sizeof(psxCounter)];

	gzfreezel(psxCounters);
	gzfreezel(Unused);

	return 0;
}

}
