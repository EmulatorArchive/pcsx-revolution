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

#ifndef __PPC_H__
#define __PPC_H__

// include basic types
#include "psxcommon.h"
#include "ppc_mnemonics.h"

#define PPCARG1 3
#define PPCARG2 4

void Write32(u32 val);
void ReleaseArgs();

void CALLFunc(u32 func);

extern u32 *ppcPtr;

void ppcInit();
void ppcSetPtr(u32 *ptr);
void ppcShutdown();

void ppcAlign(int bytes);

#define B_TRUE 12
#define B_FALSE 4

// Less than
#define LT 0
// Greater than
#define GT 1
// Equal
#define EQ 2
// Summary overflow
#define SO 3

// Greater than or equal
#define GE 0
// Less than or equal
#define LE 1
// Not equal
#define NE 2
// Not summary overflow
#define NS 3

enum branch_type {
	BT_UN = 0,		// unconditional

	BT_LT = ((B_TRUE << 5) | LT),
	BT_GT,
	BT_EQ,
	BT_SO,
	
	BT_GE = ((B_FALSE << 5) | GE),
	BT_LE,
	BT_NE,
	BT_NS
};

class BranchTarget {
	branch_type Type;
	u32 *Ptr;
public:
	BranchTarget(branch_type type) : Type(type) {
		Ptr = ppcPtr;
		if(Type == BT_UN) {
			B(0);
		}
		else {
			BC(Type >> 5, (Type & 0x1f), 0);
		}
	}

	void setTarget() {
		u32 addr = (u32)ppcPtr - (u32)Ptr;
		u32 mask = Type == BT_UN ? 0xffffff : 0x3fff;
		*Ptr = *Ptr | (addr & mask);
	}
};

extern "C" {

void recRun(void (*func)(), u32 hw1, u32 hw2);
void returnPC();

};

u8 dynMemRead8(u32 mem);
u16 dynMemRead16(u32 mem);
u32 dynMemRead32(u32 mem);
void dynMemWrite32(u32 mem, u32 val);

#endif /* __PPC_H__ */
