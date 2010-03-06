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

#ifndef __PSXDMA_H__
#define __PSXDMA_H__

#include "psxcommon.h"

#ifdef __cplusplus
extern "C" {
#endif

void psxDmaInterrupt(u32 channel);
void psxDma3(u32 madr, u32 bcr, u32 chcr);

#ifdef __cplusplus
} // extern "C" 
#endif

void psxDmaUpdate();
void psxDma2(u32 madr, u32 bcr, u32 chcr);
void psxDma4(u32 madr, u32 bcr, u32 chcr);
void psxDma6(u32 madr, u32 bcr, u32 chcr);
void gpuInterrupt();
void otcInterrupt();

#endif /* __PSXDMA_H__ */
