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

#include <network.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <smb.h>

#include "wiismb.h"
#include "Config.h"

static mount_state ShareConnected;
static bool NetworkConnected;

static void CloseShare();
static void NetworkInit();

static void NetworkInit()
{
	// stop if we're already initialized, or if auto-init has failed before
	// in which case, manual initialization is required
	if(NetworkConnected)
		return;

	if(!NetworkConnected) // check again if the network was inited
	{
		char ip[16];
		s32 initResult = if_config(ip, NULL, NULL, true);

		if(initResult == 0)
		{
			NetworkConnected = true;
		}
	}
}

static void CloseShare()
{
	if(ShareConnected)
		smbClose("smb");
	ShareConnected = NOT_MOUNTED;
	NetworkConnected = false; // trigger a network reinit
}

mount_state ConnectShare (int i)
{
	// Crashes or stalls system in GameCube mode - so disable
	#ifndef HW_RVL
	return NOT_MOUNTED;
	#endif

	if( SMBReadConfig() == -1 )
	{
		printf("Config not found");
		return NOT_MOUNTED;
	}
	int chkU = (strlen(Settings.smb.user)	> 0) ? 0 : 1;
	int chkP = (strlen(Settings.smb.pwd)	> 0) ? 0 : 1;
	int chkS = (strlen(Settings.smb.share)	> 0) ? 0 : 1;
	int chkI = (strlen(Settings.smb.ip)		> 0) ? 0 : 1;

	// check that all parameters have been set
	if(chkU + chkP + chkS + chkI > 0)
	{
		char msg[50];
		char msg2[100];
		if(chkU + chkP + chkS + chkI > 1) // more than one thing is wrong
			sprintf(msg, "Check settings.xml.");
		else if(chkU)
			sprintf(msg, "Username is blank.");
		else if(chkP)
			sprintf(msg, "Password is blank.");
		else if(chkS)
				sprintf(msg, "Share name is blank.");
		else if(chkI)
			sprintf(msg, "Share IP is blank.");
			printf("Invalid network settings - %s", msg);
		return NOT_MOUNTED;
	}

	if(ShareConnected != MOUNTED)
		CloseShare();

	if(NetworkConnected != true)
		NetworkInit();

	if( NetworkConnected )
	{
		if(ShareConnected != MOUNTED)
		{
			if( smbInit(Settings.smb.user, Settings.smb.pwd,
					Settings.smb.share, Settings.smb.ip) == true )
			{
				ShareConnected = MOUNTED;
			}
		}
	}
	return ShareConnected;
} 
