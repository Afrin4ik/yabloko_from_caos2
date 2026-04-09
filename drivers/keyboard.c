#include "keyboard.h"
#include "cpu/isr.h"
#include <stdint.h>
#include "port.h"
#include "keyboard_event.h"

enum {
    KBD_EVENT_QUEUE_CAPACITY = 32,
};

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

static int kbd_event_queue[KBD_EVENT_QUEUE_CAPACITY];
static unsigned kbd_event_head;
static unsigned kbd_event_tail;
static unsigned kbd_event_size;

enum {
    kbd_data_port = 0x60,
    kbd_extended_scancode_prefix_e0 = 0xe0,
    kbd_extended_scancode_prefix_e1 = 0xe1,
    kbd_set1_release_bit = 0x80,
    kbd_set1_scancode_mask = 0x7f,

    left_shift_pressed_scancode = 0x2a,
    right_shift_pressed_scancode = 0x36,
    backspace_scancode = 0x0e,
};

static void interrupt_handler(registers_t *r) {
    (void)r;
    uint8_t raw_scancode = port_byte_in(kbd_data_port);
    if (raw_scancode == kbd_extended_scancode_prefix_e0 || raw_scancode == kbd_extended_scancode_prefix_e1) {
        return;
    }

    int pressed = (raw_scancode & kbd_set1_release_bit) == 0;
    uint8_t scancode = raw_scancode & kbd_set1_scancode_mask;

    if (scancode == left_shift_pressed_scancode) {
        left_shift_pressed = pressed;
        shift_pressed = left_shift_pressed || right_shift_pressed;
    } else if (scancode == right_shift_pressed_scancode) {
        right_shift_pressed = pressed;
        shift_pressed = left_shift_pressed || right_shift_pressed;
    }

    uint8_t keycode = 0;
    if (scancode == backspace_scancode) {
        keycode = '\b';
    } else if (scancode < sizeof(sc_ascii)) {
        char c = shift_pressed ? sc_ascii_shift[scancode] : sc_ascii[scancode];
        if (c != '?') {
            keycode = (uint8_t)c;
        }
    }

    int event = kbd_event_pack(scancode, keycode, pressed);
    if (kbd_event_size == KBD_EVENT_QUEUE_CAPACITY) {
        kbd_event_tail = (kbd_event_tail + 1) % KBD_EVENT_QUEUE_CAPACITY;
        --kbd_event_size;
    }
    kbd_event_queue[kbd_event_head] = event;
    kbd_event_head = (kbd_event_head + 1) % KBD_EVENT_QUEUE_CAPACITY;
    ++kbd_event_size;
}

static uint32_t irq_save_disable(void) {
    uint32_t eflags;
    asm volatile("pushfl; popl %0; cli" : "=r"(eflags) : : "memory", "cc");
    return eflags;
}

static void irq_restore(uint32_t eflags) {
    asm volatile("pushl %0; popfl" : : "r"(eflags) : "memory", "cc");
}

int kbd_pop_event(void) {
    uint32_t eflags = irq_save_disable();
    if (kbd_event_size == 0) {
        irq_restore(eflags);
        return 0;
    }

    int event = kbd_event_queue[kbd_event_tail];
    kbd_event_tail = (kbd_event_tail + 1) % KBD_EVENT_QUEUE_CAPACITY;
    --kbd_event_size;
    irq_restore(eflags);
    return event;
}

void kbd_clear_buffer(void) {
    uint32_t eflags = irq_save_disable();
    kbd_event_head = 0;
    kbd_event_tail = 0;
    kbd_event_size = 0;
    irq_restore(eflags);
}

void init_keyboard() {
    left_shift_pressed = 0;
    right_shift_pressed = 0;
    shift_pressed = 0;
    kbd_event_head = 0;
    kbd_event_tail = 0;
    kbd_event_size = 0;

    register_interrupt_handler(IRQ1, interrupt_handler);
}
