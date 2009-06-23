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
#include <malloc.h>

static u8 audio_buffer[2][3840] __attribute__((aligned(32)));
static u8 *mixbuffer;
static u8 IsPlaying = 0;
static s32 BufLen;
static u8 whichab = 0;

static void AudioSwitchBuffers()
{
	//memset(audio_buffer[whichab], 0, BufLen);
	memcpy(audio_buffer[whichab], mixbuffer, BufLen);

	DCFlushRange(audio_buffer[whichab], BufLen);
	AUDIO_InitDMA((u32)audio_buffer[whichab], (u32)BufLen);

	whichab ^= 1;
	if(!IsPlaying)
	{
		AUDIO_StartDMA();
		IsPlaying = 1;
	}
}

////////////////////////////////////////////////////////////////////////

void SetupSound(void)
{
	AUDIO_Init(NULL);
	AUDIO_SetDSPSampleRate(AI_SAMPLERATE_48KHZ);
	AUDIO_RegisterDMACallback (AudioSwitchBuffers);
	memset(audio_buffer, 0, 3840 * 2);
	DEBUG_print("SetupSound called",12);
}

////////////////////////////////////////////////////////////////////////

void RemoveSound(void)
{
	AUDIO_StopDMA();
	AUDIO_RegisterDMACallback(NULL);
	DEBUG_print("RemoveSound called",12);
	IsPlaying = 0;
}

////////////////////////////////////////////////////////////////////////

u32 SoundGetBytesBuffered(void)
{
	return 0;/*
	u32 l = AUDIO_GetDMABytesLeft();
#ifdef PEOPS_SDLOG
 	sprintf(txtbuffer,"SoundGetBytesBuffered returns approx: %d bytes", l);
 	DEBUG_print(txtbuffer, 12);
#endif
	if( l <= 0 ) return 0;
	if( l < BufLen / 2 )
		l = SOUNDSIZE;
	else
		l = 0;
	return l;*/
}

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
