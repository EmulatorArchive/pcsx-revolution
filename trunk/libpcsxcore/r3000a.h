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

#ifndef __R3000A_H__
#define __R3000A_H__

#include "psxcommon.h"
#include "psxmem.h"
#include "psxcounters.h"
#include "psxbios.h"

typedef struct {
	int  (*Init)();
	void (*Reset)();
	void (*Execute)();		/* executes up to a break */
	void (*ExecuteBlock)();	/* executes up to a jump */
	void (*Clear)(u32 Addr, u32 Size);
	void (*Shutdown)();
} R3000Acpu;

R3000Acpu *psxCpu;
extern R3000Acpu psxInt;
#if (defined(__x86_64__) || defined(__i386__) || defined(__sh__) || defined(__ppc__)) && !defined(NOPSXREC)
extern R3000Acpu psxRec;
#define PSXREC
#endif

//#define GTE_TIMING

enum PsxEventType
{
	PsxEvt_Counter0 = 0,

	PsxEvt_Counter1,
	PsxEvt_Counter2,
	PsxEvt_Counter3,
	PsxEvt_Counter4,

	PsxEvt_Exception,		// 5
	PsxEvt_SIO,				// 6
	PsxEvt_GPU,				// 7

	PsxEvt_Cdrom,			// 8
	PsxEvt_CdromRead,		// 9
	PsxEvt_SPU,				// 10
	
	PsxEvt_MDEC,			// 11

	PsxEvt_CountNonIdle,

	// Idle state, no events scheduled.  Placed at -1 since it has no actual
	// entry in the Event System's event schedule table.
	PsxEvt_Idle = PsxEvt_CountNonIdle,

	PsxEvt_CountAll		// total number of schedulable event types in the Psx
};

typedef struct int_timer {
	u32 time;
	u32 cycle;
	void (*Execute)();
	struct int_timer *next;
} events_t;

typedef struct {
	events_t list[PsxEvt_CountAll];
	events_t *next;
} Events_t;

Events_t Events;

typedef union
{
#if defined(__BIGENDIAN__) || defined(GEKKO)
	struct { u8 h3, h2, h, l; } b;
	struct { s8 h3, h2, h, l; } sb;
	struct { u16 h, l; } w;
	struct { s16 h, l; } sw;
#else
	struct { u8 l, h, h2, h3; } b;
	struct { u16 l, h; } w;
	struct { s8 l, h, h2, h3; } sb;
	struct { s16 l, h; } sw;
#endif
	u32 d;
	s32 sd;
} PAIR;

typedef union {
	struct {
		u32   r0, at, v0, v1, a0, a1, a2, a3,
						t0, t1, t2, t3, t4, t5, t6, t7,
						s0, s1, s2, s3, s4, s5, s6, s7,
						t8, t9, k0, k1, gp, sp, s8, ra, lo, hi;
	} n;
	PAIR r[34]; /* Lo, Hi in r[32] and r[33] */
} psxGPRRegs;

typedef union {
	struct {
		u32	Index,     Random,    EntryLo0,  EntryLo1,
						Context,   PageMask,  Wired,     Reserved0,
						BadVAddr,  Count,     EntryHi,   Compare,
						Status,    Cause,     EPC,       PRid,
						Config,    LLAddr,    WatchLO,   WatchHI,
						XContext,  Reserved1, Reserved2, Reserved3,
						Reserved4, Reserved5, ECC,       CacheErr,
						TagLo,     TagHi,     ErrorEPC,  Reserved6;
	} n;
	PAIR r[32];
} psxCP0Regs;

typedef struct {
	psxGPRRegs GPR;		/* General Purpose Registers */
	psxCP0Regs CP0;		/* Coprocessor0 Registers */
	PAIR cp2d[32];	 	/* Cop2 data registers */
	PAIR cp2c[32]; 		/* Cop2 ctrl registers */
    u32 pc;				/* Program counter */
    u32 code;			/* The instruction */
	u32 cycle;

	u32 IsDelaySlot:1;

	// marks the original duration of time for the current pending event.  This is
	// typically used to determine the amount of time passed since the last update
	// to psxRegs.cycle:
	//  currentcycle = cycle + ( evtCycleDuration - evtCycleCountdown );
	s32 evtCycleDuration;

	// marks the *current* duration of time until the current pending event. In
	// other words: counts down from evtCycleDuration to 0; event is raised when 0
	// is reached.
	s32 evtCycleCountdown;

#ifdef GTE_TIMING
	// number of cycles pending on the current GTE instruction 
	// Any zero-or-negative values mean the unit is free and no stalls incurred for executing
	// a new instruction on the pipeline.  Negative values are flushed to 0 during BranchTest executions.
	// (faster than flushing to zero on every cycle update).
	s32 GteUnitCycles;
#endif
} psxRegisters;

extern psxRegisters psxRegs;

/**** R3000A Instruction Macros ****/
#define _PC_       psxRegs.pc       // The next PC to be executed

#define _fOp_(code)		((code >> 26) & 0x3F)  // The opcode part of the instruction register 
#define _fFunct_(code)	((code      ) & 0x3F)  // The funct part of the instruction register 
#define _fRd_(code)		((code >> 11) & 0x1F)  // The rd part of the instruction register 
#define _fRt_(code)		((code >> 16) & 0x1F)  // The rt part of the instruction register 
#define _fRs_(code)		((code >> 21) & 0x1F)  // The rs part of the instruction register 
#define _fSa_(code)		((code >>  6) & 0x1F)  // The sa part of the instruction register
#define _fIm_(code)		((u16)code)            // The immediate part of the instruction register
#define _fTarget_(code)	(code & 0x3ffffff)    // The target part of the instruction register

#define _fImm_(code)	((s16)code)            // sign-extended immediate
#define _fImmU_(code)	(code&0xffff)          // zero-extended immediate

#define _Op_     _fOp_(psxRegs.code)
#define _Funct_  _fFunct_(psxRegs.code)
#define _Rd_     _fRd_(psxRegs.code)
#define _Rt_     _fRt_(psxRegs.code)
#define _Rs_     _fRs_(psxRegs.code)
#define _Sa_     _fSa_(psxRegs.code)
#define _Im_     _fIm_(psxRegs.code)
#define _Target_ _fTarget_(psxRegs.code)

#define _Imm_	 _fImm_(psxRegs.code)
#define _ImmU_	 _fImmU_(psxRegs.code)

#define _JumpTarget_    ((_Target_ * 4) + (_PC_ & 0xf0000000))   // Calculates the target during a jump instruction
#define _BranchTarget_  ((s16)_Im_ * 4 + _PC_)                 // Calculates the target during a branch instruction

#define _SetLink(x)     psxRegs.GPR.r[x].d = _PC_ + 4;       // Sets the return address in the link register

#define _rRtU_	(psxRegs.GPR.r[_Rt_].d)
#define _rRsU_	(psxRegs.GPR.r[_Rs_].d)
#define _rRdU_	(psxRegs.GPR.r[_Rd_].d)
#define _rFsU_	(psxRegs.CP0.r[_Rd_].d)
#define _rLoU_	(psxRegs.GPR.r[32].d)
#define _rHiU_	(psxRegs.GPR.r[33].d)

#define _rRtS_	(psxRegs.GPR.r[_Rt_].sd)
#define _rRsS_	(psxRegs.GPR.r[_Rs_].sd)
#define _rRdS_	(psxRegs.GPR.r[_Rd_].sd)
#define _rFsS_	(psxRegs.CP0.r[_Rd_].sd)
#define _rLoS_	(psxRegs.GPR.r[32].sd)
#define _rHiS_	(psxRegs.GPR.r[33].sd)

int  psxInit();
void psxReset();
void psxShutdown();
void psxException(u32 code, u32 bd);
void psxBranchTest();
void psxExecuteBios();
int  psxTestLoadDelay(int reg, u32 tmp);
void psxDelayTest(int reg, u32 bpc);
void psxTestSWInts();
void psxTestHWInts();
void psxJumpTest();

u32 psxGetCycle();
s32 GetPendingCycles();

void psx_int_add(int n, s32 ecycle);
void psx_int_remove(int n);
void psxRaiseExtInt( uint irq );
u8 psxIsActiveEvent(int n);

void advance_pc(s32 offset);
void psxTestIntc();

#endif /* __R3000A_H__ */
