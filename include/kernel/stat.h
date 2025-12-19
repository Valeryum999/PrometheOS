#ifndef _KERNEL_STAT_H
#define _KERNEL_STAT_H

#define S_IFMT   0170000
#define S_IFREG  0100000
#define S_IFDIR  0040000
#define S_IFLNK  0120000
#define S_IFCHR  0020000
#define S_IFBLK  0060000
#define S_IFIFO  0010000
#define S_IFSOCK 0140000

#define S_IRUSR 0400
#define S_IWUSR 0200
#define S_IXUSR 0100
#define S_IRGRP 0040
#define S_IWGRP 0020
#define S_IXGRP 0010
#define S_IROTH 0004
#define S_IWOTH 0002
#define S_IXOTH 0001

#define S_ALL S_IRUSR + S_IWUSR + S_IXUSR + S_IRGRP + S_IWGRP + S_IXGRP + S_IROTH + S_IWOTH + S_IXOTH

#define S_ISUID 04000
#define S_ISGID 02000
#define S_ISVTX 01000

typedef long long unsigned int  dev_t;
typedef unsigned int        	mode_t;
typedef unsigned long       	nlink_t;
typedef unsigned int        	uid_t;
typedef unsigned int        	gid_t;
typedef long                	off_t;
typedef long                	blksize_t;
typedef long unsigned int   	blkcnt_t;
typedef long long unsigned int  ino_t;
typedef long		        	__kernel_long_t;
typedef __kernel_long_t	    	__kernel_old_time_t;

struct timespec {
	__kernel_old_time_t	tv_sec;		/* seconds */
	long			tv_nsec;	/* nanoseconds */
};  

struct stat {
	dev_t st_dev;
	int __st_dev_padding;
	long __st_ino_truncated;
	mode_t st_mode;
	nlink_t st_nlink;
	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	int __st_rdev_padding;
	off_t st_size;
	blksize_t st_blksize;
	blkcnt_t st_blocks;
	struct {
		long tv_sec;
		long tv_nsec;
	} __st_atim32, __st_mtim32, __st_ctim32;
	ino_t st_ino;

	/* These fields are not in the ABI. Their values are */
	/* copied from __st_atim32, __st_mtim32, __st_ctim32 */
	/* accordingly. */

	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;
} __attribute__((packed));

#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12
#define DT_WHT 14

typedef struct linux_dirent {
    ino_t d_ino; \
	off_t d_off; \
	unsigned short d_reclen; \
	unsigned char d_type; \
	char d_name[12];  /* Filename (null-terminated) */
                      /* length is actually (d_reclen - 2 -
                         offsetof(struct linux_dirent, d_name)) */
} linux_dirent;

#endif