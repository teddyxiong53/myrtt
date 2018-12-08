#ifndef __DFS_FILE_H__
#define __DFS_FILE_H__

struct dfs_fd;
struct dfs_file_ops {
	int (*open)(struct dfs_fd *fd);
	int (*close)(struct dfs_fd *fd);
	int (*ioctl)(struct dfs_fd *fd, int cmd, void *arg);
	int (*read)(struct dfs_fd *fd, void *buf, size_t count );
	int (*write)(struct dfs_fd *fd, const void *buf, size_t count);
	int (*flush)(struct dfs_fd *fd);
	int (*lseek)(struct dfs_fd *fd, off_t offset);
	int (*getdents)(struct dfs_fd *fd, struct dirent *dirp, uint32_t count);
	//int (*poll)(struct dfs_fd *fd, struct rt_pollreq *req);
};

#define DFS_FD_MAGIC 0xfdfd

struct dfs_fd {
	uint16_t magic;
	uint16_t type;

	char *path;
	int ref_count;

	const struct dfs_file_ops *fops;

	uint32_t flags;
	size_t size;
	off_t pos;

	void *data;
};
#endif