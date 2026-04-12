#include "snake_render.h"

#include "../lib/gfx.h"

void snake_render_full(const snake_model_t* model) {
    clear(0);

    for (int y = 0; y < SNAKE_FIELD_HEIGHT; ++y) {
        for (int x = 0; x < SNAKE_FIELD_WIDTH; ++x) {
            if (snake_model_is_occupied(model, x, y)) {
                draw_cell(x, y, 2);
            }
        }
    }

    draw_cell((int)model->tail.x, (int)model->tail.y, 10);
    draw_cell((int)model->head.x, (int)model->head.y, 12);
}

void snake_render_step(const snake_model_t* model, snake_cell_t prev_head, snake_cell_t prev_tail) {
    draw_cell((int)prev_tail.x, (int)prev_tail.y, 0);
    draw_cell((int)prev_head.x, (int)prev_head.y, 2);
    draw_cell((int)model->tail.x, (int)model->tail.y, 10);
    draw_cell((int)model->head.x, (int)model->head.y, 12);
}
