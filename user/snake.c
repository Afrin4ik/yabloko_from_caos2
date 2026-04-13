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
};

typedef struct {
    snake_model_t model;
    snake_input_t input;
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

int main() {
    snake_game_state_t state;
    snake_dir_t initial_dir = SNAKE_DIR_RIGHT;
    const uint64_t base_tick_ms = 120;
    const uint64_t fast_tick_ms = 60;
    const uint64_t frame_time_cap_ms = 50;

    snake_reset_game(&state, initial_dir, SNAKE_INITIAL_LENGTH, SNAKE_TARGET_APPLE_COUNT);

    snake_runtime_log("SNAKE MINI-LOG\n");

    if ((int)GFX_GRID_WIDTH != (int)SNAKE_FIELD_WIDTH || (int)GFX_GRID_HEIGHT != (int)SNAKE_FIELD_HEIGHT) {
        snake_runtime_log("grid mismatch\n");
        return 1;
    }

    state.last_time_ms = snake_runtime_now_ms();
    snake_render_full(&state.model);
    if (snake_runtime_present() != 0) {
        return 1;
    }

    while (1) {
        int processed = snake_input_poll(&state.input);
        if (processed < 0) {
            snake_runtime_log("poll failed\n");
            break;
        }

        if (snake_input_take_action(&state.input, SNAKE_INPUT_ACTION_QUIT)) {
            snake_runtime_log("quit\n");
            break;
        }

        if (snake_input_take_action(&state.input, SNAKE_INPUT_ACTION_RESTART)) {
            snake_reset_game(&state, initial_dir, SNAKE_INITIAL_LENGTH, SNAKE_TARGET_APPLE_COUNT);
            snake_runtime_log("restart\n");
            snake_render_full(&state.model);
            if (snake_runtime_present() != 0) {
                break;
            }
        }

        if (snake_input_take_action(&state.input, SNAKE_INPUT_ACTION_PAUSE_TOGGLE)) {
            state.paused = !state.paused;
            state.accumulator_ms = 0;
            state.last_time_ms = snake_runtime_now_ms();
            snake_input_set_turn_queue_enabled(&state.input, !state.paused);
            if (state.paused) {
                snake_input_clear_turn_queue(&state.input);
            }
            snake_runtime_log(state.paused ? "pause on\n" : "pause off\n");
        }

        uint64_t tick_ms = snake_input_is_held(&state.input, SNAKE_INPUT_KEY_SPEED) ? fast_tick_ms : base_tick_ms;

        uint64_t current_time_ms = snake_runtime_now_ms();
        uint64_t elapsed_ms = current_time_ms - state.last_time_ms;
        state.last_time_ms = current_time_ms;
        if (elapsed_ms > frame_time_cap_ms) {
            elapsed_ms = frame_time_cap_ms;
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
                snake_runtime_log("game over\n");
            } else {
                if (snake_model_try_consume_apple(&state.model)) {
                    snake_fill_apples(&state.model, SNAKE_TARGET_APPLE_COUNT);
                }
                snake_render_step(&state.model, prev_head, prev_tail);
            }

            state.accumulator_ms -= tick_ms;
        }

        if (snake_runtime_present() != 0) {
            break;
        }

        if (syscall(SYS_sleep, 1) != 0) {
            snake_runtime_log("sleep failed\n");
            break;
        }
    }

    syscall(SYS_leave13h, 0);

    return 0;
}
