#include "lab3/music.h"
#include "lab3/tim.h"
#include "main.h"

#define MUSIC_VOLUME_MAX   10
#define MUSIC_VOLUME_MUTE  0

extern TIM_HandleTypeDef htim2;

struct Note modes[] = {
    {
        .name = '1',
        .freq = 200000,
        .led_mode = {GREEN, 20},
    },
    {
        .name = '2', // ДО
        .freq = 261630,
        .led_mode = {GREEN, 50},
    },
    {
        .name = '3', // РЕ
        .freq = 293670,
        .led_mode = {GREEN, 100},
    },
    {
        .name = '4', // МИ
        .freq = 329630,
        .led_mode = {YELLOW, 20},
    },
    {
        .name = '5', // ФА
        .freq = 349230,
        .led_mode = {YELLOW, 50},
    },
    {
        .name = '6', // СОЛЬ
        .freq = 392000,
        .led_mode = {YELLOW, 100},
    },
    {
        .name = '7', // ЛЯ
        .freq = 440000,
        .led_mode = {RED, 20},
    },{
        .name = '8', // СИ
        .freq = 493880,
        .led_mode = {RED, 50},
    },{
        .name = '9',
        .freq = 550000,
        .led_mode = {RED, 100},
    },
};

void Music_init(void) {
    HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Init(&htim2);
}

static void Music_set_volume(uint16_t volume) {
    if (volume > MUSIC_VOLUME_MAX)
        volume = MUSIC_VOLUME_MAX;
    TIM2->CCR1 = volume;
}

void set_frequency(uint32_t freq_millis) {
    if (freq_millis == 0) {
        // Остановить звук - установить 0 громкость
        Music_set_volume(MUSIC_VOLUME_MUTE);
    } else {
        // Конвертируем из милли-Гц в Гц
        uint32_t freq_hz = freq_millis / 1000;
        if (freq_hz > 0) {
            TIM2->PSC = ((2 * HAL_RCC_GetPCLK1Freq()) / (2 * MUSIC_VOLUME_MAX * freq_hz)) - 1;
            Music_set_volume(MUSIC_VOLUME_MAX);
        }
    }
}

void play_note(struct Note note) {
    set_frequency(note.freq);
}

void peeck_note(struct Note note, uint16_t delay) {
    set_frequency(note.freq);
    HAL_Delay(delay);
    Music_set_volume(MUSIC_VOLUME_MUTE);
}