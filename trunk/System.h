/*  Pcsx - Pc Psx Emulator
 *  Copyright (C) 1999-2003  Pcsx Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

extern int  SysInit();							// Init mem and plugins
extern void SysReset();						// Resets mem
extern void SysPrintf(char *fmt, ...);			// Printf used by bios syscalls
extern void SysMessage(char *fmt, ...);		// Message used to print msg to users
extern void *SysLoadLibrary(char *lib);		// Loads Library
extern void *SysLoadSym(void *lib, char *sym);	// Loads Symbol from Library
extern const char *SysLibError();				// Gets previous error loading sysbols
extern void SysCloseLibrary(void *lib);		// Closes Library
extern void SysUpdate();						// Called on VBlank (to update i.e. pads)
extern void SysRunGui();						// Returns to the Gui
extern void SysClose();						// Close mem and plugins

#endif /* __SYSTEM_H__ */
