// config.h — Piny dla Raspberry Pi Pico 2W + Display Pack 2.8"

// SD card na SPI1
#define SD_SPI0         1       // używamy spi1
#define SD_SCLK_PIN     10
#define SD_MOSI_PIN     11
#define SD_MISO_PIN     12
#define SD_CS_PIN       13

// LCD ST7789 na SPI0
#define LCD_SPI1        0       // spi0
#define LCD_SCK_PIN     18
#define LCD_MOSI_PIN    19
#define LCD_MISO_PIN    -1
#define LCD_CS_PIN      17
#define LCD_DC_PIN      16
#define LCD_RST_PIN     -1
#define LCD_BL_PIN      20

// Przyciski Display Pack 2.8" — oficjalny pinout Pimoroni
#define BTN_A_PIN       12      // SW_A
#define BTN_B_PIN       13      // SW_B
#define BTN_X_PIN       14      // SW_X
#define BTN_Y_PIN       15      // SW_Y

// Aliasy używane przez key_event.c i stage3.c
#define BTN_UP          12      // SW_A
#define BTN_DOWN        13      // SW_B
#define BTN_ENTER       14      // SW_X
#define BTN_BACK        15      // SW_Y

// LED
#define LED_PIN         25

// SD card detect pin — nie używamy, definiujemy jako pin który zawsze zwraca "karta włożona"
// Display Pack 2.8" nie ma pinu SD detect
#define SD_DET_PIN      22
