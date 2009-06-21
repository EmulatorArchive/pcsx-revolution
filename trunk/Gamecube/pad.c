#include "pad.h"

struct_pad pads[2];

void init_gc_pad(struct_pad *pad)
{
	pad->LEFT		= PAD_BUTTON_LEFT;
	pad->UP			= PAD_BUTTON_UP;
	pad->DOWN		= PAD_BUTTON_DOWN;
	pad->RIGHT		= PAD_BUTTON_RIGHT;
		
	pad->START 		= PAD_BUTTON_START;
	pad->SELECT 	= PAD_BUTTON_MENU;
	
	pad->MENU 		= PAD_BUTTON_MENU + PAD_BUTTON_START;

	pad->CROSS 		= PAD_BUTTON_A;
	pad->SQUARE 	= PAD_BUTTON_B;
	pad->CIRCLE 	= PAD_BUTTON_X;
	pad->TRIANGLE 	= PAD_BUTTON_Y;

	pad->L1 		= PAD_TRIGGER_L;
	pad->L2 		= PAD_TRIGGER_Z + PAD_TRIGGER_L;
	pad->R1 		= PAD_TRIGGER_R;
	pad->R2 		= PAD_TRIGGER_Z + PAD_TRIGGER_R;
}

void init_default_pads()
{
	// Default Wiimote
	int i = 0;
	for(; i < 2; i++)
	{
#ifdef HW_RVL
		init_wiimote(&pads[i]);
		pads[i].type 		= 1;
#else
		init_gc_pad(&pads[i]);
		pads[i].type 		= 0;
#endif 
		pads[i].analog 		= 4;
		pads[i].num 		= i;
	}
}
#ifdef HW_RVL
void init_classic_controller(struct_pad *pad)
{
	pad->LEFT		= WPAD_CLASSIC_BUTTON_LEFT;
	pad->UP			= WPAD_CLASSIC_BUTTON_UP;
	pad->DOWN		= WPAD_CLASSIC_BUTTON_DOWN;
	pad->RIGHT		= WPAD_CLASSIC_BUTTON_RIGHT;

	pad->START 		= WPAD_CLASSIC_BUTTON_PLUS;
	pad->SELECT 	= WPAD_CLASSIC_BUTTON_MINUS;
	
	pad->MENU 		= WPAD_CLASSIC_BUTTON_HOME;

	pad->CROSS 		= WPAD_CLASSIC_BUTTON_A;
	pad->SQUARE 	= WPAD_CLASSIC_BUTTON_B;
	pad->CIRCLE 	= WPAD_CLASSIC_BUTTON_X;
	pad->TRIANGLE 	= WPAD_CLASSIC_BUTTON_Y;

	pad->L1 		= WPAD_CLASSIC_BUTTON_FULL_L;
	pad->L2 		= WPAD_CLASSIC_BUTTON_ZL;
	pad->R1 		= WPAD_CLASSIC_BUTTON_FULL_R;
	pad->R2 		= WPAD_CLASSIC_BUTTON_ZR;
}

void init_wiimote(struct_pad *pad)
{
	pad->LEFT		= WPAD_BUTTON_UP;
	pad->UP			= WPAD_BUTTON_RIGHT;
	pad->DOWN		= WPAD_BUTTON_LEFT;
	pad->RIGHT		= WPAD_BUTTON_DOWN;
		
	pad->START 		= WPAD_BUTTON_PLUS;
	pad->SELECT 	= WPAD_BUTTON_MINUS;
	
	pad->MENU 		= WPAD_BUTTON_HOME;

	pad->CROSS 		= WPAD_BUTTON_A;
	pad->SQUARE 	= WPAD_BUTTON_B;
	pad->CIRCLE 	= WPAD_BUTTON_1;
	pad->TRIANGLE 	= WPAD_BUTTON_2;

	pad->L1 		= WPAD_BUTTON_A + WPAD_BUTTON_1;
	pad->L2 		= WPAD_BUTTON_A + WPAD_BUTTON_2;
	pad->R1 		= WPAD_BUTTON_B + WPAD_BUTTON_1;
	pad->R2 		= WPAD_BUTTON_B + WPAD_BUTTON_2;
}
#endif
