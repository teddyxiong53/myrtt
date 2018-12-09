#include <dfs.h>
#include <dfs_file.h>
#include <dfs_fs.h>
#include <dfs_private.h>

const char *dfs_subdir(const char *directory, const char *filename)
{
	const char *dir;
	if(strlen(directory) == strlen(filename)) {
		return NULL;
	}
	dir = filename + strlen(directory);
	if((*dir!='/') && (dir!=filename)) {
		dir--;
	}
	return dir;
}

int dfs_file_open(
	struct dfs_fd *fd,
	const char *path,
	int flags
)
{
	struct dfs_filesystem *fs;
	char *fullpath;
	int result;
	if(fd == NULL) {
		return -EINVAL;
	}
	fullpath = dfs_normalize_path(NULL, path);
	if(fullpath = NULL) {
		return -ENOMEM;
	}
	if(fd_is_open(fullpath) == 0) {
		//is open already
		rt_free(fullpath);
		return -EBUSY;
	}
	fs = dfs_filesystem_lookup(fullpath);
	if(fs == NULL) {
		rt_free(fullpath);
		return -ENOENT;
	}
	fd->fops = fs->ops->fops;
	fd->type = FT_REGULAR;
	fd->flags = flags;
	fd->size = 0;
	fd->pos = 0;
	fd->data =fs;
	
	if(!(fs->ops->flags & DFS_FS_FLAG_FULLPATH)) {
		if(dfs_subdir(fs->path, fullpath) == NULL) {
			fd->path = rt_strdup("/");
		} else {
			fd->path = rt_strdup(dfs_subdir(fs->path, fullpath));
		}
		
	} else {
		fd->path = fullpath;
	}
	if(fd->fops->open == NULL) {
		rt_free(fd->path);
		fd->path = NULL;
		return -ENOSYS;
	}
	if((result = fd->fops->open(fd)) < 0) {
		rt_free(fd->path);
		fd->path = NULL;
		return result;
	}
	fd->flags |= DFS_F_OPEN;
	if(flags & O_DIRECTORY) {
		fd->type = FT_DIRECTORY;
		fd->flags |= DFS_F_DIRECTORY;
	}
	return 0;
}

int dfs_file_getdents(struct dfs_fd *fd, 
	struct dirent *dirp,
	size_t nbytes
)
{
	if(fd == NULL || fd->type != FT_DIRECTORY) {
		return -EINVAL;
	}
	if(fd->fops->getdents != NULL) {
		return fd->fops->getdents(fd, dirp, nbytes);
	}
	return -ENOSYS;
}

int dfs_file_stat(const char *path, struct stat *buf)
{
	int result;
	char *fullpath;
	struct dfs_filesystem *fs;

	fullpath = dfs_normalize_path(NULL, path);
	if(fullpath == NULL) {
		return -1;
	}
	if((fs = dfs_filesystem_lookup(fullpath)) == NULL) {
		rt_free(fullpath);
		return -ENOENT;
	}
	if((fullpath[0]=='/') && (fullpath[1]=='\0') 
		|| (dfs_subdir(fs->path, fullpath) == NULL)) {
		//this means it is root dir
		buf->st_dev = 0;
        buf->st_mode  = S_IRUSR | S_IRGRP | S_IROTH |
                        S_IWUSR | S_IWGRP | S_IWOTH;
        buf->st_mode |= S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
		buf->st_size = 0;
		buf->st_mtime = 0;
		rt_free(fullpath);
		return RT_EOK;
	} else {
		if(fs->ops->stat == NULL) {
			rt_free(fullpath);
			return -ENOSYS;
		}
		if(fs->ops->flags & DFS_FS_FLAG_FULLPATH) {
			result = fs->ops->stat(fs, fullpath, buf);
		} else {
			result = fs->ops->stat(fs, dfs_subdir(fs->path, fullpath), buf);
		}
	}
	rt_free(fullpath);
	return result;
		
}


int dfs_file_close(struct dfs_fd *fd)
{
	int result = 0;
	if(fd == NULL) {
		return -ENXIO;
		
	}
	if(fd->fops->close != NULL) {
		result = fd->fops->close(fd);
	}
	if(result < 0) {
		return result;
	}
	rt_free(fd->path);
	fd->path = NULL;
	return result;
}

static struct dfs_fd fd;
static struct dirent dirent;

void ls(const char *pathname)
{
	struct stat stat;
	int length;
	char *fullpath, *path;
	fullpath = NULL;
	if(pathname == NULL) {
		path = rt_strdup("/");
		if(path == NULL) {
			return;
		}
	} else {
		path = (char *)pathname;
	}
	if(dfs_file_open(&fd, path, O_DIRECTORY) == 0) {
		rt_kprintf("Direcotry %s: \n",path);
		do {
			memset(&dirent, 0,sizeof(dirent));
			length = dfs_file_getdents(&fd, &dirent, sizeof(struct dirent));
			if(length > 0) {
				memset(&stat, 0, sizeof(struct stat));
				fullpath = dfs_normalize_path(path, dirent.d_name);
				if(fullpath == NULL) {
					break;
				}
				if(dfs_file_stat(fullpath, &stat) == 0) {
					rt_kprintf("%-20s", dirent.d_name);
					if(S_ISDIR(stat.st_mode)) {
						rt_kprintf("%-25s\n", "<DIR>");
					} else {
						rt_kprintf("%-25lu\n", stat.st_size);
					}
				} else {
					rt_kprintf("bad file: %s\n", dirent.d_name);
				}
			}
		} while(length>0);
		dfs_file_close(&fd);
	}else {
		rt_kprintf("No such directory\n");
	}
	if(pathname == NULL) {
		rt_free(path);
	}
	
}
