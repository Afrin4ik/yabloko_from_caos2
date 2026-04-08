#include "../syscall.h"
#include "../video.h"
#include <stdint.h>

int main() {
    static uint8_t framebuffer[VIDEO_MODE13_FRAMEBUFFER_SIZE];
    syscall(SYS_enter13h, (uint32_t)framebuffer);

    uint8_t color = 0;
    while (1) {
        int key = syscall(SYS_getc, 0);
        if (key == 'q' || key == 'Q') {
            break;
        }

        for (int i = 0; i < VIDEO_MODE13_FRAMEBUFFER_SIZE; ++i) {
            framebuffer[i] = color;
        }

        ++color;

        // Small busy-wait delay so color changes are visible.
        for (volatile int spin = 0; spin < 200000; ++spin) {
        }
    }

    syscall(SYS_leave13h, 0);
    return 0;
}
