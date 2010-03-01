#include "gcMisc.h"

int wii_shutdown;		// 1-shutdown, 2-return to HBC
int wii_reset;
int Running;

inline void clrscr()
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
	wii_reset		= 1;
}

void ShutdownWii()
{
	if(Running) SysClose();
	SYS_ResetSystem(SYS_POWEROFF, 0, 0);
}

#endif
