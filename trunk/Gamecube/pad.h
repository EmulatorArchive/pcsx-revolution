#ifndef PAD_H
#define PAD_H

#ifdef HW_RVL
#include <wiiuse/wpad.h>
#endif
#include <ogc/pad.h>

enum {
	PAD_STANDARD = 4,
	PAD_ANALOG = 7
} PsxPadType;

enum {
	GCPAD = 0
,	REMOTE
,	NUNCHAK		//  Remote+Nunchak / Classic.
,	CLASSIC = NUNCHAK
} WiiPadType;

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
	
	int num;		// 0 - first; 	 1 - second.
	int type;		// WiiPadType
	int analog;		// PsxPadType
} struct_pad;

extern struct_pad pads[2];

void init_default_pads();
void init_gc_pad(struct_pad *pad);
#ifdef HW_RVL
void init_classic_controller(struct_pad *pad);
void init_wiimote(struct_pad *pad);
#endif

#endif
