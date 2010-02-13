#ifndef __MOUNT_H__
#define __MOUNT_H__

enum {
	DEVICE_SD = 0
,	DEVICE_USB
,	DEVICE_SMB
,	DEVICE_DVD
};

#ifdef HW_RVL

#define DEVICES			3

static const char *device[DEVICES] = {
	"sd:/"
,	"usb:/"
,	"smb:/"
};

#else	//!HW_RVL

#define DEVICES			2

static const char *device[DEVICES] = {
	"carda:/"
,	"cardb:/"
};

#endif	//HW_RVL

#endif
