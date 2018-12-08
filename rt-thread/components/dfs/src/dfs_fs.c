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

	return RT_EOK;
}

