#include "snake_render.h"

#include "../lib/gfx.h"

enum {
    SNAKE_COLOR_BACKGROUND = 0,
    SNAKE_COLOR_BODY = 2,
    SNAKE_COLOR_APPLE = 4,
    SNAKE_COLOR_OBSTACLE = 8,
    SNAKE_COLOR_TAIL = 10,
    SNAKE_COLOR_HEAD = 12,
};

void snake_render_full(const snake_model_t* model) {
    clear(SNAKE_COLOR_BACKGROUND);

    for (uint8_t i = 0; i < model->obstacle_count; ++i) {
        draw_cell((int)model->obstacles[i].x, (int)model->obstacles[i].y, SNAKE_COLOR_OBSTACLE);
    }

    for (uint8_t i = 0; i < model->apple_count; ++i) {
        draw_cell((int)model->apples[i].x, (int)model->apples[i].y, SNAKE_COLOR_APPLE);
    }

    for (int y = 0; y < SNAKE_FIELD_HEIGHT; ++y) {
        for (int x = 0; x < SNAKE_FIELD_WIDTH; ++x) {
            if (snake_model_is_occupied(model, x, y)) {
                draw_cell(x, y, SNAKE_COLOR_BODY);
            }
        }
    }

    draw_cell((int)model->tail.x, (int)model->tail.y, SNAKE_COLOR_TAIL);
    draw_cell((int)model->head.x, (int)model->head.y, SNAKE_COLOR_HEAD);
}

void snake_render_step(const snake_model_t* model, snake_cell_t prev_head, snake_cell_t prev_tail) {
    for (uint8_t i = 0; i < model->apple_count; ++i) {
        draw_cell((int)model->apples[i].x, (int)model->apples[i].y, SNAKE_COLOR_APPLE);
    }
    draw_cell((int)prev_tail.x,
              (int)prev_tail.y,
              snake_model_has_obstacle(model, (int)prev_tail.x, (int)prev_tail.y)
                  ? SNAKE_COLOR_OBSTACLE
                  : SNAKE_COLOR_BACKGROUND);
    draw_cell((int)prev_head.x, (int)prev_head.y, SNAKE_COLOR_BODY);
    draw_cell((int)model->tail.x, (int)model->tail.y, SNAKE_COLOR_TAIL);
    draw_cell((int)model->head.x, (int)model->head.y, SNAKE_COLOR_HEAD);
}
