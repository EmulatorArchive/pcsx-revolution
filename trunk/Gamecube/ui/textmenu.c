#include "textmenu.h"
#include "wiismb.h"
#include "wiifat.h"
#include "mount.h"

#ifdef HW_RVL

#define DEVICES			3

char *device[DEVICES] = {
	"sd:/"
,	"usb:/"
,	"smb:/"
};

char *devicename[DEVICES] = {
	"Front SD"
,	"USB Storage"
,	"SMB"
};

#else	//!HW_RVL

#define DEVICES			2

char *device[DEVICES] = {
	"carda:/"
,	"cardb:/"
}

char *devicename[DEVICES] = {
	"Memcard A"
,	"Memcard B"
}

#endif	//HW_RVL

static int NeedReset = 0;
extern int Running;

static int OpenBrowser()
{
	switch(Settings.device)
	{
		case DEVICE_SD:
		case DEVICE_USB:
			if(MountFAT(Settings.device) == -1) 
				return -1;
			break;
		case DEVICE_SMB:
			if(ConnectShare != 0)
				return -1;
	}
	sprintf(Settings.filename, "%s%s", device[Settings.device], "wiisx/games");
	if(textFileBrowser(Settings.filename) == -1)
	{
		sprintf(Settings.filename, "%s", device[Settings.device]);
		return textFileBrowser(Settings.filename);
	}
	return 0;
}

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
			usleep(100000);
			draw = 1;
		}

		if(GetHeld(DOWN, DOWN, DOWN))
		{
			if(index < 5) index++;
			usleep(100000);
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
					if(strlen(Settings.filename) > 6)
					{
						clrscr();
						if( RunEmu() == 0 )
							return psxCpu->Execute();
						else
							msg = "Error loading CD image";
					}
					else 
						msg = "Select a file first";
					draw = 1;
					clrscr();
					break;
					
				case 1:
					NeedReset = 1;
					break;

				case 2:
				case 3: 
					NeedReset = 1;

					msg = NULL;
					if( OpenBrowser() == 0 )
						index = 0;
					else 
						msg = "Device not found";

					clrscr();
					draw = 1;
					break;
					
				case 4:
					Config_menu();
					clrscr();
					draw = 1;
					break;

				case 5: 
					to_loader();
					break;
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
			printf("\tSelect source: %s\n", devicename[Settings.device]);

			printf("\x1b[%um", (index == 3) ? 32 : 37);
			printf("\tSelect file\n");

			printf("\x1b[%um", (index == 4) ? 32 : 37);
			printf("\tConfig\n");

			printf("\x1b[%um", (index == 5) ? 32 : 37);
			printf("\tExit to HBC\n\n");

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
