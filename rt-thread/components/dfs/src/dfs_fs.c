#include <dfs_fs.h>
#include <dfs_file.h>
#include "dfs_private.h"




char *dfs_normalize_path(const char *directory, const char *filename)
{
    char *fullpath;
    char *dst0, *dst, *src;

    /* check parameters */
    RT_ASSERT(filename != NULL);


    if (directory == NULL) /* shall use working directory */
        directory = &working_directory[0];


    if (filename[0] != '/') /* it's a absolute path, use it directly */
    {
        fullpath = rt_malloc(strlen(directory) + strlen(filename) + 2);

        if (fullpath == NULL)
            return NULL;

        /* join path and file name */
        rt_snprintf(fullpath, strlen(directory) + strlen(filename) + 2,
            "%s/%s", directory, filename);
    }
    else
    {
        fullpath = rt_strdup(filename); /* copy string */

        if (fullpath == NULL)
            return NULL;
    }

    src = fullpath;
    dst = fullpath;

    dst0 = dst;
    while (1)
    {
        char c = *src;

        if (c == '.')
        {
            if (!src[1]) src ++; /* '.' and ends */
            else if (src[1] == '/')
            {
                /* './' case */
                src += 2;

                while ((*src == '/') && (*src != '\0'))
                    src ++;
                continue;
            }
            else if (src[1] == '.')
            {
                if (!src[2])
                {
                    /* '..' and ends case */
                    src += 2;
                    goto up_one;
                }
                else if (src[2] == '/')
                {
                    /* '../' case */
                    src += 3;

                    while ((*src == '/') && (*src != '\0'))
                        src ++;
                    goto up_one;
                }
            }
        }

        /* copy up the next '/' and erase all '/' */
        while ((c = *src++) != '\0' && c != '/')
            *dst ++ = c;

        if (c == '/')
        {
            *dst ++ = '/';
            while (c == '/')
                c = *src++;

            src --;
        }
        else if (!c)
            break;

        continue;

up_one:
        dst --;
        if (dst < dst0)
        {
            rt_free(fullpath);
            return NULL;
        }
        while (dst0 < dst && dst[-1] != '/')
            dst --;
    }

    *dst = '\0';

    /* remove '/' in the end of path if exist */
    dst --;
    if ((dst != fullpath) && (*dst == '/'))
        *dst = '\0';

    /* final check fullpath is not empty, for the special path of lwext "/.." */
    if ('\0' == fullpath[0])
    {
        fullpath[0] = '/';
        fullpath[1] = '\0';
    }

    return fullpath;
}

int dfs_mount(
	const char *device_name,
	const char *path,
	const char *filesystemtype,
	unsigned long rwflag,
	const void *data
)
{
	const struct dfs_filesystem_ops **ops;
	struct dfs_filesystem *iter;
	struct dfs_filesystem *fs =NULL;
	char *fullpath = NULL;

	rt_device_t dev_id;
	int i;
	if(device_name == NULL) {
		dev_id = NULL;
	} else if((dev_id = rt_device_find(device_name)) == NULL) {
		rt_set_errno(-ENODEV);
		return -1;
	} 
	dfs_lock();
	for(ops = &filesystem_operation_table[0], i=0;
		ops < &filesystem_operation_table[DFS_FILESYSTEM_MAX];
		ops++, i++) {
		if((*ops != NULL) && (strcmp((*ops)->name, filesystemtype) == 0)) {
			break;
		}
	}
	dfs_unlock();
	if(i>=DFS_FILESYSTEM_MAX) {
		//not found
		rt_set_errno(-ENODEV);
		return -1;
	}
	fullpath = dfs_normalize_path(NULL, path);
	if(fullpath == NULL) {
		rt_set_errno(-ENOTDIR);
		return -1;
	}
	//now check if the filesystem is already mounted or not
	
	dfs_lock();
	for(iter=&filesystem_table[0], i=0;
		iter<&filesystem_table[DFS_FILESYSTEM_MAX];
		iter++,i++) {
		if(iter->ops == NULL) {
			if(fs == NULL) {
				fs = iter;
			}
		} else if(strcmp(iter->path, path)==0){//already mounted this path 
			rt_set_errno(-EINVAL);
			goto err1;
		}
	}
	if((fs ==NULL) && (i>=DFS_FILESYSTEM_MAX)) {
		//not found
		rt_set_errno(-ENOSPC);
		goto err1;
	}

	fs->path = fullpath;
	fs->ops = ops;
	fs->dev_id = dev_id;

	dfs_unlock();
	if(dev_id != NULL) {
		if(rt_device_open(fs->dev_id, RT_DEVICE_OFLAG_RDWR) != RT_EOK) {
			dfs_lock();
			memset(fs, 0, sizeof(struct dfs_filesystem));
			goto err1;
		}
	}
	//now mount 
	if((*ops)->mount(fs, rwflag, data) < 0) {
		if(dev_id != NULL) {
			rt_device_close(fs->dev_id);
		}
		dfs_lock();
		memset(fs, 0, sizeof(struct dfs_filesystem));
		goto err1;
	}
	return 0;
err1:
	dfs_unlock();
	rt_free(fullpath);
	return -1;
}


int dfs_register(
	const struct dfs_filesystem_ops *ops
)
{
	int ret = RT_EOK;
	const struct dfs_filesystem_ops **empty = NULL;
	const struct dfs_filesystem_ops **iter;

	dfs_lock();
	for(iter=&filesystem_operation_table[0];
		iter < &filesystem_operation_table[DFS_FILESYSTEM_MAX];
		iter ++) {
		if(*iter == NULL) {
			if(empty == NULL) {
				empty = iter;
			}
		} else if(strcmp((*iter)->name, ops->name)==0) {
			ret = -1;
			break;
		}
	}
	if(empty == NULL) {
		rt_set_errno(-ENOSPC);
		ret = -1;
	} else if(ret == RT_EOK) {
		*empty = ops;
	}
	dfs_unlock();
	return ret;
}


struct dfs_filesystem *dfs_filesystem_lookup(const char *path)
{
	struct dfs_filesystem *iter; 
	struct dfs_filesystem *fs =NULL;
	uint32_t fspath, prefixlen;
	prefixlen = 0;
	dfs_lock();

	for(iter=&filesystem_table[0];
		iter < &filesystem_table[DFS_FILESYSTEM_MAX]; iter++) {
		if((iter->path == NULL) || (iter->ops == NULL)) {
			continue;
		}
		fspath = strlen(iter->path);
		if((fspath < prefixlen) ||
			(strncmp(iter->path, path, fspath) != 0)) {
			continue;		
		}
		if(fspath > 1 && (strlen(path) > fspath) && (path[fspath]!='/')) {
			continue;
		}
		fs = iter;
		prefixlen = fspath;
	}
	dfs_unlock();
	return fs;
}