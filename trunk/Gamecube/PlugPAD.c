/* 	
	Basic Analog PAD plugin for PCSX Gamecube
	by emu_kidid based on the DC/MacOSX HID plugin
	
	TODO: Rumble?
*/

#include <ogc/pad.h>
#include "plugins.h"
#include "PsxCommon.h"
//#include "PSEmu_Plugin_Defs.h"

#include "pad.h"
#include "gcMisc.h"

/* Button Bits */
#define PSX_BUTTON_TRIANGLE ~(1 << 12)
#define PSX_BUTTON_SQUARE 	~(1 << 15)
#define PSX_BUTTON_CROSS	~(1 << 14)
#define PSX_BUTTON_CIRCLE	~(1 << 13)
#define PSX_BUTTON_L2		~(1 << 8)
#define PSX_BUTTON_R2		~(1 << 9)
#define PSX_BUTTON_L1		~(1 << 10)
#define PSX_BUTTON_R1		~(1 << 11)
#define PSX_BUTTON_SELECT	~(1 << 0)
#define PSX_BUTTON_START	~(1 << 3)
#define PSX_BUTTON_DUP		~(1 << 4)
#define PSX_BUTTON_DRIGHT	~(1 << 5)
#define PSX_BUTTON_DDOWN	~(1 << 6)
#define PSX_BUTTON_DLEFT	~(1 << 7)

#define PAD_PORT() \
		if(pads[0].type != pads[1].type && (pads[0].type == 0 !| pads[1].type == 0)) 0 \
		else 1

long PadFlags = 0;

long PAD__init(long flags) {
	SysPrintf("start PAD_init()\r\n");
	PadFlags |= flags;
	/* Read Configuration here */
	SysPrintf("end PAD_init()\r\n");
	return PSE_PAD_ERR_SUCCESS;
}

long PAD__shutdown(void) {
	return PSE_PAD_ERR_SUCCESS;
}

long PAD__open(void) {
	return PSE_PAD_ERR_SUCCESS;
}

long PAD__close(void) {
	return PSE_PAD_ERR_SUCCESS;
}

extern struct_pad pads[2];

static long read_keys(int port, PadDataS* pad)
{
	CHECK_POWER_BUTTONS();
	u32 b;
	uint16_t pad_status = 0xFFFF;				//bit pointless why is this done this way?
	struct_pad *cpad 	= &pads[port];
	int wii_port = cpad->num;
#ifdef HW_RVL
	WPADData *data;

	if(pads[0].type != pads[1].type && (pads[0].type == 0 || pads[1].type == 0)) wii_port = 0; // If Wii Remote and GC pad, then we must read from 0 on both.

	if(cpad->type)
	{
		data = WPAD_Data(wii_port);
		b = WPAD_ButtonsHeld(wii_port);
	}
	else
#endif	
		b = PAD_ButtonsHeld(wii_port);
	
	if (b & cpad->START)
		pad_status &= PSX_BUTTON_START;
	if (b & cpad->SELECT)
		pad_status &= PSX_BUTTON_SELECT;
	if (b & cpad->CROSS)
		pad_status &= PSX_BUTTON_CROSS;
	if (b & cpad->CIRCLE)
		pad_status &= PSX_BUTTON_CIRCLE;
	if (b & cpad->SQUARE)
		pad_status &= PSX_BUTTON_SQUARE;
	if (b & cpad->TRIANGLE)
		pad_status &= PSX_BUTTON_TRIANGLE;
		
	if (b & cpad->R2)
		pad_status &= PSX_BUTTON_R2;
	if (b & cpad->L2)
		pad_status &= PSX_BUTTON_L2;
	if (b & cpad->R1)
		pad_status &= PSX_BUTTON_R1;
	if (b & cpad->L1)
		pad_status &= PSX_BUTTON_L1;

#ifdef HW_RVL
	if(data->exp.type == WPAD_EXP_NUNCHUK && cpad->analog == 4)
	{
		if(data->exp.nunchuk.js.pos.y > 140)
			pad_status &= PSX_BUTTON_DUP;
		if(data->exp.nunchuk.js.pos.y < 110)
			pad_status &= PSX_BUTTON_DDOWN;
		if(data->exp.nunchuk.js.pos.x < 110)
			pad_status &= PSX_BUTTON_DLEFT;
		if(data->exp.nunchuk.js.pos.x > 140)
			pad_status &= PSX_BUTTON_DRIGHT;
	}
	else
#endif
	{
		if (b & cpad->UP)
			pad_status &= PSX_BUTTON_DUP;
		if (b & cpad->DOWN)
			pad_status &= PSX_BUTTON_DDOWN;
		if (b & cpad->LEFT)
			pad_status &= PSX_BUTTON_DLEFT;
		if (b & cpad->RIGHT)
			pad_status &= PSX_BUTTON_DRIGHT;
	}

	if (b & cpad->MENU)
	{
		ClosePlugins();
		SysRunGui();
	}
					
	
	if(cpad->analog == 7)
	{
		switch(cpad->type)
		{
			case 0:														// GC Pad
				pad->leftJoyX  = (u8)(PAD_StickX(wii_port)+128);
				pad->leftJoyY  = (u8)(PAD_StickY(wii_port)+128);
				pad->rightJoyX = (u8)(PAD_SubStickX(wii_port)+128);
				pad->rightJoyY = (u8)(PAD_SubStickY(wii_port)+128);
				break;
#ifdef HW_RVL			
			case 1:														// Wiimote
				pad->controllerType = 4;								// Standart Pad
				break;

			case 2:
				if(data->exp.type == WPAD_EXP_NUNCHUK)
				{
					//TODO: Check this
					gforce_t gforce;
					WPAD_GForce(wii_port, &gforce);
					pad->leftJoyX 	= data->exp.nunchuk.js.pos.x;
					pad->leftJoyY 	= data->exp.nunchuk.js.pos.y;
					pad->rightJoyX 	= gforce.x;
					pad->rightJoyY 	= gforce.y;
				}
				else
				{
					pad->leftJoyX  = (u8)(data->exp.classic.ljs.pos.x+128);
					pad->leftJoyY  = (u8)(data->exp.classic.ljs.pos.y+128);
					pad->rightJoyX = (u8)(data->exp.classic.rjs.pos.x+128);
					pad->rightJoyY = (u8)(data->exp.classic.rjs.pos.y+128);
				}
				break;
#endif
		}
	}
	pad->controllerType = cpad->analog;
	pad->buttonStatus = pad_status;									// Copy Buttons
	return PSE_PAD_ERR_SUCCESS;
}

long PAD__readPort1(PadDataS* pad) {
	return read_keys(0, pad);
}

long PAD__readPort2(PadDataS* pad) {
	return read_keys(1, pad);
}
