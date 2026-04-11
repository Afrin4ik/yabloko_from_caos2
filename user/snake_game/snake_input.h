#pragma once

#include <stdint.h>

typedef enum {
    SNAKE_DIR_UP = 0,
    SNAKE_DIR_RIGHT = 1,
    SNAKE_DIR_DOWN = 2,
    SNAKE_DIR_LEFT = 3,
} snake_dir_t;

enum {
    SNAKE_INPUT_KEY_UP = 1 << 0,
    SNAKE_INPUT_KEY_RIGHT = 1 << 1,
    SNAKE_INPUT_KEY_DOWN = 1 << 2,
    SNAKE_INPUT_KEY_LEFT = 1 << 3,
    SNAKE_INPUT_KEY_SPEED = 1 << 4,
};

enum {
    SNAKE_INPUT_ACTION_QUIT = 1 << 0,
    SNAKE_INPUT_ACTION_RESTART = 1 << 1,
    SNAKE_INPUT_ACTION_PAUSE_TOGGLE = 1 << 2,
};

enum {
    SNAKE_INPUT_TURN_QUEUE_CAPACITY = 8,
};

typedef struct {
    uint8_t held_keys;
    uint8_t pending_actions;
    uint8_t committed_dir;

    uint8_t queue_head;
    uint8_t queue_size;
    uint8_t turn_queue[SNAKE_INPUT_TURN_QUEUE_CAPACITY];
} snake_input_t;

void snake_input_init(snake_input_t* input, snake_dir_t initial_dir);
int snake_input_poll(snake_input_t* input);
int snake_input_pop_turn(snake_input_t* input, snake_dir_t* out_dir);
int snake_input_is_held(const snake_input_t* input, uint8_t key_mask);
int snake_input_take_action(snake_input_t* input, uint8_t action_mask);
snake_dir_t snake_input_current_dir(const snake_input_t* input);
int snake_input_queue_size(const snake_input_t* input);
