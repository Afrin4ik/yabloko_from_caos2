#pragma once

#include <stdint.h>

uint64_t snake_runtime_now_ms(void);
void snake_runtime_log(const char* message);
int snake_runtime_present(void);
void snake_runtime_beep(uint16_t frequency_hz, uint16_t duration_ms);
