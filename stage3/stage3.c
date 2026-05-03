#include <pico.h>
#include <pico/time.h>
#include <pico/bootrom.h>
#include <boot/picobin.h>
#include <hardware/flash.h>
#include <hardware/gpio.h>
#include "proginfo.h"
#include "pff.h"
#include "uf2.h"
#include "debug.h"

// Piny przycisków Display Pack 2.8"
#define BTN_UP    6   // SW_A — wejście w menu / SD
#define BTN_DOWN  7   // SW_B — BOOTSEL
#define BTN_ENTER 8   // SW_X
#define BTN_BACK  9   // SW_Y

void _Noreturn infinite_loop(void)
{
  while (1)
  {
    tight_loop_contents();
  }
}

#if PICO_RP2040
#include "hardware/regs/m0plus.h"
#define LOADER "BOOT2040.UF2"
void _Noreturn launch_application_from(uint32_t *app_location)
{
  uint32_t *new_vector_table = app_location;
  volatile uint32_t *vtor    = (uint32_t *)(PPB_BASE + M0PLUS_VTOR_OFFSET);
  *vtor                      = (uint32_t)new_vector_table;
  asm volatile(
      "msr msp, %0\n"
      "bx %1\n"
      :
      : "r"(new_vector_table[0]), "r"(new_vector_table[1])
      :);
  infinite_loop();
}
void launch_application(void)
{
  if (bl_proginfo_valid())
  {
#if ENABLE_DEBUG
    stdio_deinit_all();
#endif
    launch_application_from((void *)XIP_BASE + 0x100);
  }
}
void launch_application_from_ram(void)
{
#if ENABLE_DEBUG
  stdio_deinit_all();
#endif
  launch_application_from((void *)(SRAM_BASE + 0x100));
}

#elif PICO_RP2350
#define LOADER "BOOT2350.UF2"
uint8_t __attribute__((aligned(1024))) workarea[4 * 1024];
uintptr_t app_start_offset = 0;
uint32_t app_size          = 0;
void launch_application(void)
{
  stdio_deinit_all();
  rom_chain_image(workarea, sizeof(workarea), (XIP_BASE + app_start_offset), app_size);
}
void launch_application_from_ram(void)
{
  rom_chain_image(workarea, sizeof(workarea), SRAM_BASE, 0x1000);
}
#endif

static void init_buttons(void)
{
  gpio_init(BTN_UP);    gpio_set_dir(BTN_UP,    GPIO_IN); gpio_pull_up(BTN_UP);
  gpio_init(BTN_DOWN);  gpio_set_dir(BTN_DOWN,  GPIO_IN); gpio_pull_up(BTN_DOWN);
  gpio_init(BTN_ENTER); gpio_set_dir(BTN_ENTER, GPIO_IN); gpio_pull_up(BTN_ENTER);
  gpio_init(BTN_BACK);  gpio_set_dir(BTN_BACK,  GPIO_IN); gpio_pull_up(BTN_BACK);
}

enum bootmode_e read_bootmode()
{
  init_buttons();

  // Odczekaj chwilę żeby przyciski się ustabilizowały
  sleep_ms(50);

  int end_time = time_us_32() + 500000; // 0.5s
  while (((int)time_us_32() - end_time) < 0)
  {
    // BTN_UP (SW_A) — wejdź w menu SD
    if (!gpio_get(BTN_UP) || !gpio_get(BTN_ENTER))
    {
      return BOOT_SD;
    }
    // BTN_DOWN (SW_B) — wejdź w BOOTSEL
    if (!gpio_get(BTN_DOWN))
    {
      return BOOT_UPDATE;
    }
  }

  return BOOT_DEFAULT;
}

int main()
{
  char *filename = LOADER;

#ifdef ENABLE_DEBUG
  stdio_init_all();
#endif

  enum bootmode_e mode = read_bootmode();
  uint32_t arg;
  if (bl_get_command(&mode, &arg))
  {
    if (mode == BOOT_RAM)
    {
      filename = (char*)arg;
    }
  }

#if PICO_RP2350
  if (!bl_app_partition_get_info(workarea, sizeof(workarea), &app_start_offset, &app_size))
  {
    mode = BOOT_UPDATE;
  }
#endif

  if (mode == BOOT_UPDATE)
  {
    DEBUG_PRINT("Entering BOOTSEL mode\n");
    reset_usb_boot(0, 0);
  }

  if (mode == BOOT_DEFAULT)
  {
    DEBUG_PRINT("Boot application from flash\n");
    launch_application();
  }

  DEBUG_PRINT("Loading UI\n");
  FATFS fs;
  FRESULT fr = FR_NOT_READY;
  for (int retry = 5; retry > 0; retry--)
  {
    fr = pf_mount(&fs);
    if (fr == FR_OK)
    {
      break;
    }
    sleep_ms(500);
  }
  if (fr == FR_OK)
  {
    if (load_application_from_uf2(filename))
    {
      DEBUG_PRINT("Launch UI\n");
      launch_application_from_ram();
    }
  }

  reset_usb_boot(0, 0);
}
