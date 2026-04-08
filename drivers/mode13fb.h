#pragma once

#include <stdint.h>

void mode13_fb_init(void);
void mode13_fb_bind(uint32_t user_fb_ptr);
void mode13_fb_unbind(void);
_Bool mode13_fb_is_active(void);
