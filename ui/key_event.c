#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "key_event.h"
#include "config.h"

#define DEBOUNCE_MS  50   // czas na stabilizację
#define REPEAT_MS   300   // czas przed autorepeat (trzymanie)

static uint32_t last_press_time = 0;
static int      last_key        = 0;  // ostatni wciśnięty klawisz
static bool     waiting_release = false; // czekamy na puszczenie

void keypad_init(void) {
    gpio_init(BTN_UP);
    gpio_init(BTN_DOWN);
    gpio_init(BTN_ENTER);
    gpio_init(BTN_BACK);

    gpio_set_dir(BTN_UP,    GPIO_IN);
    gpio_set_dir(BTN_DOWN,  GPIO_IN);
    gpio_set_dir(BTN_ENTER, GPIO_IN);
    gpio_set_dir(BTN_BACK,  GPIO_IN);

    gpio_pull_up(BTN_UP);
    gpio_pull_up(BTN_DOWN);
    gpio_pull_up(BTN_ENTER);
    gpio_pull_up(BTN_BACK);
}

int keypad_get_key(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    // Sprawdź stan przycisków (active-low)
    int pressed = 0;
    if (!gpio_get(BTN_UP))    pressed = KEY_ARROW_UP;
    else if (!gpio_get(BTN_DOWN))  pressed = KEY_ARROW_DOWN;
    else if (!gpio_get(BTN_ENTER)) pressed = KEY_ENTER;
    else if (!gpio_get(BTN_BACK))  pressed = KEY_BACKSPACE;

    // Żaden przycisk nie jest wciśnięty — reset stanu
    if (pressed == 0) {
        waiting_release = false;
        last_key = 0;
        return 0;
    }

    // Czekamy na puszczenie poprzedniego przycisku
    if (waiting_release) {
        return 0;
    }

    // Debounce — ignoruj jeśli za szybko po poprzednim
    if (now - last_press_time < DEBOUNCE_MS) {
        return 0;
    }

    // Nowy klawisz lub autorepeat po REPEAT_MS
    if (pressed != last_key || (now - last_press_time) >= REPEAT_MS) {
        last_press_time = now;
        last_key = pressed;

        // Dla ENTER i BACK — tylko jedno zdarzenie, czekaj na puszczenie
        if (pressed == KEY_ENTER || pressed == KEY_BACKSPACE) {
            waiting_release = true;
        }

        return pressed;
    }

    return 0;
}

int keypad_get_battery(void) {
    return -1;
}
