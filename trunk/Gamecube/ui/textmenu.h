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

#else	//!HW_RVL

#define GetInput(Wiimote, GC, Classic) \
		Input(PAD_BUTTON_##GC)

static u32 Input(u32 GC)
{
	return (PAD_ButtonsDown(0) & GC);
}
#endif	//HW_RVL

void Main_menu();
int textFileBrowser(char *directory);
void Config_menu();

#endif
