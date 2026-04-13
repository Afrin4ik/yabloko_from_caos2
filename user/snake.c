#include "../syscall.h"
#include "lib/gfx.h"
#include "snake_game/snake_input.h"
#include "snake_game/snake_model.h"
#include "snake_game/snake_render.h"
#include "snake_game/snake_runtime.h"

int main() {
    snake_input_t input;
    snake_model_t model;
    const uint16_t initial_length = 5;
    snake_dir_t initial_dir = SNAKE_DIR_RIGHT;
    const uint64_t base_tick_ms = 120;
    const uint64_t fast_tick_ms = 60;
    const uint64_t frame_time_cap_ms = 50;
    uint64_t accumulator_ms = 0;
    uint64_t last_time_ms;
    int alive = 1;
    int paused = 0;

    snake_input_init(&input, initial_dir);
    snake_model_init_center(&model, initial_dir, initial_length);
    snake_model_seed_random(&model, (uint32_t)snake_runtime_now_ms());
    (void)snake_model_spawn_apple(&model);

    snake_runtime_log("SNAKE MINI-LOG\n");

    if ((int)GFX_GRID_WIDTH != (int)SNAKE_FIELD_WIDTH || (int)GFX_GRID_HEIGHT != (int)SNAKE_FIELD_HEIGHT) {
        snake_runtime_log("grid mismatch\n");
        return 1;
    }

    last_time_ms = snake_runtime_now_ms();
    snake_render_full(&model);
    if (snake_runtime_present() != 0) {
        return 1;
    }

    while (1) {
        int processed = snake_input_poll(&input);
        if (processed < 0) {
            snake_runtime_log("poll failed\n");
            break;
        }

        if (snake_input_take_action(&input, SNAKE_INPUT_ACTION_QUIT)) {
            snake_runtime_log("quit\n");
            break;
        }

        if (snake_input_take_action(&input, SNAKE_INPUT_ACTION_RESTART)) {
            snake_input_init(&input, initial_dir);
            snake_model_init_center(&model, initial_dir, initial_length);
            snake_model_seed_random(&model, (uint32_t)snake_runtime_now_ms());
            (void)snake_model_spawn_apple(&model);
            alive = 1;
            paused = 0;
            accumulator_ms = 0;
            last_time_ms = snake_runtime_now_ms();
            snake_runtime_log("restart\n");
            snake_render_full(&model);
            if (snake_runtime_present() != 0) {
                break;
            }
        }

        if (snake_input_take_action(&input, SNAKE_INPUT_ACTION_PAUSE_TOGGLE)) {
            paused = !paused;
            accumulator_ms = 0;
            last_time_ms = snake_runtime_now_ms();
            snake_input_set_turn_queue_enabled(&input, !paused);
            if (paused) {
                snake_input_clear_turn_queue(&input);
            }
            snake_runtime_log(paused ? "pause on\n" : "pause off\n");
        }

        uint64_t tick_ms = snake_input_is_held(&input, SNAKE_INPUT_KEY_SPEED) ? fast_tick_ms : base_tick_ms;

        uint64_t current_time_ms = snake_runtime_now_ms();
        uint64_t elapsed_ms = current_time_ms - last_time_ms;
        last_time_ms = current_time_ms;
        if (elapsed_ms > frame_time_cap_ms) {
            elapsed_ms = frame_time_cap_ms;
        }

        if (!paused) {
            accumulator_ms += elapsed_ms;
        }

        while (!paused && alive && accumulator_ms >= tick_ms) {
            snake_dir_t tick_dir = model.dir;
            snake_dir_t queued_dir;
            snake_cell_t prev_head = model.head;
            snake_cell_t prev_tail = model.tail;

            if (snake_input_pop_turn(&input, &queued_dir)) {
                tick_dir = queued_dir;
            }

            if (!snake_model_step(&model, tick_dir)) {
                alive = 0;
                snake_runtime_log("game over\n");
            } else {
                if (snake_model_try_consume_apple(&model)) {
                    (void)snake_model_spawn_apple(&model);
                }
                snake_render_step(&model, prev_head, prev_tail);
            }

            accumulator_ms -= tick_ms;
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
