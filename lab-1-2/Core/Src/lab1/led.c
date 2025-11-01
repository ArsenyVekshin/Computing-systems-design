#pragma once

#include "lab1/led.h"
#include "gpio.h"
#include "main.h"

void set_led_state(struct LedMode ledMode) {
	if (ledMode.brigthness > 0) ledMode.brigthness = 100;

	switch (ledMode.color) {
	case GREEN:
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, ledMode.brigthness > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
		break;
	case YELLOW:
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET); // RED
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, ledMode.brigthness > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET); // YELLOW
		break;
	case RED:
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET); // YELLOW
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, ledMode.brigthness > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET); // RED
		break;
	}
}

void blink_by_led(enum Color led, uint16_t delay) {
	set_led_state((struct LedMode){led, 0});
	HAL_Delay(WAIT_BEFORE_BLINK);
	set_led_state((struct LedMode){led, 100});
	HAL_Delay(delay);
	set_led_state((struct LedMode){led, 0});
}

void disable_all_leds() {
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET); // GREEN
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET); // YELLOW
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET); // RED
}

void blink_for_time(enum Color led, uint16_t blink_delay, uint16_t no_blink_delay, uint16_t whole_time) {
	uint32_t start = HAL_GetTick();
	while (HAL_GetTick() - start < whole_time) {
        set_led_state((struct LedMode){led, 0});
		HAL_Delay(no_blink_delay);
		set_led_state((struct LedMode){led, 100});
		HAL_Delay(blink_delay);
		set_led_state((struct LedMode){led, 0});
    }
}
