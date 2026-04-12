#include "snake_runtime.h"

#include "../../syscall.h"
#include "../lib/gfx.h"

uint64_t snake_runtime_now_ms(void) {
    uint64_t now = 0;
    if (syscall(SYS_time_ms, (int)&now) != 0) {
        return 0;
    }
    return now;
}

void snake_runtime_log(const char* message) {
    syscall(SYS_puts, (int)message);
}

int snake_runtime_present(void) {
    if (present() != 0) {
        snake_runtime_log("enter13h failed\n");
        return -1;
    }
    return 0;
}
