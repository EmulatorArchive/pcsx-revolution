#ifndef __MOUNT_H__
#define __MOUNT_H__

typedef enum {
	FAT_DEVICE_0 = 0
,	FAT_DEVICE_1
#ifdef HW_RVL
,	DEVICE_SMB
#ifdef DVD_FIXED
,	DEVICE_DVD
#endif
#endif
,	DEVICES_COUNT
,	FAT_DEVICES_COUNT = FAT_DEVICE_1+1
} gekko_device;

typedef enum {
	NOT_MOUNTED = -1,
	MOUNTED = 1
} mount_state;

static const char *device[DEVICES_COUNT] = {
#ifdef HW_RVL
	"sd:/"
,	"usb:/"
,	"smb:/"
#ifdef DVD_FIXED
,	"dvd:/"
#endif
#else
	"carda:/"
,	"cardb:/"
#endif
};

#endif
