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

/*
* Handles PSX DMA functions.
*/

#include "r3000a.h"
#include "psxhw.h"
#include "psxmem.h"
#include "psxdma.h"
#include "plugins.h"
#include "psxhw.h"
#include "psxevents.h"

using namespace R3000A;

// Dma0/1 in Mdec.c
// Dma3   in CdRom.c

#define DMA_UPDATE

// MAME code
void psxDmaUpdate() {
#ifdef DMA_UPDATE
	int n_int = (SWAPu32(HW_DMA_ICR) >> 24) & 0x7f;
	int n_mask = (SWAPu32(HW_DMA_ICR) >> 16) & 0xff;
	
	if( ( n_mask & 0x80 ) != 0 && ( n_int & n_mask ) != 0 )
	{
		HW_DMA_ICR |= SWAPu32(0x80000000);
		psxRaiseExtInt( PsxInt_DMA );
	}
	else if( ( HW_DMA_ICR & SWAPu32(0x80000000) ) != 0 )
	{
		HW_DMA_ICR &= SWAPu32(~0x80000000);
	}

	HW_DMA_ICR &= SWAPu32(0x00ffffff | ( SWAPu32(HW_DMA_ICR) << 8 ));
#endif
}

void psxDmaInterrupt(u32 channel) {
#ifndef DMA_UPDATE
	if (SWAPu32(HW_DMA_ICR) & (1 << (16 + channel))) 
#endif
	{
		
		HW_DMA_ICR|= SWAP32(1 << (24 + channel));
		psxRegs.CP0.n.Cause |= 1 << (9 + channel);
#ifdef DMA_UPDATE
		psxDmaUpdate();
#else
		psxRaiseExtInt( PsxInt_DMA );
#endif
		
	}
}

void psxDma4(u32 madr, u32 bcr, u32 chcr) { // SPU
	u16 *ptr;

	u32 size = (bcr >> 16);
	if(size == 0) 
		size = 0x10000;
	 size *= (bcr & 0xFFFF);
	 size *= 2;

	switch (chcr) {
		case 0x01000201: //cpu to spu transfer
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA4 SPU - mem2spu *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			ptr = (u16 *)PSXM(madr);
			if (ptr == NULL) {
#ifdef CPU_LOG
				CPU_LOG("*** DMA4 SPU - mem2spu *** NULL Pointer!!!\n");
#endif
				break;
			}
			SPU_writeDMAMem(ptr, size);
			break;

		case 0x01000200: //spu to cpu transfer
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA4 SPU - spu2mem *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			ptr = (u16 *)PSXM(madr);
			if (ptr == NULL) {
#ifdef CPU_LOG
				CPU_LOG("*** DMA4 SPU - spu2mem *** NULL Pointer!!!\n");
#endif
				break;
			}

    		SPU_readDMAMem(ptr, size);
			psxCpu->Clear(madr, size);
			break;

#ifdef PSXDMA_LOG
		default:
			PSXDMA_LOG("*** DMA4 SPU - unknown *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
			break;
#endif
	}

	HW_DMA4_CHCR &= SWAP32(~0x11000000);
	psxDmaInterrupt(4);
}

void psxDma2(u32 madr, u32 bcr, u32 chcr) { // GPU
	u32 *ptr;
	u32 size = (bcr >> 16);
	if(size == 0)
		size = 0x10000;
	 size *= (bcr & 0xFFFF);

	switch(chcr) {
		case 0x01000200: // vram2mem
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA2 GPU - vram2mem *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			ptr = (u32 *)PSXM(madr);
			if (ptr == NULL) {
#ifdef CPU_LOG
				CPU_LOG("*** DMA2 GPU - vram2mem *** NULL Pointer!!!\n");
#endif
				break;
			}
			GPU_readDataMem(ptr, size);
			psxCpu->Clear(madr, size);
			break;

		case 0x01000201: // mem2vram
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA 2 - GPU mem2vram *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			ptr = (u32 *)PSXM(madr);
			if (ptr == NULL) {
#ifdef CPU_LOG
				CPU_LOG("*** DMA2 GPU - mem2vram *** NULL Pointer!!!\n");
#endif
				break;
			}
			GPU_writeDataMem(ptr, size);
			Interrupt.Schedule(PsxEvt_GPU, (size / 4) / BIAS);
			return;

		case 0x01000401: // dma chain
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA 2 - GPU dma chain *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			GPU_dmaChain((u32 *)psxM, madr & 0x1fffff);
			break;

#ifdef PSXDMA_LOG
		default:
			PSXDMA_LOG("*** DMA 2 - GPU unknown *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
			break;
#endif
	}

	gpuInterrupt();
}

void gpuInterrupt() {
	HW_DMA2_CHCR &= SWAP32(~0x11000000);
	psxDmaInterrupt(2);
}

void otcInterrupt() {
	HW_DMA6_CHCR &= SWAP32(~0x11000000);
	psxDmaInterrupt(6);
}

void psxDma6(u32 madr, u32 bcr, u32 chcr) {
	u32 *mem = (u32 *)PSXM(madr);

#ifdef PSXDMA_LOG
	PSXDMA_LOG("*** DMA6 OT *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif

	if (chcr == 0x11000002) {
		if (mem == NULL) {
#ifdef CPU_LOG
			CPU_LOG("*** DMA6 OT *** NULL Pointer!!!\n");
#endif
			otcInterrupt();
			return;
		}
		while (bcr--) {
			madr -= 4;
			*mem-- = SWAP32(madr & 0xffffff);
		}
		mem++; *mem = 0xffffff;
	}
#ifdef PSXDMA_LOG
	else {
		// Unknown option
		PSXDMA_LOG("*** DMA6 OT - unknown *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
	}
#endif
	Interrupt.Schedule(PsxEvt_OTC, 2150);		// cnt1?
}
