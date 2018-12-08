#ifndef __DFS_FS_H__
#define __DFS_FS_H__

#include "dfs.h"

struct dfs_fd;
struct dfs_filesystem;
struct dfs_filesystem_ops {
	char *name;
	uint32_t flags;

	const struct dfs_file_ops *fops;

	int (*mount)(struct dfs_filesystem *fs, unsigned long rwflag, const void *data);
	int (*unmount)(struct dfs_filesystem *fs);

	int (*mkfs)(rt_device_t devid);
	int (*statfs)(struct dfs_filesystem *fs, struct statfs *buf);
	int (*stat)(struct dfs_filesystem *fs, const char *filename, struct stat *buf);
	int (*rename)(struct dfs_filesystem *fs, const char *oldpath, const char *newpath);
};


struct dfs_filesystem {
	rt_device_t dev_id;
	char *path;
	const struct dfs_filesystem_ops *ops;
	void *data;
};

#endif
