#include "pit.h"
#include "port.h"
#include "../cpu/isr.h"
#include "speaker.h"

enum {
    PIT_CRYSTAL_HZ = 1193182,

    CLOCK_PRECISION_HZ = 100,

    PIT_PROGRAM_REG = PIT_CRYSTAL_HZ / CLOCK_PRECISION_HZ,

    PIT_SELECT_CHANNEL0 = 0x0,
    PIT_SELECT_CHANNEL1 = 0x1,
    PIT_SELECT_CHANNEL2 = 0x2,
    PIT_SELECT_CHANNEL_RB = 0x3,
    PIT_ACCESS_MODE_LATCH_COUNT_VALUE_COMMAND = 0x0,
    PIT_ACCESS_MODE_LOBYTE_ONLY = 0x1,
    PIT_ACCESS_MODE_HIBYTE_ONLY = 0x2,
    PIT_ACCESS_MODE_LOHIBYTE = 0x3,
    PIT_MODE_INTERRUPT_ON_TERMINAL_COUNT = 0x0,
    PIT_MODE_HW_ONESHOT = 0x1,
    PIT_MODE_RATE_GENERATOR = 0x2,
    PIT_MODES_SQUARE_WAVE_GENERATOR = 0x3,
    PIT_MODES_SW_TRIGGERRED_STROBE = 0x4,
    PIT_MODES_HW_TRIGGERRED_STROBE = 0x5,
};

static timer_callback callbacks[100];
static int registered_callbacks = 0;
static volatile uint64_t monotonic_ms = 0;

void add_timer_callback(timer_callback tc) {
    if (registered_callbacks >= (int)(sizeof(callbacks) / sizeof(callbacks[0]))) {
        return;
    }
    callbacks[registered_callbacks++] = tc;
}

static void timer_interrupt_handler(registers_t *r) {
    (void)r;
    monotonic_ms += (1000u / CLOCK_PRECISION_HZ);

    for (int i = 0; i < registered_callbacks; ++i) {
        callbacks[i]();
    }
}

struct pit_command_t {
    unsigned char bcd : 1;
    unsigned char operating_mode : 3;
    unsigned char access_mode : 2;
    unsigned char select_channel : 2;
} __attribute__((packed));

uint64_t pit_monotonic_ms(void) {
    enum {
        EFLAGS_IF = 1u << 9
    };
    uint32_t eflags;
    uint64_t ms;

    asm volatile("pushf; pop %0" : "=r"(eflags));
    cli();
    ms = monotonic_ms;
    if (eflags & EFLAGS_IF) {
        sti();
    }
    return ms;
}

void init_pit() {
    struct pit_command_t cmd = {
        .select_channel = PIT_SELECT_CHANNEL0,
        .access_mode = PIT_ACCESS_MODE_LOHIBYTE,
        .operating_mode = PIT_MODES_SQUARE_WAVE_GENERATOR,
        .bcd = 0,
    };
    port_byte_out(0x43, *(unsigned char *)(&cmd));
    port_byte_out(0x40, PIT_PROGRAM_REG & 0xff);
    port_byte_out(0x40, (PIT_PROGRAM_REG & 0xff00) >> 8);

    register_interrupt_handler(IRQ0, timer_interrupt_handler);
    speaker_init();
}

void msleep(int ms) {
    if (ms <= 0) {
        return;
    }

    uint64_t deadline = pit_monotonic_ms() + (uint64_t)ms;
    while (pit_monotonic_ms() < deadline) {
        asm volatile("hlt" ::: "memory");
    }
}
