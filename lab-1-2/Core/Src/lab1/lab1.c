#pragma once
#include "lab1/lab1.h"
#include "lab2/uart_lib.h"

bool enter_password_flow() {
  for (uint8_t i = 0; i < PASSWORD_LENGTH; i++) {
    uint32_t start = HAL_GetTick();
    enum Click input = NONE;
    while (input == NONE && HAL_GetTick() - start < ENTER_SYMB_TIMEOUT) {
      input = get_click();
    }

    if (input == password[i]) {
      blink_by_led(YELLOW, BLINK_TIME);
    } else {
      if (input == NONE) blink_by_led(YELLOW, LONG_BLINK_TIME);
      else blink_by_led(RED, BLINK_TIME);     
      fails_counter++;
      return false;
    }    
  }

  return true;
}

void lab1_while_func() {
    if (enter_password_flow()) {
        blink_by_led(GREEN, LONG_BLINK_TIME);
        fails_counter = 0;
    } else if (fails_counter >= 3) {
        uint32_t start = HAL_GetTick();
        while (HAL_GetTick() - start < ERROR_DELAY) {
            blink_by_led(RED, BLINK_TIME);
        }
        fails_counter = 0;
    }
}
