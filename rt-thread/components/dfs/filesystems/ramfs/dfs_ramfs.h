#ifndef _DFS_RAMFS_H_
#define _DFS_RAMFS_H_

#include <rtthread.h>
#include <rtservice.h>

#define RAMFS_NAME_MAX 32
#define RAMFS_MAGIC 0x0a0a0a0a

struct ramfs_dirent {
	rt_list_t list;
	struct dfs_ramfs *fs;
	char name[RAMFS_NAME_MAX];
	rt_uint8_t *data;
	rt_size_t size;
		
};

struct dfs_ramfs {
	rt_uint32_t magic;
	struct rt_memheap memheap;
	struct ramfs_dirent root;
};

#endif