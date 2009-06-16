#ifndef GCMISC_H
#define GCMISC_H

#include <gccore.h>
#include <ogcsys.h>
#include <stdlib.h>
#include <stdio.h>
#include <fat.h>
#include <unistd.h>
#include "../PsxCommon.h"

void clrscr();

extern u32* xfb[2];

extern GXRModeObj *vmode;

#ifdef HW_RVL
#define CHECK_POWER_BUTTONS() \
	PAD_ScanPads(); \
	WPAD_ScanPads(); \
	if(wii_shutdown) ShutdownWii(); \
	if(wii_reset) to_loader();

int wii_shutdown;		// 1-shutdown, 2-return to HBC
int wii_reset;

void ShutdownCB();
void ResetCB();
void ShutdownWii();
#else
#define CHECK_POWER_BUTTONS() \
	PAD_ScanPads();
#endif

int Running;
void to_loader();

#endif

