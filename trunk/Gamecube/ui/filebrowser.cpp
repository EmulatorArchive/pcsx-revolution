/*  PCSX-Revolution - PS Emulator for Nintendo Wii
 *  Copyright (C) 2009-2010  PCSX-Revolution Dev Team
 *
 *  PCSX-Revolution is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public 
 *  License as published by the Free Software Foundation, either 
 *  version 2 of the License, or (at your option) any later version.
 *
 *  PCSX-Revolution is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with PCSX-Revolution.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "textmenu.h"
#include <string.h>
#include <stdio.h>
#include "storage/wiismb.h"
#include "storage/wiifat.h"
#include "storage/wiidvd.h"
#include "storage/mount.h"
#include "gcMisc.h"
#include "Config.h"

#include <dirent.h>

#define TYPE_FILTER(x)	(strstr(x, ".bin") || strstr(x, ".BIN")   \
			|| strstr(x, ".iso") || strstr(x, ".ISO") \
			|| strstr(x, ".mdf") || strstr(x, ".MDF") \
			|| strstr(x, ".img") || strstr(x, ".IMG"))

typedef enum {
	BROWSER_FILE_NOT_FOUND	= -1,
	BROWSER_FILE_SELECTED	= 0,
	BROWSER_CANCELED	= 1,
	BROWSER_CHANGE_FOLDER	= 2
} ret_action;

typedef struct {
	char name[MAXPATHLEN];
	bool is_dir;
} dir_ent;

static int const per_page = 20;

typedef struct {
	char title[30];
	char path[MAXPATHLEN];
	int filter;
} file_browser_st;

static void browse_back(char *str){
	int length = strlen(str);
	int idx;
	for( idx = length; idx > 0; --idx ) {
		char ch = str[idx];
		str[idx] = '\0';
		if( ch == '/' ) {
			if( str[idx-1] == ':' )		// root folder.
				str[idx] = '/';
			break;
		}
	}
}

static ret_action textFileBrowser(file_browser_st *file_struct){
	// Set everything up to read
	DIR* dp = opendir(file_struct->path);

	if(!dp)
		return BROWSER_FILE_NOT_FOUND;

	struct dirent *entry;

	int num_entries = 1, i = 0;
	dir_ent* dir = (dir_ent*) malloc( num_entries * sizeof(dir_ent) );
	// Read each entry of the directory
	while( (entry = readdir(dp)) != NULL ){

		if(((entry->d_type == DT_DIR) && strcmp(entry->d_name, ".") != 0) || (file_struct->filter ? TYPE_FILTER(entry->d_name) : 1) )
		{
			// Make sure we have room for this one
			if(i == num_entries){
				++num_entries;
				dir = (dir_ent*) realloc( dir, num_entries * sizeof(dir_ent) ); 
			}
			strcpy(dir[i].name, entry->d_name);
			dir[i].is_dir = entry->d_type == DT_DIR;
			++i;
		}
	}

	closedir(dp);

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
		else if(GetHeld(DOWN, DOWN, DOWN))
		{
			if(index < num_entries - 1) index++;
			usleep(150000);
			draw = 1;
		}
		else if(GetInput(LEFT, LEFT, LEFT))
		{
			index = 0;
			draw = 1;
		}
		else if(GetInput(RIGHT, RIGHT, RIGHT))
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

			bool is_dir = dir[index].is_dir;
			free(dir);

			return is_dir ? BROWSER_CHANGE_FOLDER : BROWSER_FILE_SELECTED;
		}
		else if(GetInput(B, B, B)) 
		{
			free(dir);
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
				printf("\t%s\t%s\n", dir[temp].is_dir ? "DIR" : "   ", dir[temp].name);
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
,	MountDVD
#ifdef HW_RVL
,	ConnectShare
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

        DIR *dp = NULL;

	if(Settings.filename) {
		strcpy(game_filename.path, Settings.filename);
		browse_back(game_filename.path);	// delete game filename
                dp = opendir(game_filename.path);
	}

	if(dp) {
                closedir(dp);
	}
	else {
		sprintf(game_filename.path, "%s/%s", device[Settings.device], "pcsx-r/games");
                dp = opendir(game_filename.path);
		if(dp) 
                        closedir(dp);
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
