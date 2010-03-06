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

#ifndef __MISC_H__
#define __MISC_H__

#include "psxcommon.h"

#undef s_addr

typedef struct {
	unsigned char id[8];
    u32 text;                   
    u32 data;                    
    u32 pc0;
    u32 gp0;                     
    u32 t_addr;
    u32 t_size;
    u32 d_addr;                  
    u32 d_size;                  
    u32 b_addr;                  
    u32 b_size;                  
    u32 s_addr;
    u32 s_size;
    u32 SavedSP;
    u32 SavedFP;
    u32 SavedGP;
    u32 SavedRA;
    u32 SavedS0;
} EXE_HEADER;

extern char CdromId[10];
extern char CdromLabel[33];

#ifdef __cplusplus
extern "C" {
#endif

int LoadCdrom();
int LoadCdromFile(char *filename, EXE_HEADER *head);
int CheckCdrom();
int Load(char *ExePath);

int SaveState(char *file);
int LoadState(char *file);
int CheckState(char *file);

int SendPcsxInfo();
int RecvPcsxInfo();

void trim(char *str);

#ifdef __cplusplus
} // extern "C" 
#endif

extern char *LabelAuthors;
extern char *LabelGreets;

#endif /* __MISC_H__ */
