/**
 * PicoCalc UF2 Firmware Loader
 * Modified for Pimoroni Pico Display Pack 2.8" (ST7789, 320x240)
 */

#include <pico.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "hardware/gpio.h"
#include "debug.h"
#include "lcdspi.h"
#include <hardware/flash.h>
#include <hardware/watchdog.h>
#include "config.h"

#include "ff.h"

#if ENABLE_USB
#include "usb_msc.h"
#include "tusb.h"
#endif

#include "text_directory_ui.h"
#include "key_event.h"

#include "proginfo.h"
#include "uf2.h"
#include "ui.h"

FATFS fat;

// Brak detektora SD — zakładamy że karta zawsze jest
bool sd_insert_state = true;

bool sd_card_inserted(void) {
    return true;
}

void fs_deinit(void) { f_unmount("/"); }

bool fs_init(void) {
    DEBUG_PRINT("fs init SD\n");
    FRESULT res = f_mount(&fat, "/", 1);
    if (res != FR_OK) {
        DEBUG_PRINT("mount err: %d\n", res);
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

void load_firmware_by_path(const char *path) {
    text_directory_ui_set_status("Loading app...");

    enum uf2_result_e result = load_application_from_uf2(path);

    switch (result) {
        case UF2_LOADED:
            text_directory_ui_set_status("Launching...");
            DEBUG_PRINT("launching app\n");
            reboot();
            break;
        case UF2_WRONG_PLATFORM:
            text_directory_ui_set_status("ERR: Not for this device");
            break;
        case UF2_BAD:
            text_directory_ui_set_status("ERR: Bad UF2 file");
            break;
        default:
            text_directory_ui_set_status("ERR: Unexpected error");
            break;
    }
}

void final_selection_callback(const char *path) {
    char status_message[128];
    const char *extension = ".uf2";

    if (path == NULL) {
        reboot();
    }

    DEBUG_PRINT("selected: %s\n", path);

    size_t path_len = strlen(path);
    size_t ext_len  = strlen(extension);

    if (path_len < ext_len ||
        strcmp(path + path_len - ext_len, extension) != 0) {
        snprintf(status_message, sizeof(status_message), "ERR: File must be .uf2");
        text_directory_ui_set_status(status_message);
        return;
    }

    snprintf(status_message, sizeof(status_message), "SEL: %s", path);
    text_directory_ui_set_status(status_message);
    load_firmware_by_path(path);
}

int main() {
    stdio_init_all();

#if PICO_RP2350
    {
        uint8_t __attribute__((aligned(4))) workarea[4 * 1024];
        uintptr_t app_start_offset = 0;
        uint32_t app_size = 0;

        if (!bl_app_partition_get_info(workarea, sizeof(workarea),
                                       &app_start_offset, &app_size)) {
            return false;
        }
        bl_remap_flash(app_start_offset, app_size);
    }
#endif

    lcd_init();

    if (!fs_init()) {
        text_directory_ui_set_status("Failed to mount SD card!");
        DEBUG_PRINT("Failed to mount SD card\n");
        sleep_ms(2000);
        reboot();
    }

#if ENABLE_USB
    usb_msc_init();
#endif

    text_directory_ui_init();
    text_directory_ui_set_final_callback(final_selection_callback);

    keypad_init();

    while (keypad_get_key() > 0) {
        // drain input buffer
    }

    text_directory_ui_update_title();
    while (true) {
        text_directory_ui_run();
    }
}
