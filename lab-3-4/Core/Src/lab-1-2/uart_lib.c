#include "lab-1-2/led.h"

#include "lab-1-2/uart_lib.h"
#include "lab-1-2/utils.h"

#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>

struct UART_status uart = {
    .last_received_char = '\0',
    .cmd = "",
    .use_interrupt = false,
    .is_transmitting = false,
    .tx_byte = '\0',
    .rx_byte = '\0'
};

void tx_buffer_init(RingBuffer_TX* buf) {
    buf->head = 0;
    buf->tail = 0;
    buf->count = 0;
}

void rx_buffer_init(RingBuffer_RX* buf) {
    buf->head = 0;
    buf->tail = 0;
    buf->count = 0;
}

bool tx_buffer_put(RingBuffer_TX* buf, char data) {
    if (buf->count >= TX_BUFFER_SIZE) {
        return false; // буфер переполнен
    }
    buf->buffer[buf->head] = data;
    buf->head = (buf->head + 1) % TX_BUFFER_SIZE;
    buf->count++;
    return true;
}

bool tx_buffer_get(RingBuffer_TX* buf, char* data) {
    if (buf->count == 0) {
        return false; // буфер пуст
    }
    *data = buf->buffer[buf->tail];
    buf->tail = (buf->tail + 1) % TX_BUFFER_SIZE;
    buf->count--;
    return true;
}

bool rx_buffer_put(RingBuffer_RX* buf, char data) {
    if (buf->count >= RX_BUFFER_SIZE) {
        return false; // буфер переполнен
    }
    buf->buffer[buf->head] = data;
    buf->head = (buf->head + 1) % RX_BUFFER_SIZE;
    buf->count++;
    return true;
}

bool rx_buffer_get(RingBuffer_RX* buf, char* data) {
    if (buf->count == 0) {
        return false; // буфер пуст
    }
    *data = buf->buffer[buf->tail];
    buf->tail = (buf->tail + 1) % RX_BUFFER_SIZE;
    buf->count--;
    return true;
}

uint16_t tx_buffer_available(RingBuffer_TX* buf) {
    return buf->count;
}

uint16_t rx_buffer_available(RingBuffer_RX* buf) {
    return buf->count;
}

// Функция для запуска передачи из TX буфера
void uart_start_tx() {
    if (!uart.is_transmitting && tx_buffer_available(&uart.tx_buffer) > 0) {
        if (tx_buffer_get(&uart.tx_buffer, &uart.tx_byte)) {
            uart.is_transmitting = true;
            HAL_UART_Transmit_IT(&huart6, (uint8_t*)&uart.tx_byte, 1);
        }
    }
}


void println_char(const char c) {
    char buf[2];
    buf[0] = c;
    buf[1] = '\0';
    println(buf);
}

void print(const char * content) {
	if (uart.use_interrupt) {
		// В режиме прерываний добавляем данные в TX буфер
		size_t len = strlen(content);
		for (size_t i = 0; i < len; i++) {
			// Ждем, пока в буфере появится место (если переполнен)
			while (!tx_buffer_put(&uart.tx_buffer, content[i])) { }
		}
		// Запускаем передачу, если она еще не идет
		uart_start_tx();
	} else {
		// В обычном режиме - блокирующая передача
		HAL_UART_Transmit(&huart6, (void *) content, strlen(content), UART_TIMEOUT);
	}
}

void println(const char *message) {
	print(message);
	print("\r\n");
}

void print_format(const char * format, ...) {
	static char buffer[1024];
	va_list ap;
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer), format, ap);
	va_end(ap);
	println(buffer);
}


// receive cmd on pattern "*\n"
bool receive() {
    // if (uart.use_interrupt) {
    //     // В режиме прерываний читаем из RX буфера
    //     if (!rx_buffer_get(&uart.rx_buffer, &uart.last_received_char)) {
    //         return false; // буфер пуст, нет данных
    //     }
    // } else {
    //     // В обычном режиме - блокирующий прием
    //     if (HAL_UART_Receive(&huart6, (void *) &uart.last_received_char, sizeof(uart.last_received_char), UART_TIMEOUT) != HAL_OK) {
    //         return false;
    //     }
    // }
    
    // println(" ");
    
    // switch (uart.last_received_char) {
    // 	case '\b':
    // 	case 0x7F: {
    // 	    const uint8_t cmd_len = strlen(uart.cmd);
    // 	    if (cmd_len > 0) uart.cmd[cmd_len - 1] = '\0';
    // 		return false;
    // 	}
    //     //case '\n':
    // 	case '\r':
    // 	    println("> \\n");
    //         break;
    //     case '+':
    //         println("> +\r\nenter new password below");
    //         state.new_pass_flag = true;
    //         break;
    //     default:
    //         print("> ");
    //         println_char(uart.last_received_char);
    // }
    // if (!state.new_pass_flag) return true; // если не флоу нового пароля - продолжаем ввод посимвольно

    // const uint32_t command_line_length = strlen(uart.cmd);
    // if (command_line_length == sizeof(uart.cmd) - 1) {
    // 	println("\r\n invalid command: buffer overflow");
    // 	memset(uart.cmd, '\0', sizeof(uart.cmd));
    // 	return false;
    // }
    // if (uart.last_received_char != '\n' && uart.last_received_char != '\r') {
    //     state.pointer++;
    //     uart.cmd[command_line_length] = uart.last_received_char;
    // }
    // print_format("DEBUG: curr cmd = '%s'", uart.cmd);
    // if (
    //     uart.last_received_char == '\n' 
    //     || uart.last_received_char == '\r'
    //     || strlen(uart.cmd) == 9
    //     || (strlen(uart.cmd) == 8 && !starts_with("+", uart.cmd))
    // ) {
    //     uart.cmd[command_line_length+1] = '\0';
    //     return true;
    // }
    return false;
}
