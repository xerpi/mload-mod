#ifndef FAT_H
#define FAT_H

#include "types.h"

#define FAT_MAXPATH	256

s32 FAT_Mount(u32 device, u32 partition);
s32 FAT_Umount(u32 device);
s32 FAT_GetPartition(u32 device, u32 *partition);
s32 FAT_ReadDir(const char *dirpath, void *outbuf, u32 *entries);

#endif
