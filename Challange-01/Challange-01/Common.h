#pragma once


#define IOCTL_PRIORITY_BOOSTER_SET_PRIORITY CTL_CODE(0x8000, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)

struct ThreadData {
	ULONG ThreadId;
	int Priority;
};
