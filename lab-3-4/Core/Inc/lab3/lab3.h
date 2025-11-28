#pragma once

#include "lab-1-2/button.h"
#include "lab-1-2/led.h"
#include "lab3/music.h"

static char password[9] = "password";

#define MAX_ATTEMPTS 3
#define PASSWORD_LENGTH 8
#define ENTER_SYMB_TIMEOUT 5000
#define ERROR_DELAY 5000

void lab3_while_func();