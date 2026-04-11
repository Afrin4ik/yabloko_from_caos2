#pragma once

#include <stdint.h>

enum {
    GFX_CELL_SIZE = 10,
    GFX_GRID_WIDTH = 320 / GFX_CELL_SIZE,
    GFX_GRID_HEIGHT = 200 / GFX_CELL_SIZE,
};

void clear(uint8_t color);
void draw_cell(int x, int y, uint8_t color);
void present(void);