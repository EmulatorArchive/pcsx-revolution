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

// #include "stdafx.h"
#include "externals.h"
#include "cube_audio.h"

#include "Config.h"
#include "DEBUG.h"
#include <gctypes.h>
#include <ogc/audio.h>
#include <ogc/cache.h>

#include <string.h>
#include <malloc.h>
#include <math.h>
#include <unistd.h>

// cube audio globals

#define NUM_BUFFERS 4
#define BUFFER_SIZE 3240
static char buffer[NUM_BUFFERS][BUFFER_SIZE] __attribute__((aligned(32)));
static int which_buffer = 0;
static unsigned int buffer_offset = 0;
#define NEXT(x) (x=(x+1)%NUM_BUFFERS)
static const float freq_ratio = 44100.0f / 32000.0f;
static const float inv_freq_ratio = 32000.0f / 44100.0f;
//static enum { BUFFER_SIZE_32_60 = 2112, BUFFER_SIZE_32_50 = 2560 } buffer_size;

static s32 buffer_size;
#define thread_buffer which_buffer

static void inline play_buffer(void);
static void done_playing(void);
static void copy_to_buffer_mono(void* out, void* in, unsigned int samples);
static void copy_to_buffer_stereo(void* out, void* in, unsigned int samples);
static void (*copy_to_buffer)(void* out, void* in, unsigned int samples);

void SetupSound(void)
{
	AUDIO_Init(NULL);
	int n;
	for(n = 0; n < NUM_BUFFERS; n++)
	{
		memset(buffer[n], 0, BUFFER_SIZE);
		DCFlushRange(buffer[n], BUFFER_SIZE);
	}
	//buffer_size = Config.PsxType ? BUFFER_SIZE_32_50 : BUFFER_SIZE_32_60;
	AUDIO_SetDSPSampleRate(AI_SAMPLERATE_48KHZ);
	copy_to_buffer = iDisStereo ? copy_to_buffer_mono : copy_to_buffer_stereo;
	which_buffer = 0;
	buffer_offset = 0;
	buffer_size = 0;
	//printf("SetupSound\n");
}

void RemoveSound(void)
{
	AUDIO_StopDMA();

	int n;
	for(n = 0; n < NUM_BUFFERS; n++)
	{
		memset(buffer[n], 0, BUFFER_SIZE);
		DCFlushRange(buffer[n], BUFFER_SIZE);
	}

	AUDIO_RegisterDMACallback(NULL);
	AUDIO_InitDMA((u32)buffer[0], 32);
	AUDIO_StartDMA();
	
	usleep(100);
	
	while (AUDIO_GetDMABytesLeft() > 0)
		usleep(100);
	
	AUDIO_StopDMA();
	
	which_buffer = 0;
	copy_to_buffer = NULL;
	buffer_offset = 0;
	buffer_size = 0;
	//DEBUG_print("RemoveSound called",12);
}

unsigned long SoundGetBytesBuffered(void)
{
	/*sprintf(txtbuffer,"SoundGetBytesBuffered returns approx: %d bytes",
	buffer_size - AUDIO_GetDMABytesLeft());
	DEBUG_print(txtbuffer,12);*/
	if(!AUDIO_GetDMABytesLeft())
		return 0;
	return buffer_size - AUDIO_GetDMABytesLeft();
}

static void inline play_buffer(void){
	// Make sure the buffer is in RAM, not the cache
	DCFlushRange(buffer[thread_buffer], buffer_size);

	// Actually send the buffer out to be played next
	AUDIO_InitDMA((unsigned int)&buffer[thread_buffer], buffer_size);

	// Start playing the buffer
	AUDIO_StartDMA();
}

static void copy_to_buffer_mono(void* b, void* s, unsigned int length){
	// NOTE: length is in resampled samples (mono (1) short)
	short* buffer = (short *)b;
	short* stream = (short *)s;
	int di;
	float si;
	for(di = 0, si = 0.0f; di < length; ++di, si += freq_ratio){
		// Linear interpolation between current and next sample
		float t = si - floorf(si);
		buffer[di] = (1.0f - t)*stream[(int)si] + t*stream[(int)ceilf(si)];
	}
}

static void copy_to_buffer_stereo(void* b, void* s, unsigned int length){
	// NOTE: length is in resampled samples (stereo (2) shorts)
	int *buffer = (int *)b;
	int *stream = (int *)s;
	int di;
	float si;
	for(di = 0, si = 0.0f; di < length; ++di, si += freq_ratio){
#if 1
		// Linear interpolation between current and next sample
		float t = si - floorf(si);
		short* osample  = (short*)(buffer + di);
		short* isample1 = (short*)(stream + (int)si);
		short* isample2 = (short*)(stream + (int)ceilf(si));
		// Left and right
		osample[0] = (1.0f - t)*isample1[0] + t*isample2[0];
		osample[1] = (1.0f - t)*isample1[1] + t*isample2[1];
#else
		// Quick and dirty resampling: skip over or repeat samples
		buffer[di] = stream[(int)si];
#endif
	}
}

static void inline add_to_buffer(void* stream, unsigned int length){
	// This shouldn't lose any data and works for any size
	unsigned int stream_offset = 0;
	// Length calculations are in samples
	//   (Either pairs of shorts (stereo) or shorts (mono))
	const int shift = iDisStereo ? 1 : 2;
	unsigned int lengthi, rlengthi;
	unsigned int lengthLeft = length >> shift;
	unsigned int rlengthLeft = ceilf(lengthLeft * inv_freq_ratio);

	while(rlengthLeft > 0){
		rlengthi = (buffer_offset + (rlengthLeft << shift) <= buffer_size) ?
				rlengthLeft : ((buffer_size - buffer_offset) >> shift);
		lengthi  = rlengthi * freq_ratio;

		copy_to_buffer(buffer[which_buffer] + buffer_offset,
				stream + stream_offset, rlengthi);

		if(buffer_offset + (rlengthLeft << shift) < buffer_size){
			buffer_offset += rlengthi << shift;
			return;
		}

		lengthLeft    -= lengthi;
		stream_offset += lengthi << shift;
		rlengthLeft   -= rlengthi;

		play_buffer();

		NEXT(which_buffer);
		buffer_offset = 0;
	}

}

// FEED SOUND DATA

void SoundFeedStreamData(unsigned char* pSound,long lBytes)
{
	buffer_size = lBytes;
	if( !Settings.SPU.Disable ) {
		add_to_buffer(pSound, lBytes);
	}
}

void PEOPS_SPUplayCDDAchannel(short* pcm, int nbytes) {
	// TODO
}
