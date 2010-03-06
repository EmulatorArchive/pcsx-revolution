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


#include <stdlib.h>

#include "system.h"
#include "psxcommon.h"
#include "plugins.h"
#include "r3000a.h"
#include "ui/textmenu.h"

using namespace R3000A;


void SysPrintf(const char *fmt, ...) {
	if (Config.PsxOut) {
		va_list list;
		char msg[512];

		va_start(list, fmt);
		vsprintf(msg, fmt, list);
		va_end(list);
	
		printf ("%s", msg);
	}
}

int SysInit() {
    SysPrintf("start SysInit()\r\n");

    SysPrintf("psxInit()\r\n");
	psxInit();

    SysPrintf("LoadPlugins()\r\n");
	if(LoadPlugins()==-1)
		SysPrintf("ErrorLoadingPlugins()\r\n");
	SysPrintf("end SysInit()\r\n");
	return 0;
}

void SysReset() {
    SysPrintf("start SysReset()\r\n");
	psxReset();
	SysPrintf("end SysReset()\r\n");
}

void SysClose() {
	psxShutdown();
	ReleasePlugins();
} 

void SysRunGui() {
	Main_menu();
	psxCpu->Execute();
}

void SysMessage(const char *fmt, ...) {
	
}

const char *SysLibError() {
	return NULL;
}

void SysCloseLibrary(void *lib) {
//	dlclose(lib);
}

int framesdone = 0;
void SysUpdate() {
#ifdef SHOW_DEBUG

#endif
}
