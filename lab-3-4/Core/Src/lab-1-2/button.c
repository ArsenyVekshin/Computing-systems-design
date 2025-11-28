#include "lab-1-2/button.h"

enum Click get_click() {
    uint32_t value = 0;
    uint32_t start = HAL_GetTick();
    while (HAL_GetTick() - start < READ_LENGTH) {
        value += HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == 0;
        if (value > LONG_CLICK) return LONG;
    }

    if (value > SHORT_CLICK) return SHORT;
    return NONE;
}

bool is_btn_pressed() {
    uint32_t value = 0;
    uint32_t start = HAL_GetTick();
    while (HAL_GetTick() - start < READ_LENGTH) {
        value += HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == 0;
        if (value > SHORT_CLICK) return true;
    }
    return false;
}