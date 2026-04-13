#include "snake_runtime.h"

#include "../../syscall.h"
#include "../lib/gfx.h"

enum {
    SNAKE_BEEP_TONE_SHIFT = 16,
    SNAKE_BEEP_TONE_MASK = 0xffffu,
};

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
        snake_runtime_log("present failed\n");
        return -1;
    }
    return 0;
}

void snake_runtime_beep(uint16_t frequency_hz, uint16_t duration_ms) {
    uint32_t packed_tone = ((uint32_t)duration_ms << SNAKE_BEEP_TONE_SHIFT) | ((uint32_t)frequency_hz & SNAKE_BEEP_TONE_MASK);
    syscall(SYS_beep, (int)packed_tone);
}
