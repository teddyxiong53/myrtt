#include <stdio.h>
#include <rtthread.h>

int main(void)
{
	int i = 0;
	while(1) {
		rt_kprintf("main thread, count:%d \n", i++);
		rt_thread_delay(100);
	}
	
}


