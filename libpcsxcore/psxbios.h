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

#ifndef __PSXBIOS_H__
#define __PSXBIOS_H__

extern const char *biosA0n[256];
extern const char *biosB0n[256];
extern const char *biosC0n[256];

void psxBiosInit();
void psxBiosShutdown();
void psxBiosException();
void psxBiosFreeze(int Mode);

extern void (*biosA0[256])();
extern void (*biosB0[256])();
extern void (*biosC0[256])();

#endif /* __PSXBIOS_H__ */
