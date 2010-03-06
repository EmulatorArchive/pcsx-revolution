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

#ifndef CONFIG_H
#define CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

int LoadConfig();

#ifdef __cplusplus
} // extern "C" 
#endif

void SaveConfig();

void SPUWriteConfig();
void GPUWriteConfig();
void PADWriteConfig();

void GPUReadConfig();
void SPUReadConfig();
int SMBReadConfig();

typedef struct {
	int ShowFPS;			// 0-1
	int FrameSkip;			// 0-1
	int LimitFPS;			// 0-1
	int Dithering;			// 0-2
} GPU_t;

typedef struct {
	int Volume;				// 1-4
	int Reverb;				// 0-2
	int Interpolation;		// 0-3
	int EnableXA;			// 0-1
	int XAPitch;			// 0-1
	int CompatMode;			// 0, 2
	int SPU_IRQ_Wait;		// 0-1
	int SingleChMode;		// 0-1
	int Disable;			// 0-1
} SPU_t;

typedef struct {
	char	ip[16];
	char	user[20];
	char	pwd[20];
	char	share[20];
} SMB_t;

typedef struct {
	char filename[255];
	int device;
	GPU_t GPU;
	SPU_t SPU;
	SMB_t smb;
} settings;

extern settings Settings;

#endif
