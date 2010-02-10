#include "textmenu.h"
#include "r3000a.h"
#include "../save_state.h"

#ifdef HW_RVL

#define DEVICES			3

static const char *devicename[DEVICES] = {
	"Front SD"
,	"USB Storage"
,	"SMB"
};

#else	//!HW_RVL

#define DEVICES			2

static const char *devicename[DEVICES] = {
	"Memcard A"
,	"Memcard B"
};

#endif	//HW_RVL

#define SAVE_STATE 0

static int NeedReset = 0;
extern int Running;

static int RunEmu()
{
	if(!Running)
	{
		if(SysInit() == -1) 
		{
			return -1;
		}
		OpenPlugins();
		SysReset();

		if(CheckCdrom() == -1 || LoadCdrom() == -1)
		{
			ClosePlugins();
			return -1;
		}
	}
	else
	{
		OpenPlugins();
		if(NeedReset) 
		{
			SysReset();
			if(CheckCdrom() == -1 || LoadCdrom() == -1)
			{
				ClosePlugins();
				return -1;
			}
		}
	}
	Running 	= 1;
	NeedReset 	= 0;
	VIDEO_WaitVSync();
	return 0;
}

void Main_menu()
{
	u8 index = 0;
	u8 draw  = 1;

	char *msg = NULL;

	clrscr();
	while(1)
	{
		CHECK_POWER_BUTTONS();
		if(GetHeld(UP, UP, UP))
		{
			if(index) index--;
			usleep(150000);
			draw = 1;
		}

		if(GetHeld(DOWN, DOWN, DOWN))
		{
#if SAVE_STATE
			if(index < 6) 
#else
#ifdef HW_RVL
			if(index < 4) 
#else
			if(index < 3) 
#endif
#endif
				index++;
			usleep(150000);
			draw = 1;
		}

		if(GetInput(RIGHT, RIGHT, RIGHT))
		{
			switch(index)
			{
				case 2: 
					if(Settings.device < DEVICES-1) Settings.device++;
					msg = NULL;
					draw = 1;
					clrscr();
					break;
			}
		}

		if(GetInput(LEFT, LEFT, LEFT))
		{
			switch(index)
			{
				case 2: 
					if(Settings.device) Settings.device--;
					msg = NULL;
					draw = 1;
					clrscr();
					break;
			}
		}

		if(GetInput(A, A, A)) 
		{
			switch(index)
			{
				case 0:
					clrscr();
					if(strlen(Settings.filename) > 6)
					{
						FILE *f = NULL;
						char bios[256];
						sprintf (bios,"%s%s",Config.BiosDir, Config.Bios);
						f = fopen(bios, "rb");
						if(!f)
							msg = "Bios not found";
						else
						{
							fclose(f);
							if( RunEmu() == 0 )
								return psxCpu->Execute();
							else
								msg = "Error loading CD image";
						}
					}
					else 
						msg = "Select a file first";
					draw = 1;
					break;
					
				case 1:
					NeedReset = 1;
					index = 0;
					break;

				case 2:
					NeedReset = 1;

					msg = NULL;
					if( GameBrowser() == 0 )
						index = 0;
					else 
						msg = "Device not found";

					clrscr();
					draw = 1;
					break;
					
				case 3:
					Config_menu();
					clrscr();
					draw = 1;
					break;
#if SAVE_STATE
				case 4:
					if(on_states_save() == -1)
						msg = "Error saving save state";
					clrscr();
					draw = 1;
					break;

				case 5:
					if(on_states_load() == -1)
					{
						clrscr();
						draw = 1;
					}
					break;

				case 6: 
					to_loader();
					break;
#else
				case 4: 
					to_loader();
					break;
#endif
			}
		}

		if(draw)
		{
			draw = 0;
			printf("\x1B[2;2H");

			printf("\x1b[33m");
			printf("\tMain menu\n\n");

			printf("\x1b[%um", (index == 0) ? 32 : 37); 	// Set Color
			printf("\tStart\n");

			printf("\x1b[%um", (index == 1) ? 32 : 37);
			printf("\tReset\n");

			printf("\x1b[%um", (index == 2) ? 32 : 37);
			printf("\tSource: %s\n", devicename[Settings.device]);

			printf("\x1b[%um", (index == 3) ? 32 : 37);
			printf("\tConfig\n");
#if SAVE_STATE
			printf("\x1b[%um", (index == 4) ? 32 : 37);
			printf("\tSave state\n");

			printf("\x1b[%um", (index == 5) ? 32 : 37);
			printf("\tLoad state\n\n");

#ifdef HW_RVL
			printf("\x1b[%um", (index == 6) ? 32 : 37);
			printf("\tExit to HBC\n\n");
#endif
#else
#ifdef HW_RVL
			printf("\x1b[%um", (index == 4) ? 32 : 37);
			printf("\tExit to HBC\n");
#endif
#endif
			if(msg)
			{
				printf("\x1b[36m");
				printf("\t%s\n\n", msg);
			}
			if(strlen(Settings.filename) > 4)
			{
				printf("\x1b[36m");
				printf("\tSelected file: %s\n\n", Settings.filename);
			}

			printf("\x1b[37m");								// Reset Color
		}
	}
}
