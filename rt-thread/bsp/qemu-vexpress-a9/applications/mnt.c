#include <rtthread.h>

#include <dfs_fs.h>

int mnt_init()
{
	//rt_thread_delay();
	if(dfs_mount(RT_NULL, "/", "ram", 0, dfs_ramfs_create( rt_malloc(10240),10240)) == 0) {
		rt_kprintf("ramfs mount ok\n");
	} else {
		rt_kprintf("ramfs mount fail\n");
	}
	return 0;
}

INIT_ENV_EXPORT(mnt_init);


