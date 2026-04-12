#pragma once

#include <stdint.h>

#include "snake_input.h"

enum {
    SNAKE_FIELD_WIDTH = 32,
    SNAKE_FIELD_HEIGHT = 20,
    SNAKE_MAX_LENGTH = SNAKE_FIELD_WIDTH * SNAKE_FIELD_HEIGHT,
};

typedef struct {
    uint8_t x;
    uint8_t y;
} snake_cell_t;

typedef struct {
    snake_dir_t dir;
    uint16_t length;

    snake_cell_t head;
    snake_cell_t tail;

    snake_cell_t body[SNAKE_MAX_LENGTH];
    uint8_t occupancy[SNAKE_FIELD_HEIGHT][SNAKE_FIELD_WIDTH];
} snake_model_t;

void snake_model_init_center(snake_model_t* model, snake_dir_t initial_dir, uint16_t initial_length);
int snake_model_is_occupied(const snake_model_t* model, int x, int y);
int snake_model_step_no_growth(snake_model_t* model, snake_dir_t next_dir);
