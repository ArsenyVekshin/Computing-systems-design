#pragma once
#include "main.h"
#include "gpio.h"
#include "button.h"
#include "led.h"

static enum Click password[] = { SHORT, LONG, SHORT, LONG, SHORT, LONG, SHORT, LONG };

static uint8_t fails_counter = 0;

#define PASSWORD_LENGTH 8
#define ENTER_SYMB_TIMEOUT 5000
#define ERROR_DELAY 5000

void lab1_while_func();
