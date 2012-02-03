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

#ifndef __MOUNT_H__
#define __MOUNT_H__

typedef enum {
	FAT_DEVICE_0 = 0
,	FAT_DEVICE_1
,	DEVICE_DVD
#ifdef HW_RVL
,	DEVICE_SMB
#endif
,	DEVICES_COUNT
,	FAT_DEVICES_COUNT = FAT_DEVICE_1+1
} gekko_device;

typedef enum {
	NOT_MOUNTED = -1,
	MOUNTED = 1
} mount_state;

static const char *device[DEVICES_COUNT] = {
#ifdef HW_RVL
	"sd:/"
,	"usb:/"
,	"dvd:/"
,	"smb:/"
#else
	"carda:/"
,	"cardb:/"
,	"dvd:/"
#endif
};

#endif
