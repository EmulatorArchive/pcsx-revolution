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

#include <stdio.h>
#include <string.h>

#include "ppc.h"

// General Purpose hardware registers
/*int cpuHWRegisters[NUM_HW_REGISTERS] = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 16, 17, 18, 19, 20, 
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};*/

u32 *ppcPtr;

void ppcInit() {
}
void ppcSetPtr(u32 *ptr) {
	ppcPtr = ptr;
}
void ppcAlign(int bytes) {
	// forward align
	ppcPtr = (u32*)(((u32)ppcPtr + bytes) & ~(bytes - 1));
}

void ppcShutdown() {
}

void Write32(u32 val)
{
	*(u32*)ppcPtr = val;
	ppcPtr++;
}

void CALLFunc(u32 func) {
    //ReleaseArgs();
    if ((func & 0x1fffffc) == func) {
        BLA(func);
    } else {
        LIW(0, func);
        MTCTR(0);
        BCTRL();
    }
}
