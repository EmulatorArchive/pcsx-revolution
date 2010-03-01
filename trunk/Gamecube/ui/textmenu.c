#include "textmenu.h"
#include "r3000a.h"
#include "../save_state.h"
#include "system.h"
#include "storage/mount.h"

#define SAVE_STATE 0

#ifdef HW_RVL

#if SAVE_STATE
#	define TEXT_MENU_OPTIONS 7
#else
#	define TEXT_MENU_OPTIONS 5
#endif

#else	//!HW_RVL

#if SAVE_STATE
#	define TEXT_MENU_OPTIONS 6
#else
#	define TEXT_MENU_OPTIONS 4
#endif

#endif	//HW_RVL

static const char *devicename[DEVICES_COUNT] = {
#ifdef HW_RVL
	"Front SD"
,	"USB Storage"
,	"SMB"
#ifdef DVD_FIXED
,	"DVD"
#endif
#else
	"Memcard A"
,	"Memcard B"
#endif
};

typedef enum {
	NO_ACTION			= 0,
	ERR_BROWSER			= 1,
	ERR_CDROM			= 2,
	ERR_BIOS			= 3,
	ERR_SYSTEM_INIT		= 4,
	ERR_SSTATE_SAVE		= 5,
	ERR_SSTATE_LOAD		= 6,
	RUN_GAME			= 7,
	UPDATE_MENU			= 8,
	ACTIONS_COUNT
} ret_action;

static char *errors[ACTIONS_COUNT] = {
	"",
	"File not found",
	"It's not a PSX game",
	"Bios not found",
	"Error system init",
	"Error saving state",
	"Error loading state"
	"",
	"",
};

static char *str_options[TEXT_MENU_OPTIONS] = {
	"Start",
	"Reset",
	"Source: ",
	"Config"
#if SAVE_STATE
.	"Save state"
,	"Load state"
#endif
#ifdef HW_RVL
,	"Exit to HBC"
#endif
};

static u8 NeedReset = 0;
extern int Running;

#define TEXT_MENU_ACTION static ret_action
#define TEXT_MENU_OPTION static void

TEXT_MENU_ACTION (*menu_option[TEXT_MENU_OPTIONS])();

TEXT_MENU_ACTION RunEmu() {
	if(!Running)
	{
		if(SysInit() == -1) 
		{
			return ERR_SYSTEM_INIT;
		}
		OpenPlugins();
		SysReset();

		if(CheckCdrom() == -1 || LoadCdrom() == -1)
		{
			ClosePlugins();
			return ERR_CDROM;
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
				return ERR_CDROM;
			}
		}
	}
	Running 	= 1;
	NeedReset 	= 0;
	VIDEO_WaitVSync();
	return RUN_GAME;
}

TEXT_MENU_ACTION Run() {
	clrscr();
	int ret;
	FILE *f = NULL;
	char bios[256];
	sprintf (bios,"%s%s",Config.BiosDir, Config.Bios);
	f = fopen(bios, "rb");
	if(!f) {
		ret = ERR_BIOS;
	}
	else
	{
		fclose(f);
		ret = RunEmu();
	}

	return ret;
}

TEXT_MENU_ACTION Reset() {
	NeedReset = 1;
	return Run();
}

TEXT_MENU_ACTION SelectGame() {

	int ret = GameBrowser();
	if( ret == 0 ) {
		return Reset();
	}
	else if( ret < 0 ) {
		return ERR_BROWSER;
	}

	clrscr();
	return UPDATE_MENU;
}

TEXT_MENU_ACTION Configure() {
	Config_menu();
	clrscr();
	return UPDATE_MENU;
}

TEXT_MENU_ACTION Exit() {
	to_loader();
}

TEXT_MENU_ACTION SaveState() {
	if(on_states_save() == ERR_SSTATE_SAVE)
		return ERR_SSTATE_SAVE;

	clrscr();
	return UPDATE_MENU;
}

TEXT_MENU_ACTION LoadState() {
	if(on_states_load() == ERR_SSTATE_LOAD)
	{
		clrscr();
		return ERR_SSTATE_LOAD;
	}
	else 
		return RUN_GAME;
}

TEXT_MENU_ACTION menu_null() {
	
}

TEXT_MENU_ACTION (*menu_option[TEXT_MENU_OPTIONS])() = {
	Run
,	Reset
,	SelectGame
,	Configure

#if SAVE_STATE
,	SaveState
,	LoadState
#endif

#ifdef HW_RVL
,	Exit
#endif
};

TEXT_MENU_OPTION print_option(int option, int color) {
	printf("\x1b[%um", color);

	printf("\t%s\n", str_options[option]);
}

TEXT_MENU_OPTION print_option_device(int option, int color) {
	printf("\x1b[%um", color);

	printf("\t%s%s\n", str_options[option], devicename[Settings.device]);
}

TEXT_MENU_OPTION (*print_option_str[TEXT_MENU_OPTIONS])(int, int);

void Main_menu()
{
	u8 index = 0;
	int action = UPDATE_MENU;
	char *msg = NULL;

	int i;
	for(i = 0; i < TEXT_MENU_OPTIONS; i++)
		print_option_str[i] = print_option;

	print_option_str[2] = print_option_device;

	clrscr();
	while(1)
	{
		CHECK_POWER_BUTTONS();
		if(GetHeld(UP, UP, UP))
		{
			if(index) index--;
			usleep(150000);
			action = UPDATE_MENU;
		}

		if(GetHeld(DOWN, DOWN, DOWN))
		{
			if(index < TEXT_MENU_OPTIONS-1) 
				index++;
			usleep(150000);
			action = UPDATE_MENU;
		}

		if(GetInput(RIGHT, RIGHT, RIGHT))
		{
			switch(index)
			{
				case 2: 
					if(Settings.device < DEVICES_COUNT-1) Settings.device++;
					msg = NULL;
					action = UPDATE_MENU;
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
					action = UPDATE_MENU;
					clrscr();
					break;
			}
		}

		if(GetInput(A, A, A)) 
		{
			action = menu_option[index]();
			if(action == RUN_GAME) return;
			msg = errors[action];
		}

		if(action != NO_ACTION)
		{
			action = NO_ACTION;
			printf("\x1B[2;2H");

			printf("\x1b[33m");
			printf("\tMain menu\n\n");

			for(i = 0; i < TEXT_MENU_OPTIONS; i++)
				print_option_str[i]( i, (index == i ? 32 : 37) );

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
