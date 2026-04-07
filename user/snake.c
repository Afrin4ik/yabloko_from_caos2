#include "../syscall.h"
#include "../video.h"
#include <stdint.h>

int main() {
    static uint8_t framebuffer[VIDEO_MODE13_FRAMEBUFFER_SIZE];

    for (int i = 0; i < VIDEO_MODE13_FRAMEBUFFER_SIZE; ++i) {
        framebuffer[i] = 0x3a;
    }

    syscall(SYS_enter13h, (uint32_t)framebuffer);
    return 0;
}
