/***************************************************************************
 *   Copyright (C) 2007 PCSX-df Team                                       *
 *   Copyright (C) 2009 Wei Mingzhi                                        *
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

// TODO: implement CDDA & subchannel support.

#include "plugins.h"
#include <stdlib.h>
#include <sys/stat.h>
#ifdef __GAMECUBE__
#define STATIC 
#include "PsxCommon.h"
#include "Gamecube/Config.h"
#else //!__GAMECUBE__
#define STATIC static
#include "psxcommon.h"
#endif //__GAMECUBE__

#define MSF2SECT(m, s, f)	(((m) * 60 + (s) - 2) * 75 + (f))
#define btoi(b)			((b) / 16 * 10 + (b) % 16) /* BCD to u_char */

#define CD_FRAMESIZE_RAW	2352
#define DATA_SIZE		(CD_FRAMESIZE_RAW - 12)

FILE *cdHandle = NULL;
static unsigned char cdbuffer[CD_FRAMESIZE_RAW * 10];

char* CALLBACK CDR__getDriveLetter(void);
#ifdef __GAMECUBE__
unsigned char* CALLBACK CDR__getBufferSub(void) { return NULL; };
#else
unsigned char* CALLBACK CDR__getBufferSub(void);
#endif
long CALLBACK CDR__configure(void);
long CALLBACK CDR__test(void);
void CALLBACK CDR__about(void);
long CALLBACK CDR__setfilename(char *filename);
long CALLBACK CDR__getStatus(struct CdrStat *stat);

extern void *hCDRDriver;

struct trackinfo {
	enum {DATA, CDDA} type;
	char start[3];		// MSF-format
	char length[3];		// MSF-format
	char gap[3];		// MSF-format
};

#define MAXTRACKS 100 /* How many tracks can a CD hold? */

static int numtracks = 0;
static struct trackinfo ti[MAXTRACKS];

// get a sector from a msf-array
static unsigned int msf2sec(char *msf) {
	return ((msf[0] * 60 + msf[1]) * 75) + msf[2];
}

static void sec2msf(unsigned int s, char *msf) {
	msf[0] = s / 75 / 60;
	s = s - msf[0] * 75 * 60;
	msf[1] = s / 75;
	s = s - msf[1] * 75;
	msf[2] = s;
}

// get size of a track given the sector
unsigned int ISOgetTrackLength(unsigned int s) {
	int i = 1;

	while ((msf2sec(ti[i].start) < s) && i <= numtracks)
		i++;

	return msf2sec(ti[--i].length);
}

// divide a string of xx:yy:zz into m, s, f
static void tok2msf(char *time, char *msf) {
	char *token;

	token = strtok(time, ":");
	if (token)
		msf[0] = atoi(token);
	else
		msf[0]=0;

	token = strtok(NULL, ":");
	if (token)
		msf[1] = atoi(token);
	else
		msf[1] = 0;

	token = strtok(NULL, ":");
	if (token)
		msf[2] = atoi(token);
	else
		msf[2] = 0;
}

// this function tries to get the .cue file of the given .bin
// the neccessary data is put into the ti (trackinformation)-array
static int parsecue(const char* isofile)
{
	FILE* cue;
	char *token;
	char cuename[256];
	char name[256];
	char time[20];
	char linebuf[256], dummy[256];
	unsigned int i, t;
	strcpy(cuename, isofile);
	token = strstr(cuename, ".bin");
	if (token)
		sprintf((char *)token, ".cue");
	else
		return -1;

	numtracks = 0;
	// Get the filesize and open the file
	cue = fopen(cuename, "r");
	if(!cue)
	{
		SysPrintf(_("Could not open %s.\n"), cuename);
		return -1;
	}
	
	memset(&ti, 0, sizeof(ti));

	while (fgets(linebuf, sizeof(linebuf), cue) != NULL) {
		strncpy(dummy, linebuf, sizeof(linebuf));
		token = strtok(dummy, " ");

		if( !strcmp(token, "FILE") ){
			 sscanf(linebuf, "FILE %s %s", name, dummy);
		} 
		if( !strcmp(token, "TRACK") ){
			token = strtok(NULL, " ");
			numtracks++;
			if( !strcmp(token, "AUDIO") )
			{
				ti[numtracks].type = CDDA;
			}
			else if( !strcmp(token, "MODE1/2352") )
			{
				ti[numtracks].type = DATA;
				sec2msf(2 * 75, ti[numtracks].start); // assume data track on 0:2:0
			}
			else if( !strcmp(token, "MODE2/2352") )
			{
				ti[numtracks].type = DATA;
				sec2msf(2 * 75, ti[numtracks].start);
			}

		} 
		if( !strcmp(token, "INDEX") ){
			sscanf(linebuf, "    INDEX 01 %s", time);
			SysPrintf(_("Track beginning at %s\n"), time);
			tok2msf((char *)&time, (char *)&ti[numtracks].start);
			// If we've already seen another track, this is its end
			if(numtracks > 1){
				ti[numtracks-1].length[0] = ti[numtracks].start[0];
				ti[numtracks-1].length[1] = ti[numtracks].start[1];
				ti[numtracks-1].length[2] = ti[numtracks].start[2];
			}
		}
	}

	fclose(cue);

	struct stat binInfo;
	if( stat(isofile, &binInfo) ){
		return -1;
	}
	// Fill out the last track's end based on size
	unsigned int blocks = binInfo.st_size / 2352;
	sec2msf(blocks, ti[numtracks].length);

	// calculate the true start of each track
	// start+datasize (+2 secs of silence ? I dunno...)
	for(i = 2; i <= numtracks; i++) {
		t = msf2sec(ti[1].start) + msf2sec(ti[1].length) + msf2sec(ti[i].start);
		sec2msf(t, ti[i].start);
	}

	return 0;
}

// this function tries to get the .toc file of the given .bin
// the neccessary data is put into the ti (trackinformation)-array
static int parsetoc(char *isofile) {
	char tocname[MAXPATHLEN];
	FILE *fi;
	char linebuf[256], dummy[256];
	char *token;
	char name[256];
	char time[20], time2[20];
	unsigned int i, t;

	numtracks = 0;

	// copy name of the iso and change extension from .bin to .toc
	strncpy(tocname, isofile, sizeof(tocname));
	token = strstr(tocname, ".bin");
	if (token)
		sprintf((char *)token, ".toc");
	else
		return -1;

	if ((fi = fopen(tocname, "r")) == NULL) {
		//SysPrintf(_("Could not open %s.\n"), tocname);
		return -1;
	}

	memset(&ti, 0, sizeof(ti));

	// parse the .toc file
	while (fgets(linebuf, sizeof(linebuf), fi) != NULL) {
		// search for tracks
		strncpy(dummy, linebuf, sizeof(linebuf));
		token = strtok(dummy, " ");

		// a new track is found
		if (!strcmp(token, "TRACK")) {
			// get type of track
			token = strtok(NULL, " ");

			numtracks++;

			if (!strcmp(token, "MODE2_RAW\n")) {
				ti[numtracks].type = DATA;
				sec2msf(2 * 75, ti[numtracks].start); // assume data track on 0:2:0
			}

			if (!strcmp(token, "AUDIO\n"))
				ti[numtracks].type = CDDA;
		}

		// interpretation of other lines
		if (!strcmp(token, "DATAFILE")) {
			sscanf(linebuf, "DATAFILE %s %s", name, time);
			tok2msf((char *)&time, (char *)&ti[numtracks].length);
		}

		if (!strcmp(token, "FILE")) {
			sscanf(linebuf, "FILE %s %s %s %s", name, dummy, time, time2);
			tok2msf((char *)&time, (char *)&ti[numtracks].start);
			tok2msf((char *)&time2, (char *)&ti[numtracks].length);
		}

		if (!strcmp(token, "START")) {
			sscanf(linebuf, "START %s", time);
			tok2msf((char *)&time, (char *)&ti[numtracks].gap);
		}
	} 

	fclose(fi);

	// calculate the true start of each track
	// start+gap+datasize (+2 secs of silence ? I dunno...)
	for(i = 2; i <= numtracks; i++) {
		t = msf2sec(ti[1].start) + msf2sec(ti[1].length) + msf2sec(ti[i].start) + msf2sec(ti[i].gap);
		sec2msf(t, ti[i].start);
	}

	return 0;
}


STATIC long CALLBACK ISOinit(void) {
#ifdef __GAMECUBE__
	SysPrintf("start CDR_init()\r\n");
	numtracks = 0;
	SysPrintf("end CDR_init()\r\n");
#else
	assert(cdHandle == NULL);
#endif
	return 0; // do nothing
}

STATIC long CALLBACK ISOshutdown(void) {
	if (cdHandle != NULL) {
		fclose(cdHandle);
		cdHandle = NULL;
	}
	return 0;
}

// This function is invoked by the front-end when opening an ISO
// file for playback
STATIC long CALLBACK ISOopen(void) {
#ifdef __GAMECUBE__
	char *cdrfilename = Settings.filename;
#endif
/*	if (cdHandle != NULL)
		return 0; // it's already open
	cdHandle = fopen(Settings.filename, "rb");
	if (cdHandle == NULL)
		return -1;
	parsetoc(Settings.filename);
	parsecue(Settings.filename);
	return 0;
}

*/
	if (cdHandle != NULL)
		return 0; // it's already open
	cdHandle = fopen(cdrfilename, "rb");
	if (cdHandle == NULL)
		return -1;
	parsetoc(cdrfilename);
	parsecue(cdrfilename);
	return 0;
}

STATIC long CALLBACK ISOclose(void) {
	if (cdHandle != NULL) {
		fclose(cdHandle);
		cdHandle = NULL;
	}
	return 0;
}

// return Starting and Ending Track
// buffer:
//  byte 0 - start track
//  byte 1 - end track
STATIC long CALLBACK ISOgetTN(unsigned char *buffer) {
	buffer[0] = 1;

	if (numtracks > 0)
		buffer[1] = numtracks;
	else
		buffer[1] = 1;

	return 0;
}

// return Track Time
// buffer:
//  byte 0 - frame
//  byte 1 - second
//  byte 2 - minute
STATIC long CALLBACK ISOgetTD(unsigned char track, unsigned char *buffer) {
	if (numtracks > 0 && track <= numtracks) {
		buffer[2] = ti[track].start[0];
		buffer[1] = ti[track].start[1];
		buffer[0] = ti[track].start[2];
	} else {
		buffer[2] = 0;
		buffer[1] = 2;
		buffer[0] = 0;
	}

	return 0;
}

// read track
// time: byte 0 - minute; byte 1 - second; byte 2 - frame
// uses bcd format
STATIC long CALLBACK ISOreadTrack(unsigned char *time) {
	if (cdHandle == NULL)
		return -1;

	fseek(cdHandle, MSF2SECT(btoi(time[0]), btoi(time[1]), btoi(time[2])) * CD_FRAMESIZE_RAW + 12, SEEK_SET);
	fread(cdbuffer, 1, DATA_SIZE, cdHandle);

	return 0;
}

// return readed track
STATIC unsigned char * CALLBACK ISOgetBuffer(void) {
	return (unsigned char *)&cdbuffer;
}

#ifndef __GAMECUBE__
// plays cdda audio
// sector: byte 0 - minute; byte 1 - second; byte 2 - frame
// does NOT uses bcd format
STATIC long CALLBACK ISOplay(unsigned char *time) {
	return 0; // TODO
}

// stops cdda audio
STATIC long CALLBACK ISOstop(void) {
	return 0; // TODO
}
#endif

void imageReaderInit(void) {
#ifndef __GAMECUBE__
	assert(hCDRDriver == NULL);

	CDR_init = ISOinit;
	CDR_shutdown = ISOshutdown;
	CDR_open = ISOopen;
	CDR_close = ISOclose;
	CDR_getTN = ISOgetTN;
	CDR_getTD = ISOgetTD;
	CDR_readTrack = ISOreadTrack;
	CDR_getBuffer = ISOgetBuffer;
	CDR_play = ISOplay;
	CDR_stop = ISOstop;

	CDR_getStatus = CDR__getStatus;
	CDR_getDriveLetter = CDR__getDriveLetter;
	CDR_getBufferSub = CDR__getBufferSub;
	CDR_configure = CDR__configure;
	CDR_test = CDR__test;
	CDR_about = CDR__about;
	CDR_setfilename = CDR__setfilename;
	numtracks = 0;
#endif
}
