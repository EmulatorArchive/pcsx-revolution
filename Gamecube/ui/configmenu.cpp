/*  PCSX-Revolution - PS Emulator for Nintendo Wii
 *  Copyright (C) 2009-2010  PCSX-Revolution Dev Team
 *
 *  PCSX-Revolution is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public 
 *  License as published by the Free Software Foundation, either 
 *  version 2 of the License, or (at your option) any later version.
 *
 *  PCSX-Revolution is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with PCSX-Revolution.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "textmenu.h"
#include "Config.h"
#include "gcMisc.h"
#include "PAD/pad.h"

static void ConfigurePAD(pad_t *pad);
static void ConfigureSPU();
static void ConfigureGPU();

static u32 set_button(const char *msg, int type, int pad_num);

#define CONFIG_OPTIONS 6

static const char *opt_yes = "yes";
static const char *opt_no = "no";

static const char *buttons_names[11] = {
	"Start",
	"Select",
	"Cross",
	"Square",
	"Circle",
	"Triangle",
	"L1",
	"L2",
	"R1",
	"R2",
	"MENU"
};

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
		else if(GetInput(DOWN, DOWN, DOWN))
		{
			if(index < CONFIG_OPTIONS-1) index++;
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
					Config.Cpu ^= 1;
					break;

				case 5:
					SaveConfig();
					return;
			}
			clrscr();
		}
		else if(GetInput(B, B, B)) 
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
		printf("\tCPU: %s\n", Config.Cpu ? "Interpreter" : "Recompiler");

		printf("\x1b[%um", (index == 5) ? 32 : 37);

		printf("\n\tReturn\n\n");

		printf("\x1b[37m");								// Reset Color

		VIDEO_WaitVSync();
	}
}

static void ConfigurePAD(pad_t *pad)
{
	u8 index 	= 0;
	u8 device 	= pad->type;
	
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
		if(data->exp.type == WPAD_EXP_NUNCHUK) strcpy(thirddevice, "Wiimote + nunchak");
		if(data->exp.type == WPAD_EXP_CLASSIC) strcpy(thirddevice, "Classic controller");
#endif
		if(GetInput(UP, UP, UP))
		{
			if(index) index--;
		}
		else if(GetInput(DOWN, DOWN, DOWN))
		{
			if(index < 3) index++;
		}
		else if(GetInput(RIGHT, RIGHT, RIGHT))
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
					if(pad->analog != PAD_ANALOG) pad->analog = PAD_ANALOG;
			}
		}
		else if(GetInput(LEFT, LEFT, LEFT))
		{
			clrscr();
			switch(index)
			{
				case 0: 
					if(device) device--;
					pad->type = device;
					break;
						
				case 2: 
					if(pad->analog != PAD_STANDARD) pad->analog = PAD_STANDARD;
			}
		}

		if(GetInput(A, A, A)) 
		{
			clrscr();

			switch(index)
			{
				case 1: 
				{
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
					u8 pad_port = pad->num;
					if(pads[0].type != pads[1].type 
						&& (pads[0].type == GCPAD || pads[1].type == GCPAD))
					{
						pad_port = 0;							// If Wii Remote and GC pad, then we must read from 0 on both.
					}
					pad->START 		= set_button(buttons_names[0], pad->type, pad_port);
					pad->SELECT 	= set_button(buttons_names[1], pad->type, pad_port);
					pad->CROSS 		= set_button(buttons_names[2], pad->type, pad_port);
					pad->SQUARE 	= set_button(buttons_names[3], pad->type, pad_port);
					pad->CIRCLE 	= set_button(buttons_names[4], pad->type, pad_port);
					pad->TRIANGLE 	= set_button(buttons_names[5], pad->type, pad_port);
					pad->L1 		= set_button(buttons_names[6], pad->type, pad_port);
					pad->L2 		= set_button(buttons_names[7], pad->type, pad_port);
					pad->R1 		= set_button(buttons_names[8], pad->type, pad_port);
					pad->R2 		= set_button(buttons_names[9], pad->type, pad_port);
					pad->MENU 		= set_button(buttons_names[10], pad->type, pad_port);
					
					clrscr();
				}
					break;
					
				case 3:
					PADWriteConfig();
					return;
			}
		}
		else if(GetInput(B, B, B)) 
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
			case GCPAD: 
				printf("GC pad");
				break;

			case REMOTE:
				printf("Wii Remote");
				break;

			case CLASSIC:
				if(thirddevice != NULL)
					printf("%s", thirddevice);
				else device--;
				break;
		}

		printf("\x1b[%um", (index == 1) ? 32 : 37);
		printf("\n\tConfigure buttons\n" );

		printf("\x1b[%um", (index == 2) ? 32 : 37);
		printf("\tType: %s\n", pad->analog == PAD_ANALOG ? "analog" : "standard");

		printf("\x1b[%um", (index == 3) ? 32 : 37);
		printf("\n\tBack\n\n");

		if(pad->analog == PAD_ANALOG)
		{
			printf("\x1b[36m");
			printf("\tanalog: Input may be broken in some games.");
		}

		printf("\x1b[37m");								// Reset Color

		VIDEO_WaitVSync();
	}
}

static void ConfigureGPU()
{
	int index = 0;
	GPUReadConfig();
	clrscr();
	while(1)
	{
		CHECK_POWER_BUTTONS();
		
		if(GetInput(UP, UP, UP))
		{
			if(index) index--;
		}
		else if(GetInput(DOWN, DOWN, DOWN))
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
		else if(GetInput(B, B, B)) 
		{
			return;
		}

		printf("\x1B[2;2H");

		printf("\x1b[33m");
		printf("\tGPU config\n\n");

		printf("\x1b[%um", (index == 0) ? 32 : 37);
		printf("\tShow FPS: %s\n", Settings.GPU.ShowFPS ? opt_yes : opt_no);

		printf("\x1b[%um", (index == 1) ? 32 : 37);
		printf("\tEnable Frame Skip: %s\n", Settings.GPU.FrameSkip ? opt_yes : opt_no);

		printf("\x1b[%um", (index == 2) ? 32 : 37);
		printf("\tLimit FPS: %s\n", Settings.GPU.LimitFPS ? opt_yes : opt_no);

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
		else if(GetInput(DOWN, DOWN, DOWN))
		{
			if(index < 9) index++;
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
					Settings.SPU.Disable ^= 1;
					break;

				case 9:
					SPUWriteConfig();
					return;
			}
		}
		else if(GetInput(B, B, B)) 
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
		printf("\tEnable XA: %s\n", Settings.SPU.EnableXA ? opt_yes : opt_no);

		printf("\x1b[%um", (index == 4) ? 32 : 37);
		printf("\tAdjust XA speed: %s\n", Settings.SPU.XAPitch ? opt_yes : opt_no);

		printf("\x1b[%um", (index == 5) ? 32 : 37);
		printf("\tCompatible mode: %s\n", Settings.SPU.CompatMode ? opt_yes : opt_no);

		printf("\x1b[%um", (index == 6) ? 32 : 37);
		printf("\tSPU IRQ Wait: %s\n", Settings.SPU.SPU_IRQ_Wait ? opt_yes : opt_no);

		printf("\x1b[%um", (index == 7) ? 32 : 37);
		printf("\tSingle channel: %s\n", Settings.SPU.SingleChMode ? opt_yes : opt_no);
		
		printf("\x1b[%um", (index == 8) ? 32 : 37);
		printf("\tDisable sound: %s\n", Settings.SPU.Disable ? opt_yes : opt_no);

		printf("\x1b[%um", (index == 9) ? 32 : 37);
		printf("\n\tBack\n\n");

		printf("\x1b[37m");								// Reset Color

		VIDEO_WaitVSync();
	}
}

u32 set_button(const char *msg, int type, int pad_num)
{
	VIDEO_WaitVSync();
	u32 b;
	clrscr();
	printf("\tRelease all buttons");
	
#ifdef HW_RVL
	if(type)												// All except GC pad
	{
		while(WPAD_ButtonsHeld(pad_num)) 
		{
			CHECK_POWER_BUTTONS();
			VIDEO_WaitVSync();
		}
		
		clrscr();
		printf("\tPress button for %s", msg);
		
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
		printf("\tPress button for %s", msg);
		
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
