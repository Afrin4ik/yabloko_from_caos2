#pragma once

#include <stdint.h>

void speaker_init(void);
void speaker_beep(uint16_t frequency_hz, uint16_t duration_ms);