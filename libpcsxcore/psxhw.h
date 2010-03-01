/***************************************************************************
 *   Copyright (C) 2007 Ryan Schultz, PCSX-df Team, PCSX team              *
 *   schultz.ryan@gmail.com, http://rschultz.ath.cx/code.php               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02111-1307 USA.            *
 ***************************************************************************/

#ifndef __PSXHW_H__
#define __PSXHW_H__

#include "psxcommon.h"
#include "psxmem.h"

#define HW_DMA0_MADR (psxHu32ref(0x1080)) // MDEC in DMA
#define HW_DMA0_BCR  (psxHu32ref(0x1084))
#define HW_DMA0_CHCR (psxHu32ref(0x1088))

#define HW_DMA1_MADR (psxHu32ref(0x1090)) // MDEC out DMA
#define HW_DMA1_BCR  (psxHu32ref(0x1094))
#define HW_DMA1_CHCR (psxHu32ref(0x1098))

#define HW_DMA2_MADR (psxHu32ref(0x10a0)) // GPU DMA
#define HW_DMA2_BCR  (psxHu32ref(0x10a4))
#define HW_DMA2_CHCR (psxHu32ref(0x10a8))

#define HW_DMA3_MADR (psxHu32ref(0x10b0)) // CDROM DMA
#define HW_DMA3_BCR  (psxHu32ref(0x10b4))
#define HW_DMA3_CHCR (psxHu32ref(0x10b8))

#define HW_DMA4_MADR (psxHu32ref(0x10c0)) // SPU DMA
#define HW_DMA4_BCR  (psxHu32ref(0x10c4))
#define HW_DMA4_CHCR (psxHu32ref(0x10c8))

#define HW_DMA6_MADR (psxHu32ref(0x10e0)) // GPU DMA (OT)
#define HW_DMA6_BCR  (psxHu32ref(0x10e4))
#define HW_DMA6_CHCR (psxHu32ref(0x10e8))

#define HW_DMA_PCR   (psxHu32ref(0x10f0))
#define HW_DMA_ICR   (psxHu32ref(0x10f4))

typedef enum
{
	PsxInt_VBlank		= 0,
	PsxInt_GPU			= 1,
	PsxInt_CDROM		= 2,
	PsxInt_DMA			= 3,
	PsxInt_RTC0			= 4,
	PsxInt_RTC1			= 5,
	PsxInt_RTC2			= 6,
	PsxInt_SIO0			= 7,
	PsxInt_SIO1			= 8,
	PsxInt_SPU			= 9,
	PsxInt_PIO			= 10,
	PsxInt_VBlankEnd	= 11,
	PsxInt_Mcd1			= 12,
	PsxInt_Mcd2			= 13,
	PsxInt_Mcd3			= 14,
} PsxIrq;

void psxRaiseExtInt( PsxIrq irq );
void psxHwReset();
u8   psxHwRead8 (u32 add);
u16  psxHwRead16(u32 add);
u32  psxHwRead32(u32 add);
void psxHwWrite8 (u32 add, u8  value);
void psxHwWrite16(u32 add, u16 value);
void psxHwWrite32(u32 add, u32 value);
int psxHwFreeze(gzFile f, int Mode);

#endif /* __PSXHW_H__ */
