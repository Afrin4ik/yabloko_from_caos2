#include "snake_render.h"

#include "../lib/gfx.h"

enum {
    SNAKE_COLOR_PANEL = 7,
    SNAKE_COLOR_BACKGROUND = 8,
    SNAKE_COLOR_BODY = 2,
    SNAKE_COLOR_BODY_ACCENT = 10,
    SNAKE_COLOR_APPLE = 4,
    SNAKE_COLOR_APPLE_ACCENT = 12,
    SNAKE_COLOR_STEM = 6,
    SNAKE_COLOR_LEAF = 2,
    SNAKE_COLOR_OBSTACLE = 7,
    SNAKE_COLOR_OBSTACLE_ACCENT = 15,
    SNAKE_COLOR_OBSTACLE_DARK = 8,
    SNAKE_COLOR_TAIL = 10,
    SNAKE_COLOR_HEAD = 12,
    SNAKE_COLOR_TEXT = 15,
    SNAKE_COLOR_EYE = 0,
    SNAKE_SCORE_X = 4,
    SNAKE_SCORE_Y = 4,
    SNAKE_SCORE_W = 80,
    SNAKE_SCORE_H = 13,
};

static void snake_cell_to_pixels(int x, int y, int* px, int* py) {
    *px = x * GFX_CELL_SIZE;
    *py = y * GFX_CELL_SIZE;
}

static void snake_draw_body_cell(int x, int y) {
    int px = 0;
    int py = 0;
    snake_cell_to_pixels(x, y, &px, &py);

    draw_cell(x, y, SNAKE_COLOR_BACKGROUND);
    fill_rect(px + 1, py + 1, 8, 8, SNAKE_COLOR_BODY);
    fill_rect(px + 2, py + 2, 6, 6, SNAKE_COLOR_BODY_ACCENT);
    fill_rect(px + 3, py + 3, 4, 4, SNAKE_COLOR_BODY);
}

static void snake_draw_tail_cell(int x, int y) {
    int px = 0;
    int py = 0;
    snake_cell_to_pixels(x, y, &px, &py);

    draw_cell(x, y, SNAKE_COLOR_BACKGROUND);
    fill_rect(px + 2, py + 2, 6, 6, SNAKE_COLOR_TAIL);
    fill_rect(px + 3, py + 3, 4, 4, SNAKE_COLOR_BODY);
}

static void snake_draw_head_cell(const snake_model_t* model, int x, int y) {
    int px = 0;
    int py = 0;
    int eye1_x = 0;
    int eye1_y = 0;
    int eye2_x = 0;
    int eye2_y = 0;

    snake_cell_to_pixels(x, y, &px, &py);

    eye1_x = px + 3;
    eye1_y = py + 3;
    eye2_x = px + 6;
    eye2_y = py + 6;

    draw_cell(x, y, SNAKE_COLOR_BACKGROUND);
    fill_rect(px + 1, py + 1, 8, 8, SNAKE_COLOR_HEAD);
    fill_rect(px + 2, py + 2, 6, 6, SNAKE_COLOR_BODY_ACCENT);

    switch (model->dir) {
        case SNAKE_DIR_UP:
            eye1_x = px + 3;
            eye1_y = py + 3;
            eye2_x = px + 6;
            eye2_y = py + 3;
            fill_rect(px + 3, py + 1, 4, 2, SNAKE_COLOR_APPLE_ACCENT);
            break;
        case SNAKE_DIR_RIGHT:
            eye1_x = px + 6;
            eye1_y = py + 3;
            eye2_x = px + 6;
            eye2_y = py + 6;
            fill_rect(px + 7, py + 3, 2, 4, SNAKE_COLOR_APPLE_ACCENT);
            break;
        case SNAKE_DIR_DOWN:
            eye1_x = px + 3;
            eye1_y = py + 6;
            eye2_x = px + 6;
            eye2_y = py + 6;
            fill_rect(px + 3, py + 7, 4, 2, SNAKE_COLOR_APPLE_ACCENT);
            break;
        case SNAKE_DIR_LEFT:
            eye1_x = px + 3;
            eye1_y = py + 3;
            eye2_x = px + 3;
            eye2_y = py + 6;
            fill_rect(px + 1, py + 3, 2, 4, SNAKE_COLOR_APPLE_ACCENT);
            break;
        default:
            break;
    }

    fill_rect(eye1_x, eye1_y, 1, 1, SNAKE_COLOR_EYE);
    fill_rect(eye2_x, eye2_y, 1, 1, SNAKE_COLOR_EYE);
}

static void snake_draw_apple_cell(int x, int y) {
    int px = 0;
    int py = 0;
    snake_cell_to_pixels(x, y, &px, &py);

    draw_cell(x, y, SNAKE_COLOR_BACKGROUND);

    fill_rect(px + 4, py + 1, 1, 2, SNAKE_COLOR_STEM);
    fill_rect(px + 5, py + 1, 2, 1, SNAKE_COLOR_LEAF);

    fill_rect(px + 3, py + 2, 4, 1, SNAKE_COLOR_APPLE_ACCENT);
    fill_rect(px + 2, py + 3, 6, 1, SNAKE_COLOR_APPLE);
    fill_rect(px + 1, py + 4, 8, 3, SNAKE_COLOR_APPLE);
    fill_rect(px + 2, py + 7, 6, 1, SNAKE_COLOR_APPLE);
    fill_rect(px + 3, py + 8, 4, 1, SNAKE_COLOR_APPLE_ACCENT);
    fill_rect(px + 3, py + 4, 2, 2, SNAKE_COLOR_APPLE_ACCENT);
}

static void snake_draw_obstacle_cell(int x, int y) {
    int px = 0;
    int py = 0;
    snake_cell_to_pixels(x, y, &px, &py);

    draw_cell(x, y, SNAKE_COLOR_BACKGROUND);
    fill_rect(px + 1, py + 1, 8, 8, SNAKE_COLOR_OBSTACLE);
    fill_rect(px + 2, py + 2, 6, 6, SNAKE_COLOR_OBSTACLE_DARK);

    fill_rect(px + 2, py + 2, 5, 1, SNAKE_COLOR_OBSTACLE_ACCENT);
    fill_rect(px + 2, py + 5, 6, 1, SNAKE_COLOR_OBSTACLE_ACCENT);
    fill_rect(px + 3, py + 8, 5, 1, SNAKE_COLOR_OBSTACLE_ACCENT);

    fill_rect(px + 4, py + 3, 1, 1, SNAKE_COLOR_OBSTACLE_ACCENT);
    fill_rect(px + 6, py + 6, 1, 1, SNAKE_COLOR_OBSTACLE_ACCENT);
}

static void snake_render_panel(int x, int y, int width, int height) {
    fill_rect(x, y, width, height, SNAKE_COLOR_PANEL);
    fill_rect(x + 4, y + 4, width - 8, height - 8, SNAKE_COLOR_BACKGROUND);
}

static int snake_u16_to_string(uint16_t value, char* out, int out_size) {
    char reverse[5];
    int count = 0;

    if (!out || out_size <= 1) {
        return 0;
    }

    if (value == 0) {
        if (out_size < 2) {
            return 0;
        }
        out[0] = '0';
        out[1] = '\0';
        return 1;
    }

    while (value > 0 && count < (int)sizeof(reverse)) {
        reverse[count++] = (char)('0' + (value % 10));
        value = (uint16_t)(value / 10);
    }

    if (count + 1 > out_size) {
        return 0;
    }

    for (int i = 0; i < count; ++i) {
        out[i] = reverse[count - 1 - i];
    }
    out[count] = '\0';

    return count;
}

static int snake_u8_to_string(uint8_t value, char* out, int out_size) {
    return snake_u16_to_string((uint16_t)value, out, out_size);
}

static void snake_render_score(const snake_model_t* model) {
    char score_digits[6];
    char score_text[12] = "SCORE ";
    int digits_len;

    if (!model) {
        return;
    }

    digits_len = snake_u16_to_string(model->score, score_digits, (int)sizeof(score_digits));
    if (digits_len <= 0) {
        return;
    }

    for (int i = 0; i < digits_len; ++i) {
        score_text[6 + i] = score_digits[i];
    }
    score_text[6 + digits_len] = '\0';

    fill_rect(SNAKE_SCORE_X, SNAKE_SCORE_Y, SNAKE_SCORE_W, SNAKE_SCORE_H, SNAKE_COLOR_PANEL);
    fill_rect(SNAKE_SCORE_X + 1, SNAKE_SCORE_Y + 1, SNAKE_SCORE_W - 2, SNAKE_SCORE_H - 2, SNAKE_COLOR_BACKGROUND);
    draw_text(SNAKE_SCORE_X + 4, SNAKE_SCORE_Y + 3, SNAKE_COLOR_TEXT, score_text, 1);
}

void snake_render_menu(uint8_t apple_count) {
    char apple_digits[4];
    char apple_text[16] = "APPLES ";
    int digits_len;

    digits_len = snake_u8_to_string(apple_count, apple_digits, (int)sizeof(apple_digits));
    if (digits_len > 0) {
        for (int i = 0; i < digits_len; ++i) {
            apple_text[7 + i] = apple_digits[i];
        }
        apple_text[7 + digits_len] = '\0';
    }

    clear(SNAKE_COLOR_BACKGROUND);

    fill_rect(0, 0, 320, 8, SNAKE_COLOR_PANEL);
    fill_rect(0, 192, 320, 8, SNAKE_COLOR_PANEL);
    fill_rect(0, 0, 8, 200, SNAKE_COLOR_PANEL);
    fill_rect(312, 0, 8, 200, SNAKE_COLOR_PANEL);

    snake_render_panel(34, 24, 252, 152);

    draw_text_centered(34, SNAKE_COLOR_TEXT, "YABLOKO SNAKE", 3);
    draw_text_centered(72, SNAKE_COLOR_TEXT, "ENTER OR R TO START", 2);
    draw_text_centered(98, SNAKE_COLOR_TEXT, "A D APPLES", 1);
    draw_text_centered(110, SNAKE_COLOR_TEXT, apple_text, 1);
    draw_text_centered(126, SNAKE_COLOR_TEXT, "WASD MOVE", 2);
    draw_text_centered(146, SNAKE_COLOR_TEXT, "SPACE SPEED", 1);
    draw_text_centered(156, SNAKE_COLOR_TEXT, "P PAUSE OR RESUME", 1);
    draw_text_centered(166, SNAKE_COLOR_TEXT, "Q EXIT TO CONSOLE", 1);
}

void snake_render_full(const snake_model_t* model) {
    clear(SNAKE_COLOR_BACKGROUND);

    for (uint8_t i = 0; i < model->obstacle_count; ++i) {
        snake_draw_obstacle_cell((int)model->obstacles[i].x, (int)model->obstacles[i].y);
    }

    for (uint8_t i = 0; i < model->apple_count; ++i) {
        snake_draw_apple_cell((int)model->apples[i].x, (int)model->apples[i].y);
    }

    for (int y = 0; y < SNAKE_FIELD_HEIGHT; ++y) {
        for (int x = 0; x < SNAKE_FIELD_WIDTH; ++x) {
            if (snake_model_is_occupied(model, x, y)) {
                snake_draw_body_cell(x, y);
            }
        }
    }

    snake_draw_tail_cell((int)model->tail.x, (int)model->tail.y);
    snake_draw_head_cell(model, (int)model->head.x, (int)model->head.y);
    snake_render_score(model);
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
        snake_draw_apple_cell((int)model->apples[i].x, (int)model->apples[i].y);
    }

    if (snake_model_has_obstacle(model, (int)prev_tail.x, (int)prev_tail.y)) {
        snake_draw_obstacle_cell((int)prev_tail.x, (int)prev_tail.y);
    } else {
        draw_cell((int)prev_tail.x, (int)prev_tail.y, SNAKE_COLOR_BACKGROUND);
    }

    snake_draw_body_cell((int)prev_head.x, (int)prev_head.y);
    snake_draw_tail_cell((int)model->tail.x, (int)model->tail.y);
    snake_draw_head_cell(model, (int)model->head.x, (int)model->head.y);
    snake_render_score(model);
}
