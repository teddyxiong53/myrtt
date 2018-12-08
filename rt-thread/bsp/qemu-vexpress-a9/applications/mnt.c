#include <rtthread.h>

#include <dfs_fs.h>

int mnt_init()
{
	//rt_thread_delay();
	if(dfs_mount("ram0", "/", "ram", 0, 0) == 0) {
		rt_kprintf("ramfs mount ok\n");
	}
	return 0;
}

INIT_ENV_EXPORT(mnt_init);


