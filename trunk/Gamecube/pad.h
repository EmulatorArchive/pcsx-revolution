#ifndef PAD_H
#define PAD_H

#ifdef HW_RVL
#include <wiiuse/wpad.h>
#endif
#include <ogc/pad.h>

typedef struct users_pad {
	u32 UP;
	u32 DOWN;
	u32 LEFT;
	u32 RIGHT;
	
	u32 START;
	u32 SELECT;
	
	u32 SQUARE;
	u32 CROSS;
	u32 CIRCLE;
	u32 TRIANGLE;
	
	u32 L1;
	u32 L2;
	u32 R1;
	u32 R2;
	
	u32 MENU;
	
	int type;		// 0 - pad; 	 1 - Remote; 	2 - Remote+Nunchak / Classic.
	int num;		// 0 - first; 	 1 - second.
	int analog;		// 0 - standart; 1 - analog.
} struct_pad;
struct_pad pads[2];

void init_default_pads();
void init_gc_pad(struct_pad *pad);
#ifdef HW_RVL
void init_classic_controller(struct_pad *pad);
void init_wiimote(struct_pad *pad);
#endif
u32 set_button(char msg[15], int type, int pad_num);

#endif
