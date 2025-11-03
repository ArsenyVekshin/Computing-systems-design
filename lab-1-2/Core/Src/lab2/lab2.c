#pragma once
#include "lab2/lab2.h"

struct State state = {
    .pointer = 0,
    .fails_counter = 0,
    .password = "password",
    .new_pass_flag = false,
};

void check_symb() {
    if (uart.last_received_char == state.password[state.pointer]) {
        state.pointer++;
        blink_by_led(YELLOW, BLINK_TIME);
        if (state.pointer == strlen(state.password)) {
            state.pointer = 0;
            state.fails_counter = 0;
            println("login succesfuly!");
            blink_by_led(GREEN, LONG_BLINK_TIME);
        }
        return;
    }
    state.pointer = 0;
    state.fails_counter++;
    if (state.fails_counter >= MAX_ATTEMPTS) {
        println("max attempts reached!");
        blink_by_led(RED, LONG_BLINK_TIME);
        state.fails_counter = 0;
    } else {
        println("wrong symb");
        blink_by_led(RED, BLINK_TIME);
    }
    
}

void parse_cmd() {
    print("try to parse cmd = \'");
    print(uart.cmd);
    println("\'");
    if (strlen(uart.cmd) <= 0) return;
    
    if (string_equals("OK", uart.cmd)) println("OK");
    else if(starts_with("+", uart.cmd)) {
        strncpy(state.password, uart.cmd + 1, sizeof(state.password) - 1);
        state.password[sizeof(state.password) - 1] = '\0';
        print_format("password changed to '%s'", state.password);
        state.new_pass_flag = false;
    }
    else {
        if (strlen(uart.cmd) == 8 && string_equals(password, uart.cmd)) {
            println("login succesfuly!");
            blink_by_led(GREEN, LONG_BLINK_TIME);
        } else {
            println("wrong password"); 
            blink_by_led(RED, BLINK_TIME);
            state.fails_counter++;
        }
    }
    memset(uart.cmd, '\0', sizeof(uart.cmd));
}

void lab2_while_func() {
    if (is_btn_pressed()) {
        bool new_mode = !uart.use_interrupt;
        print_format("interupt enabled = %d", new_mode);
        blink_by_led(RED, BLINK_TIME);
        blink_by_led(RED, BLINK_TIME);

        if (new_mode) {
            // Включаем режим прерываний - запускаем прием
            uart.use_interrupt = true;
            HAL_UART_Receive_IT(&huart6, (uint8_t*)&uart.rx_byte, 1);
        } else {
            // Выключаем режим прерываний - останавливаем прием
            uart.use_interrupt = false;
            HAL_UART_AbortReceive_IT(&huart6);
        }
    }
    if(receive()) {
        if (state.new_pass_flag) {
            parse_cmd();
        }
        else check_symb();        
    }
}
