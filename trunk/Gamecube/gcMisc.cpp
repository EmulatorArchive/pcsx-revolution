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

#include "gcMisc.h"

int wii_shutdown;		// 1-shutdown, 2-return to HBC
int wii_reset;
int Running;

void clrscr()
{
	printf("\x1B[2J");
	VIDEO_WaitVSync();
	printf("\x1B[2;2H");
}

void to_loader()
{
	if(Running) SysClose();
	exit(0);
}

#ifdef HW_RVL

void ShutdownCB()
{
	wii_shutdown	= 1;
}

void ResetCB()
{
	wii_reset	= 1;
}

void ShutdownWii()
{
	if(Running) SysClose();
	SYS_ResetSystem(SYS_POWEROFF, 0, 0);
}

#endif
