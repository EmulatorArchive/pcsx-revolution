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

#ifndef __R3000A_H__
#define __R3000A_H__

#include "psxcommon.h"

namespace R3000A {

struct R3000Acpu {
	int  (*Init)();
	void (*Reset)();
	void (*Execute)();		/* executes up to a break */
	void (*ExecuteBlock)();	/* executes up to a jump */
	void (*Clear)(u32 Addr, u32 Size);
	void (*Shutdown)();
};

extern R3000Acpu *psxCpu;

extern R3000Acpu psxInt;
#if (defined(__x86_64__) || defined(__i386__) || defined(__sh__) || defined(__ppc__)) && !defined(NOPSXREC)
extern R3000Acpu psxRec;
#define PSXREC
#endif

//#define GTE_TIMING

typedef struct {
	short x, y;
} SVector2D;

typedef struct {
	short z, pad;
} SVector2Dz;

typedef struct {
	short x, y, z, pad;
} SVector3D;

typedef struct {
	short x, y, z, pad;
} LVector3D;

typedef struct {
	unsigned char r, g, b, c;
} CBGR;

typedef struct {
	short m11, m12, m13, m21, m22, m23, m31, m32, m33, pad;
} SMatrix3D;

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
		SVector3D	v0, v1, v2;
		CBGR		rgb;
		s32			otz;
		s32			ir0, ir1, ir2, ir3;
		SVector2D	sxy0, sxy1, sxy2, sxyp;
		SVector2Dz	sz0, sz1, sz2, sz3;
		CBGR		rgb0, rgb1, rgb2;
		s32			reserved;
		s32			mac0, mac1, mac2, mac3;
		u32			irgb, orgb;
		s32			lzcs, lzcr;
	} n;
	PAIR r[32];
} psxCP2Data;

typedef union {
	struct {
		SMatrix3D rMatrix;
		s32      trX, trY, trZ;
		SMatrix3D lMatrix;
		s32      rbk, gbk, bbk;
		SMatrix3D cMatrix;
		s32      rfc, gfc, bfc;
		s32      ofx, ofy;
		s32      h;
		s32      dqa, dqb;
		s32      zsf3, zsf4;
		s32      flag;
	} n;
	PAIR r[32];
} psxCP2Ctrl;

typedef union {
	struct {
		u32 	r0, at, v0, v1, a0, a1, a2, a3,
				t0, t1, t2, t3, t4, t5, t6, t7,
				s0, s1, s2, s3, s4, s5, s6, s7,
				t8, t9, k0, k1, gp, sp, s8, ra, lo, hi;
	} n;
	PAIR r[34]; /* Lo, Hi in r[32] and r[33] */
} psxGPRRegs;

typedef union {
	struct {
		u32		Index,     Random,    EntryLo0,  EntryLo1,
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

struct psxRegisters {
	psxGPRRegs GPR;		/* General Purpose Registers */
	psxCP0Regs CP0;		/* Coprocessor0 Registers */
	psxCP2Data CP2D;	/* Cop2 data registers */
	psxCP2Ctrl CP2C; 	/* Cop2 ctrl registers */
	u32 cycle;
	u32 pc;				/* Program counter */
	u32 code;			/* The instruction */

	bool IsDelaySlot;

	// marks the original duration of time for the current pending event.  This is
	// typically used to determine the amount of time passed since the last update
	// to psxRegs.cycle:
	//  currentcycle = cycle + ( evtCycleDuration - evtCycleCountdown );
	s32 evtCycleDuration;

	// marks the *current* duration of time until the current pending event. In
	// other words: counts down from evtCycleDuration to 0; event is raised when 0
	// is reached.
	s32 evtCycleCountdown;
	
	u32 GetCycle() {
		return cycle + (evtCycleDuration - evtCycleCountdown);
	}

	s32 GetPendingCycles() {
		return evtCycleDuration - evtCycleCountdown;
	}

#ifdef GTE_TIMING
	// number of cycles pending on the current GTE instruction 
	// Any zero-or-negative values mean the unit is free and no stalls incurred for executing
	// a new instruction on the pipeline.  Negative values are flushed to 0 during BranchTest executions.
	// (faster than flushing to zero on every cycle update).
	s32 GteUnitCycles;
#endif
};

extern psxRegisters psxRegs;

/**** R3000A Instruction Macros ****/
#define _PC_       psxRegs.pc       // The next PC to be executed

#define _fOp_(code)	((code >> 26)       )  // The opcode part of the instruction register 
#define _fFunct_(code)	((code      ) & 0x3F)  // The funct part of the instruction register 
#define _fRd_(code)	((code >> 11) & 0x1F)  // The rd part of the instruction register 
#define _fRt_(code)	((code >> 16) & 0x1F)  // The rt part of the instruction register 
#define _fRs_(code)	((code >> 21) & 0x1F)  // The rs part of the instruction register 
#define _fSa_(code)	((code >>  6) & 0x1F)  // The sa part of the instruction register
#define _fIm_(code)	((u16)code)            // The immediate part of the instruction register
#define _fTarget_(code)	(code & 0x3ffffff)     // The target part of the instruction register

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

void psxException(u32 code, bool bd);
void psxExecuteBios();
int  psxTestLoadDelay(int reg, u32 tmp);
void psxDelayTest(int reg, u32 bpc);
void psxTestSWInts();
void psxTestHWInts();

void psxJumpTest();

void advance_pc(s32 offset);

} // namespace R3000A

#endif /* __R3000A_H__ */
