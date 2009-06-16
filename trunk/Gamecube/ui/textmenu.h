#ifndef TEXTMENU_H
#define TEXTMENU_H

#include "../gcMisc.h"

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

void Main_menu();
int textFileBrowser(char *directory);
void Config_menu();

#endif
