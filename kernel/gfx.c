#include "gfx.h"

#include "cpu/memlayout.h"
#include "drivers/vga.h"
#include "lib/string.h"

enum {
    MODE13_WIDTH = 320,
    MODE13_HEIGHT = 200,
    MODE13_FB_SIZE = MODE13_WIDTH * MODE13_HEIGHT,
    VGA_MODE13_PA = 0xA0000,
};

static const char* active_user_fb;

int gfx_enter_mode13(const void* user_framebuffer) {
    if (!user_framebuffer) {
        return -1;
    }

    active_user_fb = (const char*)user_framebuffer;
    vgaMode13();
    kmemmove((char*)(KERNBASE + VGA_MODE13_PA), (char*)active_user_fb, MODE13_FB_SIZE);
    return 0;
}

void gfx_tick(void) {
    if (!active_user_fb) {
        return;
    }

    kmemmove((char*)(KERNBASE + VGA_MODE13_PA), (char*)active_user_fb, MODE13_FB_SIZE);
}

void gfx_detach_user_fb(void) {
    active_user_fb = 0;
}
