#pragma once
#include "usart.h"

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

#define UART_TIMEOUT 50
static const char OK_MSG[] = "OK";

struct UART_status
{
    char last_received_char;
    char cmd[256];
    bool use_interrupt;      // флаг: используется ли прерывание
    bool is_transmitted;     // сообщение отправлено
    bool is_char_received;   // символ получен
};
extern struct UART_status uart;


void print(const char * content);
void println(const char * message);
void print_format(const char * format, ...);

bool receive();

// static void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) { uart.is_char_received = true; }
// static void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) { uart.is_transmitted = true; }