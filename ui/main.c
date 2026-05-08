/**
 * PicoCalc UF2 Firmware Loader
 *
 * Originally by : Hsuan Han Lai
 * Email: hsuan.han.lai@gmail.com
 * Website: https://hsuanhanlai.com
 * Year: 2025
 *
 *
 * This project is a bootloader for the PicoCalc device, designed to load and execute
 * firmware applications from an SD card.
 *
 */

#include <pico.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "debug.h"
#include "lcdspi.h"
#include <hardware/flash.h>
#include <hardware/watchdog.h>
#include "config.h"

#include "ff.h"

#if ENABLE_USB
#endif

#include "text_directory_ui.h"
#include "key_event.h"

#include "proginfo.h"
#include "uf2.h"
#include "ui.h"

FATFS fat;

#define DEBOUNCE_LIMIT 25

// SD card guaranteed present on initial startup
bool sd_insert_state = true;

// Display Pack 2.8" nie ma pinu SD detect — zawsze zwracamy true
bool sd_card_inserted(void)
{
  return true;
}

void fs_deinit(void) { f_unmount("/"); }

bool fs_init(void)
{
  DEBUG_PRINT("fs init SD\n");
  FRESULT res = f_mount(&fat, "/", 1);
  if (res != FR_OK)
  {
    DEBUG_PRINT("mount err: %s\n", res);
    fs_deinit();
    return false;
  }

  return true;
}

void reboot(void) {
#if ENABLE_USB
  usb_msc_init();
#endif
  fs_deinit();

  watchdog_reboot(0, 0, 0);
}

void load_firmware_by_path(const char *path)
{
  // Pokaż pełną ścieżkę w statusie żeby sprawdzić czy jest poprawna
  {
    char dbg[128];
    snprintf(dbg, sizeof(dbg), "Opening: %s", path);
    text_directory_ui_set_status(dbg);
    sleep_ms(2000);
  }

  // Attempt to load the application from the SD card
  enum uf2_result_e result = load_application_from_uf2(path);

  switch (result)
  {
    case UF2_LOADED:
      text_directory_ui_set_status("Launching...");
      DEBUG_PRINT("launching app\n");
      // trigger a reboot - stage3 will launch the app
      reboot();
      break;
    case UF2_WRONG_PLATFORM:
      text_directory_ui_set_status("ERR: Not for this device");
      DEBUG_PRINT("wrong platform\n");
      break;
    case UF2_BAD:
      text_directory_ui_set_status("ERR: Bad UF2 file");
      DEBUG_PRINT("bad uf2\n");
      break;
    default:
      text_directory_ui_set_status("ERR: Unexpected error");
      DEBUG_PRINT("unexpected error\n");
      break;
  }
}

void final_selection_callback(const char *path)
{
  // WAŻNE: kopiujemy ścieżkę na własny bufor zaraz na wejściu
  // bo 'path' wskazuje na lokalny bufor w text_directory_ui.c
  // który może być nadpisany przez kolejne wywołania funkcji
  static char path_copy[128];
  char status_message[128];
  const char *extension = ".uf2";

  if (path == NULL)
  {
    // Reboot into current app
    reboot();
  }

  strlcpy(path_copy, path, sizeof(path_copy));

  // Trigger firmware loading with the selected path
  DEBUG_PRINT("selected: %s\n", path_copy);

  size_t path_len = strlen(path_copy);
  size_t ext_len = strlen(extension);

  if (path_len < ext_len || strcmp(path_copy + path_len - ext_len, extension) != 0)
  {
    DEBUG_PRINT("not a uf2: %s\n", path_copy);
    snprintf(status_message, sizeof(status_message), "ERR: File must be .uf2");
    text_directory_ui_set_status(status_message);
    return;
  }

  snprintf(status_message, sizeof(status_message), "Loading: %s", path_copy);
  text_directory_ui_set_status(status_message);

  load_firmware_by_path(path_copy);
}

int main()
{
  stdio_init_all();

  uart_init(uart0, 115200);
  uart_set_format(uart0, 8, 1, UART_PARITY_NONE);  // 8-N-1
  uart_set_fifo_enabled(uart0, false);

  // Initialize SD card detection pin

#if PICO_RP2350
  {
    uint8_t __attribute__((aligned(4))) workarea[4 * 1024];
    uintptr_t app_start_offset = 0;
    uint32_t app_size = 0;

    if (!bl_app_partition_get_info(workarea, sizeof(workarea), &app_start_offset, &app_size))
    {
      // FIXME: this should be an explicit infinite loop...
      return false;
    }
    // After this 0x10000000 should be remapped to the start of the app partition
    bl_remap_flash(app_start_offset, app_size);
  }
#endif

  lcd_init();

  // Initialize filesystem
  if (!fs_init())
  {
    text_directory_ui_set_status("Failed to mount SD card!");
    DEBUG_PRINT("Failed to mount SD card\n");
    sleep_ms(2000);
    reboot();
  }

#if ENABLE_USB
  usb_msc_init();
#endif

  // TEST: spróbuj otworzyć plik od razu po fs_init
  {
    FIL test_fp;
    FRESULT test_res = f_open(&test_fp, "/pico2-apps/micropython.uf2", FA_READ);
    char dbg[64];
    snprintf(dbg, sizeof(dbg), "Early f_open: %d", (int)test_res);
    if (test_res == FR_OK) {
      f_close(&test_fp);
      snprintf(dbg, sizeof(dbg), "Early f_open: OK!");
    }
    // Pokaż wynik przez 3 sekundy zanim uruchomi menu
    // Tymczasowy LCD print - użyjemy lcd_set_cursor + lcd_print_string
  }

  text_directory_ui_init();
  text_directory_ui_set_final_callback(final_selection_callback);

  keypad_init();

  while (keypad_get_key() > 0)
  {
    // drain the keypad input buffer
  }

  text_directory_ui_update_title();
  while (true)
  {
    text_directory_ui_run();
  }
}
