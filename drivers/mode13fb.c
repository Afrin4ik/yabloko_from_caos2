#include "mode13fb.h"

#include "pit.h"
#include "vga.h"
#include "cpu/memlayout.h"
#include "../video.h"
#include "../lib/string.h"

static uint32_t mode13_user_fb_ptr = 0;
static _Bool mode13_enabled = 0;

static void mode13_fb_tick(void) {
    if (!mode13_enabled) {
        return;
    }

    kmemmove((char*)P2V(0xA0000), (char*)mode13_user_fb_ptr, VIDEO_MODE13_FRAMEBUFFER_SIZE);
}

void mode13_fb_init(void) {
    add_timer_callback(mode13_fb_tick);
}

void mode13_fb_bind(uint32_t user_fb_ptr) {
    mode13_user_fb_ptr = user_fb_ptr;
    mode13_enabled = 1;

    vgaMode13();
    mode13_fb_tick();
}

void mode13_fb_unbind(void) {
    mode13_enabled = 0;
    mode13_user_fb_ptr = 0;
}
