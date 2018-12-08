#include <dfs.h>
#include <dfs_fs.h>
#include <dfs_file.h>
#include "dfs_private.h"


const struct dfs_filesystem_ops *filesystem_operation_table[DFS_FILESYSTEM_MAX];

struct dfs_filesystem filesystem_table[DFS_FILESYSTEM_MAX];

char working_directory[DFS_PATH_MAX] = {"/"};

struct dfs_fd fd_table[DFS_FD_MAX];

static struct rt_mutex fslock;


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



