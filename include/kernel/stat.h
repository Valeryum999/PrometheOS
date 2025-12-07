#ifndef _KERNEL_STAT_H
#define _KERNEL_STAT_H

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
};

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