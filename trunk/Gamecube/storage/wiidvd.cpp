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

#include <di/di.h>
#include <iso9660.h>
#include <unistd.h>
#include "mount.h"

static const short retries = 20;

mount_state MountDVD()
{
	#ifdef HW_RVL
	mount_state ret = NOT_MOUNTED;
	int x = retries;
	DI_Mount();
	while (DI_GetStatus() & DVD_INIT) usleep(5000);
	while (!(DI_GetStatus() & DVD_READY) && x--) usleep(50000);
	if (DI_GetStatus() & DVD_READY) ret = (mount_state) ISO9660_Mount();
	return ret;
	#endif
}

void UnMountDVD()
{
	#ifdef HW_RVL
	DI_StopMotor();
	ISO9660_Unmount();
	DI_Close();
	#endif
}

void InitDVD()
{
	#ifdef HW_RVL
	DI_Init();
	#endif
}
