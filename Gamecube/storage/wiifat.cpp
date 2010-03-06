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

static mount_state isMounted[FAT_DEVICES_COUNT] = {NOT_MOUNTED, NOT_MOUNTED};

static const DISC_INTERFACE *fat_interface[FAT_DEVICES_COUNT] = {
#ifdef HW_RVL
	&__io_wiisd,
	&__io_usbstorage
#else
	&__io_gcsda,
	&__io_gcsdb
#endif
};

static const char *fat_name[FAT_DEVICES_COUNT] = {
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
	if (device >= FAT_DEVICES_COUNT) return NOT_MOUNTED;

	mount_state mounted = MOUNTED; // assume our disc is already mounted

	if(isMounted[device] == NOT_MOUNTED)
	{
		if(!fat_interface[device]->startup() || !fatMountSimple( fat_name[device], fat_interface[device] ))
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
