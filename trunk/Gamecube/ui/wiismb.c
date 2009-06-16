#include <smb.h>
#include <network.h>
#include <string.h>
#include <stdio.h>

#include "wiismb.h"
#include "../Config.h"

static int ShareConnected;
static int NetworkConnected;

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
			NetworkConnected = 1;
		}
	}
}

static void CloseShare()
{
	if(ShareConnected)
		smbClose("smb");
	ShareConnected = 0;
	NetworkConnected = 0; // trigger a network reinit
}

int ConnectShare ()
{
	// Crashes or stalls system in GameCube mode - so disable
	#ifndef HW_RVL
	return -1;
	#endif

	int ret = 1;

	if( SMBReadConfig() == -1 )
	{
		printf("Config not found");
		usleep(5000000);
		return -1;
	}
	int chkU = (strlen(Settings.smb.user) > 0) ? 0:1;
	int chkP = (strlen(Settings.smb.pwd) > 0) ? 0:1;
	int chkS = (strlen(Settings.smb.share) > 0) ? 0:1;
	int chkI = (strlen(Settings.smb.ip) > 0) ? 0:1;

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
			usleep(5000000);
		return -1;
	}

	if(ShareConnected)
		CloseShare();

	if(!NetworkConnected)
		NetworkInit();

	if(NetworkConnected)
	{
		if(!ShareConnected)
		{
			if( ret = smbInit(Settings.smb.user, Settings.smb.pwd,
					Settings.smb.share, Settings.smb.ip) == 0 )
			{
				ShareConnected = 1;
			}
		}
	}
	return ret;
} 


