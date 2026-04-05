#include "keyboard.h"
#include "cpu/isr.h"
#include "cpu/memlayout.h"
#include "console.h"
#include "port.h"
#include "kernel/mem.h"
#include "vga.h"

static const char sc_ascii[] = {
    '?', '?', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '?', '?', 'q', 'w', 'e', 'r', 't', 'y',
    'u', 'i', 'o', 'p', '[', ']', '\n', '?', 'a', 's', 'd', 'f', 'g',
    'h', 'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', '?', '?', '?', ' ',
};

static const char sc_ascii_shift[] = {
    '?', '?', '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '_', '+', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y',
    'U', 'I', 'O', 'P', '{', '}', '\n', '?', 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ':', '"', '~', '?', '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', '?', '?', '?', ' ',
};

static int left_shift_pressed = 0;
static int right_shift_pressed = 0;
static int shift_pressed = 0;

enum { kbd_buf_capacity = PGSIZE };

enum {
    left_shift_pressed_scancode = 0x2a,
    left_shift_released_scancode = 0xaa,
    right_shift_pressed_scancode = 0x36,
    right_shift_released_scancode = 0xb6,
    backspace_scancode = 0x0e,
};

static void interrupt_handler(registers_t *r) {
    uint8_t scancode = port_byte_in(0x60);

    if (scancode == left_shift_pressed_scancode) {
        left_shift_pressed = 1;
        shift_pressed = 1;
        return;
    }

    if (scancode == left_shift_released_scancode) {
        left_shift_pressed = 0;
        shift_pressed = left_shift_pressed || right_shift_pressed;
        return;
    }

    if (scancode == right_shift_pressed_scancode) {
        right_shift_pressed = 1;
        shift_pressed = 1;
        return;
    }

    if (scancode == right_shift_released_scancode) {
        right_shift_pressed = 0;
        shift_pressed = left_shift_pressed || right_shift_pressed;
        return;
    }

    if (scancode == backspace_scancode) {
        if (kbd_buf_size > 0) {
            --kbd_buf_size;
            vga_backspace();
        }
        return;
    }

    if (!(scancode & 0x80) && scancode < sizeof(sc_ascii)) {
        char c = shift_pressed ? sc_ascii_shift[scancode] : sc_ascii[scancode];
        if (c != '?') {
            if (kbd_buf_size < kbd_buf_capacity) {
                kbd_buf[kbd_buf_size++] = c;
            }
            char string[] = {c, '\0'};
            printk(string);
        }
    }
}

char* kbd_buf;
unsigned kbd_buf_size;

void init_keyboard() {
    kbd_buf = kalloc();

    register_interrupt_handler(IRQ1, interrupt_handler);
}
