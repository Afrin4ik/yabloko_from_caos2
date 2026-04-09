#include "console.h"
#include "cpu/isr.h"
#include "cpu/gdt.h"
#include "cpu/memlayout.h"
#include "drivers/keyboard.h"
#include "drivers/keyboard_event.h"
#include "drivers/mode13fb.h"
#include "drivers/vga.h"
#include "drivers/ata.h"
#include "drivers/misc.h"
#include "drivers/pit.h"
#include "drivers/uart.h"
#include "fs/fs.h"
#include "lib/string.h"
#include "proc.h"
#include "kernel/mem.h"

enum {
    SPINNER_TICKS_PER_FRAME = 25,
    SPINNER_ROW = 0,
    SPINNER_COL = 79,
    SPINNER_FG_WHITE = 0x0f,
    SPINNER_BG_BLACK = 0x00,
    SHELL_CMD_CAPACITY = 128,
};

static char shell_cmd[SHELL_CMD_CAPACITY];
static unsigned shell_cmd_size;

static void update_spinner(void) {
    static const unsigned char spinner_chars[] = {0x18, 0x1a, 0x19, 0x1b};
    static unsigned ticks = 0;
    static unsigned frame = 0;

    ++ticks;
    if (ticks < SPINNER_TICKS_PER_FRAME) {
        return;
    }

    ticks = 0;
    vga_put_char_at(SPINNER_COL, SPINNER_ROW, (char) spinner_chars[frame], SPINNER_FG_WHITE, SPINNER_BG_BLACK);
    frame = (frame + 1) % (sizeof(spinner_chars) / sizeof(spinner_chars[0]));
}

void vga_set_pixel(int x, int y, int color) {
    unsigned char* pixel = (unsigned char*) (KERNBASE + 0xA0000 + 320 * y + x);
    *pixel = color;
}

void graphtest() {
    vgaMode13();
    for (int i = 0; i < 320; ++i) {
        for (int j = 0; j < 200; ++j) {
            vga_set_pixel(i, j, (i+j)/2);
        }
    }
    msleep(5000);
    vgaMode3();
    vga_clear_screen();
}

static void shell_reset(void) {
    shell_cmd_size = 0;
}

static void shell_execute_line(void) {
    shell_cmd[shell_cmd_size] = '\0';
    printk("\n");

    if (shell_cmd_size == 4 && !strncmp("halt", shell_cmd, 4)) {
        qemu_shutdown();
    } else if (shell_cmd_size == 4 && !strncmp("work", shell_cmd, 4)) {
        for (int i = 0; i < 5; ++i) {
            msleep(1000);
            printk(".");
        }
    } else if (shell_cmd_size >= 4 && !strncmp("run ", shell_cmd, 4)) {
        const char* cmd = shell_cmd + 4;
        run_elf(cmd);
    } else if (shell_cmd_size == 9 && !strncmp("graphtest", shell_cmd, 9)) {
        graphtest();
    } else {
        printk("unknown command, try: halt | run CMD");
    }

    shell_reset();
    printk("\n> ");
}

static void shell_handle_key_event(int event) {
    if (!kbd_event_is_pressed(event)) {
        return;
    }

    char keycode = (char)kbd_event_keycode(event);
    if (!keycode) {
        return;
    }

    if (keycode == '\b') {
        if (shell_cmd_size > 0) {
            --shell_cmd_size;
            vga_backspace();
        }
        return;
    }

    if (keycode == '\n') {
        shell_execute_line();
        return;
    }

    if (shell_cmd_size + 1 >= SHELL_CMD_CAPACITY) {
        return;
    }

    shell_cmd[shell_cmd_size++] = keycode;
    char out[] = {keycode, '\0'};
    printk(out);
}

void kmain() {
    freerange(P2V(1u<<20), P2V(2u<<20)); // 1MB - 2MB
    kvmalloc();  // map all of physical memory at KERNBASE
    freerange(P2V(2u<<20), P2V(PHYSTOP));

    load_gdt();
    init_keyboard();
    init_pit();
    mode13_fb_init();
    uartinit();
    load_idt();
    sti();

    vga_clear_screen();
    add_timer_callback(update_spinner);
    printk("YABLOKO\n");

    shell_reset();
    printk("\n> ");
    while (1) {
        int event;
        while ((event = kbd_pop_event()) != 0) {
            shell_handle_key_event(event);
        }
        asm("hlt");
    }
}
