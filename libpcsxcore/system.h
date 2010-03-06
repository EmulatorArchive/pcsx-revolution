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

#ifndef __PSXSYSTEM_H__
#define __PSXSYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

int  SysInit();							// Init mem and plugins
void SysReset();						// Resets mem
void SysPrintf(const char *fmt, ...);			// Printf used by bios syscalls
void SysMessage(const char *fmt, ...);		// Message used to print msg to users
void *SysLoadLibrary(const char *lib);		// Loads Library
void *SysLoadSym(void *lib, const char *sym);	// Loads Symbol from Library
const char *SysLibError();				// Gets previous error loading sysbols
void SysCloseLibrary(void *lib);		// Closes Library
void SysUpdate();						// Called on VBlank (to update i.e. pads)
void SysRunGui();						// Returns to the Gui
void SysClose();						// Close mem and plugins

#ifdef __cplusplus
} // extern "C" 
#endif


#endif /* __PSXSYSTEM_H__ */
