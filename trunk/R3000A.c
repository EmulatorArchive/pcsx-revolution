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

//#include "PsxCommon.h"

//R3000Acpu *psxCpu;
psxRegisters psxRegs;

int psxInit() {

	psxCpu = &psxInt;
#if defined(__i386__) || defined(__sh__) || defined(__ppc__)
	if (!Config.Cpu) psxCpu = &psxRec;
#endif
	Log=0; // GP

	if (psxMemInit() == -1) {
		SysMessage("Failed to allocate memory for emulation\n Please restart the emulator and try again.");
		return -1;
	}
	
	return psxCpu->Init();
}

void psxReset() 
{
	psxCpu->Reset();
	psxMemReset();
	memset(&psxRegs, 0, sizeof(psxRegs));
	psxRegs.pc = 0xbfc00000; // Start in bootstrap
	psxRegs.CP0.r[12] = 0x10900000; // COP0 enabled | BEV = 1 | TS = 1
	psxRegs.CP0.r[15] = 0x00000002; // PRevID = Revision ID, same as R3000A
	psxHwReset();
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

	psxRegs.pc = (psxRegs.CP0.n.Status & 0x400000) ? 0xbfc00180 : 0x80000080;

	// Set the Status
	psxRegs.CP0.n.Status = (psxRegs.CP0.n.Status &~0x3f) |
						  ((psxRegs.CP0.n.Status & 0xf) << 2);
/*
	if (!Config.HLE && (((PSXMu32(psxRegs.CP0.n.EPC) >> 24) & 0xfe) == 0x4a)) {
		// "hokuto no ken" / "Crash Bandicot 2" ... fix
		PSXMu32ref(psxRegs.CP0.n.EPC)&= SWAPu32(~0x02000000);
	}
*/
	if (Config.HLE) psxBiosException();
}

static __inline int psxSetNextBranch( u32 startCycle, s32 delta )
{
	// typecast the conditional to signed so that things don't explode
	// if the startCycle is ahead of our current cpu cycle.

	if( (int)(psxRegs.NextBranchCycle - startCycle) > delta )
		psxRegs.NextBranchCycle = startCycle + delta;
}

__inline void PSX_INT( int  n, s32 ecycle )
{
	// Generally speaking games shouldn't throw ints that haven't been cleared yet.
	// It's usually indicative os something amiss in our emulation, so uncomment this
	// code to help trap those sort of things.

	// Exception: IRQ16 - SIO - it drops ints like crazy when handling PAD stuff.
	//if( /*n!=16 &&*/ iopRegs.interrupt & (1<<n) )
	//	SysPrintf( "***** IOP > Twice-thrown int on IRQ %d\n", n );

	psxRegs.interrupt |= 1 << n;

	psxRegs.sCycle[n] = psxRegs.cycle;
	psxRegs.eCycle[n] = ecycle;

	psxSetNextBranch( psxRegs.cycle, ecycle );
}

__inline int psxTestCycle( u32 startCycle, s32 delta )
{
	// typecast the conditional to signed so that things don't explode
	// if the startCycle is ahead of our current cpu cycle.

	return (int)(psxRegs.cycle - startCycle) >= delta;
}

static __inline void PsxTestEvent( unsigned int n, void (*callback)() )
{
	if( !(psxRegs.interrupt & (1 << n)) ) return;

	if( psxTestCycle( psxRegs.sCycle[n], psxRegs.eCycle[n] ) )
	{
		psxRegs.interrupt &= ~(1 << n);
		callback();
	}
	else
		psxSetNextBranch( psxRegs.sCycle[n], psxRegs.eCycle[n] );
}

__inline void _psxTestInterrupts()
{
	PsxTestEvent(PsxEvt_SIO,		sioInterrupt);
	PsxTestEvent(PsxEvt_Cdrom,		cdrInterrupt);

	// Profile-guided Optimization (sorta)
	// The following ints are rarely called.  Encasing them in a conditional
	// as follows helps speed up most games.

	if( psxRegs.interrupt & ( (1ul<<18) | (3ul<<27) | (3ul<<29) ) )
	{
		PsxTestEvent(PsxEvt_MDEC,		mdec1Interrupt);
		PsxTestEvent(PsxEvt_GPU,		gpuInterrupt);
		PsxTestEvent(PsxEvt_CdromRead,	cdrReadInterrupt);
	}
}

void psxBranchTest() {
	if( psxTestCycle( psxNextsCounter, psxNextCounter ) )
		psxRcntUpdate();

	psxRegs.NextBranchCycle = psxNextsCounter + psxNextCounter;

	if (psxRegs.interrupt)
	{
		_psxTestInterrupts();
	}

	if (psxRegs.interrupt & 0x80000000) {
		psxRegs.interrupt&=~0x80000000;
		psxTestHWInts();
	}

	if( psxHu32(0x1078) == 0 ) return;
	if( (psxHu32(0x1070) & psxHu32(0x1074)) == 0 ) return;

	if ((psxRegs.CP0.n.Status & 0xFE01) >= 0x401)
	{
		//PSXCPU_LOG("Interrupt: %x  %x", psxHu32(0x1070), psxHu32(0x1074));
		psxException(0, 0);
	}
//	if (psxRegs.cycle > 0xd29c6500) Log=1;
}

void psxTestIntc()
{
	if( psxHu32(0x1078) == 0 ) return;
	if( (psxHu32(0x1070) & psxHu32(0x1074)) == 0 ) return;

	psxSetNextBranch( psxRegs.cycle, 2 );
}

void psxTestHWInts() {
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

