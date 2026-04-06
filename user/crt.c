#include "../syscall.h"

int main();

int syscall(int call, int arg) {
    asm volatile("int $0x84" : "+a"(call) : "b"(arg) : "memory", "cc");
    return call;
}

_Noreturn
void _exit(int exit_status) {
    syscall(SYS_exit, exit_status);
    __builtin_unreachable();
}

void _start() {
    _exit(main());
}
