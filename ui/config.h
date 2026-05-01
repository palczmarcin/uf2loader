#ifndef CONFIG_H
#define CONFIG_H

// GPIOs for SPI interface (SD card) — SPI1, GP10-GP13
#define SD_SPI_ID       1
#define SD_SCLK_PIN     10
#define SD_MOSI_PIN     11
#define SD_MISO_PIN     12
#define SD_CS_PIN       13
// Brak detektora karty SD — zakładamy że karta zawsze jest

// GPIOs for LCD (Display Pack 2.8" — ST7789, SPI0)
#define LCD_SPI_ID      0
#define LCD_SCK_PIN     2
#define LCD_MOSI_PIN    0
#define LCD_CS_PIN      1
#define LCD_DC_PIN      3
#define LCD_BL_PIN      20
// Brak LCD_RST_PIN — RESET podłączony do RUN
// Brak LCD_MISO_PIN — Display Pack nie ma MISO

// GPIOs for buttons (Display Pack 2.8")
// A = w górę / poprzedni
// B = w dół / następny
// X = potwierdź / enter
// Y = wstecz / anuluj
#define BTN_UP      6   // SW_A
#define BTN_DOWN    7   // SW_B
#define BTN_ENTER   8   // SW_X
#define BTN_BACK    9   // SW_Y

// Pico-internal GPIOs
#define PICO_PS     23
#define LED_PIN     25

#endif // CONFIG_H
