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
* This file contains common definitions and includes for all parts of the 
* emulator core.
*/

#ifndef __PSXCOMMON_H__
#define __PSXCOMMON_H__

#ifdef GEKKO
#include <gccore.h>
// #include "Config.h"
#else
#include "config.h"
#endif

/* System includes */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <assert.h>
#include <zlib.h>

#ifndef GEKKO
/* Define types */
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#endif

typedef intptr_t sptr;
typedef uintptr_t uptr;

/* Local includes */
#include "system.h"
#include "debug.h"

#if defined (__LINUX__) || defined (__MACOSX__)
#define strnicmp strncasecmp
#endif
#define __inline inline

/* Enables NLS/internationalization if active */
#ifdef ENABLE_NLS

#include <libintl.h>

#undef _
#define _(String) gettext(String)
#ifdef gettext_noop
#  define N_(String) gettext_noop (String)
#else
#  define N_(String) (String)
#endif

#else

#define _(msgid) msgid
#define N_(msgid) msgid

#endif

extern int Log;
void __Log(char *fmt, ...);

typedef struct
{
	char Filename[256];
	u32 Enabled:1;
} McdConfig;

typedef struct {
	char Gpu[MAXPATHLEN];
	char Spu[MAXPATHLEN];
	char Cdr[MAXPATHLEN];
	char Pad1[MAXPATHLEN];
	char Pad2[MAXPATHLEN];
	char Net[MAXPATHLEN];
	char Mcd1[MAXPATHLEN];
	char Mcd2[MAXPATHLEN];
	McdConfig Mcd[2];
	char Bios[MAXPATHLEN];
	char BiosDir[MAXPATHLEN];
	char PluginsDir[MAXPATHLEN];
	long Xa;
	long Sio;
	long Mdec;
	long PsxAuto;
	long PsxType;		/* NTSC or PAL */
	long Cdda;
	long HLE;
	long Cpu;
	long Debug;
	long PsxOut;
	long SpuIrq;
	long RCntFix;
	long UseNet;
	long VSyncWA;
#ifdef _WIN32
	char Lang[256];
#endif
} PcsxConfig;

extern PcsxConfig Config;

extern int StatesC;
extern int cdOpenCase;
extern int NetOpened;

#define gzfreeze(ptr, size) \
	if (Mode == 1) gzwrite(f, ptr, size); \
	if (Mode == 0) gzread(f, ptr, size);

#define gzfreezel(ptr) gzfreeze(ptr, sizeof(ptr))

//#define BIAS	4
#define BIAS	2
#define PSXCLK	(33868800ULL)	/* 33.8688 Mhz */

enum {
	BIOS_USER_DEFINED,
	BIOS_HLE
};	/* BIOS Types */

enum {
	PSX_TYPE_NTSC,
	PSX_TYPE_PAL
};	/* PSX Type */


#endif /* __PSXCOMMON_H__ */
