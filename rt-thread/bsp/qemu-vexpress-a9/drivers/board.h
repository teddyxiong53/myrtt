
#ifndef __BOARD_H__
#define __BOARD_H__

extern int __bss_end;
#define HEAP_BEGIN ((void *)&__bss_end)

#define HEAP_END (void *)(0x60000000 + 8*1024*1024)

void rt_hw_board_init();

#endif

