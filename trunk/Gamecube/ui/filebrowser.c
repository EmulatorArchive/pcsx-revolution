#include "textmenu.h"
#include <sys/dir.h>

#define TYPE_FILTER(x)	(strstr(x, ".bin")   \
						|| strstr(x, ".iso") \
						|| strstr(x, ".mdf") \
						|| strstr(x, ".img"))

typedef struct {
	char name[255];
	int  size;
	int  attr;
} dir_ent;

int textFileBrowser(char* directory){
	// Set everything up to read
	DIR_ITER* dp = diropen(directory);
	if(!dp){ return -1; }
	struct stat fstat;
	char filename[MAXPATHLEN];
	int num_entries = 1, i = 0;
	dir_ent* dir = malloc( num_entries * sizeof(dir_ent) );
	// Read each entry of the directory
	while( dirnext(dp, filename, &fstat) == 0 && i < 20 ){
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
	
	clrscr();
	while(1)
	{
		CHECK_POWER_BUTTONS();
		if(GetInput(UP, UP, UP))
		{
			if(index) index--; 
		}
		
		if(GetInput(DOWN, DOWN, DOWN))
		{
			if(index <= num_entries - 1) index++; 
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

		printf("\x1B[2;2H");	

		printf("\x1b[33m");
		printf("\tFile browser\n\n");
		printf("\tbrowsing %s:\n\n", directory);

		for(temp = 0; temp < num_entries; temp++)
		{
			printf("\x1b[%um", (index == temp) ? 32 : 37);
			printf("\t%s\t%s\n", (dir[temp].attr & S_IFDIR) ? "DIR" : "   ", dir[temp].name);
		}

		printf("\x1b[%um", (index == num_entries) ? 32 : 37);
		printf("\tReturn to menu\n");
		printf("\x1b[37m");
		VIDEO_WaitVSync();
	}
} 
