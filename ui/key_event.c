#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "key_event.h"
#include "config.h"

// Debounce w ms
#define DEBOUNCE_MS 50

static uint32_t last_press_time = 0;

void keypad_init(void) {
    gpio_init(BTN_UP);
    gpio_init(BTN_DOWN);
    gpio_init(BTN_ENTER);
    gpio_init(BTN_BACK);

    gpio_set_dir(BTN_UP,    GPIO_IN);
    gpio_set_dir(BTN_DOWN,  GPIO_IN);
    gpio_set_dir(BTN_ENTER, GPIO_IN);
    gpio_set_dir(BTN_BACK,  GPIO_IN);

    // Display Pack ma pull-up na przyciskach — wciśnięty = LOW
    gpio_pull_up(BTN_UP);
    gpio_pull_up(BTN_DOWN);
    gpio_pull_up(BTN_ENTER);
    gpio_pull_up(BTN_BACK);
}

int keypad_get_key(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    // Debounce
    if (now - last_press_time < DEBOUNCE_MS) {
        return 0;
    }

    // Przyciski active-low (wciśnięty = 0)
    if (!gpio_get(BTN_UP)) {
        last_press_time = now;
        return KEY_ARROW_UP;
    }
    if (!gpio_get(BTN_DOWN)) {
        last_press_time = now;
        return KEY_ARROW_DOWN;
    }
    if (!gpio_get(BTN_ENTER)) {
        last_press_time = now;
        return KEY_ENTER;
    }
    if (!gpio_get(BTN_BACK)) {
        last_press_time = now;
        return KEY_BACKSPACE;
    }

    return 0;
}

// Display Pack nie ma baterii — zwracamy -1 żeby UI pominęło ikonę
int keypad_get_battery(void) {
    return -1;
}
