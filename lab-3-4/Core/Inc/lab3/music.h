#pragma once
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#include "lab-1-2/led.h"

struct Note {
    char name;
    uint32_t freq; // мГц
    struct LedMode led_mode; 
};

extern struct Note modes[];

void Music_init(void);

void set_frequency(uint32_t freq_millis);
void peeck_note(struct Note note, uint16_t delay); 
void play_note(struct Note note); 