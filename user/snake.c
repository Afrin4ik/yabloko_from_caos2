#include "../syscall.h"
#include <stdint.h>

enum {
    FB_WIDTH = 320,
    FB_HEIGHT = 200,
    FB_SIZE = FB_WIDTH * FB_HEIGHT,
};

static unsigned char framebuffer[FB_SIZE];

int main() {
    for (int i = 0; i < FB_SIZE; ++i) {
        framebuffer[i] = 0x3A;
    }

    int rc = syscall(SYS_enter13h, (uint32_t)framebuffer);
    if (rc != 0) {
        syscall(SYS_puts, (uint32_t)"enter13h failed\n");
        return 1;
    }

    while (1) {
    }

    return 0;
}
