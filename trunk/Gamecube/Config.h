#ifndef CONFIG_H
#define CONFIG_H

#include "pad.h"

void SaveConfig();
int LoadConfig();

void SPUWriteConfig();
void GPUWriteConfig();
void PADWriteConfig();

void GPUReadConfig();
void SPUReadConfig();

typedef struct {
	int ShowFPS;			// 0-1
	int FrameSkip;			// 0-1
	int LimitFPS;			// 0-1
	int Dithering;			// 0-2
} video_config;

typedef struct {
	int Volume;				// 1-4
	int Reverb;				// 0-2
	int Interpolation;		// 0-3
	int EnableXA;			// 0-1
	int XAPitch;			// 0-1
	int CompatMode;			// 0, 2
	int SPU_IRQ_Wait;		// 0-1
	int SingleChMode;		// 0-1
} audio_config;

typedef struct {
	char filename[255];
	int device;
	video_config GPU;
	audio_config SPU;
} settings;

extern settings Settings;

#endif
