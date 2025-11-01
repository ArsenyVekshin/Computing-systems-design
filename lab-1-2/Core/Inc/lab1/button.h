#include <stdbool.h>
#include <stdint.h>

#include "main.h"

#define READ_LENGTH 1000 // окно считывания (мс)
#define SHORT_CLICK 10000 // короткий клик (такт)
#define LONG_CLICK 50000  // динный клик (такт)

enum Click { 
    NONE = 0,
    SHORT = 1,
    LONG = 2
};

enum Click get_click();
bool is_btn_pressed();
