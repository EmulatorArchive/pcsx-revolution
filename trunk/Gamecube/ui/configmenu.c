#include "textmenu.h"
#include "../Config.h"
#include "../pad.h"

static void ConfigurePAD(struct_pad *pad);
static void ConfigureSPU();
static void ConfigureGPU();

static u32 set_button(char msg[15], int type, int pad_num);

//extern struct_pad pads[2];

void Config_menu()
{
	int index 	= 0;
	
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
			if(index < 4) index++;
		}
			
		if(GetInput(A, A, A)) 
		{
			switch(index)
			{				
				case 0:
					ConfigurePAD(&pads[0]);
					break;

				case 1:
					ConfigurePAD(&pads[1]);
					break;

				case 2:
					ConfigureGPU();
					break;

				case 3:
					ConfigureSPU();
					break;

				case 4:
					SaveConfig();
					return;
			}
			clrscr();
		}
		
		if(GetInput(B, B, B)) 
		{
			return;
		}

		printf("\x1B[2;2H");
	
		printf("\x1b[33m");
		printf("\tConfig menu\n\n");

		printf("\x1b[%um", (index == 0) ? 32 : 37);
		printf("\tConfigure pad1\n");

		printf("\x1b[%um", (index == 1) ? 32 : 37);
		printf("\tConfigure pad2\n");

		printf("\x1b[%um", (index == 2) ? 32 : 37);
		printf("\tConfigure GPU\n");

		printf("\x1b[%um", (index == 3) ? 32 : 37);
		printf("\tConfigure SPU\n");

		printf("\x1b[%um", (index == 4) ? 32 : 37);
		printf("\tReturn\n\n");

		printf("\x1b[37m");								// Reset Color

		VIDEO_WaitVSync();
	}
}

static void ConfigurePAD(struct_pad *pad)
{
	int index 	= 0;
	int device 	= pad->type ? pad->type : 0;
	
	char *thirddevice = NULL;

#ifdef HW_RVL
	WPADData *data;
#endif
	clrscr();
	while(1)
	{
		CHECK_POWER_BUTTONS();
#ifdef HW_RVL
		data 	= WPAD_Data(pad->num);
		if(data->exp.type == WPAD_EXP_NUNCHUK) thirddevice = "Wiimote + nunchak";
		if(data->exp.type == WPAD_EXP_CLASSIC) thirddevice = "Classic controller";
#endif
		if(GetInput(UP, UP, UP))
		{
			if(index) index--;
		}
		if(GetInput(DOWN, DOWN, DOWN))
		{
			if(index < 3) index++;
		}
		if(GetInput(RIGHT, RIGHT, RIGHT))
		{
			clrscr();
			switch(index)
			{
				case 0: 
					if(thirddevice != NULL)
					{
						if(device < 2) device++;
					}
					else 
					{
						if(device == 0) device++;
					}
					pad->type = device;
					break;

				case 2: 
					if(pad->analog == 4) pad->analog = 7;
			}
		}
		
		if(GetInput(LEFT, LEFT, LEFT))
		{
			clrscr();
			switch(index)
			{
				case 0: 
					if(device) device--;
					pad->type = device;
					break;
						
				case 2: 
					if(pad->analog == 7) pad->analog = 4;
			}
		}
			
		if(GetInput(A, A, A)) 
		{
			clrscr();

			switch(index)
			{								
				case 1: 
					if(!pad->type)									// GC Pad
					{
						init_gc_pad(pad);
					}
#ifdef HW_RVL
					else if(data->exp.type == WPAD_EXP_CLASSIC)		// Classic controller
					{
						init_classic_controller(pad);
					}
					else											// Wiimote / Wiimote + nunchak
					{
						init_wiimote(pad);
					}
#endif
					pad->START 		= set_button("Start", pad->type, pad->num);
					pad->SELECT 	= set_button("Select", pad->type, pad->num);
					pad->CROSS 		= set_button("Cross", pad->type, pad->num);
					pad->SQUARE 	= set_button("Square", pad->type, pad->num);
					pad->CIRCLE 	= set_button("Circle", pad->type, pad->num);
					pad->TRIANGLE 	= set_button("Triangle", pad->type, pad->num);
					pad->L1 		= set_button("L1", pad->type, pad->num);
					pad->L2 		= set_button("L2", pad->type, pad->num);
					pad->R1 		= set_button("R1", pad->type, pad->num);
					pad->R2 		= set_button("R2", pad->type, pad->num);
					pad->MENU 		= set_button("MENU", pad->type, pad->num);
					
					clrscr();
					break;
					
				case 3:
					PADWriteConfig();
					return;
					break;
			}
		}
		
		if(GetInput(B, B, B)) 
		{
			return;
		}

		printf("\x1B[2;2H");

		printf("\x1b[33m");
		printf("\tPad configuration menu\n\n");

		printf("\x1b[%um", (index == 0) ? 32 : 37); 	// Set Color
		printf("\tSelect input type: ");

		switch(device)
		{
			case 0: 
				printf("GC pad");
				break;
			
			case 1:
				printf("Wii Remote");
				break;
				
			case 2:
				if(thirddevice != NULL)
					printf("%s", thirddevice);
				else device--;
				break;
		}

		printf("\x1b[%um", (index == 1) ? 32 : 37);
		printf("\n\tConfigure buttons\n" );

		printf("\x1b[%um", (index == 2) ? 32 : 37);
		printf("\tType: %s\n", pad->analog == 7 ? "analog" : "standart");

		printf("\x1b[%um", (index == 3) ? 32 : 37);
		printf("\tBack\n\n");

		if(pad->analog == 7)
		{
			printf("\x1b[36m");
			printf("\tInput may be broken in some games.");
		}
		
		printf("\x1b[37m");								// Reset Color

		VIDEO_WaitVSync();
	}
}

static void ConfigureGPU()
{
	int index 	= 0;
	GPUReadConfig();
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
			if(index < 4) index++;
		}

		if(GetInput(A, A, A)) 
		{
			clrscr();

			switch(index)
			{								
				case 0: 
					Settings.GPU.ShowFPS ^= 1;
					break;

				case 1: 
					Settings.GPU.FrameSkip ^= 1;
					break;

				case 2: 
					Settings.GPU.LimitFPS ^= 1;
					break;

				case 3: 
					if(Settings.GPU.Dithering < 2)
						Settings.GPU.Dithering++;
					else
						Settings.GPU.Dithering = 0;
					break;

				case 4:
					GPUWriteConfig();
					return;
					break;
			}
		}

		if(GetInput(B, B, B)) 
		{
			return;
		}

		printf("\x1B[2;2H");

		printf("\x1b[33m");
		printf("\tGPU config\n\n");

		printf("\x1b[%um", (index == 0) ? 32 : 37);
		printf("\tShow FPS: %s\n", Settings.GPU.ShowFPS ? "yes" : "no");

		printf("\x1b[%um", (index == 1) ? 32 : 37);
		printf("\tEnable Frame Skip: %s\n", Settings.GPU.FrameSkip ? "yes" : "no");

		printf("\x1b[%um", (index == 2) ? 32 : 37);
		printf("\tLimit FPS: %s\n", Settings.GPU.LimitFPS ? "yes" : "no");

		printf("\x1b[%um", (index == 3) ? 32 : 37);
		printf("\tDithering: %s\n", Settings.GPU.Dithering < 2 ? ( Settings.GPU.Dithering == 0 ? "off (fastest)" : "Game depending") : "Always");

		printf("\x1b[%um", (index == 4) ? 32 : 37);
		printf("\n\tBack\n\n");

		if(Settings.GPU.ShowFPS)
		{
			printf("\x1b[36m");
			printf("\tCan cause Code Dump in some circumstances.");
		}

		printf("\x1b[37m");								// Reset Color

		VIDEO_WaitVSync();
	}
}

static void ConfigureSPU()
{
	int index 	= 0;
	SPUReadConfig();
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
			if(index < 8) index++;
		}

		if(GetInput(A, A, A)) 
		{
			clrscr();

			switch(index)
			{								
				case 0: 
					if(Settings.SPU.Volume < 4)
						Settings.SPU.Volume++;
					else
						Settings.SPU.Volume = 0;
					break;

				case 1: 
					if(Settings.SPU.Reverb < 2)
						Settings.SPU.Reverb++;
					else
						Settings.SPU.Reverb = 0;
					break;

				case 2: 
					if(Settings.SPU.Interpolation < 3)
						Settings.SPU.Interpolation++;
					else
						Settings.SPU.Interpolation = 0;
					break;

				case 3: 
					Settings.SPU.EnableXA ^= 1;
					break;

				case 4: 
					Settings.SPU.XAPitch ^= 1;
					break;

				case 5: 
					Settings.SPU.CompatMode = Settings.SPU.CompatMode ? 0 : 2;
					break;

				case 6: 
					Settings.SPU.SPU_IRQ_Wait ^= 1;
					break;

				case 7: 
					Settings.SPU.SingleChMode ^= 1;
					break;

				case 8:
					SPUWriteConfig();
					return;
					break;
			}
		}

		if(GetInput(B, B, B)) 
		{
			return;
		}

		printf("\x1B[2;2H");

		printf("\x1b[33m");
		printf("\tGPU config\n\n");

		printf("\x1b[%um", (index == 0) ? 32 : 37);
		printf("\tVolume: %d\n", Settings.SPU.Volume);

		printf("\x1b[%um", (index == 1) ? 32 : 37);
		printf("\tReverb: %d\n", Settings.SPU.Reverb);

		printf("\x1b[%um", (index == 2) ? 32 : 37);
		printf("\tInterpolation: %d\n", Settings.SPU.Interpolation);

		printf("\x1b[%um", (index == 3) ? 32 : 37);
		printf("\tEnable XA: %s\n", Settings.SPU.EnableXA ? "yes" : "no");

		printf("\x1b[%um", (index == 4) ? 32 : 37);
		printf("\tAdjust XA speed: %s\n", Settings.SPU.XAPitch ? "yes" : "no");

		printf("\x1b[%um", (index == 5) ? 32 : 37);
		printf("\tCompatible mode: %s\n", Settings.SPU.CompatMode ? "yes" : "no");

		printf("\x1b[%um", (index == 6) ? 32 : 37);
		printf("\tSPU IRQ Wait: %s\n", Settings.SPU.SPU_IRQ_Wait ? "yes" : "no");

		printf("\x1b[%um", (index == 7) ? 32 : 37);
		printf("\tSingle channel: %s\n", Settings.SPU.SingleChMode ? "yes" : "no");

		printf("\x1b[%um", (index == 8) ? 32 : 37);
		printf("\n\tBack\n\n");

		printf("\x1b[37m");								// Reset Color

		VIDEO_WaitVSync();
	}
}

u32 set_button(char msg[15], int type, int pad_num)
{
	VIDEO_WaitVSync();
	u32 b;
	clrscr();
	SysPrintf("\tRelease all buttons");
	
#ifdef HW_RVL
	if(type)												// All except GC pad
	{
		while(WPAD_ButtonsHeld(pad_num)) 
		{
			CHECK_POWER_BUTTONS();
			VIDEO_WaitVSync();
		}
		
		clrscr();
		SysPrintf("\tPress button for %s", msg);
		
		while(!WPAD_ButtonsDown(pad_num)) 
		{
			CHECK_POWER_BUTTONS();
			VIDEO_WaitVSync();
		}

		b = WPAD_ButtonsHeld(pad_num);
		while(WPAD_ButtonsHeld(pad_num) & b)
		{
			if(WPAD_ButtonsHeld(pad_num) > b)
			{
				b = WPAD_ButtonsHeld(pad_num);
				break;
			}
			CHECK_POWER_BUTTONS();
			VIDEO_WaitVSync();
		}
	}
	else
#endif
	{
		while(PAD_ButtonsHeld(pad_num)) 
		{
			CHECK_POWER_BUTTONS();
			VIDEO_WaitVSync();
		}
		
		clrscr();
		SysPrintf("\tPress button for %s", msg);
		
		while(!PAD_ButtonsDown(pad_num)) 
		{
			CHECK_POWER_BUTTONS();
			VIDEO_WaitVSync();
		}

		b = PAD_ButtonsHeld(pad_num);
		while(PAD_ButtonsHeld(pad_num) & b)
		{
			if(PAD_ButtonsHeld(pad_num) > b)
			{
				b = PAD_ButtonsHeld(pad_num);
				break;
			}
			CHECK_POWER_BUTTONS();
			VIDEO_WaitVSync();
		}
	}
	VIDEO_WaitVSync();
	return b;
}
