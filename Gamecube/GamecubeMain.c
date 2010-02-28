/*  Pcsx - Pc Psx Emulator
 *  Copyright (C) 1999-2002  Pcsx Team
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

#include <gccore.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include "system.h"
#include "r3000a.h"
#include "DEBUG.h"

#include "Config.h"
#include "gcMisc.h"

#include "pad.h"

#ifdef USE_GUI
#include "gui/guimenu.h"
#else
#include "ui/textmenu.h"
#endif

#include "storage/wiifat.h"
//#include "storage/wiidvd.h"

/* function prototypes */
int  SysInit();							// Init mem and plugins
void SysReset();						// Resets mem
void SysPrintf(char *fmt, ...);			// Printf used by bios syscalls
void SysMessage(char *fmt, ...);		// Message used to print msg to users
void *SysLoadLibrary(char *lib);		// Loads Library
void *SysLoadSym(void *lib, char *sym);	// Loads Symbol from Library
const char *SysLibError();				// Gets previous error loading sysbols
void SysCloseLibrary(void *lib);		// Closes Library
void SysUpdate();						// Called on VBlank (to update i.e. pads)
void SysRunGui();						// Returns to the Gui
void SysClose();						// Close mem and plugins

u32 *xfb[2] = { NULL, NULL };			/*** Framebuffers ***/
int whichfb = 0;						/*** Frame buffer toggle ***/
GXRModeObj *vmode;				/*** Graphics Mode Object ***/
#define DEFAULT_FIFO_SIZE ( 256 * 1024 )
//static u8 gp_fifo[DEFAULT_FIFO_SIZE] ATTRIBUTE_ALIGN(32); /*** 3D GX FIFO ***/

#define max(a, b) ((a > b) ? b : a)

static inline void VideoInit()
{
	VIDEO_Init();
	whichfb = 0;				/*** Frame buffer toggle ***/
	vmode = VIDEO_GetPreferredMode(NULL);

	int videowidth = VI_MAX_WIDTH_NTSC;
	int videoheight = VI_MAX_HEIGHT_NTSC;
	
	if ((vmode->viTVMode >> 2) == VI_PAL)
	{
		videowidth = VI_MAX_WIDTH_PAL;
		videoheight = VI_MAX_HEIGHT_PAL;
	}
	
	vmode->viHeight = ceil((float)(videoheight * 0.95) / 8) * 8;
	
	vmode->xfbHeight = vmode->viHeight;
	vmode->efbHeight = max(vmode->xfbHeight, 528);
#ifdef HW_RVL
	if (CONF_GetAspectRatio() == CONF_ASPECT_16_9)
	{
        vmode->viWidth = videowidth * 0.95;
	}
	else
#endif
	{
        vmode->viWidth = videowidth * 0.93;
	}
	
	vmode->viWidth = ceil((float)vmode->viWidth / 16) * 16;
	
	vmode->viXOrigin = (videowidth - vmode->viWidth) / 2;
	vmode->viYOrigin = (videoheight - vmode->viHeight) / 2;

#ifdef HW_RVL
	s8 hor_offset = 0;
	
	if (CONF_GetDisplayOffsetH(&hor_offset) > 0)
		vmode->viXOrigin += hor_offset;
#endif
	
	VIDEO_Configure(vmode);
	
	xfb[0] = (u32 *)MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));
	xfb[1] = (u32 *)MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));

	console_init(xfb[0], 20, 64, vmode->fbWidth, vmode->xfbHeight, vmode->fbWidth * 2);
	VIDEO_ClearFrameBuffer(vmode, xfb[0], COLOR_BLACK);
	VIDEO_ClearFrameBuffer(vmode, xfb[1], COLOR_BLACK);
	
	VIDEO_SetNextFramebuffer(xfb[whichfb]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	
	if (vmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	else
	    while (VIDEO_GetNextField())
	    	VIDEO_WaitVSync();

	// setup the fifo and then init GX
	void *gp_fifo = NULL;
	gp_fifo = MEM_K0_TO_K1 (memalign (32, DEFAULT_FIFO_SIZE));
	memset (gp_fifo, 0, DEFAULT_FIFO_SIZE);
 
	GX_Init (gp_fifo, DEFAULT_FIFO_SIZE);
 
	// clears the bg to color and clears the z buffer
	GX_SetCopyClear ((GXColor){0,0,0,255}, GX_MAX_Z24);
	// init viewport
	GX_SetViewport (0, 0, vmode->fbWidth, vmode->efbHeight, 0, 1);
	// Set the correct y scaling for efb->xfb copy operation
	f32 yscale = GX_GetYScaleFactor(vmode->efbHeight, vmode->xfbHeight);
	u32 xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetScissor(0, 0, max(vmode->fbWidth, 640), vmode->efbHeight);
	GX_SetDispCopySrc(0, 0, max(vmode->fbWidth, 640), vmode->efbHeight);
	GX_SetDispCopyDst(vmode->fbWidth, xfbHeight);
	GX_SetCopyFilter(vmode->aa, vmode->sample_pattern, GX_TRUE, vmode->vfilter);
	
	GX_SetCullMode(GX_CULL_NONE);
	GX_CopyDisp(xfb[0], GX_TRUE);
}

static void Initialise (void){
	//InitDVD();
	VideoInit();
	PAD_Init();

#ifdef HW_RVL
	WPAD_Init();
#endif

#ifdef HW_RVL
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
	// Wii Power/Reset buttons
	// gcMisc,h
	WPAD_SetPowerButtonCallback((WPADShutdownCallback)ShutdownCB);
	SYS_SetPowerCallback(ShutdownCB);
	SYS_SetResetCallback(ResetCB);
#endif

	MountAllFAT();
}

// Plugin structure
#include "GamecubePlugins.h"
PluginTable plugins[] =
	{ PLUGIN_SLOT_0,
	  PLUGIN_SLOT_1,
	  PLUGIN_SLOT_2,
	  PLUGIN_SLOT_3,
	  PLUGIN_SLOT_4,
	  PLUGIN_SLOT_5,
	  PLUGIN_SLOT_6,
	  PLUGIN_SLOT_7 
};

long LoadCdBios;

int main(int argc, char *argv[]) {

	Initialise();

	/* Configure pcsx */
	memset(&Config, 0, sizeof(PcsxConfig));
	strcpy(Config.Net, "Disabled");
	if (LoadConfig() == -1)
	{
		strcpy(Config.Bios, "SCPH1001.BIN"); // Use actual BIOS
#ifdef HW_RVL
		strcpy(Config.BiosDir, "sd:/pcsx-r/bios/");

		strcpy(Config.Mcd1, "sd:/pcsx-r/memcards/Mcd001.mcr");
		strcpy(Config.Mcd2, "sd:/pcsx-r/memcards/Mcd002.mcr");
#else
		strcpy(Config.BiosDir, "/pcsx-r/bios/");

		strcpy(Config.Mcd1, "/pcsx-r/memcards/Mcd001.mcr");
		strcpy(Config.Mcd2, "/pcsx-r/memcards/Mcd002.mcr");
#endif
		Config.Cpu 		= 1;	//interpreter = 1, dynarec = 0

		Config.PsxOut 	= 0;
		Config.HLE 		= 0;
		Config.Xa 		= 0;	//XA enabled
		Config.Cdda 	= 0;
		Config.PsxAuto 	= 1;	//Autodetect
		init_default_pads();
	}

	Main_menu();

	return 0;
}

int SysInit() {
    SysPrintf("start SysInit()\r\n");

    SysPrintf("psxInit()\r\n");
	psxInit();

    SysPrintf("LoadPlugins()\r\n");
	if(LoadPlugins()==-1)
		SysPrintf("ErrorLoadingPlugins()\r\n");
	SysPrintf("end SysInit()\r\n");
	return 0;
}

void SysReset() {
    SysPrintf("start SysReset()\r\n");
	psxReset();
	SysPrintf("end SysReset()\r\n");
}

void SysClose() {
	psxShutdown();
	ReleasePlugins();

	if (emuLog != NULL) fclose(emuLog);
}

void SysPrintf(char *fmt, ...) {
	if (Config.PsxOut) {
		va_list list;
		char msg[512];

		va_start(list, fmt);
		vsprintf(msg, fmt, list);
		va_end(list);
	
		printf ("%s", msg);
#if defined (CPU_LOG) || defined(DMA_LOG) || defined(CDR_LOG) || defined(HW_LOG) || \
	defined(BIOS_LOG) || defined(GTE_LOG) || defined(PAD_LOG)
		fprintf(emuLog, "%s", msg);
#endif
	}
}

void *SysLoadLibrary(char *lib) {
	int i;
	for(i=0; i<NUM_PLUGINS; i++)
		if((plugins[i].lib != NULL) && (!strcmp(lib, plugins[i].lib)))
			return (void*)i;
	return NULL;
}

void *SysLoadSym(void *lib, char *sym) {
	PluginTable* plugin = plugins + (int)lib;
	int i;
	for(i=0; i<plugin->numSyms; i++)
		if(plugin->syms[i].sym && !strcmp(sym, plugin->syms[i].sym))
			return plugin->syms[i].pntr;
	return NULL;
}

const char *SysLibError() {
	return NULL;
}

void SysCloseLibrary(void *lib) {
//	dlclose(lib);
}

int framesdone = 0;
void SysUpdate() {
#ifdef SHOW_DEBUG

#endif
}

void SysRunGui() {
	Main_menu();
}

void SysMessage(char *fmt, ...) {
	
}
