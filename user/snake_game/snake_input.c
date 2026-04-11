#include "snake_input.h"

#include "../../drivers/keyboard_event.h"
#include "../../syscall.h"

static int snake_input_is_opposite(snake_dir_t a, snake_dir_t b) {
    return ((int)a ^ (int)b) == 2;
}

static int snake_input_enqueue_turn(snake_input_t* input, snake_dir_t dir) {
    snake_dir_t tail_dir = (snake_dir_t)input->committed_dir;

    if (input->queue_size > 0) {
        uint8_t tail_index = (uint8_t)((input->queue_head + input->queue_size - 1) % SNAKE_INPUT_TURN_QUEUE_CAPACITY);
        tail_dir = (snake_dir_t)input->turn_queue[tail_index];
    }

    if (dir == tail_dir || snake_input_is_opposite(dir, tail_dir)) {
        return 0;
    }
    if (input->queue_size >= SNAKE_INPUT_TURN_QUEUE_CAPACITY) {
        return 0;
    }

    uint8_t write_index = (uint8_t)((input->queue_head + input->queue_size) % SNAKE_INPUT_TURN_QUEUE_CAPACITY);
    input->turn_queue[write_index] = (uint8_t)dir;
    ++input->queue_size;
    return 1;
}

static int snake_input_key_to_dir(uint8_t keycode, snake_dir_t* out_dir) {
    switch (keycode) {
        case 'w':
        case 'W':
            *out_dir = SNAKE_DIR_UP;
            return 1;
        case 'd':
        case 'D':
            *out_dir = SNAKE_DIR_RIGHT;
            return 1;
        case 's':
        case 'S':
            *out_dir = SNAKE_DIR_DOWN;
            return 1;
        case 'a':
        case 'A':
            *out_dir = SNAKE_DIR_LEFT;
            return 1;
        default:
            return 0;
    }
}

void snake_input_init(snake_input_t* input, snake_dir_t initial_dir) {
    input->held_keys = 0;
    input->pending_actions = 0;
    input->committed_dir = (uint8_t)initial_dir;
    input->queue_head = 0;
    input->queue_size = 0;
}

int snake_input_poll(snake_input_t* input) {
    int processed = 0;

    while (1) {
        int event = 0;
        int has_event = syscall(SYS_poll, (int)&event);
        if (has_event < 0) {
            return -1;
        }
        if (has_event == 0) {
            return processed;
        }

        ++processed;

        uint8_t keycode = kbd_event_keycode(event);
        int pressed = kbd_event_is_pressed(event);

        snake_dir_t dir;
        if (snake_input_key_to_dir(keycode, &dir)) {
            uint8_t key_mask = (uint8_t)(1u << (uint8_t)dir);

            if (pressed) {
                input->held_keys |= key_mask;
                snake_input_enqueue_turn(input, dir);
            } else {
                input->held_keys &= (uint8_t)~key_mask;
            }
            continue;
        }

        if (keycode == ' ') {
            if (pressed) {
                input->held_keys |= SNAKE_INPUT_KEY_SPEED;
            } else {
                input->held_keys &= (uint8_t)~SNAKE_INPUT_KEY_SPEED;
            }
            continue;
        }

        if (!pressed) {
            continue;
        }

        if (keycode == 'q' || keycode == 'Q') {
            input->pending_actions |= SNAKE_INPUT_ACTION_QUIT;
        } else if (keycode == 'r' || keycode == 'R') {
            input->pending_actions |= SNAKE_INPUT_ACTION_RESTART;
        } else if (keycode == 'p' || keycode == 'P' || keycode == '\n') {
            input->pending_actions |= SNAKE_INPUT_ACTION_PAUSE_TOGGLE;
        }
    }
}

int snake_input_pop_turn(snake_input_t* input, snake_dir_t* out_dir) {
    if (input->queue_size == 0) {
        return 0;
    }

    snake_dir_t dir = (snake_dir_t)input->turn_queue[input->queue_head];
    input->queue_head = (uint8_t)((input->queue_head + 1) % SNAKE_INPUT_TURN_QUEUE_CAPACITY);
    --input->queue_size;
    input->committed_dir = (uint8_t)dir;
    if (out_dir) {
        *out_dir = dir;
    }
    return 1;
}

int snake_input_is_held(const snake_input_t* input, uint8_t key_mask) {
    return (input->held_keys & key_mask) != 0;
}

int snake_input_take_action(snake_input_t* input, uint8_t action_mask) {
    int is_set = (input->pending_actions & action_mask) != 0;
    input->pending_actions &= (uint8_t)~action_mask;
    return is_set;
}

snake_dir_t snake_input_current_dir(const snake_input_t* input) {
    return (snake_dir_t)input->committed_dir;
}

int snake_input_queue_size(const snake_input_t* input) {
    return input->queue_size;
}
