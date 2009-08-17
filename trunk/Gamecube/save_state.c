#include "save_state.h"
#include "plugins.h"
#include "ui/textmenu.h"
#include "../R3000A/R3000A.h"
#include "../Misc.h"
#include <malloc.h>

static char* get_state_filename (int i) {
	char *state_filename;
	char SStateFile[64];
	char trimlabel[33];
	int j;

	strncpy(trimlabel, CdromLabel, 32);
	trimlabel[32] = 0;
	for (j = 31; j >= 0; j--)
		if (trimlabel[j] == ' ')
			trimlabel[j] = 0;
		else
			continue;

	sprintf(SStateFile, "sd:/pcsx-r/sstates/%.32s-%.9s.%3.3d", trimlabel, CdromId, i);
	state_filename = SStateFile;

	return state_filename;
}

static void state_load (char *state_filename) {
	int ret;
	FILE *fp;

	/* check if the state file actually exists */
	fp = fopen(state_filename, "rb");
	if (fp == NULL) {
		/* file does not exist */
		return;
	}
	fclose(fp);

	if (OpenPlugins() == -1) {
		/* TODO Error message */
		//SysRunGui();
		return;
	}

	SysReset();

	ret = LoadState(state_filename);
	if (ret == 0) {
		/* Check the CD ROM is valid */
		if (CheckCdrom() == -1) {
			/* TODO Error message */
			ClosePlugins ();
			//SysRunGui();
			return;
		}
		psxCpu->Execute();
	}
	else
	{
		ClosePlugins();
		//SysRunGui();
		return;
	}
}

static int state_save (char *state_filename) {
	return SaveState(state_filename);
}

int on_states_load () {
	char *state_filename = NULL;
	state_filename = StateBrowser();
	if(state_filename != NULL)
	{
		state_load(state_filename);
		return 0;
	}
	return -1;
}

int on_states_save () {
	char *state_filename;
	int state = 0;

	while(1)
	{
		state_filename = get_state_filename(state);
		FILE *fp = fopen(state_filename, "rb");
		if(fp == NULL) break;
		state++;
	}
	return state_save(state_filename);
} 
