#include "snake_render.h"

#include "../lib/gfx.h"

enum {
    SNAKE_COLOR_PANEL = 1,
    SNAKE_COLOR_BACKGROUND = 0,
    SNAKE_COLOR_BODY = 2,
    SNAKE_COLOR_APPLE = 4,
    SNAKE_COLOR_OBSTACLE = 8,
    SNAKE_COLOR_TAIL = 10,
    SNAKE_COLOR_HEAD = 12,
    SNAKE_COLOR_TEXT = 15,
};

static void snake_render_panel(int x, int y, int width, int height) {
    fill_rect(x, y, width, height, SNAKE_COLOR_PANEL);
    fill_rect(x + 4, y + 4, width - 8, height - 8, SNAKE_COLOR_BACKGROUND);
}

void snake_render_menu(void) {
    clear(SNAKE_COLOR_BACKGROUND);

    fill_rect(0, 0, 320, 8, SNAKE_COLOR_PANEL);
    fill_rect(0, 192, 320, 8, SNAKE_COLOR_PANEL);
    fill_rect(0, 0, 8, 200, SNAKE_COLOR_PANEL);
    fill_rect(312, 0, 8, 200, SNAKE_COLOR_PANEL);

    snake_render_panel(34, 24, 252, 152);

    draw_text_centered(36, SNAKE_COLOR_TEXT, "YABLOKO SNAKE", 3);
    draw_text_centered(76, SNAKE_COLOR_TEXT, "ENTER OR R TO START", 2);
    draw_text_centered(100, SNAKE_COLOR_TEXT, "WASD MOVE", 2);
    draw_text_centered(122, SNAKE_COLOR_TEXT, "SPACE SPEED", 2);
    draw_text_centered(144, SNAKE_COLOR_TEXT, "P PAUSE OR RESUME", 2);
    draw_text_centered(162, SNAKE_COLOR_TEXT, "Q EXIT TO CONSOLE", 1);
}

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

void snake_render_pause(const snake_model_t* model) {
    snake_render_full(model);

    snake_render_panel(32, 34, 256, 132);
    draw_text_centered(48, SNAKE_COLOR_TEXT, "PAUSED", 4);
    draw_text_centered(88, SNAKE_COLOR_TEXT, "P RESUME", 2);
    draw_text_centered(110, SNAKE_COLOR_TEXT, "R RESTART", 2);
    draw_text_centered(132, SNAKE_COLOR_TEXT, "Q EXIT TO CONSOLE", 2);
}

void snake_render_game_over(const snake_model_t* model) {
    snake_render_full(model);

    snake_render_panel(40, 52, 240, 96);
    draw_text_centered(64, SNAKE_COLOR_TEXT, "GAME OVER", 4);
    draw_text_centered(102, SNAKE_COLOR_TEXT, "R RESTART", 2);
    draw_text_centered(126, SNAKE_COLOR_TEXT, "Q EXIT TO CONSOLE", 2);
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
