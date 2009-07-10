#include "textmenu.h"
#include <sys/dir.h>
#include <string.h>
#include <stdio.h>
#include "../storage/wiismb.h"
#include "../storage/wiifat.h"
#include "../storage/mount.h"

#define TYPE_FILTER(x)	(strstr(x, ".bin") || strstr(x, ".BIN")   \
						|| strstr(x, ".iso") || strstr(x, ".ISO") \
						|| strstr(x, ".mdf") || strstr(x, ".MDF") \
						|| strstr(x, ".img") || strstr(x, ".IMG"))

#ifdef HW_RVL

#define DEVICES			3

static const char *device[DEVICES] = {
	"sd:/"
,	"usb:/"
,	"smb:/"
};

#else	//!HW_RVL

#define DEVICES			2

static const char *device[DEVICES] = {
	"carda:/"
,	"cardb:/"
};

#endif	//HW_RVL

enum {
	BROWSER_CANCELED = -2,
	BROWSER_FILE_NOT_FOUND = -1,
	BROWSER_FILE_CHOSED = 0
};

typedef struct {
	char name[255];
	int  size;
	int  attr;
} dir_ent;

static int const per_page = 20;

typedef struct {
	char title[30];
	char path[255];
	int filter;
} file_browser_st;

static int textFileBrowser(file_browser_st *file_struct){
	// Set everything up to read
	DIR_ITER* dp = diropen(file_struct->path);
	if(!dp)
		return -1;
	struct stat fstat;
	char filename[MAXPATHLEN];
	int num_entries = 1, i = 0;
	dir_ent* dir = malloc( num_entries * sizeof(dir_ent) );
	// Read each entry of the directory
	while( dirnext(dp, filename, &fstat) == 0 ){
		if((strcmp(filename, ".") != 0 && (fstat.st_mode & S_IFDIR)) || (file_struct->filter ? TYPE_FILTER(filename) : 1))
		{
			// Make sure we have room for this one
			if(i == num_entries){
				++num_entries;
				dir = realloc( dir, num_entries * sizeof(dir_ent) ); 
			}
			strcpy(dir[i].name, filename);
			dir[i].size = fstat.st_size;
			dir[i].attr = fstat.st_mode;
			++i;
		}
	}
	
	dirclose(dp);
	
	int index	= 0;
	int temp	= 0;

	u8 page, start, limit;
	short draw = 1;

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
			if(index < num_entries - 1) index++;
			usleep(150000);
			draw = 1;
		}

		if(GetInput(LEFT, LEFT, LEFT))
		{
			index = 0;
			draw = 1;
		}

		if(GetInput(RIGHT, RIGHT, RIGHT))
		{
			index = num_entries - 1;
			draw = 1;
		}

		if(GetInput(A, A, A))
		{
			if(index == num_entries) 
			{
				return 0;
			}

			sprintf(file_struct->path, "%s/%s", file_struct->path, dir[index].name);
			BOOL atr = dir[index].attr & S_IFDIR;
			free(dir);
			if(atr)
				return textFileBrowser(file_struct);
			else
			{
				strcpy(Settings.filename, file_struct->path);
				return 0;
			}
		}
		
		if(GetInput(B, B, B)) 
		{
			return -2;
		}

		if(draw)
		{
			draw = 0;
			printf("\x1B[2;2H");	

			printf("\x1b[33m");
			printf("\t%s\n\n", file_struct->title);
			printf("\tbrowsing %s:\n\n", file_struct->path);

			page = index / per_page;
			start = page * per_page;
			limit = ( num_entries > (start + per_page) ) ? ( start + per_page ) : num_entries;

			for(temp = start; temp < limit; temp++)
			{
				printf("\x1b[%um", (index == temp) ? 32 : 37);
				printf("\t%s\t%s\n", (dir[temp].attr & S_IFDIR) ? "DIR" : "   ", dir[temp].name);
			}

			printf("\x1b[37m");
		}
		VIDEO_WaitVSync();
	}
}

static int MountDevice(int device)
{
	int ret = 0;
	switch(Settings.device)
	{
		case DEVICE_SD:
		case DEVICE_USB:
			ret = MountFAT(Settings.device);
			break;
		case DEVICE_SMB:
			ret = ConnectShare();
			break;
		case DEVICE_DVD:
			ret = MountDVD();
			break;
	}
	return ret;
}

int GameBrowser()
{
	if( MountDevice(Settings.device) == -1 )
		return -1;

	int ret = 0;

	file_browser_st game_filename;
	strcpy(game_filename.title, "Select game image");
	game_filename.filter = 1;
	sprintf(game_filename.path, "%s%s", device[Settings.device], "pcsx-r/games");

	if(ret = textFileBrowser(&game_filename) == BROWSER_FILE_NOT_FOUND)
	{
		sprintf(game_filename.path, "%s", device[Settings.device]);
		ret = textFileBrowser(&game_filename);
	}
	return ret;
}

char *StateBrowser( )
{
	if( MountDevice(Settings.device) == -1 )
		return NULL;

	int ret = 0;
	char *statename;

	file_browser_st state_filename;
	strcpy(state_filename.title, "Select SaveState to load");
	state_filename.filter = 0;
	sprintf(state_filename.path, "%s%s", device[Settings.device], "pcsx-r/sstates");

	if(ret = textFileBrowser(&state_filename) == BROWSER_FILE_NOT_FOUND)
	{
		sprintf(state_filename.path, "%s", device[Settings.device]);
		ret = textFileBrowser(&state_filename);
	}
	if(ret == BROWSER_FILE_NOT_FOUND || ret == BROWSER_CANCELED) 
		return NULL;
	statename = state_filename.path;
	return statename;
}
