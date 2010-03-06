#ifndef PAD_H
#define PAD_H

#ifdef HW_RVL
#include <wiiuse/wpad.h>
#endif
#include <ogc/pad.h>

typedef enum {
	PAD_STANDARD = 4,
	PAD_ANALOG = 7
} PsxPadType;

typedef enum {
	GCPAD = 0
,	REMOTE
,	NUNCHAK		//  Remote+Nunchak / Classic.
,	CLASSIC = NUNCHAK
} WiiPadType;

typedef struct {
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
} pad_t;

extern pad_t pads[2];

#ifdef __cplusplus
extern "C" {
#endif

void init_default_pads();

#ifdef __cplusplus
}
#endif

void init_gc_pad(pad_t *pad);
#ifdef HW_RVL
void init_classic_controller(pad_t *pad);
void init_wiimote(pad_t *pad);
#endif

#endif
