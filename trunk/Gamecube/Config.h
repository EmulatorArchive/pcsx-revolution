#ifndef CONFIG_H
#define CONFIG_H

void SaveConfig();
int LoadConfig();

void SPUWriteConfig();
void GPUWriteConfig();
void PADWriteConfig();

void GPUReadConfig();
void SPUReadConfig();
int SMBReadConfig();

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
	char	ip[16];
	char	user[20];
	char	pwd[20];
	char	share[20];
} smb_settings;

typedef struct {
	char filename[255];
	int device;
	video_config GPU;
	audio_config SPU;
	smb_settings smb;
} settings;

extern settings Settings;

#endif
