#include <dfs.h>
#include <dfs_fs.h>
#include <dfs_file.h>
#include "dfs_private.h"


const struct dfs_filesystem_ops *filesystem_operation_table[DFS_FILESYSTEM_MAX];

struct dfs_filesystem filesystem_table[DFS_FILESYSTEM_MAX];

char working_directory[DFS_PATH_MAX] = {"/"};

struct dfs_fd fd_table[DFS_FD_MAX];

static struct rt_mutex fslock;

void dfs_lock(void)
{
	rt_err_t result;
	result = rt_mutex_take(&fslock, RT_WAITING_FOREVER);
	
}
void dfs_unlock(void)
{
	rt_mutex_release(&fslock);
}
/*
	return :
	0: ok
	-1: fail
*/
int fd_is_open(const char *pathname)
{
	char *fullpath;
	struct dfs_filesystem *fs;
	struct dfs_fd *fd;
	int index;
	fullpath = dfs_normalize_path(NULL, pathname);
	if(fullpath != NULL) {
		char *mountpath;
		fs = dfs_filesystem_lookup(fullpath);
		if(fs == NULL) {
			rt_free(fullpath);
			return -1;
		}
		if(fs->path[0]=='/' && fs->path[1]=='\0') {
			mountpath = fullpath;
		} else {
			mountpath = fullpath + strlen(fs->path);
		}
		dfs_lock();
		for(index=0; index<DFS_FD_MAX; index++) {
			fd = &(fd_table[index]);
			if(fd->fops == NULL) {
				continue;
			}
			if(fd->fops == fs->ops->fops && (strcmp(fd->path, mountpath) == 0)) {
				rt_free(fullpath);
				dfs_unlock();
				return 0;
			}
		}
		dfs_unlock();
		rt_free(fullpath);
	}
	return -1;
}

int dfs_init()
{
	memset((void *)filesystem_operation_table, 0, sizeof(filesystem_operation_table));
	memset(filesystem_table, 0 ,sizeof(filesystem_table));
	memset(fd_table, 0, sizeof(fd_table));

	rt_mutex_init(&fslock, "fslock", RT_IPC_FLAG_FIFO);
	memset(working_directory, 0, sizeof(working_directory));

	working_directory[0] = '/';

	return 0;
}

INIT_PREV_EXPORT(dfs_init);



