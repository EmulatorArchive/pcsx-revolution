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
* SIO functions.
*/

#include <sys/stat.h>
#include "R3000A/r3000a.h"
#include "psxmem.h"
#include "plugins.h"
#include "psxhw.h"
#include "sio.h"
#include "psxevents.h"

using namespace R3000A;

// *** FOR WORKS ON PADS AND MEMORY CARDS *****

unsigned char cardh[4] = { 0x00, 0x00, 0x5a, 0x5d };

PadDataS pad;

sio_t sio;

char Mcd1Data[MCD_SIZE], Mcd2Data[MCD_SIZE];

// clk cycle byte
// 4us * 8bits = ((PSXCLK / 1000000) * 32) / BIAS; (linuzappz)

// (PSXCLK / SIOCLK) * BIAS = 270; (SIOCLK = 250kHz) (Firnis)
static const u32 sioCycles = (PSXCLK / 250000) * BIAS;
#define SIO_INT() Interrupt.Schedule(PsxEvt_SIO, sioCycles );

void sioInit()
{
	memset(&sio, 0, sizeof(sio));

	// Transfer(?) Ready and the Buffer is Empty
	sio.StatReg = TX_RDY | TX_EMPTY;
	sio.packetsize = 0;
	sio.terminator = 0x55; // Command terminator 'U'

	LoadMcds();
}

unsigned char sioRead8() {
	unsigned char ret = 0;

	if ((sio.StatReg & RX_RDY)/* && (CtrlReg & RX_PERM)*/) {
//		StatReg &= ~RX_OVERRUN;
		ret = sio.buf[sio.parp];
		if (sio.parp == sio.bufcount) {
			sio.StatReg &= ~RX_RDY;		// Receive is not Ready now
			if (sio.mcdst == 5) {
				sio.mcdst = 0;
				if (sio.rdwr == 2) {
					switch (sio.CtrlReg&0x2002) {
						case 0x0002:
							memcpy(Mcd1Data + (sio.adrL | (sio.adrH << 8)) * 128, &sio.buf[1], 128);
							SaveMcd(Config.Mcd1, Mcd1Data, (sio.adrL | (sio.adrH << 8)) * 128, 128);
							break;
						case 0x2002:
							memcpy(Mcd2Data + (sio.adrL | (sio.adrH << 8)) * 128, &sio.buf[1], 128);
							SaveMcd(Config.Mcd2, Mcd2Data, (sio.adrL | (sio.adrH << 8)) * 128, 128);
							break;
					}
				}
			}
			if (sio.padst == 2) sio.padst = 0;
			if (sio.mcdst == 1) {
				sio.mcdst = 2;
				sio.StatReg |= RX_RDY;
			}
		}
	}

#ifdef PAD_LOG
	PAD_LOG("sio read8 ;ret = %x\n", ret);
#endif
	return ret;
}

void netError() {
	ClosePlugins();
	SysMessage(_("Connection closed!\n"));
	SysRunGui();
}

void sioWrite8(unsigned char value) {
#ifdef PAD_LOG
	PAD_LOG("sio write8 %x\n", value);
#endif
	switch (sio.padst) {
		case 1: SIO_INT();
			if ((value&0x40) == 0x40) {
				sio.padst = 2; sio.parp = 1;
				if (!Config.UseNet) {
					switch (sio.CtrlReg&0x2002) {
						case 0x0002:
							sio.buf[sio.parp] = PAD1_poll(value);
							break;
						case 0x2002:
							sio.buf[sio.parp] = PAD2_poll(value);
							break;
					}
				}/* else {
//					SysPrintf("%x: %x, %x, %x, %x\n", CtrlReg&0x2002, buf[2], buf[3], buf[4], buf[5]);
				}*/

				if (!(sio.buf[sio.parp] & 0x0f)) {
					sio.bufcount = 2 + 32;
				} else {
					sio.bufcount = 2 + (sio.buf[sio.parp] & 0x0f) * 2;
				}
				if (sio.buf[sio.parp] == 0x41) {
					switch (value) {
						case 0x43:
							sio.buf[1] = 0x43;
							break;
						case 0x45:
							sio.buf[1] = 0xf3;
							break;
					}
				}
			}
			else sio.padst = 0;
			return;
		case 2:
			sio.parp++;
/*			if (buf[1] == 0x45) {
				buf[parp] = 0;
				SIO_INT();
				return;
			}*/
			if (!Config.UseNet) {
				switch (sio.CtrlReg&0x2002) {
					case 0x0002: sio.buf[sio.parp] = PAD1_poll(value); break;
					case 0x2002: sio.buf[sio.parp] = PAD2_poll(value); break;
				}
			}

			if (sio.parp == sio.bufcount) { sio.padst = 0; return; }
			SIO_INT();
			return;
	}

	switch (sio.mcdst) {
		case 1:
			SIO_INT();
			if (sio.rdwr) { sio.parp++; return; }
			sio.parp = 1;
			switch (value) {
				case 0x52: sio.rdwr = 1; break;
				case 0x57: sio.rdwr = 2; break;
				default: sio.mcdst = 0;
			}
			return;
		case 2: // address H
			SIO_INT();
			sio.adrH = value;
			*sio.buf = 0;
			sio.parp = 0;
			sio.bufcount = 1;
			sio.mcdst = 3;
			return;
		case 3: // address L
			SIO_INT();
			sio.adrL = value;
			*sio.buf = sio.adrH;
			sio.parp = 0;
			sio.bufcount = 1;
			sio.mcdst = 4;
			return;
		case 4:
			SIO_INT();
			sio.parp = 0;
			switch (sio.rdwr) {
				case 1: // read
					sio.buf[0] = 0x5c;
					sio.buf[1] = 0x5d;
					sio.buf[2] = sio.adrH;
					sio.buf[3] = sio.adrL;
					switch (sio.CtrlReg&0x2002) {
						case 0x0002:
							memcpy(&sio.buf[4], Mcd1Data + (sio.adrL | (sio.adrH << 8)) * 128, 128);
							break;
						case 0x2002:
							memcpy(&sio.buf[4], Mcd2Data + (sio.adrL | (sio.adrH << 8)) * 128, 128);
							break;
					}
					{
						char cxor = 0;
						int i;
						for (i = 2; i < 128 + 4; i++)
							cxor ^= sio.buf[i];
						sio.buf[132] = cxor;
					}
					sio.buf[133] = 0x47;
					sio.bufcount = 133;
					break;
				case 2: // write
					sio.buf[0] = sio.adrL;
					sio.buf[1] = value;
					sio.buf[129] = 0x5c;
					sio.buf[130] = 0x5d;
					sio.buf[131] = 0x47;
					sio.bufcount = 131;
					break;
			}
			sio.mcdst = 5;
			return;
		case 5:
			sio.parp++;
			if (sio.rdwr == 2) {
				if (sio.parp < 128) sio.buf[sio.parp+1] = value;
			}
			SIO_INT();
			return;
	}

	switch (value) {
		case 0x01: // start pad
			sio.StatReg |= RX_RDY;		// Transfer is Ready

			if (!Config.UseNet) {
				switch (sio.CtrlReg&0x2002) {
					case 0x0002: sio.buf[0] = PAD1_startPoll(1); break;
					case 0x2002: sio.buf[0] = PAD2_startPoll(2); break;
				}
			} else {
				if ((sio.CtrlReg & 0x2002) == 0x0002) {
					int i, j;

					PAD1_startPoll(1);
					sio.buf[0] = 0;
					sio.buf[1] = PAD1_poll(0x42);
					if (!(sio.buf[1] & 0x0f)) {
						sio.bufcount = 32;
					} else {
						sio.bufcount = (sio.buf[1] & 0x0f) * 2;
					}
					sio.buf[2] = PAD1_poll(0);
					i = 3;
					j = sio.bufcount;
					while (j--) {
						sio.buf[i++] = PAD1_poll(0);
					}
					sio.bufcount+= 3;

					if (NET_sendPadData(sio.buf, sio.bufcount) == -1)
						netError();

					if (NET_recvPadData(sio.buf, 1) == -1)
						netError();
					if (NET_recvPadData(sio.buf+128, 2) == -1)
						netError();
				} else {
					memcpy(sio.buf, sio.buf+128, 32);
				}
			}

			sio.bufcount = 2;
			sio.parp = 0;
			sio.padst = 1;
			SIO_INT();
			return;
		case 0x81: // start memcard
			sio.StatReg |= RX_RDY;
			memcpy(sio.buf, cardh, 4);
			sio.parp = 0;
			sio.bufcount = 3;
			sio.mcdst = 1;
			sio.rdwr = 0;
			SIO_INT();
			return;
	}
}

void sioWriteCtrl16(unsigned short value) {
	sio.CtrlReg = value & ~RESET_ERR;
	if (value & RESET_ERR) sio.StatReg &= ~IRQ;
	if ((sio.CtrlReg & SIO_RESET) || (!sio.CtrlReg)) {
		sio.padst = 0; sio.mcdst = 0; sio.parp = 0;
		sio.StatReg = TX_RDY | TX_EMPTY;
		Interrupt.Cancel(PsxEvt_SIO);
	}
}

void sioInterrupt() {
#ifdef PAD_LOG
	PAD_LOG("Sio Interrupt (CP0.Status = %x)\n", psxRegs.CP0.n.Status);
#endif
//	SysPrintf("Sio Interrupt\n");
	sio.StatReg|= IRQ;
	psxRaiseExtInt( PsxInt_SIO0 );
}

void LoadMcd(int mcd, char *str) {
	FILE *f;
	char *data = NULL;

	if (mcd == 1) data = Mcd1Data;
	if (mcd == 2) data = Mcd2Data;

	if (*str == 0) {
		sprintf(str, "memcards/card%d.mcd", mcd);
		SysPrintf(_("No memory card value was specified - creating a default card %s\n"), str);
	}
	f = fopen(str, "rb");
	if (f == NULL) {
		SysPrintf(_("The memory card %s doesn't exist - creating it\n"), str);
		CreateMcd(str);
		f = fopen(str, "rb");
		if (f != NULL) {
			struct stat buf;

			if (stat(str, &buf) != -1) {
				if (buf.st_size == MCD_SIZE + 64)
					fseek(f, 64, SEEK_SET);
				else if(buf.st_size == MCD_SIZE + 3904)
					fseek(f, 3904, SEEK_SET);
			}
			fread(data, 1, MCD_SIZE, f);
			fclose(f);
		}
		else
			SysMessage(_("Memory card %s failed to load!\n"), str);
	}
	else {
		struct stat buf;
		SysPrintf(_("Loading memory card %s\n"), str);
		if (stat(str, &buf) != -1) {
			if (buf.st_size == MCD_SIZE + 64)
				fseek(f, 64, SEEK_SET);
			else if(buf.st_size == MCD_SIZE + 3904)
				fseek(f, 3904, SEEK_SET);
		}
		fread(data, 1, MCD_SIZE, f);
		fclose(f);
	}
}

void LoadMcds() {
	LoadMcd(1, Config.Mcd1);
	LoadMcd(2, Config.Mcd2);
}

void SaveMcd(char *mcd, char *data, uint32_t adr, int size) {
	FILE *f;

	f = fopen(mcd, "r+b");
	if (f != NULL) {
		struct stat buf;

		if (stat(mcd, &buf) != -1) {
			if (buf.st_size == MCD_SIZE + 64)
				fseek(f, adr + 64, SEEK_SET);
			else if (buf.st_size == MCD_SIZE + 3904)
				fseek(f, adr + 3904, SEEK_SET);
			else
				fseek(f, adr, SEEK_SET);
		} else
			fseek(f, adr, SEEK_SET);

		fwrite(data + adr, 1, size, f);
		fclose(f);
		return;
	}

#if 0
	// try to create it again if we can't open it
	f = fopen(mcd, "wb");
	if (f != NULL) {
		fwrite(data, 1, MCD_SIZE, f);
		fclose(f);
	}
#endif

	ConvertMcd(mcd, data);
}

void CreateMcd(char *mcd) {
	FILE *f;
	struct stat buf;
	int s = MCD_SIZE;
	int i = 0, j;

	f = fopen(mcd, "wb");
	if (f == NULL)
		return;

	if (stat(mcd, &buf)!=-1) {
		if ((buf.st_size == MCD_SIZE + 3904) || strstr(mcd, ".gme")) {
			s = s + 3904;
			fputc('1', f);
			s--;
			fputc('2', f);
			s--;
			fputc('3', f);
			s--;
			fputc('-', f);
			s--;
			fputc('4', f);
			s--;
			fputc('5', f);
			s--;
			fputc('6', f);
			s--;
			fputc('-', f);
			s--;
			fputc('S', f);
			s--;
			fputc('T', f);
			s--;
			fputc('D', f);
			s--;
			for (i = 0; i < 7; i++) {
				fputc(0, f);
				s--;
			}
			fputc(1, f);
			s--;
			fputc(0, f);
			s--;
			fputc(1, f);
			s--;
			fputc('M', f);
			s--;
			fputc('Q', f);
			s--;
			for (i = 0; i < 14; i++) {
				fputc(0xa0, f);
				s--;
			}
			fputc(0, f);
			s--;
			fputc(0xff, f);
			while (s-- > (MCD_SIZE + 1))
				fputc(0, f);
		} else if ((buf.st_size == MCD_SIZE + 64) || strstr(mcd, ".mem") || strstr(mcd, ".vgs")) {
			s = s + 64;
			fputc('V', f);
			s--;
			fputc('g', f);
			s--;
			fputc('s', f);
			s--;
			fputc('M', f);
			s--;
			for (i = 0; i < 3; i++) {
				fputc(1, f);
				s--;
				fputc(0, f);
				s--;
				fputc(0, f);
				s--;
				fputc(0, f);
				s--;
			}
			fputc(0, f);
			s--;
			fputc(2, f);
			while (s-- > (MCD_SIZE + 1))
				fputc(0, f);
		}
	}
	fputc('M', f);
	s--;
	fputc('C', f);
	s--;
	while (s-- > (MCD_SIZE - 127))
		fputc(0, f);
	fputc(0xe, f);
	s--;

	for (i = 0; i < 15; i++) { // 15 blocks
		fputc(0xa0, f);
		s--;
		for (j = 0; j < 126; j++) {
			fputc(0x00, f);
			s--;
		}
		fputc(0xa0, f);
		s--;
	}

	while ((s--) >= 0)
		fputc(0, f);
	fclose(f);
}

void ConvertMcd(char *mcd, char *data) {
	FILE *f;
	int i = 0;
	int s = MCD_SIZE;

	if (strstr(mcd, ".gme")) {
		f = fopen(mcd, "wb");
		if (f != NULL) {
			fwrite(data-3904, 1, MCD_SIZE+3904, f);
			fclose(f);
		}
		f = fopen(mcd, "r+");
		s = s + 3904;
		fputc('1', f); s--;
		fputc('2', f); s--;
		fputc('3', f); s--;
		fputc('-', f); s--;
		fputc('4', f); s--;
		fputc('5', f); s--;
		fputc('6', f); s--;
		fputc('-', f); s--;
		fputc('S', f); s--;
		fputc('T', f); s--;
		fputc('D', f); s--;
		for(i=0;i<7;i++) {
			fputc(0, f); s--;
		}
		fputc(1, f); s--;
		fputc(0, f); s--;
		fputc(1, f); s--;
		fputc('M', f); s--;
		fputc('Q', f); s--;
		for(i=0;i<14;i++) {
			fputc(0xa0, f); s--;
		}
		fputc(0, f); s--;
		fputc(0xff, f);
		while (s-- > (MCD_SIZE+1)) fputc(0, f);
		fclose(f);
	} else if(strstr(mcd, ".mem") || strstr(mcd,".vgs")) {
		f = fopen(mcd, "wb");
		if (f != NULL) {
			fwrite(data-64, 1, MCD_SIZE+64, f);
			fclose(f);
		}
		f = fopen(mcd, "r+");
		s = s + 64;
		fputc('V', f); s--;
		fputc('g', f); s--;
		fputc('s', f); s--;
		fputc('M', f); s--;
		for(i=0;i<3;i++) {
			fputc(1, f); s--;
			fputc(0, f); s--;
			fputc(0, f); s--;
			fputc(0, f); s--;
		}
		fputc(0, f); s--;
		fputc(2, f);
		while (s-- > (MCD_SIZE+1)) fputc(0, f);
		fclose(f);
	} else {
		f = fopen(mcd, "wb");
		if (f != NULL) {
			fwrite(data, 1, MCD_SIZE, f);
			fclose(f);
		}
	}
}

void GetMcdBlockInfo(int mcd, int block, McdBlock *Info) {
	char *ptr;
	char *data = NULL, *str;
	unsigned short clut[16];
	unsigned short c;
	int i, x;

	memset(Info, 0, sizeof(McdBlock));

	str = Info->Title;

	if (mcd == 1) data = Mcd1Data;
	if (mcd == 2) data = Mcd2Data;

	ptr = data + block * 8192 + 2;

	Info->IconCount = *ptr & 0x3;

	ptr+= 2;

	i=0;
	memcpy(Info->sTitle, ptr, 48*2);

	for (i=0; i < 48; i++) {
		c = *(ptr) << 8;
		c|= *(ptr+1);
		if (!c) break;

		if (c >= 0x8281 && c <= 0x8298)
			c = (c - 0x8281) + 'a';
		else if (c >= 0x824F && c <= 0x827A)
			c = (c - 0x824F) + '0';
		else if (c == 0x8144) c = '.';
		else if (c == 0x8146) c = ':';
		else if (c == 0x8168) c = '"';
		else if (c == 0x8169) c = '(';
		else if (c == 0x816A) c = ')';
		else if (c == 0x816D) c = '[';
		else if (c == 0x816E) c = ']';
		else if (c == 0x817C) c = '-';
		else {
			c = ' ';
		}

		str[i] = c;
		ptr+=2;
	}
	str[i] = 0;

	ptr = data + block * 8192 + 0x60; // icon palete data

	for (i = 0; i < 16; i++) {
		clut[i] = *((unsigned short*)ptr);
		ptr += 2;
	}

	for (i = 0; i < Info->IconCount; i++) {
		short *icon = &Info->Icon[i*16*16];

		ptr = data + block * 8192 + 128 + 128 * i; // icon data

		for (x = 0; x < 16 * 16; x++) {
			icon[x++] = clut[*ptr & 0xf];
			icon[x] = clut[*ptr >> 4];
			ptr++;
		}
	}

	ptr = data + block * 128;

	Info->Flags = *ptr;

	ptr += 0xa;
	strncpy(Info->ID, ptr, 12);
	Info->ID[12] = 0;
	ptr += 12;
	strncpy(Info->Name, ptr, 16);
}

int sioFreeze(gzFile f, int Mode) {
	/*char Unused[4096];

	gzfreezel(sio);
	/*gzfreezel(buf);
	gzfreezel(&StatReg);
	gzfreezel(&ModeReg);
	gzfreezel(&CtrlReg);
	gzfreezel(&BaudReg);
	gzfreezel(&bufcount);
	gzfreezel(&parp);
	gzfreezel(&mcdst);
	gzfreezel(&rdwr);
	gzfreezel(&adrH);
	gzfreezel(&adrL);
	gzfreezel(&padst);
	gzfreezel(Unused);*/

	return 0;
}
