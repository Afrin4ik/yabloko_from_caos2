#include "../syscall.h"
#include "snake_game/snake_input.h"

static void putc_sys(char c) {
    syscall(SYS_putc, c);
}

static void puts_sys(const char* s) {
    syscall(SYS_puts, (int)s);
}

static void putu_sys(unsigned value) {
    char buf[11];
    unsigned i = 0;

    if (value == 0) {
        putc_sys('0');
        return;
    }

    while (value > 0) {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (i > 0) {
        putc_sys(buf[--i]);
    }
}

static const char* dir_name(snake_dir_t dir) {
    switch (dir) {
        case SNAKE_DIR_UP:
            return "UP";
        case SNAKE_DIR_RIGHT:
            return "RIGHT";
        case SNAKE_DIR_DOWN:
            return "DOWN";
        case SNAKE_DIR_LEFT:
            return "LEFT";
        default:
            return "?";
    }
}

static void print_state(const snake_input_t* input, unsigned event_count, unsigned tick_count, int paused) {
    puts_sys("events=");
    putu_sys(event_count);

    puts_sys(" ticks=");
    putu_sys(tick_count);

    puts_sys(" paused=");
    putu_sys(paused ? 1u : 0u);

    puts_sys(" dir=");
    puts_sys(dir_name(snake_input_current_dir(input)));

    puts_sys(" q=");
    putu_sys((unsigned)snake_input_queue_size(input));

    puts_sys(" held[");
    putc_sys(snake_input_is_held(input, SNAKE_INPUT_KEY_UP) ? 'W' : '.');
    putc_sys(snake_input_is_held(input, SNAKE_INPUT_KEY_LEFT) ? 'A' : '.');
    putc_sys(snake_input_is_held(input, SNAKE_INPUT_KEY_DOWN) ? 'S' : '.');
    putc_sys(snake_input_is_held(input, SNAKE_INPUT_KEY_RIGHT) ? 'D' : '.');
    putc_sys(snake_input_is_held(input, SNAKE_INPUT_KEY_SPEED) ? ' ' : '.');
    puts_sys("]\n");
}

int main() {
    snake_input_t input;
    unsigned event_count = 0;
    unsigned tick_count = 0;
    unsigned spin = 0;
    int paused = 0;

    snake_input_init(&input, SNAKE_DIR_RIGHT);

    puts_sys("snake input test\n");
    puts_sys("WASD: enqueue turns, Space: hold speed\n");
    puts_sys("P/Enter: pause toggle, R: restart, Q: quit\n");
    print_state(&input, event_count, tick_count, paused);

    while (1) {
        int processed = snake_input_poll(&input);
        if (processed < 0) {
            puts_sys("poll failed\n");
            break;
        }
        if (processed > 0) {
            event_count += (unsigned)processed;
            print_state(&input, event_count, tick_count, paused);
        }

        if (snake_input_take_action(&input, SNAKE_INPUT_ACTION_QUIT)) {
            puts_sys("quit\n");
            break;
        }

        if (snake_input_take_action(&input, SNAKE_INPUT_ACTION_RESTART)) {
            snake_input_init(&input, SNAKE_DIR_RIGHT);
            paused = 0;
            puts_sys("restart\n");
            print_state(&input, event_count, tick_count, paused);
        }

        if (snake_input_take_action(&input, SNAKE_INPUT_ACTION_PAUSE_TOGGLE)) {
            paused = !paused;
            puts_sys(paused ? "pause on\n" : "pause off\n");
        }

        // Имитация такта: за один такт может быть затрачен один ход из очереди, как в игровом цикле
        if ((++spin & 0x3ffffu) != 0) {
            continue;
        }

        if (paused) {
            continue;
        }

        ++tick_count;

        {
            snake_dir_t turned_dir;
            if (snake_input_pop_turn(&input, &turned_dir)) {
                puts_sys("tick turn -> ");
                puts_sys(dir_name(turned_dir));
                putc_sys('\n');
            }
        }

        if ((tick_count & 7u) == 0u) {
            print_state(&input, event_count, tick_count, paused);
        }
    }

    return 0;
}
