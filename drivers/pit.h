#pragma once

#include <stdint.h>

typedef void (*timer_callback)(void);

void init_pit(void);
void add_timer_callback(timer_callback tc);

uint64_t pit_monotonic_ms(void);

void msleep(int ms);
