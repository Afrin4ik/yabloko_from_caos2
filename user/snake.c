#include "../syscall.h"
#include "../drivers/keyboard_event.h"
#include "user/lib/gfx.h"

static void draw_grid_demo(void) {
    clear(0);

    for (int y = 0; y < GFX_GRID_HEIGHT; ++y) {
        for (int x = 0; x < GFX_GRID_WIDTH; ++x) {
            uint8_t color = (uint8_t)(1 + ((x + y) % 14));

            if (x == 0 || y == 0 || x == GFX_GRID_WIDTH - 1 || y == GFX_GRID_HEIGHT - 1) {
                color = 15;
            }

            draw_cell(x, y, color);
        }
    }

    present();
}

int main() {
    // present();
    draw_grid_demo();

    while (1) {
        int event = syscall(SYS_getc, 0);
        if (!event || !kbd_event_is_pressed(event)) {
            continue;
        }

        int key = kbd_event_keycode(event);
        if (key == 'q' || key == 'Q') {
            syscall(SYS_leave13h, 0);
            break;
        }
    }

    return 0;
}
