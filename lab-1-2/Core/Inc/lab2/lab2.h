#pragma once
#include "lab1/button.h"
#include "lab1/led.h"
#include "lab2/uart_lib.h"

static char password[9] = "password";

#define MAX_ATTEMPTS 3
#define PASSWORD_LENGTH 8
#define ENTER_SYMB_TIMEOUT 5000
#define ERROR_DELAY 5000

struct State {
    uint8_t pointer;
    uint8_t fails_counter;
    char password[9];
    bool new_pass_flag;
};
extern struct State state;

void lab2_while_func();