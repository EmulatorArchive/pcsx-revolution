#include "textmenu.h"
#include <sys/dir.h>
#include "../storage/wiismb.h"
#include "../storage/wiifat.h"
#include "../storage/mount.h"

#define TYPE_FILTER(x)	(strstr(x, ".bin")   \
						|| strstr(x, ".iso") \
						|| strstr(x, ".mdf") \
						|| strstr(x, ".img"))

#ifdef HW_RVL

#define DEVICES			3

char *device[DEVICES] = {
	"sd:/"
,	"usb:/"
,	"smb:/"
};

#else	//!HW_RVL

#define DEVICES			2

char *device[DEVICES] = {
	"carda:/"
,	"cardb:/"
}

#endif	//HW_RVL


typedef struct {
	char name[255];
	int  size;
	int  attr;
} dir_ent;

static int const per_page = 20;

static int textFileBrowser(char* directory){
	// Set everything up to read
	DIR_ITER* dp = diropen(directory);
	if(!dp){ return -1; }
	struct stat fstat;
	char filename[MAXPATHLEN];
	int num_entries = 1, i = 0;
	dir_ent* dir = malloc( num_entries * sizeof(dir_ent) );
	// Read each entry of the directory
	while( dirnext(dp, filename, &fstat) == 0 ){
		if((strcmp(filename, ".") != 0 && (fstat.st_mode  & S_IFDIR)) || TYPE_FILTER(filename))
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
	
	int index	= (num_entries > 1) ? 2 : 1;
	int temp	= 0;

	unsigned short page, start, limit;
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
			
			if(dir[index].attr & S_IFDIR)
			{
				char newDir[MAXPATHLEN];
				sprintf(newDir, "%s/%s", directory, dir[index].name);
				free(dir);
				return textFileBrowser(newDir);
			}
			else 
			{
				char newDir[MAXPATHLEN];
				sprintf(newDir, "%s/%s", directory, dir[index].name);
				free(dir);
				strcpy(Settings.filename, newDir);
				return 1;
			}
		}
		
		if(GetInput(B, B, B)) 
		{
			return 0;
		}

		if(draw)
		{
			draw = 0;
			printf("\x1B[2;2H");	

			printf("\x1b[33m");
			printf("\tFile browser\n\n");
			printf("\tbrowsing %s:\n\n", directory);

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

int OpenBrowser()
{
	switch(Settings.device)
	{
		case DEVICE_SD:
		case DEVICE_USB:
			if(MountFAT(Settings.device) == -1) 
				return -1;
			break;
		case DEVICE_SMB:
			if(ConnectShare() == -1)
				return -1;
			break;
		case DEVICE_DVD:
			if(MountDVD() == -1)
				return -1;
			break;
	}
	sprintf(Settings.filename, "%s%s", device[Settings.device], "wiisx/games");
	if(textFileBrowser(Settings.filename) == -1)
	{
		sprintf(Settings.filename, "%s", device[Settings.device]);
		return textFileBrowser(Settings.filename);
	}
	return 0;
}
