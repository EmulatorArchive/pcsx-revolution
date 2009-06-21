#include <di/di.h>
#include <iso9660.h>
#include <unistd.h>
#include "mount.h"

static const short retries = 20;

int MountDVD()
{
	int x = retries;
	DI_Mount();
	while (DI_GetStatus() & DVD_INIT) usleep(5000);
	while (!(DI_GetStatus() & DVD_READY) && x--) usleep(50000);
	if (DI_GetStatus() & DVD_READY) return ISO9660_Mount();
	return -1;
}

void UnMountDVD()
{
	DI_StopMotor();
	ISO9660_Unmount();
	DI_Close();
}

void InitDVD()
{
	#ifdef HW_RVL
	DI_Init();
	#endif
}
