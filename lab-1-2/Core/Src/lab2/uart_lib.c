#include "lab1/led.h"

#include "lab2/lab2.h"
#include "lab2/uart_lib.h"
#include "lab2/utils.h"

#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>

struct UART_status uart = {
    .last_received_char = '\0',
    .cmd = "",
    .use_interrupt = false,
    .is_transmitted = true,
    .is_char_received = false
};


void println_char(const char c) {
    char buf[2];
    buf[0] = c;
    buf[1] = '\0';
    println(buf);
}

void print(const char * content) {
	if (uart.use_interrupt) {
		while (!uart.is_transmitted);
		uart.is_transmitted = false;
		HAL_UART_Transmit_IT(&huart6, (void *) content, strlen(content));
	} else HAL_UART_Transmit(&huart6, (void *) content, strlen(content), UART_TIMEOUT);
}

void println(const char *message) {
	print(message);
	print("\r\n");
}

void print_format(const char * format, ...) {
	static char buffer[1024];
	if (uart.use_interrupt) while (!uart.is_transmitted);
	va_list ap;
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer), format, ap);
	va_end(ap);
	println(buffer);
}


// receive cmd on pattern "*\n"
bool receive() {
    if (uart.use_interrupt) {
        if (!uart.is_char_received) {
            HAL_UART_Receive_IT(&huart6, (void *) &uart.last_received_char, sizeof(uart.last_received_char));
            return false;
        }
    } else if (HAL_UART_Receive(&huart6, (void *) &uart.last_received_char, sizeof(uart.last_received_char), UART_TIMEOUT) != HAL_OK) return false;
    
    println(" ");
    uart.is_char_received = false;
    
    switch (uart.last_received_char) {
    	case '\b':
    	case 0x7F: {
    	    const uint8_t cmd_len = strlen(uart.cmd);
    	    if (cmd_len > 0) uart.cmd[cmd_len - 1] = '\0';
    		return false;
    	}
        //case '\n':
    	case '\r':
    	    println("> \\n");
            break;
        case '+':
            println("> +\r\nenter new password below");
            state.new_pass_flag = true;
            break;
        default:
            print("> ");
            println_char(uart.last_received_char);
    }
    if (!state.new_pass_flag) return true; // если не флоу нового пароля - продолжаем ввод посимвольно

    const uint32_t command_line_length = strlen(uart.cmd);
    if (command_line_length == sizeof(uart.cmd) - 1) {
    	println("\r\n invalid command: buffer overflow");
    	memset(uart.cmd, '\0', sizeof(uart.cmd));
    	return false;
    }
    if (uart.last_received_char != '\n' && uart.last_received_char != '\r') {
        state.pointer++;
        uart.cmd[command_line_length] = uart.last_received_char;
    }
    print_format("DEBUG: curr cmd = '%s'", uart.cmd);
    if (
        uart.last_received_char == '\n' 
        || uart.last_received_char == '\r'
        || strlen(uart.cmd) == 9
        || (strlen(uart.cmd) == 8 && !starts_with("+", uart.cmd))
    ) {
        uart.cmd[command_line_length+1] = '\0';
        return true;
    }
    return false;
}
