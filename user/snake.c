#include "../syscall.h"
#include "lib/gfx.h"
#include "snake_game/snake_input.h"
#include "snake_game/snake_model.h"
#include "snake_game/snake_render.h"
#include "snake_game/snake_runtime.h"

enum {
    SNAKE_INITIAL_LENGTH = 5,
    SNAKE_TARGET_APPLE_COUNT = 5,
    SNAKE_TARGET_OBSTACLE_COUNT = 10,
    SNAKE_BASE_TICK_MS = 120,
    SNAKE_FAST_TICK_MS = 60,
    SNAKE_FRAME_TIME_CAP_MS = 50,
    SNAKE_EAT_BEEP_HZ = 880,
    SNAKE_EAT_BEEP_MS = 40,
};

typedef enum {
    SNAKE_PHASE_MENU = 0,
    SNAKE_PHASE_GAME = 1,
    SNAKE_PHASE_GAME_OVER = 2,
} snake_phase_t;

typedef struct {
    snake_model_t model;
    snake_input_t input;
    snake_phase_t phase;
    int alive;
    int paused;
    uint64_t accumulator_ms;
    uint64_t last_time_ms;
} snake_game_state_t;

static void snake_fill_apples(snake_model_t* model, uint8_t target_apple_count) {
    while (model->apple_count < target_apple_count && snake_model_spawn_apple(model)) {
    }
}

static void snake_reset_game(snake_game_state_t* state, snake_dir_t initial_dir, uint16_t initial_length, uint8_t target_apple_count) {
    uint64_t now_ms = snake_runtime_now_ms();

    snake_input_init(&state->input, initial_dir);
    snake_model_init_center(&state->model, initial_dir, initial_length);
    snake_model_seed_random(&state->model, (uint32_t)now_ms);
    snake_model_init_random_obstacles(&state->model, SNAKE_TARGET_OBSTACLE_COUNT);
    snake_fill_apples(&state->model, target_apple_count);

    state->alive = 1;
    state->paused = 0;
    state->accumulator_ms = 0;
    state->last_time_ms = now_ms;
}

static void snake_start_game(snake_game_state_t* state, snake_dir_t initial_dir) {
    snake_reset_game(state, initial_dir, SNAKE_INITIAL_LENGTH, SNAKE_TARGET_APPLE_COUNT);
    state->phase = SNAKE_PHASE_GAME;
    snake_render_full(&state->model);
}

static void snake_show_menu(snake_game_state_t* state) {
    state->phase = SNAKE_PHASE_MENU;
    snake_render_menu();
}

static void snake_show_game_over(snake_game_state_t* state) {
    state->phase = SNAKE_PHASE_GAME_OVER;
    snake_render_game_over(&state->model);
}

int main(void) {
    snake_game_state_t state;
    snake_dir_t initial_dir = SNAKE_DIR_RIGHT;
    int exit_code = 0;

    snake_input_init(&state.input, initial_dir);
    snake_show_menu(&state);

    snake_runtime_log("SNAKE MINI-LOG\n");

    if ((int)GFX_GRID_WIDTH != (int)SNAKE_FIELD_WIDTH || (int)GFX_GRID_HEIGHT != (int)SNAKE_FIELD_HEIGHT) {
        snake_runtime_log("grid mismatch\n");
        return 1;
    }

    if (snake_runtime_present() != 0) {
        return 1;
    }

    while (1) {
        int processed = snake_input_poll(&state.input);
        if (processed < 0) {
            snake_runtime_log("poll failed\n");
            exit_code = 1;
            break;
        }

        if (snake_input_take_action(&state.input, SNAKE_INPUT_ACTION_QUIT)) {
            snake_runtime_log("quit\n");
            break;
        }

        if (state.phase == SNAKE_PHASE_MENU) {
            if (snake_input_take_action(&state.input, SNAKE_INPUT_ACTION_RESTART) ||
                snake_input_take_action(&state.input, SNAKE_INPUT_ACTION_CONFIRM)) {
                snake_runtime_log("start\n");
                snake_start_game(&state, initial_dir);
                if (snake_runtime_present() != 0) {
                    exit_code = 1;
                    break;
                }
                continue;
            }

            if (syscall(SYS_sleep, 1) != 0) {
                snake_runtime_log("sleep failed\n");
                exit_code = 1;
                break;
            }
            continue;
        }

        if (state.phase == SNAKE_PHASE_GAME_OVER) {
            if (snake_input_take_action(&state.input, SNAKE_INPUT_ACTION_RESTART) ||
                snake_input_take_action(&state.input, SNAKE_INPUT_ACTION_CONFIRM)) {
                snake_runtime_log("restart\n");
                snake_start_game(&state, initial_dir);
                if (snake_runtime_present() != 0) {
                    exit_code = 1;
                    break;
                }
                continue;
            }

            if (syscall(SYS_sleep, 1) != 0) {
                snake_runtime_log("sleep failed\n");
                exit_code = 1;
                break;
            }
            continue;
        }

        if (snake_input_take_action(&state.input, SNAKE_INPUT_ACTION_RESTART)) {
            snake_runtime_log("restart\n");
            snake_start_game(&state, initial_dir);
            if (snake_runtime_present() != 0) {
                exit_code = 1;
                break;
            }
            continue;
        }

        if (snake_input_take_action(&state.input, SNAKE_INPUT_ACTION_PAUSE_TOGGLE)) {
            state.paused = !state.paused;
            state.accumulator_ms = 0;
            state.last_time_ms = snake_runtime_now_ms();
            snake_input_set_turn_queue_enabled(&state.input, !state.paused);
            if (state.paused) {
                snake_input_clear_turn_queue(&state.input);
                snake_render_pause(&state.model);
            } else {
                snake_render_full(&state.model);
            }
            snake_runtime_log(state.paused ? "pause on\n" : "pause off\n");
        }

        uint64_t tick_ms = snake_input_is_held(&state.input, SNAKE_INPUT_KEY_SPEED) ? SNAKE_FAST_TICK_MS : SNAKE_BASE_TICK_MS;

        uint64_t current_time_ms = snake_runtime_now_ms();
        uint64_t elapsed_ms = current_time_ms - state.last_time_ms;
        state.last_time_ms = current_time_ms;
        if (elapsed_ms > SNAKE_FRAME_TIME_CAP_MS) {
            elapsed_ms = SNAKE_FRAME_TIME_CAP_MS;
        }

        if (!state.paused) {
            state.accumulator_ms += elapsed_ms;
        }

        while (!state.paused && state.alive && state.accumulator_ms >= tick_ms) {
            snake_dir_t tick_dir = state.model.dir;
            snake_dir_t queued_dir;
            snake_cell_t prev_head = state.model.head;
            snake_cell_t prev_tail = state.model.tail;

            if (snake_input_pop_turn(&state.input, &queued_dir)) {
                tick_dir = queued_dir;
            }

            if (!snake_model_step(&state.model, tick_dir)) {
                state.alive = 0;
                state.phase = SNAKE_PHASE_GAME_OVER;
                snake_runtime_log("game over\n");
                snake_show_game_over(&state);
                if (snake_runtime_present() != 0) {
                    exit_code = 1;
                }
                break;
            } else {
                if (snake_model_try_consume_apple(&state.model)) {
                    snake_runtime_beep(SNAKE_EAT_BEEP_HZ, SNAKE_EAT_BEEP_MS);
                    snake_fill_apples(&state.model, SNAKE_TARGET_APPLE_COUNT);
                }
                snake_render_step(&state.model, prev_head, prev_tail);
            }

            state.accumulator_ms -= tick_ms;
        }

        if (exit_code != 0) {
            break;
        }

        if (snake_runtime_present() != 0) {
            exit_code = 1;
            break;
        }

        if (syscall(SYS_sleep, 1) != 0) {
            snake_runtime_log("sleep failed\n");
            exit_code = 1;
            break;
        }
    }

    syscall(SYS_leave13h, 0);

    return exit_code;
}
