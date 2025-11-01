#include "lab1/button.h"
#include "lab2/uart_lib.h"

enum Click get_click() {
    uint32_t value = 0;
    uint32_t start = HAL_GetTick();
    while (HAL_GetTick() - start < READ_LENGTH) {
        value += HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == 0;
        if (value > LONG_CLICK) {
            print_format("LONG \t%d", value);
            return LONG;
        }
    }

    if (value > SHORT_CLICK) {
        print_format("SHORT \t%d", value);
        return SHORT;
    }

    print_format("NONE \t%d", value);
    return NONE;
}

bool is_btn_pressed() {
    uint32_t value = 0;
    uint32_t start = HAL_GetTick();
    while (HAL_GetTick() - start < 10) {
        value += HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == 0;
        if (value > 100) return true;
    }
    return false;
}