#pragma once

void init_keyboard();
int kbd_pop_char(void);
void kbd_clear_buffer(void);

extern unsigned kbd_buf_size;
extern char *kbd_buf;
