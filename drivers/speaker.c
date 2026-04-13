#include "speaker.h"

#include "pit.h"
#include "port.h"

enum {
    SPEAKER_DATA_PORT = 0x61,
    PIT_CHANNEL2_PORT = 0x42,
    PIT_COMMAND_PORT = 0x43,
    PIT_INPUT_HZ = 1193182,
    PIT_CHANNEL2_SQUARE_WAVE = 0xB6,
    SPEAKER_ENABLE_BITS = 0x03,
    SPEAKER_DISABLE_BITS = 0x03,
    SPEAKER_DIVISOR_MIN = 1,
    SPEAKER_DIVISOR_MAX = 0xffffu,
    BYTE_MASK = 0xffu,
};

static volatile uint32_t speaker_off_deadline_ms = 0;
static _Bool speaker_initialized = 0;

static void speaker_off(void) {
    unsigned char value = port_byte_in(SPEAKER_DATA_PORT);
    port_byte_out(SPEAKER_DATA_PORT, (unsigned char)(value & (unsigned char)~SPEAKER_DISABLE_BITS));
}

static void speaker_on(uint16_t frequency_hz) {
    uint32_t divisor = 0;

    if (frequency_hz != 0) {
        divisor = (uint32_t)(PIT_INPUT_HZ / (uint32_t)frequency_hz);
    }

    if (divisor == 0) {
        divisor = SPEAKER_DIVISOR_MIN;
    } else if (divisor > SPEAKER_DIVISOR_MAX) {
        divisor = SPEAKER_DIVISOR_MAX;
    }

    port_byte_out(PIT_COMMAND_PORT, PIT_CHANNEL2_SQUARE_WAVE);
    port_byte_out(PIT_CHANNEL2_PORT, (unsigned char)(divisor & BYTE_MASK));
    port_byte_out(PIT_CHANNEL2_PORT, (unsigned char)((divisor >> 8) & BYTE_MASK));

    unsigned char value = port_byte_in(SPEAKER_DATA_PORT);
    port_byte_out(SPEAKER_DATA_PORT, (unsigned char)(value | SPEAKER_ENABLE_BITS));
}

static void speaker_tick(void) {
    if (speaker_off_deadline_ms == 0) {
        return;
    }

    uint32_t now_ms = (uint32_t)pit_monotonic_ms();
    if ((int32_t)(now_ms - speaker_off_deadline_ms) < 0) {
        return;
    }

    speaker_off_deadline_ms = 0;
    speaker_off();
}

void speaker_init(void) {
    if (speaker_initialized) {
        return;
    }

    speaker_initialized = 1;
    add_timer_callback(speaker_tick);
}

void speaker_beep(uint16_t frequency_hz, uint16_t duration_ms) {
    if (frequency_hz == 0 || duration_ms == 0) {
        speaker_off_deadline_ms = 0;
        speaker_off();
        return;
    }

    speaker_on(frequency_hz);
    speaker_off_deadline_ms = (uint32_t)(pit_monotonic_ms() + (uint64_t)duration_ms);
}