#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "lab4/keyboard.h"
#include "lab-1-2/uart_lib.h"

// Адрес клавиатуры PCA9538
#define KBRD_ADDR 0xE2

// Регистры PCA9538
#define INPUT_PORT 0x00
#define OUTPUT_PORT 0x01
#define POLARITY_INVERSION 0x02
#define CONFIG_REG 0x03

// Маски рядов
#define ROW1 0xFE
#define ROW2 0xFD
#define ROW3 0xFB
#define ROW4 0xF7

static const uint8_t row_masks[4] = {ROW4, ROW3, ROW2, ROW1};  // Снизу вверх

static struct kb_event buffer[BUFFER_CAPACITY] = { 0 };
static size_t buffer_start_idx = 0;
static size_t buffer_end_idx = 0;


static void kb_event_push(struct kb_event event) {
    buffer[buffer_end_idx] = event;
    INC_BUFFER_IDX(buffer_end_idx);
}

bool kb_event_has() {
    const uint32_t priMask = __get_PRIMASK();
    __disable_irq();
    const bool ret = buffer_start_idx != buffer_end_idx;
    __set_PRIMASK(priMask);
    return ret;
}

struct kb_event kb_event_pop() {
    const uint32_t priMask = __get_PRIMASK();
    __disable_irq();
    const struct kb_event evt = buffer[buffer_start_idx];
    INC_BUFFER_IDX(buffer_start_idx);
    __set_PRIMASK(priMask);
    return evt;
}

// Сброс I2C шины если она залипла
static void I2C_Bus_Reset(I2C_HandleTypeDef *hi2c) {
    __HAL_I2C_DISABLE(hi2c);
    HAL_Delay(10);
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // Переключаем SCL и SDA в режим GPIO
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
    
    // Генерируем 9 тактов SCL для сброса залипшего устройства
    for (int i = 0; i < 9; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
        HAL_Delay(1);
    }
    
    // Генерируем STOP условие
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
    HAL_Delay(1);
    
    // Возвращаем пины в режим I2C
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    __HAL_I2C_ENABLE(hi2c);
    HAL_Delay(10);
}

// Инициализация клавиатуры (как Set_Keyboard в SDK_Keyboard)
static HAL_StatusTypeDef kb_set(I2C_HandleTypeDef *i2c) {
    HAL_StatusTypeDef ret;
    uint8_t buf = 0;

    ret = HAL_I2C_Mem_Write(i2c, KBRD_ADDR & 0xFFFE, POLARITY_INVERSION, 1, &buf, 1, 100);
    if (ret != HAL_OK) return ret;

    ret = HAL_I2C_Mem_Write(i2c, KBRD_ADDR & 0xFFFE, OUTPUT_PORT, 1, &buf, 1, 100);
    return ret;
}

void kb_init(I2C_HandleTypeDef *i2c) {
    // Сброс I2C шины при инициализации
    I2C_Bus_Reset(i2c);
    kb_set(i2c);
}

// Проверка одного ряда клавиатуры (как Check_Row в SDK_Keyboard)
static uint8_t kb_check_row(I2C_HandleTypeDef *i2c, uint8_t row_mask) {
    uint8_t nkey = 0;
    uint8_t buf;
    uint8_t kbd_in;

    // Инициализация перед сканированием
    if (kb_set(i2c) != HAL_OK) return 0;

    // Установка CONFIG регистра для выбора ряда
    buf = row_mask;
    if (HAL_I2C_Mem_Write(i2c, KBRD_ADDR & 0xFFFE, CONFIG_REG, 1, &buf, 1, 100) != HAL_OK) {
        return 0;
    }

    // Чтение входов
    if (HAL_I2C_Mem_Read(i2c, KBRD_ADDR | 1, INPUT_PORT, 1, &buf, 1, 100) != HAL_OK) {
        return 0;
    }

    kbd_in = buf & 0x70;
    
    if (kbd_in != 0x70) {
        if (!(kbd_in & 0x10)) nkey |= 0x04;  // Left
        if (!(kbd_in & 0x20)) nkey |= 0x02;  // Center
        if (!(kbd_in & 0x40)) nkey |= 0x01;  // Right
    }

    return nkey;
}

void kb_scan_step(I2C_HandleTypeDef *i2c) {
    static bool input_keys[12] = { 0 };
    static bool output_keys[12] = { 0 };
    static uint32_t key_time[12] = { 0 };
    
    // Сканируем все 4 ряда
    for (uint8_t row = 0; row < 4; ++row) {
        uint8_t key_state = kb_check_row(i2c, row_masks[row]);
        
        // Декодируем состояние клавиш в ряду
        // key_state: бит 2 (0x04) = левая, бит 1 (0x02) = центр, бит 0 (0x01) = правая
        if (key_state & 0x04) {
            input_keys[row * 3 + 0] = true;
        }
        if (key_state & 0x02) {
            input_keys[row * 3 + 1] = true;
        }
        if (key_state & 0x01) {
            input_keys[row * 3 + 2] = true;
        }
    }
    
    // Проверка на одновременное нажатие более 2 клавиш (защита от ghosting)
    uint8_t count = 0;
    for (int i = 0; i < 12; ++i) {
        if (input_keys[i]) ++count;
    }
    if (count > 2) {
        memset(input_keys, 0, sizeof(input_keys));
    }
    
    // Обработка событий с debounce
    for (int i = 0; i < 12; ++i) {
        const uint32_t t = HAL_GetTick();
        if (output_keys[i] == input_keys[i]) {
            key_time[i] = 0;
        } else if (key_time[i] == 0) {
            key_time[i] = t;
        } else if (t - key_time[i] >= KB_KEY_DEBOUNCE_TIME) {
            output_keys[i] = !output_keys[i];
            if (output_keys[i]) {
                kb_event_push((struct kb_event) { .type = KB_EVENT_TYPE_PRESS, .key = i });
            } else {
                kb_event_push((struct kb_event) { .type = KB_EVENT_TYPE_RELEASE, .key = i });
            }
        }
    }
    
    memset(input_keys, 0, sizeof(input_keys));
}
