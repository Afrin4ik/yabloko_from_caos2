#pragma once

#include <stdint.h>

enum {
    GFX_WIDTH = 320,
    GFX_HEIGHT = 200,
    GFX_CELL_SIZE = 10,
    GFX_GRID_WIDTH = GFX_WIDTH / GFX_CELL_SIZE,
    GFX_GRID_HEIGHT = GFX_HEIGHT / GFX_CELL_SIZE,
};

void clear(uint8_t color);
void fill_rect(int x, int y, int width, int height, uint8_t color);
void draw_cell(int x, int y, uint8_t color);
void draw_text(int x, int y, uint8_t color, const char* text, int scale);
void draw_text_centered(int y, uint8_t color, const char* text, int scale);
int present(void);