#include "pad.h"
#include "gcMisc.h"

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
	// Default GC Pad
	int i = 0;
	for(; i < 2; i++)
	{
		init_gc_pad(&pads[i]);
		
		pads[i].analog 		= 0;
		pads[i].type 		= 0;
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
u32 set_button(char msg[15], int type, int pad_num)
{
	VIDEO_WaitVSync();
	u32 b;
	clrscr();
	SysPrintf("\tRelease all buttons");
	
#ifdef HW_RVL
	if(type)												// All except GC pad
	{
		while(WPAD_ButtonsHeld(pad_num)) VIDEO_WaitVSync();
		
		clrscr();
		SysPrintf("\tPress button for %s", msg);
		
		while(!WPAD_ButtonsDown(pad_num)) VIDEO_WaitVSync();

		b = WPAD_ButtonsHeld(pad_num);
		while(WPAD_ButtonsHeld(pad_num) & b)
		{
			if(WPAD_ButtonsHeld(pad_num) > b)
			{
				b = WPAD_ButtonsHeld(pad_num);
				break;
			}
			VIDEO_WaitVSync();
		}
	}
	else
#endif
	{
		while(PAD_ButtonsHeld(pad_num)) VIDEO_WaitVSync();
		
		clrscr();
		SysPrintf("\tPress button for %s", msg);
		
		while(!PAD_ButtonsDown(pad_num)) VIDEO_WaitVSync();

		b = PAD_ButtonsHeld(pad_num);
		while(PAD_ButtonsHeld(pad_num) & b)
		{
			if(PAD_ButtonsHeld(pad_num) > b)
			{
				b = PAD_ButtonsHeld(pad_num);
				break;
			}
			VIDEO_WaitVSync();
		}
	}
	VIDEO_WaitVSync();
	return b;
}
