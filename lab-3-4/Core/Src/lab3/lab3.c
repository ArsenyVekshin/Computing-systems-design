#pragma once
#include "lab3/lab3.h"
#include "lab-1-2/uart_lib.h"
#include "main.h"

extern UART_HandleTypeDef huart6;

// Константы игры
#define SEQUENCE_LENGTH 27  // Длина последовательности (3 цикла по 9 нот)
#define MAX_INPUT_DELAY 100  // Максимальная задержка между проверками ввода (мс)
#define SPEED_COUNT 3

// Структура состояния игры
typedef struct {
    bool is_playing;           // Игра запущена
    uint8_t speed_level;       // Уровень скорости (0-малая, 1-средняя, 2-высокая)
    uint8_t mode;              // Режим воспроизведения (0-светодиод+звук, 1-только светодиод, 2-только звук)
    uint16_t score;            // Очки
    uint8_t current_note;      // Текущая нота в последовательности
    bool input_history[SEQUENCE_LENGTH];  // История нажатий (true-правильно, false-неправильно)
    uint8_t sequence[SEQUENCE_LENGTH];    // Последовательность нот (индексы в массиве modes)
} GameState;

// Длительности импульсов для разных скоростей (мс)
static const uint16_t speed_durations[SPEED_COUNT] = {1000, 600, 350};

// Глобальное состояние игры
static GameState game = {
    .is_playing = false,
    .speed_level = 0,
    .mode = 0,
    .score = 0,
    .current_note = 0
};

// Названия цветов для вывода
static const char* color_names[] = {"Green", "Yellow", "Red"};
static const char* brightness_names[] = {"20%", "50%", "100%"};

// Инициализация последовательности нот (циклический перебор 1-9, три раза)
static void init_sequence() {
    for (uint8_t i = 0; i < SEQUENCE_LENGTH; i++) {
        game.sequence[i] = i % 9;  // Циклический перебор 0-8 (соответствует нотам 1-9)
    }
}

// Получить название режима
static const char* get_mode_name() {
    switch(game.mode) {
        case 0: return "LED + Sound";
        case 1: return "LED only";
        case 2: return "Sound only";
        default: return "Unknown";
    }
}

// Получить название скорости
static const char* get_speed_name() {
    switch(game.speed_level) {
        case 0: return "Slow";
        case 1: return "Medium";
        case 2: return "Fast";
        default: return "Unknown";
    }
}

// Переключить скорость игры
static void toggle_speed() {
    game.speed_level = (game.speed_level + 1) % SPEED_COUNT;
    print_format("Speed changed to: %s (%d ms per note)", 
                 get_speed_name(), speed_durations[game.speed_level]);
}

// Переключить режим воспроизведения
static void toggle_mode() {
    game.mode = (game.mode + 1) % 3;
    print_format("Mode changed to: %s", get_mode_name());
}

// Воспроизвести импульс (нота + светодиод)
static void play_impulse(uint8_t note_index, uint16_t duration) {
    struct Note note = modes[note_index];
    
    // Включить светодиод если режим позволяет
    if (game.mode == 0 || game.mode == 1) {
        set_led_state(note.led_mode);
    }
    
    // Включить звук если режим позволяет
    if (game.mode == 0 || game.mode == 2) {
        play_note(note);
    }
    
    HAL_Delay(duration);
    
    // Выключить всё
    disable_all_leds();
    set_frequency(0);  // Остановить звук
}

// Демонстрация импульса (для клавиш 1-9 вне игры)
static void demonstrate_impulse(uint8_t note_index) {
    struct Note note = modes[note_index];
    
    print_format("Impulse '%c': Color=%s, Brightness=%s, Frequency=%lu Hz", 
                 note.name,
                 color_names[note.led_mode.color],
                 brightness_names[note.led_mode.brigthness == 20 ? 0 : 
                                  (note.led_mode.brigthness == 50 ? 1 : 2)],
                 note.freq / 1000);
    
    // Воспроизвести импульс на 500 мс
    play_impulse(note_index, 500);
}

// Проверить ввод пользователя во время импульса
static bool check_input_during_impulse(uint8_t expected_note_index, uint16_t duration) {
    char input = '\0';
    uint32_t start_time = HAL_GetTick();
    uint16_t elapsed = 0;
    bool input_received = false;
    
    // Включить импульс
    struct Note note = modes[expected_note_index];
    if (game.mode == 0 || game.mode == 1) {
        set_led_state(note.led_mode);
    }
    if (game.mode == 0 || game.mode == 2) {
        play_note(note);
    }
    
    // Ожидать ввод в течение всего импульса
    while (elapsed < duration) {
        // Проверяем ввод каждые несколько мс
        if (HAL_UART_Receive(&huart6, (uint8_t*)&input, 1, MAX_INPUT_DELAY) == HAL_OK) {
            input_received = true;
            break;
        }
        elapsed = HAL_GetTick() - start_time;
    }
    
    // Выключить импульс
    disable_all_leds();
    set_frequency(0);
    
    // Дождаться окончания длительности импульса
    while (HAL_GetTick() - start_time < duration) {
        HAL_Delay(10);
    }
    
    // Проверить правильность ввода
    if (!input_received) {
        return false;  // Нет ввода = неправильно
    }
    
    // Проверить, что введён правильный символ
    char expected_char = modes[expected_note_index].name;
    return (input == expected_char);
}

// Запустить игру
static void start_game() {
    println("Game starting in 3 seconds...");
    HAL_Delay(3000);
    
    game.is_playing = true;
    game.score = 0;
    game.current_note = 0;
    
    // Очистить историю
    for (uint8_t i = 0; i < SEQUENCE_LENGTH; i++) {
        game.input_history[i] = false;
    }
    
    println("GO!");
    println("------------------------------------");
}

// Остановить игру и вывести результаты
static void stop_game() {
    game.is_playing = false;
    
    println("====================================");
    println("           GAME OVER");
    println("====================================");
    print_format("Final Score: %d points", game.score);
    println("");
    println("Input trace:");
    println("Expected | Actual | Result");
    println("---------|--------|--------");
    
    for (uint8_t i = 0; i < game.current_note && i < SEQUENCE_LENGTH; i++) {
        uint8_t note_idx = game.sequence[i];
        char expected = modes[note_idx].name;
        print_format("   %c     |   %c    | %s", 
                     expected,
                     game.input_history[i] ? expected : 'X',
                     game.input_history[i] ? "OK" : "FAIL");
    }
    
    println("====================================");
}

// Основной игровой цикл
static void game_loop() {
    uint16_t duration = speed_durations[game.speed_level];
    
    // Проиграть всю последовательность
    while (game.current_note < SEQUENCE_LENGTH && game.is_playing) {
        uint8_t note_idx = game.sequence[game.current_note];
        
        // Проверить ввод во время импульса
        bool correct = check_input_during_impulse(note_idx, duration);
        game.input_history[game.current_note] = correct;
        
        // Начислить очки
        if (correct) {
            // Больше очков за более высокую скорость
            uint16_t points = 1 + game.speed_level;
            game.score += points;
        }
        
        // Проверить досрочную остановку (Enter)
        char stop_check = '\0';
        if (HAL_UART_Receive(&huart6, (uint8_t*)&stop_check, 1, 0) == HAL_OK) {
            if (stop_check == '\r' || stop_check == '\n') {
                game.current_note++; // Учесть текущую ноту
                stop_game();
                return;
            }
        }
        
        game.current_note++;
        
        // Небольшая пауза между импульсами
        HAL_Delay(100);
    }
    
    // Игра закончена естественным образом
    stop_game();
}

// Обработка команд вне игры
static void process_menu_input(char input) {
    // Клавиши 1-9: демонстрация импульсов
    if (input >= '1' && input <= '9') {
        uint8_t note_idx = input - '1';
        demonstrate_impulse(note_idx);
        return;
    }
    
    switch(input) {
        case '+':
            toggle_speed();
            break;
        case 'a':
        case 'A':
            toggle_mode();
            break;
        case '\r':
        case '\n':
            start_game();
            game_loop();
            break;
        default:
            print_format("Unknown command: '%c'", input);
            break;
    }
}

// Главная функция лабораторной работы 3
void lab3_while_func() {
    static bool initialized = false;
    
    // Инициализация при первом запуске
    if (!initialized) {
        init_sequence();
        initialized = true;
        
        println("====================================");
        println("   RHYTHM GAME - Lab 3");
        println("====================================");
        println("Commands:");
        println("  1-9    : Demonstrate impulse");
        println("  +      : Change speed");
        println("  a      : Change mode (LED/Sound)");
        println("  Enter  : Start/Stop game");
        println("====================================");
        print_format("Current speed: %s", get_speed_name());
        print_format("Current mode: %s", get_mode_name());
        println("====================================");
    }
    
    // Проверить ввод
    char input = '\0';
    if (HAL_UART_Receive(&huart6, (uint8_t*)&input, 1, 100) == HAL_OK) {
        if (!game.is_playing) {
            process_menu_input(input);
        }
    }    
}