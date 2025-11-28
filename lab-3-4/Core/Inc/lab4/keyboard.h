#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "main.h"

#define BUFFER_CAPACITY (32)
#define INC_BUFFER_IDX(__idx) do { (__idx) = ((__idx) + 1) % (BUFFER_CAPACITY); } while (0)
#define KB_I2C_ADDRESS (0xE2)
#define KB_I2C_READ_ADDRESS ((KB_I2C_ADDRESS) | 1)
#define KB_I2C_WRITE_ADDRESS ((KB_I2C_ADDRESS) & ~1)
#define KB_INPUT_REG (0x0)
#define KB_OUTPUT_REG (0x1)
#define KB_CONFIG_REG (0x3)
#define KB_KEY_DEBOUNCE_TIME (50)


enum kb_event_type {
    KB_EVENT_TYPE_PRESS = 0,
    KB_EVENT_TYPE_RELEASE = 1,
};

enum kb_event_key {
    KB_EVENT_KEY_1 = 0,
    KB_EVENT_KEY_2 = 1,
    KB_EVENT_KEY_3 = 2,
    KB_EVENT_KEY_4 = 3,
    KB_EVENT_KEY_5 = 4,
    KB_EVENT_KEY_6 = 5,
    KB_EVENT_KEY_7 = 6,
    KB_EVENT_KEY_8 = 7,
    KB_EVENT_KEY_9 = 8,
    KB_EVENT_KEY_10 = 9,
    KB_EVENT_KEY_11 = 10,
    KB_EVENT_KEY_12 = 11,
};


struct kb_event {
    enum kb_event_type type;
    enum kb_event_key key;
};


bool kb_event_has();
struct kb_event kb_event_pop();
void kb_init(I2C_HandleTypeDef * i2c);
void kb_scan_step(I2C_HandleTypeDef * i2c);
