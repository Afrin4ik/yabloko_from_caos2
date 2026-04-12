#include "gfx.h"

#include "../../syscall.h"
#include "../../video.h"

static uint8_t framebuffer[VIDEO_MODE13_FRAMEBUFFER_SIZE];
static int framebuffer_bound = -1;

void clear(uint8_t color) {
    for (uint32_t i = 0; i < VIDEO_MODE13_FRAMEBUFFER_SIZE; ++i) {
        framebuffer[i] = color;
    }
}

void draw_cell(int x, int y, uint8_t color) {
    if (x < 0 || y < 0 || x >= GFX_GRID_WIDTH || y >= GFX_GRID_HEIGHT) {
        return;
    }

    int pixel_x = x * GFX_CELL_SIZE;
    int pixel_y = y * GFX_CELL_SIZE;

    for (int dy = 0; dy < GFX_CELL_SIZE; ++dy) {
        uint32_t row = (uint32_t)(pixel_y + dy) * VIDEO_MODE13_WIDTH + (uint32_t)pixel_x;
        for (int dx = 0; dx < GFX_CELL_SIZE; ++dx) {
            framebuffer[row + dx] = color;
        }
    }
}

int present(void) {
    if (framebuffer_bound == 1) {
        return 0;
    }

    if (framebuffer_bound == 0) {
        return -1;
    }

    if (syscall(SYS_enter13h, (uint32_t)framebuffer) == 0) {
        framebuffer_bound = 1;
        return 0;
    } else {
        framebuffer_bound = 0;
        return -1;
    }
}
