#include "wiifat.h"
#include "../Config.h"
#include "mount.h"
#include <stdio.h>
#include <fat.h>

#ifdef HW_RVL
#include <sdcard/wiisd_io.h>
#include <ogc/system.h>
#include <ogc/usbstorage.h>
#else
#include <sdcard/gcsd.h>
#endif

static int isMounted[FAT_DEVICES_COUNT] = {-1, -1};

static DISC_INTERFACE *fat_interface[FAT_DEVICES_COUNT] = {
#ifdef HW_RVL
	&__io_wiisd,
	&__io_usbstorage
#else
	&__io_gcsda,
	&__io_gcsdb
#endif
};

static char *fat_name[FAT_DEVICES_COUNT] = {
#ifdef HW_RVL
	"sd",
	"usb"
#else
	"carda",
	"cardb"
#endif
};

void UnmountAllFAT()
{
	int i;
	for(i = FAT_DEVICE_0; i < FAT_DEVICES_COUNT; i++)
		fatUnmount( fat_name[i] );
}

mount_state MountFAT(int device)
{
	if (device >= FAT_DEVICES_COUNT) return 1;

	int mounted = MOUNTED; // assume our disc is already mounted

	const DISC_INTERFACE* disc = fat_interface[device];

	if(isMounted[device] == NOT_MOUNTED)
	{
		if(!disc->startup() || !fatMountSimple( fat_name[device], disc ))
			mounted = NOT_MOUNTED;
	}

	isMounted[device] = mounted;
	return mounted;
}

void MountAllFAT()
{
	int i;
	for(i = FAT_DEVICE_0; i < FAT_DEVICES_COUNT; i++)
		MountFAT( i );
}
