//cube_audio.c AUDIO output via libOGC

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version. See also the license.txt file for *
 *   additional informations.                                              *
 *                                                                         *
 ***************************************************************************/

#include "stdafx.h"
#include "externals.h"

////////////////////////////////////////////////////////////////////////
// cube audio globals
////////////////////////////////////////////////////////////////////////
#include "../Gamecube/DEBUG.h"
#include <ogc/audio.h>
#include <gccore.h>
#include <malloc.h>

static u8 audio_buffer[2][4000] ATTRIBUTE_ALIGN(32);
static u8 *mixbuffer;
static int IsPlaying = 0;
static u32 BufLen;
static int whichab = 0;

static void AudioSwitchBuffers()
{
	memset(audio_buffer[whichab], 0, BufLen);
	memcpy(audio_buffer[whichab], mixbuffer, BufLen);

	AUDIO_InitDMA((u32)audio_buffer[whichab], BufLen);
	DCFlushRange(audio_buffer[whichab], BufLen);
	AUDIO_StartDMA();
	whichab ^= 1;
	IsPlaying = 1;
}

////////////////////////////////////////////////////////////////////////
// SETUP SOUND
////////////////////////////////////////////////////////////////////////

void SetupSound(void)
{
	AUDIO_Init(NULL);
	AUDIO_SetDSPSampleRate(AI_SAMPLERATE_48KHZ);
	AUDIO_RegisterDMACallback (AudioSwitchBuffers);
	memset(audio_buffer, 0, 4000*2);
	DEBUG_print("SetupSound called",12);
}

////////////////////////////////////////////////////////////////////////
// REMOVE SOUND
////////////////////////////////////////////////////////////////////////

void RemoveSound(void)
{
	AUDIO_StopDMA();
	AUDIO_RegisterDMACallback(NULL);
	DEBUG_print("RemoveSound called",12);
	IsPlaying = 0;
}

////////////////////////////////////////////////////////////////////////
// GET BYTES BUFFERED
////////////////////////////////////////////////////////////////////////

unsigned long SoundGetBytesBuffered(void)
{
#ifdef PEOPS_SDLOG
 	sprintf(txtbuffer,"SoundGetBytesBuffered returns approx: %d bytes",AUDIO_GetDMABytesLeft());
 	DEBUG_print(txtbuffer,12);
#endif
	return AUDIO_GetDMABytesLeft();
}

////////////////////////////////////////////////////////////////////////
// FEED SOUND DATA
////////////////////////////////////////////////////////////////////////
void SoundFeedStreamData( unsigned char *pSound, long lBytes)
{
	BufLen = lBytes;
	mixbuffer = pSound;
	if (!IsPlaying) AudioSwitchBuffers();
#ifdef PEOPS_SDLOG
	sprintf(txtbuffer,"SoundFeedStreamData length: %ld bytes",lBytes);
//	SysPrintf(txtbuffer);
	DEBUG_print(txtbuffer,13);
#endif
	
}
