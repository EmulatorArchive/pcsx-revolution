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

#ifndef __DEBUG_H__
#define __DEBUG_H__

enum breakpoint_types {
	E, R1, R2, R4, W1, W2, W4
};

#ifdef __cplusplus
extern "C" {
#endif

void StartDebugger();
void StopDebugger();

void DebugVSync();
void ProcessDebug();

void DebugCheckBP(u32 address, enum breakpoint_types type);

void PauseDebugger();
void ResumeDebugger();

#ifdef __cplusplus
} // extern "C" 
#endif

extern const char *disRNameCP0[];

char* disR3000AF(u32 code, u32 pc);

extern FILE *emuLog;

/* 
 * Specficies which logs should be activated.
 * Ryan TODO: These should ALL be definable with configure flags.
 */

//#define GTE_DUMP

#ifdef GTE_DUMP
FILE *gteLog;
#endif

//#define LOG_STDOUT

//#define PAD_LOG  __Log
//#define GTE_LOG  __Log
//#define CDR_LOG  __Log("%8.8lx %8.8lx: ", psxRegs.pc, psxRegs.cycle); __Log

//#define PSXHW_LOG   __Log("%8.8lx %8.8lx: ", psxRegs.pc, psxRegs.cycle); __Log
//#define PSXBIOS_LOG __Log("%8.8lx %8.8lx: ", psxRegs.pc, psxRegs.cycle); __Log
//#define PSXDMA_LOG  __Log
//#define PSXMEM_LOG  __Log("%8.8lx %8.8lx: ", psxRegs.pc, psxRegs.cycle); __Log
//#define PSXCPU_LOG  __Log

//#define CDRCMD_DEBUG

#if defined (PSXCPU_LOG) || defined(PSXDMA_LOG) || defined(CDR_LOG) || defined(PSXHW_LOG) || \
	defined(PSXBIOS_LOG) || defined(PSXMEM_LOG) || defined(GTE_LOG)    || defined(PAD_LOG)
#define EMU_LOG __Log
#endif

#endif /* __DEBUG_H__ */
