#include "wiifat.h"
#include "../Config.h"
#include "mount.h"
#include <stdio.h>
#include <fat.h>
#include <sdcard/wiisd_io.h>
#include <sdcard/gcsd.h>
#include <ogc/usbstorage.h>

static int isMounted[2] = {-1, -1};

#ifdef HW_RVL
	const DISC_INTERFACE* sd = &__io_wiisd;
	const DISC_INTERFACE* usb = &__io_usbstorage;
#else
	const DISC_INTERFACE* carda = &__io_gcsda;
	const DISC_INTERFACE* cardb = &__io_gcsdb;
#endif

void UnmountAllFAT()
{
#ifdef HW_RVL
	fatUnmount("sd:/");
	fatUnmount("usb:/");
#else
	fatUnmount("carda:/");
	fatUnmount("cardb:/");
#endif
}

int MountFAT(int method)
{
	int mounted = 1; // assume our disc is already mounted
	char name[10];
	const DISC_INTERFACE* disc = NULL;

	switch(method)
	{
#ifdef HW_RVL
		case DEVICE_SD:
			sprintf(name, "sd");
			disc = sd;
			break;
		case DEVICE_USB:
			sprintf(name, "usb");
			disc = usb;
			break;
#else
		case DEVICE_SD:
			sprintf(name, "carda");
			disc = carda;
			break;

		case DEVICE_USB:
			sprintf(name, "cardb");
			disc = cardb;
			break;
#endif
		default:
			return -1; // unknown device
	}

	//sprintf(Settings.filename, "%s:/", name);
/*
	if(unmountRequired[method])
	{
		unmountRequired[method] = 0;
		fatUnmount(Settings.filename);
		disc->shutdown();
		isMounted[method] = 0;
	}*/
	if(isMounted[method] == -1)
	{
		//if(disc->isInserted())
		{
			if(!disc->startup() || !fatMountSimple(name, disc))
				mounted = -1;
		}
	}

	isMounted[method] = mounted;
	return mounted;
}

void MountAllFAT()
{
	MountFAT(DEVICE_SD);
	MountFAT(DEVICE_USB);
}
