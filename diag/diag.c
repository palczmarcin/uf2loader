#include <pico.h>
#include <pico/time.h>
#include <hardware/gpio.h>

#include "lcdspi.h"
#include "pff.h"
// i2ckbd.h USUNIĘTE — zastąpione przyciskami GPIO

// Piny przycisków Display Pack 2.8"
#define BTN_UP    6
#define BTN_DOWN  7
#define BTN_ENTER 8
#define BTN_BACK  9

void _Noreturn infinite_loop(void) {
  while (1) {
    tight_loop_contents();
  }
}

#if PICO_RP2040
#define LOADER "BOOT2040.UF2"
#elif PICO_RP2350
#define LOADER "BOOT2350.UF2"
#endif

#define UI_X      20
#define UI_Y(l)   (20 + 12 * (l))
#define STEP_Y(l) (UI_Y((l) + 4))

static void init_buttons(void) {
  gpio_init(BTN_UP);    gpio_set_dir(BTN_UP,    GPIO_IN); gpio_pull_up(BTN_UP);
  gpio_init(BTN_DOWN);  gpio_set_dir(BTN_DOWN,  GPIO_IN); gpio_pull_up(BTN_DOWN);
  gpio_init(BTN_ENTER); gpio_set_dir(BTN_ENTER, GPIO_IN); gpio_pull_up(BTN_ENTER);
  gpio_init(BTN_BACK);  gpio_set_dir(BTN_BACK,  GPIO_IN); gpio_pull_up(BTN_BACK);
}

char *read_bootmode(void) {
  int end_time = time_us_32() + 500000;
  while (((int)time_us_32() - end_time) < 0) {
    if (!gpio_get(BTN_UP))    return "UP   ";
    if (!gpio_get(BTN_DOWN))  return "DOWN ";
    if (!gpio_get(BTN_ENTER)) return "ENTER";
    if (!gpio_get(BTN_BACK))  return "BACK ";
  }
  return "NONE ";
}

static inline void fail(int step) {
  lcd_set_cursor(UI_X + 200, STEP_Y(step));
  lcd_print_string_color("FAIL", RED, BLACK);
  infinite_loop();
}

static inline void pass(int step) {
  lcd_set_cursor(UI_X + 200, STEP_Y(step));
  lcd_print_string_color("PASS", GREEN, BLACK);
}

static inline void check_keypress(void) {
  char *keypress = read_bootmode();
  lcd_set_cursor(UI_X + 200, UI_Y(8));
  lcd_print_string_color(keypress, GREEN, BLACK);
}

int main() {
  char *filename = LOADER;

#ifdef ENABLE_DEBUG
  stdio_init_all();
#endif

  init_buttons();
  lcd_init();

  lcd_set_cursor(UI_X, UI_Y(0));
  lcd_print_string_color("UF2 Loader Diagnostics " PICO_PROGRAM_VERSION_STRING,
                         WHITE, BLACK);

  lcd_set_cursor(UI_X, STEP_Y(0));
  lcd_print_string_color("SD card init...", WHITE, BLACK);

  lcd_set_cursor(UI_X, STEP_Y(1));
  lcd_print_string_color(LOADER " open...", WHITE, BLACK);

  lcd_set_cursor(UI_X, STEP_Y(2));
  lcd_print_string_color(LOADER " read...", WHITE, BLACK);

  FATFS fs;
  FRESULT fr = FR_NOT_READY;

  for (int retry = 5; retry > 0; retry--) {
    fr = pf_mount(&fs);
    if (fr == FR_OK) break;
    sleep_ms(500);
  }

  if (fr != FR_OK) fail(0);
  pass(0);

  fr = pf_open(filename);
  if (fr != FR_OK) fail(1);
  pass(1);

  char buffer[512];
  unsigned int btr;
  fr = pf_read(buffer, sizeof(buffer), &btr);
  if (fr != FR_OK || btr != sizeof(buffer)) fail(2);
  pass(2);

  lcd_set_cursor(UI_X, UI_Y(8));
  lcd_print_string_color("Key pressed...", WHITE, BLACK);

  while (1) {
    check_keypress();
    sleep_ms(20);
  }
}
