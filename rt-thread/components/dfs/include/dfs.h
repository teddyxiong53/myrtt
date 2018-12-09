#ifndef __DFS_H__
#define __DFS_H__


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <rtthread.h>
#include <rtdevice.h>


#define DFS_FILESYSTEM_MAX 1

#define DFS_FD_MAX 4

#define DFS_FD_OFFSET 3 //stdin, stdout, stderr

#define DFS_PATH_MAX  256


#define DFS_FS_FLAG_DEFAULT 0X00
#define DFS_FS_FLAG_FULLPATH 0X01



struct statfs {
	size_t f_bsize;
	size_t f_blocks;
	size_t f_bfree;
};

struct dirent {
	uint8_t d_type;
	uint8_t d_namlen;
	uint16_t d_reclen;
	char d_name[DFS_PATH_MAX];
};



void dfs_lock();
void dfs_unlock();



#endif