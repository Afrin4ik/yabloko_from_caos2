#include "gfx.h"

#include "../../syscall.h"
#include "../../video.h"

static uint8_t framebuffer[VIDEO_MODE13_FRAMEBUFFER_SIZE];
static int framebuffer_bound = -1;

static const uint8_t font_5x7[26][7] = {
    {0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11},
    {0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e},
    {0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e},
    {0x1e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1e},
    {0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f},
    {0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10},
    {0x0e, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0e},
    {0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11},
    {0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e},
    {0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0e},
    {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11},
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f},
    {0x11, 0x1b, 0x15, 0x11, 0x11, 0x11, 0x11},
    {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11},
    {0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e},
    {0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10},
    {0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d},
    {0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11},
    {0x0f, 0x10, 0x10, 0x0e, 0x01, 0x01, 0x1e},
    {0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e},
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04},
    {0x11, 0x11, 0x11, 0x15, 0x15, 0x1b, 0x11},
    {0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11},
    {0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04},
    {0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f},
};

static int font_index(char ch) {
    if (ch >= 'a' && ch <= 'z') {
        ch = (char)(ch - 'a' + 'A');
    }

    if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A';
    }

    return -1;
}

static int string_length(const char* text) {
    int length = 0;
    if (!text) {
        return 0;
    }

    while (text[length] != '\0') {
        ++length;
    }
    return length;
}

void clear(uint8_t color) {
    for (uint32_t i = 0; i < VIDEO_MODE13_FRAMEBUFFER_SIZE; ++i) {
        framebuffer[i] = color;
    }
}

void fill_rect(int x, int y, int width, int height, uint8_t color) {
    if (width <= 0 || height <= 0) {
        return;
    }

    int left = x;
    int top = y;
    int right = x + width;
    int bottom = y + height;

    if (right <= 0 || bottom <= 0 || left >= GFX_WIDTH || top >= GFX_HEIGHT) {
        return;
    }

    if (left < 0) {
        left = 0;
    }
    if (top < 0) {
        top = 0;
    }
    if (right > GFX_WIDTH) {
        right = GFX_WIDTH;
    }
    if (bottom > GFX_HEIGHT) {
        bottom = GFX_HEIGHT;
    }

    for (int py = top; py < bottom; ++py) {
        uint32_t row = (uint32_t)py * GFX_WIDTH;
        for (int px = left; px < right; ++px) {
            framebuffer[row + (uint32_t)px] = color;
        }
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

static void draw_glyph(int x, int y, uint8_t color, char ch, int scale) {
    int index = font_index(ch);
    if (index < 0 || scale <= 0) {
        return;
    }

    const uint8_t* glyph = font_5x7[index];
    for (int row = 0; row < 7; ++row) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 5; ++col) {
            if ((bits & (uint8_t)(1u << (4 - col))) == 0) {
                continue;
            }

            fill_rect(x + col * scale, y + row * scale, scale, scale, color);
        }
    }
}

void draw_text(int x, int y, uint8_t color, const char* text, int scale) {
    if (!text || scale <= 0) {
        return;
    }

    int cursor_x = x;
    for (int i = 0; text[i] != '\0'; ++i) {
        char ch = text[i];
        if (ch == ' ') {
            cursor_x += 6 * scale;
            continue;
        }

        draw_glyph(cursor_x, y, color, ch, scale);
        cursor_x += 6 * scale;
    }
}

void draw_text_centered(int y, uint8_t color, const char* text, int scale) {
    int length = string_length(text);
    if (length == 0 || scale <= 0) {
        return;
    }

    int text_width = length * 6 * scale - scale;
    int x = (GFX_WIDTH - text_width) / 2;
    if (x < 0) {
        x = 0;
    }

    draw_text(x, y, color, text, scale);
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
