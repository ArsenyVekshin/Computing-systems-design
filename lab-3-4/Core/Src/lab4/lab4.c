#pragma once
#include "lab4/lab4.h"
#include "lab4/keyboard.h"
#include "lab-1-2/uart_lib.h"
#include "lab3/music.h"
#include "lab-1-2/led.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c1;

#define SEQUENCE_LENGTH 27

typedef struct {
    bool is_playing;           // Игра запущена
    uint8_t speed_level;       // Уровень скорости (0-малая, 1-средняя, 2-высокая)
    uint8_t mode;              // Режим воспроизведения (0-светодиод+звук, 1-только светодиод, 2-только звук)
    uint16_t score;            // Очки
    uint8_t current_note;      // Текущая нота в последовательности
    bool input_history[SEQUENCE_LENGTH];  // История нажатий (true-правильно, false-неправильно)
    uint8_t sequence[SEQUENCE_LENGTH];    // Последовательность нот (индексы в массиве modes)
} GameState;

static const uint16_t speed_durations[SPEED_COUNT] = {1000, 600, 350};

static GameState game = {
    .is_playing = false,
    .speed_level = 0,
    .mode = 0,
    .score = 0,
    .current_note = 0
};

static const char* color_names[] = {"Green", "Yellow", "Red"};
static const char* brightness_names[] = {"20%", "50%", "100%"};

static void init_sequence() {
    for (uint8_t i = 0; i < SEQUENCE_LENGTH; i++) {
        game.sequence[i] = i % 9;
}

static const char* get_mode_name() {
    switch(game.mode) {
        case 0: return "LED + Sound";
        case 1: return "LED only";
        case 2: return "Sound only";
        default: return "Unknown";
    }
}

static const char* get_speed_name() {
    switch(game.speed_level) {
        case 0: return "Slow";
        case 1: return "Medium";
        case 2: return "Fast";
        default: return "Unknown";
    }
}

static void toggle_speed() {
    game.speed_level = (game.speed_level + 1) % SPEED_COUNT;
    print_format("Speed changed to: %s (%d ms per note)", 
                 get_speed_name(), speed_durations[game.speed_level]);
}

static void toggle_mode() {
    game.mode = (game.mode + 1) % 3;
    print_format("Mode changed to: %s", get_mode_name());
}

static void play_impulse(uint8_t note_index, uint16_t duration) {
    struct Note note = modes[note_index];
    
    if (game.mode == 0 || game.mode == 1) {
        set_led_state(note.led_mode);
    }
    
    if (game.mode == 0 || game.mode == 2) {
        play_note(note);
    }
    
    HAL_Delay(duration);
    
    disable_all_leds();
    set_frequency(0);
}

static void demonstrate_impulse(uint8_t note_index) {
    struct Note note = modes[note_index];
    
    print_format("Impulse '%c': Color=%s, Brightness=%s, Frequency=%lu Hz", 
                 note.name,
                 color_names[note.led_mode.color],
                 brightness_names[note.led_mode.brigthness == 20 ? 0 : 
                                  (note.led_mode.brigthness == 50 ? 1 : 2)],
                 note.freq / 1000);
    
    play_impulse(note_index, 500);
}

static bool check_input_during_impulse(uint8_t expected_note_index, uint16_t duration) {
    uint8_t input_key = 0xFF;
    uint32_t start_time = HAL_GetTick();
    uint16_t elapsed = 0;
    bool input_received = false;
    
    struct Note note = modes[expected_note_index];
    if (game.mode == 0 || game.mode == 1) {
        set_led_state(note.led_mode);
    }
    if (game.mode == 0 || game.mode == 2) {
        play_note(note);
    }
    
    while (elapsed < duration) {
        if (kb_event_has()) {
            struct kb_event event = kb_event_pop();
            if (event.type == KB_EVENT_TYPE_PRESS) {
                input_key = event.key;
                input_received = true;
                break;
            }
        }
        
        kb_scan_step(&hi2c1);
        HAL_Delay(1);
        elapsed = HAL_GetTick() - start_time;
    }
    
    disable_all_leds();
    set_frequency(0);
    
    while (HAL_GetTick() - start_time < duration) {
        kb_scan_step(&hi2c1);
        HAL_Delay(10);
    }
    
    if (!input_received) {
        return false;
    }
    
    return (input_key == expected_note_index);
}

static void start_game() {
    println("Game starting in 3 seconds...");
    
    uint32_t start = HAL_GetTick();
    while (HAL_GetTick() - start < 3000) {
        kb_scan_step(&hi2c1);
        HAL_Delay(10);
    }
    
    game.is_playing = true;
    game.score = 0;
    game.current_note = 0;
    
    for (uint8_t i = 0; i < SEQUENCE_LENGTH; i++) {
        game.input_history[i] = false;
    }
    
    println("GO!");
    println("------------------------------------");
}

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

static void game_loop() {
    uint16_t duration = speed_durations[game.speed_level];
    
    while (game.current_note < SEQUENCE_LENGTH && game.is_playing) {
        uint8_t note_idx = game.sequence[game.current_note];
        
        bool correct = check_input_during_impulse(note_idx, duration);
        game.input_history[game.current_note] = correct;
        
        if (correct) {
            uint16_t points = 1 + game.speed_level;
            game.score += points;
        }
        
        if (kb_event_has()) {
            struct kb_event event = kb_event_pop();
            if (event.type == KB_EVENT_TYPE_PRESS && event.key == KB_EVENT_KEY_12) {
                game.current_note++;
                stop_game();
                return;
            }
        }
        
        game.current_note++;
        
        uint32_t pause_start = HAL_GetTick();
        while (HAL_GetTick() - pause_start < 100) {
            kb_scan_step(&hi2c1);
            HAL_Delay(10);
        }
    }
    
    stop_game();
}

static void process_menu_input(struct kb_event event) {
    if (event.type != KB_EVENT_TYPE_PRESS) {
        return;
    }
    
    if (event.key >= KB_EVENT_KEY_1 && event.key <= KB_EVENT_KEY_9) {
        uint8_t note_idx = event.key;  // Прямое соответствие
        demonstrate_impulse(note_idx);
        return;
    }
    
    switch(event.key) {
        case KB_EVENT_KEY_10:
            toggle_speed();
            break;
        case KB_EVENT_KEY_11:
            toggle_mode();
            break;
        case KB_EVENT_KEY_12:
            start_game();
            game_loop();
            break;
        default:
            print_format("Unknown key: %d", event.key);
            break;
    }
}

void lab4_while_func() {
    static bool initialized = false;
    
    if (!initialized) {
        kb_init(&hi2c1);  // сброс I2C шины
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
    
    kb_scan_step(&hi2c1);
    
    if (kb_event_has()) {
        struct kb_event event = kb_event_pop();
        if (!game.is_playing) {
            process_menu_input(event);
        }
    }
    
    HAL_Delay(10);
}
