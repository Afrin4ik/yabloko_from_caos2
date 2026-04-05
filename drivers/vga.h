#pragma once

void vga_set_char(unsigned offset, char c, unsigned char fg, unsigned char bg);
void vga_put_char_at(unsigned col, unsigned row, char c, unsigned char fg, unsigned char bg);

void vga_clear_screen();

void vga_print_string(const char* s);

void vga_backspace();

void vgaMode13();
void vgaMode3();
