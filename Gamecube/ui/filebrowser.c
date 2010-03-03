#include "textmenu.h"
#include <sys/dir.h>
#include <string.h>
#include <stdio.h>
#include "../storage/wiismb.h"
#include "../storage/wiifat.h"
#include "../storage/wiidvd.h"
#include "../storage/mount.h"

#define TYPE_FILTER(x)	(strstr(x, ".bin") || strstr(x, ".BIN")   \
						|| strstr(x, ".iso") || strstr(x, ".ISO") \
						|| strstr(x, ".mdf") || strstr(x, ".MDF") \
						|| strstr(x, ".img") || strstr(x, ".IMG"))

typedef enum {
	BROWSER_FILE_NOT_FOUND	= -1,
	BROWSER_FILE_SELECTED	= 0,
	BROWSER_CANCELED		= 1,
	BROWSER_CHANGE_FOLDER	= 2
} ret_action;

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

static void browse_back(char *str){
	int length = strlen(str);
	int idx;
	for( idx = length; idx > 0; idx-- ) {
		char ch = str[idx];
		str[idx] = '\0';
		if( ch == '/' ) {
			if( str[idx-1] == ':' )		// root folder.
				str[idx] = '/';			// Check is here, because it happens only once per call.
			break;
		}
	}
}

static ret_action textFileBrowser(file_browser_st *file_struct){
	// Set everything up to read
	DIR_ITER* dp = diropen(file_struct->path);

	if(!dp)
		return BROWSER_FILE_NOT_FOUND;

	struct stat fstat;
	char filename[MAXPATHLEN];
	int num_entries = 1, i = 0;
	dir_ent* dir = (dir_ent*) malloc( num_entries * sizeof(dir_ent) );
	// Read each entry of the directory
	while( dirnext(dp, filename, &fstat) == 0 ){
		if((strcmp(filename, ".") != 0 && (fstat.st_mode & S_IFDIR)) || (file_struct->filter ? TYPE_FILTER(filename) : 1) )
		{
			// Make sure we have room for this one
			if(i == num_entries){
				++num_entries;
				dir = (dir_ent*) realloc( dir, num_entries * sizeof(dir_ent) ); 
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
	u8 draw = 1;

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
			if(index == 0 && strcmp(dir[index].name, "..") == 0) {
				browse_back(file_struct->path);
			}
			else 
				sprintf(file_struct->path, "%s/%s", file_struct->path, dir[index].name);

			BOOL is_dir = (dir[index].attr & S_IFDIR);
			free(dir);
			if(is_dir) {
				return BROWSER_CHANGE_FOLDER;
			}
			else
				return BROWSER_FILE_SELECTED;
		}
		
		if(GetInput(B, B, B)) 
		{
			return BROWSER_CANCELED;
		}

		if(draw)
		{
			u8 old_page = page;
			page = index / per_page;
			if( old_page != page ) {
				clrscr();
			}
			start = page * per_page;
			limit = ( num_entries > (start + per_page) ) ? ( start + per_page ) : num_entries;
			printf("\x1B[2;2H");
			printf("\x1b[33m");
			printf("\t%s\n\n", file_struct->title);

			printf("\tbrowsing %s\n\n", file_struct->path);

			for(temp = start; temp < limit; temp++)
			{
				printf("\x1b[%um", (index == temp) ? 32 : 37);
				printf("\t%s\t%s\n", (dir[temp].attr & S_IFDIR) ? "DIR" : "   ", dir[temp].name);
			}

			printf("\x1b[37m");
			draw = 0;
		}
		VIDEO_WaitVSync();
	}
}

static mount_state (*mount_dev[DEVICES_COUNT])(int) = {
	MountFAT
,	MountFAT
#ifdef HW_RVL
,	ConnectShare
#ifdef DVD_FIXED
,	MountDVD
#endif
#endif
};

static int MountDevice(int device)
{
	if( Settings.device >= DEVICES_COUNT ) 
		return BROWSER_FILE_NOT_FOUND;

	return mount_dev[Settings.device](Settings.device);
}

int GameBrowser() {
	if( MountDevice(Settings.device) == BROWSER_FILE_NOT_FOUND ) \
		return BROWSER_FILE_NOT_FOUND;
	
	int ret = 0;

	file_browser_st game_filename;
	strcpy(game_filename.title, "Select game image");
	game_filename.filter = 1;

	DIR_ITER *dp = NULL;

	if(Settings.filename) {
		strcpy(game_filename.path, Settings.filename);
		browse_back(game_filename.path);	// delete game filename
		dp = diropen(game_filename.path);
	}

	if(dp) {
		dirclose(dp);
	}
	else {
		sprintf(game_filename.path, "%s/%s", device[Settings.device], "pcsx-r/games");
		dp = diropen(game_filename.path);
		if(dp) 
			dirclose(dp);
		else 
			sprintf(game_filename.path, "%s", device[Settings.device]);
	}

	ret = textFileBrowser(&game_filename);

	while(ret == BROWSER_CHANGE_FOLDER)
	{
		ret = textFileBrowser(&game_filename);
	}
	if (ret == BROWSER_FILE_SELECTED) {
		strcpy(Settings.filename, game_filename.path);
	}
	return ret;
}

char *StateBrowser( )
{
	if( MountDevice(Settings.device) == BROWSER_FILE_NOT_FOUND )
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
