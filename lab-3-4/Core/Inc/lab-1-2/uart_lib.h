#pragma once
#include "lab-1-2/usart.h"

#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

#define UART_TIMEOUT 50
#define TX_BUFFER_SIZE 1024
#define RX_BUFFER_SIZE 256

static const char OK_MSG[] = "OK";

// Кольцевой буфер для передачи
typedef struct {
    char buffer[TX_BUFFER_SIZE];
    volatile uint16_t head;  // индекс для записи
    volatile uint16_t tail;  // индекс для чтения
    volatile uint16_t count; // количество данных в буфере
} RingBuffer_TX;

// Кольцевой буфер для приема
typedef struct {
    char buffer[RX_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint16_t count;
} RingBuffer_RX;

struct UART_status
{
    char last_received_char;
    char cmd[256];
    bool use_interrupt;      // флаг: используется ли прерывание
    
    // Буферы для прерываний
    RingBuffer_TX tx_buffer;
    RingBuffer_RX rx_buffer;
    volatile bool is_transmitting;  // флаг: идет ли передача
    char tx_byte;  // текущий байт для передачи
    char rx_byte;  // текущий принятый байт
};
extern struct UART_status uart;

// Функции для работы с буферами
void tx_buffer_init(RingBuffer_TX* buf);
void rx_buffer_init(RingBuffer_RX* buf);
bool tx_buffer_put(RingBuffer_TX* buf, char data);
bool tx_buffer_get(RingBuffer_TX* buf, char* data);
bool rx_buffer_put(RingBuffer_RX* buf, char data);
bool rx_buffer_get(RingBuffer_RX* buf, char* data);
uint16_t tx_buffer_available(RingBuffer_TX* buf);
uint16_t rx_buffer_available(RingBuffer_RX* buf);

// Функция для запуска передачи из буфера
void uart_start_tx();


void print(const char * content);
void println(const char * message);
void print_format(const char * format, ...);

bool receive();

// static void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) { uart.is_char_received = true; }
// static void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) { uart.is_transmitted = true; }