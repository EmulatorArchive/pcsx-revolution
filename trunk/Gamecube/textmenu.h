#ifndef TEXTMENU_H
#define TEXTMENU_H

#include "Config.h"
#include "gcMisc.h"

void Main_menu();
u32 set_button(char msg[15], int type, int pad_num);

typedef struct {
	char name[255];
	int  size;
	int  attr;
} dir_ent;

#endif
