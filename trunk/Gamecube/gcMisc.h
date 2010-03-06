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

#ifndef GCMISC_H
#define GCMISC_H

#include <gccore.h>
#include <ogcsys.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "psxcommon.h"
#include "pad.h"

void clrscr();

#ifdef __cplusplus
extern "C" {
#endif

extern u32* xfb[2];

extern GXRModeObj *vmode;

#ifdef HW_RVL

#define GetInput(Wiimote, GC, Classic) \
		Input(WPAD_BUTTON_##Wiimote, PAD_BUTTON_##GC, WPAD_CLASSIC_BUTTON_##Classic)
		
static u32 Input(u32 Wiimote, u32 GC, u32 Classic)
{
	return ((WPAD_ButtonsDown(0) & Wiimote) || (PAD_ButtonsDown(0) & GC) || (WPAD_ButtonsDown(0) & Classic));
}

#define GetHeld(Wiimote, GC, Classic) \
		Held(WPAD_BUTTON_##Wiimote, PAD_BUTTON_##GC, WPAD_CLASSIC_BUTTON_##Classic)
		
static u32 Held(u32 Wiimote, u32 GC, u32 Classic)
{
	return ((WPAD_ButtonsHeld(0) & Wiimote) || (PAD_ButtonsHeld(0) & GC) || (WPAD_ButtonsHeld(0) & Classic));
}

#else	//!HW_RVL

#define GetInput(Wiimote, GC, Classic) \
		Input(PAD_BUTTON_##GC)

static u32 Input(u32 GC)
{
	return (PAD_ButtonsDown(0) & GC);
}

#define GetHeld(Wiimote, GC, Classic) \
		Held(PAD_BUTTON_##GC)

static u32 Held(u32 GC)
{
	return (PAD_ButtonsHeld(0) & GC);
}
#endif	//HW_RVL

#ifdef HW_RVL
#define CHECK_POWER_BUTTONS() \
	PAD_ScanPads(); \
	WPAD_ScanPads(); \
	if(wii_shutdown) ShutdownWii(); \
	if(wii_reset) to_loader();

extern int wii_shutdown;		// 1-shutdown, 2-return to HBC
extern int wii_reset;

void ShutdownCB();
void ResetCB();
void ShutdownWii();
#else
#define CHECK_POWER_BUTTONS() \
	PAD_ScanPads();
#endif

extern int Running;
void to_loader();

#ifdef __cplusplus
}
#endif

#endif

