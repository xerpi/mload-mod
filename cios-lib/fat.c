#include <string.h>
#include "fat.h"
#include "ipc.h"
#include "syscalls.h"

/* IOCTL commands */
#define IOCTL_FAT_FILESTATS	11

/* IOCTLV commands */
#define IOCTL_FAT_MKDIR		0x01
#define IOCTL_FAT_MKFILE	0x02
#define IOCTL_FAT_READDIR_FS	0x03
#define IOCTL_FAT_READDIR	0x04
#define IOCTL_FAT_DELETE	0x05
#define IOCTL_FAT_DELETEDIR	0x06
#define IOCTL_FAT_RENAME	0x07
#define IOCTL_FAT_STATS		0x08
#define IOCTL_FAT_GETUSAGE_FS	0x09
#define IOCTL_FAT_GETUSAGE	0x0A
#define IOCTL_FAT_MOUNT_SD	0xF0
#define IOCTL_FAT_UMOUNT_SD	0xF1
#define IOCTL_FAT_MOUNT_USB	0xF2
#define IOCTL_FAT_UMOUNT_USB	0xF3
#define IOCTL_FAT_GETPARTITION	0xF4

/* Stats structure */
struct stats {
	/* Size */
	u32 size;

	/* Date and time */
	u16 date;
	u16 time;

	/* Attributes */
	u8 attrib;

	/* Padding */
	u8 pad[3];
};

/* File stats structure */
struct fstats {
	/* Length and position */
	u32 length;
	u32 pos;
};

/* FAT structure */
typedef union {
	char filename[FAT_MAXPATH];

	struct fstats fstats;

	struct {
		u32 partition;
	} mount;

	struct {
		u32 device;
		int partition;
	} get_partition;

	struct {
		char filename[FAT_MAXPATH];
		s32  mode;
	} open;

	struct {
		char filename[FAT_MAXPATH];
		s32  entries;
	} dir;

	struct {
		char oldname[FAT_MAXPATH];
		char newname[FAT_MAXPATH];
	} rename;

	struct {
		char filename[FAT_MAXPATH];
		struct stats stats;
	} stats;

	struct {
		char filename[FAT_MAXPATH];
		u64  size;
		u8   padding0[24];
		u32  files;
		u8   padding1[28];
		u32  dirs;
	} usage;
} ATTRIBUTE_PACKED fatBuf;


s32 FAT_Mount(u32 device, u32 partition)
{
	fatBuf iobuf ATTRIBUTE_ALIGN(32);
	ioctlv vector[1];
	s32 fatFd, ret;

	/* Open FAT module */
	fatFd = os_open("fat", 0);
	if (fatFd < 0)
		return fatFd;

	/* Set command */
	u32 cmd = (device == 0 ? IOCTL_FAT_MOUNT_SD : IOCTL_FAT_MOUNT_USB);

	/* Set partition */
	iobuf.mount.partition = partition;

	/* Setup vector */
	vector[0].data = &iobuf.mount.partition;
	vector[0].len = sizeof(u32);

	/* Flush cache */
	//os_sync_after_write(iobuf, sizeof(*iobuf));

	/* Mount device */
	ret = os_ioctlv(fatFd, cmd, 1, 0, vector);

	/* Close FAT module */
	os_close(fatFd);

	return ret;
}

s32 FAT_GetPartition(u32 device, u32 *partition)
{
	fatBuf iobuf ATTRIBUTE_ALIGN(32);
	ioctlv vector[2];
	s32 fatFd, ret;

	/* Open FAT module */
	fatFd = os_open("fat", 0);
	if (fatFd < 0)
		return fatFd;

	/* Set device */
	iobuf.get_partition.device = device;

	/* Setup vector */
	vector[0].data = &iobuf.get_partition.device;
	vector[0].len = sizeof(u32);
	vector[1].data = &iobuf.get_partition.partition;
	vector[1].len = sizeof(u32);

	/* Flush cache */
	//os_sync_after_write(iobuf, sizeof(*iobuf));

	/* Retrieve  partition */
	ret = os_ioctlv(fatFd, IOCTL_FAT_GETPARTITION, 1, 1, vector);

	/* Invalidate cache */
	//os_sync_before_read(iobuf, sizeof(*iobuf));

	/* Set partition */
	*partition = iobuf.get_partition.partition;

	/* Close FAT module */
	os_close(fatFd);

	return ret;
}

s32 FAT_ReadDir(const char *dirpath, void *outbuf, u32 *entries)
{
	fatBuf iobuf ATTRIBUTE_ALIGN(32);
	ioctlv vector[4];
	u32 in = 1, io = 1;
	s32 fatFd, ret;

	/* Open FAT module */
	fatFd = os_open("fat", 0);
	if (fatFd < 0)
		return fatFd;

	/* Copy path */
	strcpy(iobuf.dir.filename, dirpath);

	/* Setup vector */
	vector[0].data = iobuf.dir.filename;
	vector[0].len  = FAT_MAXPATH;
	vector[1].data = &iobuf.dir.entries;
	vector[1].len  = 4;

	if (outbuf) {
		u32 cnt = *entries;

		/* Input/Output buffers */
		in = io = 2;

		/* Entries value */
		iobuf.dir.entries = cnt;

		/* Setup vector */
		vector[2].data = outbuf;
		vector[2].len  = (FAT_MAXPATH * cnt);
		vector[3].data = &iobuf.dir.entries;
		vector[3].len  = 4;
	}

	/* Flush cache */
	os_sync_after_write(&iobuf, sizeof(iobuf));

	/* Read directory */
	ret = os_ioctlv(fatFd, IOCTL_FAT_READDIR_FS, in, io, vector);

	os_sync_before_read(&iobuf, sizeof(iobuf));

	/* Copy data */
	if (ret >= 0)
		*entries = iobuf.dir.entries;

	return ret;
}
