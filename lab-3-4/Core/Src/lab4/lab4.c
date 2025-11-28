#pragma once
#include "lab4/lab4.h"
#include "lab4/keyboard.h"
#include "lab-1-2/uart_lib.h"
#include "lab3/music.h"
#include "lab-1-2/led.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c1;

// Константы игры
#define SEQUENCE_LENGTH 27  // Длина последовательности (3 цикла по 9 нот)
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

// Проверить ввод с клавиатуры во время импульса
static bool check_input_during_impulse(uint8_t expected_note_index, uint16_t duration) {
    uint8_t input_key = 0xFF;  // Недействительная клавиша
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
        // Проверяем события клавиатуры
        if (kb_event_has()) {
            struct kb_event event = kb_event_pop();
            // Обрабатываем только нажатия (не отпускания)
            if (event.type == KB_EVENT_TYPE_PRESS) {
                input_key = event.key;
                input_received = true;
                break;
            }
        }
        
        // Продолжаем сканирование клавиатуры
        kb_scan_step(&hi2c1);
        HAL_Delay(1);
        elapsed = HAL_GetTick() - start_time;
    }
    
    // Выключить импульс
    disable_all_leds();
    set_frequency(0);
    
    // Дождаться окончания длительности импульса
    while (HAL_GetTick() - start_time < duration) {
        kb_scan_step(&hi2c1);
        HAL_Delay(10);
    }
    
    // Проверить правильность ввода
    if (!input_received) {
        return false;  // Нет ввода = неправильно
    }
    
    // Проверить, что введена правильная клавиша (клавиши 0-8 соответствуют нотам 1-9)
    return (input_key == expected_note_index);
}

// Запустить игру
static void start_game() {
    println("Game starting in 3 seconds...");
    
    // Ждём 3 секунды, продолжая сканировать клавиатуру
    uint32_t start = HAL_GetTick();
    while (HAL_GetTick() - start < 3000) {
        kb_scan_step(&hi2c1);
        HAL_Delay(10);
    }
    
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
        
        // Проверить досрочную остановку (клавиша 12 - "Enter")
        if (kb_event_has()) {
            struct kb_event event = kb_event_pop();
            if (event.type == KB_EVENT_TYPE_PRESS && event.key == KB_EVENT_KEY_12) {
                game.current_note++; // Учесть текущую ноту
                stop_game();
                return;
            }
        }
        
        game.current_note++;
        
        // Небольшая пауза между импульсами
        uint32_t pause_start = HAL_GetTick();
        while (HAL_GetTick() - pause_start < 100) {
            kb_scan_step(&hi2c1);
            HAL_Delay(10);
        }
    }
    
    // Игра закончена естественным образом
    stop_game();
}

// Обработка команд вне игры
static void process_menu_input(struct kb_event event) {
    // Обрабатываем только нажатия
    if (event.type != KB_EVENT_TYPE_PRESS) {
        return;
    }
    
    // Клавиши 1-9 (KB_EVENT_KEY_1 до KB_EVENT_KEY_9): демонстрация импульсов
    if (event.key >= KB_EVENT_KEY_1 && event.key <= KB_EVENT_KEY_9) {
        uint8_t note_idx = event.key;  // Прямое соответствие
        demonstrate_impulse(note_idx);
        return;
    }
    
    switch(event.key) {
        case KB_EVENT_KEY_10:  // Клавиша 10 = "+"
            toggle_speed();
            break;
        case KB_EVENT_KEY_11:  // Клавиша 11 = "a"
            toggle_mode();
            break;
        case KB_EVENT_KEY_12:  // Клавиша 12 = "Enter"
            start_game();
            game_loop();
            break;
        default:
            print_format("Unknown key: %d", event.key);
            break;
    }
}

// Главная функция лабораторной работы 4
void lab4_while_func() {
    static bool initialized = false;
    
    // Инициализация при первом запуске
    if (!initialized) {
        kb_init(&hi2c1);  // Включает сброс I2C шины
        init_sequence();
        initialized = true;
        
        println("====================================");
        println("   RHYTHM GAME - Lab 4");
        println("====================================");
        println("Commands (Keyboard):");
        println("  Keys 1-9  : Demonstrate impulse");
        println("  Key 10    : Change speed");
        println("  Key 11    : Change mode (LED/Sound)");
        println("  Key 12    : Start/Stop game");
        println("====================================");
        print_format("Current speed: %s", get_speed_name());
        print_format("Current mode: %s", get_mode_name());
        println("====================================");
    }
    
    // Сканирование клавиатуры
    kb_scan_step(&hi2c1);
    
    // Проверить события клавиатуры
    if (kb_event_has()) {
        struct kb_event event = kb_event_pop();
        if (!game.is_playing) {
            process_menu_input(event);
        }
    }
    
    // Небольшая задержка для стабильной работы I2C
    HAL_Delay(10);
}
