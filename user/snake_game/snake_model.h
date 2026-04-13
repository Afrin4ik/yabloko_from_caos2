#pragma once

#include <stdint.h>

#include "snake_input.h"

enum {
    SNAKE_FIELD_WIDTH = 32,
    SNAKE_FIELD_HEIGHT = 20,
    SNAKE_MAX_LENGTH = SNAKE_FIELD_WIDTH * SNAKE_FIELD_HEIGHT,
    SNAKE_MAX_APPLES = 16,
    SNAKE_MAX_OBSTACLES = 64,
};

typedef struct {
    uint8_t x;
    uint8_t y;
} snake_cell_t;

typedef struct {
    snake_dir_t dir;
    uint16_t length;
    uint32_t rng_state;

    snake_cell_t head;
    snake_cell_t tail;
    snake_cell_t apples[SNAKE_MAX_APPLES];
    uint8_t apple_count;
    snake_cell_t obstacles[SNAKE_MAX_OBSTACLES];
    uint8_t obstacle_count;

    snake_cell_t growth_points[SNAKE_MAX_LENGTH];
    uint16_t growth_head;
    uint16_t growth_count;

    snake_cell_t body[SNAKE_MAX_LENGTH];
    uint8_t occupancy[SNAKE_FIELD_HEIGHT][SNAKE_FIELD_WIDTH];
    uint8_t obstacle_map[SNAKE_FIELD_HEIGHT][SNAKE_FIELD_WIDTH];
} snake_model_t;

void snake_model_init_center(snake_model_t* model, snake_dir_t initial_dir, uint16_t initial_length);
int snake_model_is_occupied(const snake_model_t* model, int x, int y);
int snake_model_step(snake_model_t* model, snake_dir_t next_dir);
int snake_model_try_consume_apple(snake_model_t* model);
void snake_model_seed_random(snake_model_t* model, uint32_t seed);
int snake_model_spawn_apple(snake_model_t* model);
int snake_model_has_obstacle(const snake_model_t* model, int x, int y);
int snake_model_spawn_obstacle(snake_model_t* model);
void snake_model_init_random_obstacles(snake_model_t* model, uint8_t target_count);
