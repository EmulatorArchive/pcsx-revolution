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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(__DREAMCAST__)
#define st_size size
//struct stat {
//	uint32 st_size;
//};
#else
#include <sys/stat.h>
#endif

#include "PsxCommon.h"
#include "R3000A.h"
#include "PsxMem.h"
#include "plugins.h"
#include "PSEmu_Plugin_Defs.h"
#include "Sio.h"

#ifdef _MSC_VER_
#pragma warning(disable:4244)
#endif

// *** FOR WORKS ON PADS AND MEMORY CARDS *****

unsigned char cardh[4] = { 0x00, 0x00, 0x5a, 0x5d };

PadDataS pad;

_sio sio;

// clk cycle byte
// 4us * 8bits = ((PSXCLK / 1000000) * 32) / BIAS; (linuzappz)

#define SIO_INT() psx_int_add(PsxEvt_SIO, 540 );

void sioInit()
{
	memset(&sio, 0, sizeof(sio));

	// Transfer(?) Ready and the Buffer is Empty
	sio.StatReg = TX_RDY | TX_EMPTY;
	sio.packetsize = 0;
	sio.terminator = 0x55; // Command terminator 'U'
	
	LoadMcds();
}

void psxSIOShutdown()
{
	//ShutdownMcds();	//FIXME
}

unsigned char sioRead8() {
	unsigned char ret = 0;

	if ((sio.StatReg & RX_RDY)/* && (sio.CtrlReg & RX_PERM)*/) {
//		sio.StatReg &= ~RX_OVERRUN;
		ret = sio.buf[sio.parp];
		if (sio.parp == sio.bufcount) {
			sio.StatReg &= ~RX_RDY;		// Receive is not Ready now
			if (sio.mcdst == 5) {
				sio.mcdst = 0;
				if (sio.rdwr == 2) {
					switch (sio.CtrlReg&0x2002) {
						case 0x0002:
							memcpy(Mcd1Data + (sio.adrL | (sio.adrH << 8)) * 128, &sio.buf[1], 128);
							SaveMcd(0, Mcd1Data, (sio.adrL | (sio.adrH << 8)) * 128, 128);
							break;
						case 0x2002:
							memcpy(Mcd2Data + (sio.adrL | (sio.adrH << 8)) * 128, &sio.buf[1], 128);
							SaveMcd(1, Mcd2Data, (sio.adrL | (sio.adrH << 8)) * 128, 128);
							break;
					}
				}
			}
			if (sio.padst == 2) sio.padst = 0;
			if (sio.mcdst == 1) {
				sio.mcdst = 2;
				sio.StatReg|= RX_RDY;
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
	SysMessage(_("Connection closed\n"));
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
//					SysPrintf("%x: %x, %x, %x, %x\n", sio.CtrlReg&0x2002, sio.buf[2], sio.buf[3], sio.buf[4], sio.buf[5]);
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
/*			if (sio.buf[1] == 0x45) {
				sio.buf[sio.parp] = 0;
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
					char xor = 0;
					int i;
					for (i=2;i<128+4;i++)
						xor^=sio.buf[i];
					sio.buf[132] = xor;
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
		psx_int_remove(PsxEvt_SIO);
	}
}

void sioInterrupt() {
#ifdef PAD_LOG
	PAD_LOG("Sio Interrupt (CP0.Status = %x)\n", psxRegs.CP0.n.Status);
#endif
//	SysPrintf("Sio Interrupt\n");
	sio.StatReg|= IRQ;
	psxHu32ref(0x1070) |= SWAPu32( 0x80 );
}

int sioFreeze(gzFile f, int Mode) {
	char Unused[4096];

	gzfreezel(sio.buf);
	gzfreezel(&sio.StatReg);
	gzfreezel(&sio.ModeReg);
	gzfreezel(&sio.CtrlReg);
	gzfreezel(&sio.BaudReg);
	gzfreezel(&sio.bufcount);
	gzfreezel(&sio.parp);
	gzfreezel(&sio.mcdst);
	gzfreezel(&sio.rdwr);
	gzfreezel(&sio.adrH);
	gzfreezel(&sio.adrL);
	gzfreezel(&sio.padst);
	gzfreezel(Unused);

	return 0;
}
