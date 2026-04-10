#include "../syscall.h"
#include "../drivers/keyboard_event.h"
#include "user/lib/gfx.h"

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

static void print_status(unsigned event_count, unsigned idle_heartbeats) {
    puts_sys("events=");
    putu_sys(event_count);
    puts_sys(" idle=");
    putu_sys(idle_heartbeats);
    putc_sys('\n');
}

int main() {
    unsigned event_count = 0;
    unsigned idle_heartbeats = 0;

    puts_sys("snake poll test: press keys, q to exit\n");
    print_status(event_count, idle_heartbeats);

    while (1) {
        int event = 0;
        int has_event = syscall(SYS_poll, (int)&event);
        if (has_event < 0) {
            puts_sys("poll failed\n");
            break;
        }

        if (has_event == 0) {
            // Периодический вывод данных показывает, что цикл ожидания продолжает работать и не блокируется
            if ((++idle_heartbeats & 0x3fffff) == 0) {
                print_status(event_count, idle_heartbeats);
            }
            continue;
        }

        ++event_count;
        print_status(event_count, idle_heartbeats);

        if (!kbd_event_is_pressed(event)) {
            continue;
        }

        int key = (int)kbd_event_keycode(event);
        if (key == 'q' || key == 'Q') {
            break;
        }
    }

    return 0;
}
