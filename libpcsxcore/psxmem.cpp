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
* PSX memory functions.
*/

#include "psxmem.h"
#include "r3000a.h"
#include "psxhw.h"

#ifdef HW_RVL
#include "../Gamecube/MEM2.h"
#endif

s8 *psxM;
s8 *psxP;
s8 *psxR;
s8 *psxH;
u8** psxMemWLUT;
u8** psxMemRLUT;

/*  Playstation Memory Map (from Playstation doc by Joshua Walker)
0x0000_0000-0x0000_ffff		Kernel (64K)	
0x0001_0000-0x001f_ffff		User Memory (1.9 Meg)	

0x1f00_0000-0x1f00_ffff		Parallel Port (64K)	

0x1f80_0000-0x1f80_03ff		Scratch Pad (1024 bytes)	

0x1f80_1000-0x1f80_2fff		Hardware Registers (8K)	

0x8000_0000-0x801f_ffff		Kernel and User Memory Mirror (2 Meg) Cached	

0xa000_0000-0xa01f_ffff		Kernel and User Memory Mirror (2 Meg) Uncached	

0xbfc0_0000-0xbfc7_ffff		BIOS (512K)
*/

int psxMemInit() {
	int i;

	psxMemRLUT = (u8**)malloc(0x10000 * sizeof(void*));
	psxMemWLUT = (u8**)malloc(0x10000 * sizeof(void*));
	memset(psxMemRLUT, 0, 0x10000 * sizeof(void*));
	memset(psxMemWLUT, 0, 0x10000 * sizeof(void*));
	psxM = (s8 *) malloc(0x00220000);
	
	psxP = &psxM[0x200000];
	psxH = &psxM[0x210000];
#ifdef HW_RVL
	psxR = (s8*)BIOS_LO;
#else
	psxR = (s8*)malloc(0x00080000);
#endif

	if (psxMemRLUT == NULL || psxMemWLUT == NULL || 
		psxM == NULL || psxP == NULL || psxH == NULL) {
		SysMessage(_("Error allocating memory!"));
		return -1;
	}

// MemR
	for (i=0; i<0x80; i++) psxMemRLUT[i + 0x0000] = (u8*)&psxM[(i & 0x1f) << 16];

	memcpy(psxMemRLUT + 0x8000, psxMemRLUT, 0x80 * sizeof(void*));
	memcpy(psxMemRLUT + 0xa000, psxMemRLUT, 0x80 * sizeof(void*));

	for (i=0; i<0x01; i++) psxMemRLUT[i + 0x1f00] = (u8*)&psxP[i << 16];

	for (i=0; i<0x01; i++) psxMemRLUT[i + 0x1f80] = (u8*)&psxH[i << 16];

	for (i=0; i<0x08; i++) psxMemRLUT[i + 0xbfc0] = (u8*)&psxR[i << 16];

// MemW
	for (i=0; i<0x80; i++) psxMemWLUT[i + 0x0000] = (u8*)&psxM[(i & 0x1f) << 16];
	memcpy(psxMemWLUT + 0x8000, psxMemWLUT, 0x80 * sizeof(void*));
	memcpy(psxMemWLUT + 0xa000, psxMemWLUT, 0x80 * sizeof(void*));

	for (i=0; i<0x01; i++) psxMemWLUT[i + 0x1f00] = (u8*)&psxP[i << 16];

	for (i=0; i<0x01; i++) psxMemWLUT[i + 0x1f80] = (u8*)&psxH[i << 16];

	return 0;
}

void psxMemReset() {
	FILE *f = NULL;
	char bios[1024];

	memset(psxM, 0, 0x00200000);
	memset(psxP, 0, 0x00010000);

	if (strcmp(Config.Bios, "HLE")) {
		sprintf(bios, "%s/%s", Config.BiosDir, Config.Bios);
		f = fopen(bios, "rb");

		if (f == NULL) {
			SysMessage (_("Could not open BIOS:\"%s\". Enabling HLE Bios!\n"), bios);
			memset(psxR, 0, 0x80000);
			Config.HLE = BIOS_HLE;
		}
		else {
			fread(psxR, 1, 0x80000, f);
			fclose(f);
			Config.HLE = BIOS_USER_DEFINED;
		}
	} else Config.HLE = BIOS_HLE;
}

void psxMemShutdown() {
#ifndef HW_RVL
	free(psxR);
#endif
	free(psxM);

	free(psxMemRLUT);
	free(psxMemWLUT);
}

static int writeok = 1;

u8 psxMemRead8(u32 mem) {
	char *p;
	u32 t;

	t = mem >> 16;
	if (t == 0x1f80) {
		if (mem < 0x1f801000)
			return psxHu8(mem);
		else
			return psxHwRead8(mem);
	} else {
		p = (char *)(psxMemRLUT[t]);
		if (p != NULL) {
#ifndef GEKKO
			if (Config.Debug)
				DebugCheckBP((mem & 0xffffff) | 0x80000000, R1);
#endif
			return *(u8 *)(p + (mem & 0xffff));
		} else {
#ifdef PSXMEM_LOG
			PSXMEM_LOG("err lb %8.8lx\n", mem);
#endif
			return 0;
		}
	}
}

u16 psxMemRead16(u32 mem) {
	char *p;
	u32 t;

	t = mem >> 16;
	if (t == 0x1f80) {
		if (mem < 0x1f801000)
			return psxHu16(mem);
		else
			return psxHwRead16(mem);
	} else {
		p = (char *)(psxMemRLUT[t]);
		if (p != NULL) {
#ifndef GEKKO
			if (Config.Debug)
				DebugCheckBP((mem & 0xffffff) | 0x80000000, R2);
#endif
			return GETLE16((u16 *)(p + (mem & 0xffff)));
		} else {
#ifdef PSXMEM_LOG
			PSXMEM_LOG("err lh %8.8lx\n", mem);
#endif
			return 0;
		}
	}
}

u32 psxMemRead32(u32 mem) {
	char *p;
	u32 t;

	t = mem >> 16;
	if (t == 0x1f80) {
		if (mem < 0x1f801000)
			return psxHu32(mem);
		else
			return psxHwRead32(mem);
	} else {
		p = (char *)(psxMemRLUT[t]);
		if (p != NULL) {
#ifndef GEKKO
			if (Config.Debug)
				DebugCheckBP((mem & 0xffffff) | 0x80000000, R4);
#endif
			return GETLE32((u32 *)(p + (mem & 0xffff)));
		} else {
#ifdef PSXMEM_LOG
			if (writeok) { PSXMEM_LOG("err lw %8.8lx\n", mem); }
#endif
			return 0;
		}
	}
}

void psxMemWrite8(u32 mem, u8 value) {
	char *p;
	u32 t;

	t = mem >> 16;
	if (t == 0x1f80) {
		if (mem < 0x1f801000)
			psxHu8(mem) = value;
		else
			psxHwWrite8(mem, value);
	} else {
		p = (char *)(psxMemWLUT[t]);
		if (p != NULL) {
#ifndef GEKKO
			if (Config.Debug)
				DebugCheckBP((mem & 0xffffff) | 0x80000000, W1);
#endif
			*(u8 *)(p + (mem & 0xffff)) = value;
#ifdef PSXREC
			psxCpu->Clear((mem & (~3)), 1);
#endif
		} else {
#ifdef PSXMEM_LOG
			PSXMEM_LOG("err sb %8.8lx\n", mem);
#endif
		}
	}
}

void psxMemWrite16(u32 mem, u16 value) {
	char *p;
	u32 t;

	t = mem >> 16;
	if (t == 0x1f80) {
		if (mem < 0x1f801000)
			psxHu16ref(mem) = SWAPu16(value);
		else
			psxHwWrite16(mem, value);
	} else {
		p = (char *)(psxMemWLUT[t]);
		if (p != NULL) {
#ifndef GEKKO
			if (Config.Debug)
				DebugCheckBP((mem & 0xffffff) | 0x80000000, W2);
#endif
			PUTLE16((u16 *)(p + (mem & 0xffff)), value);
#ifdef PSXREC
			psxCpu->Clear((mem & (~1)), 1);
#endif
		} else {
#ifdef PSXMEM_LOG
			PSXMEM_LOG("err sh %8.8lx\n", mem);
#endif
		}
	}
}

void psxMemWrite32(u32 mem, u32 value) {
	char *p;
	u32 t;

//	if ((mem&0x1fffff) == 0x71E18 || value == 0x48088800) SysPrintf("t2fix!!\n");
	t = mem >> 16;
	if (t == 0x1f80) {
		if (mem < 0x1f801000)
			psxHu32ref(mem) = SWAPu32(value);
		else
			psxHwWrite32(mem, value);
	} else {
		p = (char *)(psxMemWLUT[t]);
		if (p != NULL) {
#ifndef GEKKO
			if (Config.Debug)
				DebugCheckBP((mem & 0xffffff) | 0x80000000, W4);
#endif
			PUTLE32((u32 *)(p + (mem & 0xffff)), value);
#ifdef PSXREC
			psxCpu->Clear(mem, 1);
#endif
		} else {
			if (mem != 0xfffe0130) {
#ifdef PSXREC
				if (!writeok)
					psxCpu->Clear(mem, 1);
#endif

#ifdef PSXMEM_LOG
				if (writeok) { PSXMEM_LOG("err sw %8.8lx\n", mem); }
#endif
			} else {
				int i;

				switch (value) {
					case 0x800: case 0x804:
						if (writeok == 0) break;
						writeok = 0;
						memset(psxMemWLUT + 0x0000, 0, 0x80 * sizeof(void*));
						memset(psxMemWLUT + 0x8000, 0, 0x80 * sizeof(void*));
						memset(psxMemWLUT + 0xa000, 0, 0x80 * sizeof(void*));
						break;
					case 0x1e988:
						if (writeok == 1) break;
						writeok = 1;
						for (i=0; i<0x80; i++) psxMemWLUT[i + 0x0000] = (u8 *)&psxM[(i & 0x1f) << 16];
						memcpy(psxMemWLUT + 0x8000, psxMemWLUT, 0x80 * sizeof(void*));
						memcpy(psxMemWLUT + 0xa000, psxMemWLUT, 0x80 * sizeof(void*));
						break;
					default:
#ifdef PSXMEM_LOG
						PSXMEM_LOG("unk %8.8lx = %x\n", mem, value);
#endif
						break;
				}
			}
		}
	}
}

void *psxMemPointer(u32 mem) {
	char *p;
	u32 t;

	t = mem >> 16;
	if (t == 0x1f80) {
		if (mem < 0x1f801000)
			return (void *)&psxH[mem];
		else
			return NULL;
	} else {
		p = (char *)(psxMemWLUT[t]);
		if (p != NULL) {
			return (void *)(p + (mem & 0xffff));
		}
		return NULL;
	}
}
