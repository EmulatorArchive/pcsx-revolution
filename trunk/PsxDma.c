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

#include "PsxCommon.h"
#include "R3000A.h"
#include "PsxHw.h"
#include "PsxMem.h"
#include "PsxDma.h"
#include "PsxCounters.h"

// Dma0/1 in Mdec.c
// Dma3   in CdRom.c

void psxDma2(u32 madr, u32 bcr, u32 chcr) { // GPU
	const u32 size = (bcr >> 16) * (bcr & 0xFFFF);

	switch(chcr) {
		case 0x01000200: // vram2mem
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA2 GPU - vram2mem *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			GPU_readDataMem((u32 *)PSXM(madr), size);
			psxCpu->Clear(madr, size);
			break;

		case 0x01000201: // mem2vram
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA 2 - GPU mem2vram *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			GPU_writeDataMem((u32 *)PSXM(madr), size);
			break;

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
	psx_int_add(PsxEvt_GPU, (size / 4) / BIAS);
}

void psxDma4(u32 madr, u32 bcr, u32 chcr) { // SPU
	const int size = (bcr >> 16) * (bcr & 0xFFFF);

	if (SPU_async)
	{
		SPU_async(psxGetCycle() - psxCounters[4].sCycle);	
		psx_int_add(PsxEvt_SPU, size * 3);
	}

	switch (chcr) {
		case 0x01000201: //cpu to spu transfer
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA4 SPU - mem2spu *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
			SPU_writeDMAMem((u16 *)PSXM(madr), size * 2);
			break;

		case 0x01000200: //spu to cpu transfer
#ifdef PSXDMA_LOG
			PSXDMA_LOG("*** DMA4 SPU - spu2mem *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif
    		SPU_readDMAMem((u16 *)PSXM(madr), size * 2);
			psxCpu->Clear(madr, size);
			break;

#ifdef PSXDMA_LOG
		default:
			PSXDMA_LOG("*** DMA4 SPU - unknown *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
			break;
#endif
	}
}

void gpuInterrupt() {
	HW_DMA2_CHCR &= SWAP32(~0x01000000);
	psxDmaInterrupt(2);
}

void spuInterrupt() {
	HW_DMA4_CHCR &= SWAP32(~0x01000000);
	psxDmaInterrupt(4);
}

void psxDma6(u32 madr, u32 bcr, u32 chcr) {
	u32 *mem = (u32 *)PSXM(madr);

#ifdef PSXDMA_LOG
	PSXDMA_LOG("*** DMA6 OT *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
#endif

	if (chcr == 0x11000002) {
		while (bcr--) {
			*mem-- = SWAP32((madr - 4) & 0xffffff);
			madr -= 4;
		}
		mem++; *mem = 0xffffff;
	}
#ifdef PSXDMA_LOG
	else {
		// Unknown option
		PSXDMA_LOG("*** DMA6 OT - unknown *** %lx addr = %lx size = %lx\n", chcr, madr, bcr);
	}
#endif

	HW_DMA6_CHCR &= SWAP32(~0x01000000);
	psxDmaInterrupt(6);
}
