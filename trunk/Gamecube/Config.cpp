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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../libpcsxcore/psxcommon.h"
#include "Config.h"
#include "PAD/pad.h"

/////////////////////////////////////////////////////////

#define GetValue(name, var) \
	tmp = strstr(data, name); \
	if (tmp != NULL) { \
		tmp+=strlen(name); \
		while ((*tmp == ' ') || (*tmp == '=')) tmp++; \
		if (*tmp != '\n') sscanf(tmp, "%s", var); \
	}

#define GetValuel(name, var) \
	tmp = strstr(data, name); \
	if (tmp != NULL) { \
		tmp+=strlen(name); \
		while ((*tmp == ' ') || (*tmp == '=')) tmp++; \
		if (*tmp != '\n') sscanf(tmp, "%x", &var); \
	}
	
#define GetValueld(name, var) \
	tmp = strstr(data, name); \
	if (tmp != NULL) { \
		tmp+=strlen(name); \
		while ((*tmp == ' ') || (*tmp == '=')) tmp++; \
		if (*tmp != '\n') sscanf(tmp, "%ld", &var); \
	}

#define SetValue(name, var) \
	fprintf (f,"%s = %s\n", name, var);

#define SetValuel(name, var) \
	fprintf (f,"%s = %x\n", name, var);
	
#define SetValueld(name, var) \
	fprintf (f,"%s = %ld\n", name, var);

/////////////////////////////////////////////////////////

static void PADReadConfig();

settings Settings;

PcsxConfig Config;

/////////////////////////////////////////////////////////

int LoadConfig()
{
	struct stat buf;
	FILE *f;
	int size;
	char *data, *tmp;

	char path[255];
#ifdef HW_RVL
	strcpy(path, "sd:/pcsx-r/config.ini");
#else
	strcpy(path, "carda:/pcsx-r/config.ini");
#endif
	if (stat(path, &buf) == -1)
	{
#ifdef HW_RVL
		strcpy(path, "usb:/pcsx-r/config.ini");
#else
		strcpy(path, "cardb:/pcsx-r/config.ini");
#endif
		if (stat(path, &buf) == -1)
			return -1;
	}
	size = buf.st_size;

	f = fopen(path, "r");
	if (f == NULL) return -1;

	data = (char*)malloc(size);
	if (data == NULL) return -1;
	rewind(f);
	fread(data, 1, size, f);
	fclose(f);
	
	GetValue("Net", Config.Net);
	GetValue("Mcd1", Config.Mcd1);
	GetValue("Mcd2", Config.Mcd2);
	GetValue("Bios", Config.Bios);
	GetValue("BiosDir", Config.BiosDir);
	GetValueld("Xa", Config.Xa);
	GetValueld("Sio", Config.Sio);
	GetValueld("Mdec", Config.Mdec);
	GetValueld("PsxAuto", Config.PsxAuto);
	GetValueld("PsxType", Config.PsxType);
	GetValueld("Cdda", Config.Cdda);
	GetValueld("HLE", Config.HLE);
	GetValueld("Cpu", Config.Cpu);
	GetValueld("PsxOut", Config.PsxOut);
	GetValueld("SpuIrq", Config.SpuIrq);
	GetValueld("RCntFix", Config.RCntFix);
	GetValueld("UseNet", Config.UseNet);
	GetValueld("VSyncWA", Config.VSyncWA);
	
	GetValuel("LastDevice", Settings.device);

	pads[0].num = 0;
	pads[1].num = 1;

	free(data);

	PADReadConfig();

	return 0;
}

/////////////////////////////////////////////////////////

void SaveConfig() {
	FILE *f;

	char path[255];
#ifdef HW_RVL
	strcpy(path, "sd:/pcsx-r/config.ini");
#else
	strcpy(path, "carda:/pcsx-r/config.ini");
#endif

	f = fopen(path, "w");
	if (f == NULL)
	{
#ifdef HW_RVL
		strcpy(path, "usb:/pcsx-r/config.ini");
#else
		strcpy(path, "cardb:/pcsx-r/config.ini");
#endif
		f = fopen(path, "w");
		if (f == NULL)
			return;
	}

	SetValue("Net", Config.Net);
	SetValue("Mcd1", Config.Mcd1);
	SetValue("Mcd2", Config.Mcd2);
	SetValue("Bios", Config.Bios);
	SetValue("BiosDir", Config.BiosDir);

	SetValueld("Xa", Config.Xa);
	SetValueld("Sio", Config.Sio);
	SetValueld("Mdec", Config.Mdec);
	SetValueld("PsxAuto", Config.PsxAuto);
	SetValueld("PsxType", Config.PsxType);
	SetValueld("Cdda", Config.Cdda);
	SetValueld("HLE", Config.HLE);
	SetValueld("Cpu", Config.Cpu);
	SetValueld("PsxOut", Config.PsxOut);
	SetValueld("SpuIrq", Config.SpuIrq);
	SetValueld("RCntFix", Config.RCntFix);
	SetValueld("UseNet", Config.UseNet);
	SetValueld("VSyncWA", Config.VSyncWA);

	SetValuel("LastDevice", Settings.device);

	fclose(f);

	return;
}

/////////////////////////////////////////////////////////

void GPUReadConfig()
{
	struct stat buf;
	FILE *f;
	int size;
	char *data, *tmp;

	Settings.GPU.Dithering = 0;
	Settings.GPU.ShowFPS = 0;
	Settings.GPU.LimitFPS = 0;
	Settings.GPU.FrameSkip = 0;

	char path[255];
	
#ifdef HW_RVL
		strcpy(path, "sd:/pcsx-r/video.ini");
#else
		strcpy(path, "carda:/pcsx-r/video.ini");
#endif

	if (stat(path, &buf) == -1)
	{
#ifdef HW_RVL
		strcpy(path, "usb:/pcsx-r/video.ini");
#else
		strcpy(path, "cardb:/pcsx-r/video.ini");
#endif

		if (stat(path, &buf) == -1)
			return;
	}
	size = buf.st_size;

	f = fopen(path, "r");
	if (f == NULL) return;

	data = (char*)malloc(size);
	if (data == NULL) return;
	rewind(f);
	fread(data, 1, size, f);
	fclose(f);

	GetValuel("Dithering", Settings.GPU.Dithering);

	GetValuel("ShowFPS", Settings.GPU.ShowFPS);
	if(Settings.GPU.ShowFPS < 0)
		Settings.GPU.ShowFPS = 0;
	if(Settings.GPU.ShowFPS > 1)
		Settings.GPU.ShowFPS = 1;

	GetValuel("UseFrameLimit", Settings.GPU.LimitFPS);
	if(Settings.GPU.LimitFPS < 0)
		Settings.GPU.LimitFPS = 0;
	if(Settings.GPU.LimitFPS > 1)
		Settings.GPU.LimitFPS = 1;

	GetValuel("UseFrameSkip", Settings.GPU.FrameSkip);
	if(Settings.GPU.FrameSkip < 0)
		Settings.GPU.FrameSkip = 0;
	if(Settings.GPU.FrameSkip > 1)
		Settings.GPU.FrameSkip = 1;

	free(data);
}

/////////////////////////////////////////////////////////

void GPUWriteConfig()
{
	FILE *f;

	char path[255];
#ifdef HW_RVL
		strcpy(path, "sd:/pcsx-r/video.ini");
#else
		strcpy(path, "carda:/pcsx-r/video.ini");
#endif

	f = fopen(path, "w");
	if (f == NULL)
	{
#ifdef HW_RVL
		strcpy(path, "usb:/pcsx-r/video.ini");
#else
		strcpy(path, "cardb:/pcsx-r/video.ini");
#endif
		f = fopen(path, "w");
		if (f == NULL)
			return;
	}

	SetValuel("Dithering", Settings.GPU.Dithering);
	SetValuel("ShowFPS", Settings.GPU.ShowFPS);
	SetValuel("UseFrameLimit", Settings.GPU.LimitFPS);
	SetValuel("UseFrameSkip", Settings.GPU.FrameSkip);

	fclose(f);
}

/////////////////////////////////////////////////////////

void PADWriteConfig() {
	FILE *f;

	char path[255];
#ifdef HW_RVL
		strcpy(path, "sd:/pcsx-r/pad.ini");
#else
		strcpy(path, "carda:/pcsx-r/pad.ini");
#endif

	f = fopen(path, "w");
	if (f == NULL)
	{
#ifdef HW_RVL
		strcpy(path, "usb:/pcsx-r/pad.ini");
#else
		strcpy(path, "cardb:/pcsx-r/pad.ini");
#endif
		f = fopen(path, "w");
		if (f == NULL)
			return;
	}

	SetValuel("UP0", pads[0].UP);
	SetValuel("DOWN0", pads[0].DOWN);
	SetValuel("LEFT0", pads[0].LEFT);
	SetValuel("RIGHT0", pads[0].RIGHT);
	SetValuel("START0", pads[0].START);
	SetValuel("SELECT0", pads[0].SELECT);
	SetValuel("MENU0", pads[0].MENU);
	SetValuel("SQUARE0", pads[0].SQUARE);
	SetValuel("CROSS0", pads[0].CROSS);
	SetValuel("CIRCLE0", pads[0].CIRCLE);
	SetValuel("TRIANGLE0", pads[0].TRIANGLE);
	SetValuel("L10", pads[0].L1);
	SetValuel("L20", pads[0].L2);
	SetValuel("R10", pads[0].R1);
	SetValuel("R20", pads[0].R2);
	SetValuel("type0", pads[0].type);
	SetValuel("analog0", pads[0].analog);

	SetValuel("UP1", pads[1].UP);
	SetValuel("DOWN1", pads[1].DOWN);
	SetValuel("LEFT1", pads[1].LEFT);
	SetValuel("RIGHT1", pads[1].RIGHT);
	SetValuel("START1", pads[1].START);
	SetValuel("SELECT1", pads[1].SELECT);
	SetValuel("MENU1", pads[1].MENU);
	SetValuel("SQUARE1", pads[1].SQUARE);
	SetValuel("CROSS1", pads[1].CROSS);
	SetValuel("CIRCLE1", pads[1].CIRCLE);
	SetValuel("TRIANGLE1", pads[1].TRIANGLE);
	SetValuel("L11", pads[1].L1);
	SetValuel("L21", pads[1].L2);
	SetValuel("R11", pads[1].R1);
	SetValuel("R21", pads[1].R2);
	SetValuel("type1", pads[1].type);
	SetValuel("analog1", pads[1].analog);

	fclose(f);

	return;
}

/////////////////////////////////////////////////////////

static void PADReadConfig() {
	struct stat buf;
	FILE *f;
	int size;
	char *data, *tmp;

	char path[255];
#ifdef HW_RVL
		strcpy(path, "sd:/pcsx-r/pad.ini");
#else
		strcpy(path, "carda:/pcsx-r/pad.ini");
#endif

	if (stat(path, &buf) == -1)
	{
#ifdef HW_RVL
		strcpy(path, "usb:/pcsx-r/pad.ini");
#else
		strcpy(path, "cardb:/pcsx-r/pad.ini");
#endif
		if (stat(path, &buf) == -1)
			return;
	}
	size = buf.st_size;

	f = fopen(path, "r");
	if (f == NULL) return;

	data = (char*)malloc(size);
	if (data == NULL) return;
	rewind(f);
	fread(data, 1, size, f);
	fclose(f);

	GetValuel("UP0", pads[0].UP);
	GetValuel("DOWN0", pads[0].DOWN);
	GetValuel("LEFT0", pads[0].LEFT);
	GetValuel("RIGHT0", pads[0].RIGHT);
	GetValuel("START0", pads[0].START);
	GetValuel("SELECT0", pads[0].SELECT);
	GetValuel("MENU0", pads[0].MENU);
	GetValuel("SQUARE0", pads[0].SQUARE);
	GetValuel("CROSS0", pads[0].CROSS);
	GetValuel("CIRCLE0", pads[0].CIRCLE);
	GetValuel("TRIANGLE0", pads[0].TRIANGLE);
	GetValuel("L10", pads[0].L1);
	GetValuel("L20", pads[0].L2);
	GetValuel("R10", pads[0].R1);
	GetValuel("R20", pads[0].R2);
	GetValuel("type0", pads[0].type);
	GetValuel("analog0", pads[0].analog);

	GetValuel("UP1", pads[1].UP);
	GetValuel("DOWN1", pads[1].DOWN);
	GetValuel("LEFT1", pads[1].LEFT);
	GetValuel("RIGHT1", pads[1].RIGHT);
	GetValuel("START1", pads[1].START);
	GetValuel("SELECT1", pads[1].SELECT);
	GetValuel("MENU1", pads[1].MENU);
	GetValuel("SQUARE1", pads[1].SQUARE);
	GetValuel("CROSS1", pads[1].CROSS);
	GetValuel("CIRCLE1", pads[1].CIRCLE);
	GetValuel("TRIANGLE1", pads[1].TRIANGLE);
	GetValuel("L11", pads[1].L1);
	GetValuel("L21", pads[1].L2);
	GetValuel("R11", pads[1].R1);
	GetValuel("R21", pads[1].R2);
	GetValuel("type1", pads[1].type);
	GetValuel("analog1", pads[1].analog);
	free(data);

	return;
}

/////////////////////////////////////////////////////////

void SPUReadConfig() {
	struct stat buf;
	FILE *f;
	int size;
	char *data, *tmp;

	Settings.SPU.Volume = 3;
	Settings.SPU.Reverb = 2;
	Settings.SPU.Interpolation = 2;
	Settings.SPU.EnableXA = 1;
	Settings.SPU.XAPitch = 1;
	Settings.SPU.CompatMode = 2;
	Settings.SPU.SPU_IRQ_Wait = 0;
	Settings.SPU.SingleChMode = 0;
	Settings.SPU.Disable = 0;

	char path[255];
#ifdef HW_RVL
		strcpy(path, "sd:/pcsx-r/audio.ini");
#else
		strcpy(path, "carda:/pcsx-r/audio.ini");
#endif

	if (stat(path, &buf) == -1)
	{
#ifdef HW_RVL
		strcpy(path, "usb:/pcsx-r/audio.ini");
#else
		strcpy(path, "cardb:/pcsx-r/audio.ini");
#endif
		if (stat(path, &buf) == -1)
			return;
	}
	size = buf.st_size;

	f = fopen(path, "r");
	if (f == NULL) return;

	data = (char*)malloc(size);
	if (data == NULL) return;
	rewind(f);
	fread(data, 1, size, f);
	fclose(f);

	GetValuel("Volume", Settings.SPU.Volume);
	GetValuel("UseReverb", Settings.SPU.Reverb);
	GetValuel("UseInterpolation", Settings.SPU.Interpolation);
	GetValuel("UseXA", Settings.SPU.EnableXA );
	GetValuel("XAPitch", Settings.SPU.XAPitch );
	GetValuel("HighCompMode", Settings.SPU.CompatMode );
	GetValuel("SPUIRQWait", Settings.SPU.SPU_IRQ_Wait );
	GetValuel("DisStereo", Settings.SPU.SingleChMode );
	GetValuel("Disable", Settings.SPU.Disable );

	free(data);

	return;
}

/////////////////////////////////////////////////////////

void SPUWriteConfig() {
	FILE *f;

	char path[255];
#ifdef HW_RVL
	strcpy(path, "sd:/pcsx-r/audio.ini");
#else
	strcpy(path, "carda:/pcsx-r/audio.ini");
#endif

	f = fopen(path, "w");
	if (f == NULL)
	{
#ifdef HW_RVL
		strcpy(path, "usb:/pcsx-r/audio.ini");
#else
		strcpy(path, "cardb:/pcsx-r/audio.ini");
#endif

		f = fopen(path, "w");
		if (f == NULL)
			return;
	}

	SetValuel("Volume", Settings.SPU.Volume);
	SetValuel("UseReverb", Settings.SPU.Reverb);
	SetValuel("UseInterpolation", Settings.SPU.Interpolation);
	SetValuel("UseXA", Settings.SPU.EnableXA );
	SetValuel("XAPitch", Settings.SPU.XAPitch );
	SetValuel("HighCompMode", Settings.SPU.CompatMode );
	SetValuel("SPUIRQWait", Settings.SPU.SPU_IRQ_Wait );
	SetValuel("DisStereo", Settings.SPU.SingleChMode );
	SetValuel("Disable", Settings.SPU.Disable );

	fclose(f);

	return;
}

/////////////////////////////////////////////////////////

int SMBReadConfig()
{
	struct stat buf;
	FILE *f;
	int size;
	char *data, *tmp;

	char path[255];
	strcpy(path, "sd:/pcsx-r/smb.ini");

	if (stat(path, &buf) == -1)
	{
		strcpy(path, "usb:/pcsx-r/smb.ini");
		if (stat(path, &buf) == -1)
			return -1;
	}
	size = buf.st_size;

	f = fopen(path, "r");
	if (f == NULL) return -1;

	data = (char*)malloc(size);
	if (data == NULL) return -1;
	rewind(f);
	fread(data, 1, size, f);
	fclose(f);
	
	GetValue("ip", Settings.smb.ip);
	GetValue("user", Settings.smb.user);
	GetValue("pwd", Settings.smb.pwd);
	GetValue("share", Settings.smb.share);

	free(data);

	return 0;
}
