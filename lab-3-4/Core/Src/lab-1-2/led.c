#include "lab-1-2/led.h"
#include "main.h"

extern TIM_HandleTypeDef htim4;

void set_led_state(struct LedMode ledMode) {
	uint16_t pwm_value = ledMode.brigthness;
	if (pwm_value > 100) pwm_value = 100;
	pwm_value = pwm_value * 10; // 0-100 -> 0-1000 (ARR = 999)

	switch (ledMode.color) {
	case GREEN:
		htim4.Instance->CCR2 = pwm_value; // GREEN
		break;
	case YELLOW:
		htim4.Instance->CCR4 = 0; // RED
		htim4.Instance->CCR3 = pwm_value; // YELLOW
		break;
	case RED:
		htim4.Instance->CCR3 = 0; // YELLOW
		htim4.Instance->CCR4 = pwm_value; // RED
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
	htim4.Instance->CCR2 = 0; // GREEN
	htim4.Instance->CCR3 = 0; // YELLOW
	htim4.Instance->CCR4 = 0; // RED
}
