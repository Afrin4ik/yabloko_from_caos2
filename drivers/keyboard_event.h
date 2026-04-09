#pragma once

#include <stdint.h>

enum {
	KBD_EVENT_KEYCODE_SHIFT = 0,
	KBD_EVENT_SCANCODE_SHIFT = 8,
	KBD_EVENT_PRESSED_SHIFT = 16,
	KBD_EVENT_KEYCODE_MASK = 0xff,
	KBD_EVENT_SCANCODE_MASK = 0xff,
	KBD_EVENT_PRESSED_MASK = 0x1,
};

static inline int kbd_event_pack(uint8_t scancode, uint8_t keycode, int pressed) {
	return ((int)keycode << KBD_EVENT_KEYCODE_SHIFT)
		| ((int)scancode << KBD_EVENT_SCANCODE_SHIFT)
		| ((pressed ? 1 : 0) << KBD_EVENT_PRESSED_SHIFT);
}

static inline uint8_t kbd_event_keycode(int event) {
	return (uint8_t)((event >> KBD_EVENT_KEYCODE_SHIFT) & KBD_EVENT_KEYCODE_MASK);
}

static inline uint8_t kbd_event_scancode(int event) {
	return (uint8_t)((event >> KBD_EVENT_SCANCODE_SHIFT) & KBD_EVENT_SCANCODE_MASK);
}

static inline int kbd_event_is_pressed(int event) {
	return (event >> KBD_EVENT_PRESSED_SHIFT) & KBD_EVENT_PRESSED_MASK;
}
