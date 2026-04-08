#pragma once

enum {
	T_SYSCALL = 0x84,
	SYS_exit = 0,
	SYS_greet = 1,
	SYS_putc = 2,
	SYS_puts = 3,
	SYS_getc = 4,
	SYS_enter13h = 5,
	SYS_leave13h = 6,
};

int syscall(int call, int arg);
