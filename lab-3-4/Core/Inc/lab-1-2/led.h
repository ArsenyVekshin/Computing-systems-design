#pragma once

#include <stdint.h>

#define WAIT_BEFORE_BLINK 100
#define BLINK_TIME 100
#define LONG_BLINK_TIME 1000

enum Color {
    GREEN = 0,
    YELLOW = 1,
    RED = 2,
};

struct LedMode {
	enum Color color;
	uint8_t brigthness;
};

void set_led_state(struct LedMode ledMode);
void blink_by_led(enum Color led, uint16_t delay);
void disable_all_leds();
