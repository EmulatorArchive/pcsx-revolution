#include "save_state.h"
#include "plugins.h"
#include "ui/textmenu.h"
#include "r3000a.h"
#include "misc.h"
#include "storage/mount.h"
#include <malloc.h>

static char* get_state_filename (int i) {
	char *state_filename;
	char SStateFile[64];
	char trimlabel[33];
	int j;

	strncpy(trimlabel, CdromLabel, 32);
	trimlabel[32] = 0;
	for (j = 31; j >= 0; j--) {
		if (trimlabel[j] == ' ')
			trimlabel[j] = 0;
		else
			continue;
	}

	sprintf(SStateFile, "%spcsx-r/sstates/%.32s-%.9s.%3.3d", device[Settings.device], trimlabel, CdromId, i);
	state_filename = SStateFile;

	return state_filename;
}

static int state_load (char *state_filename) {
	int ret;
	FILE *fp;

	/* check if the state file actually exists */
	fp = fopen(state_filename, "rb");
	if (fp == NULL) {
		/* file does not exist */
		return -1;
	}
	fclose(fp);

	if (OpenPlugins() == -1) {
		/* TODO Error message */
		//SysRunGui();
		return -1;
	}

	SysReset();

	ret = LoadState(state_filename);
	if (ret == 0) {
		/* Check the CD ROM is valid */
		if (CheckCdrom() == -1 || LoadCdrom() == -1) {
			/* TODO Error message */
			ClosePlugins ();
			//SysRunGui();
			return -1;
		}
		//return psxCpu->Execute();
	}
	else
	{
		ClosePlugins();
		//SysRunGui();
		return -1;
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
		fclose(fp);
		state++;
	}
	return state_save(state_filename);
} 
