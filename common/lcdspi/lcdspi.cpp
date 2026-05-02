/**
 * lcdspi.cpp
 * Warstwa adaptacyjna między uf2loader a sterownikiem ST7789 Pimoroni
 * Display Pack 2.8" — 320x240, SPI0, piny GP0/1/2/3/20
 */

#include "lcdspi.h"
#include "drivers/st7789/st7789.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"
#include "fonts/font1.h"

using namespace pimoroni;

// Piny Display Pack 2.8"
static const uint LCD_MOSI = 0;
static const uint LCD_CS   = 1;
static const uint LCD_SCK  = 2;
static const uint LCD_DC   = 3;
static const uint LCD_BL   = 20;

static ST7789 *st7789 = nullptr;

// Bufor ramki — 320x240x2 bajtów = 150KB
// W bootloaderze działamy z RAM więc musimy być oszczędni
// Używamy małego bufora roboczego i rysujemy liniami
static uint16_t line_buf[LCD_WIDTH];

static int cursor_x = 0;
static int cursor_y = 0;

static unsigned char *MainFont = (unsigned char *)font1;
static int font_w = 0;
static int font_h = 0;

// Kolory aktywne
static uint16_t color_fg = WHITE;
static uint16_t color_bg = BLACK;

// --- Inicjalizacja ---

void lcd_init(void) {
    SPIBus spi_bus(
        LCD_CS,
        LCD_DC,
        LCD_SCK,
        LCD_MOSI,
        PIN_UNUSED   // brak MISO
    );

    st7789 = new ST7789(
        LCD_WIDTH,
        LCD_HEIGHT,
        ROTATE_0,
        false,       // nie round
        spi_bus,
        LCD_BL
    );

    st7789->set_backlight(0);  // wyłączone na start

    font_w = MainFont[0];
    font_h = MainFont[1];

    lcd_clear();
    st7789->set_backlight(255);
}

// --- Czyszczenie ---

void lcd_clear(void) {
    if (!st7789) return;

    // Wyślij czarny kolor na cały ekran
    uint8_t cmd = 0x2C; // RAMWR

    // Ustaw okno na cały ekran
    uint8_t caset[] = {0x00, 0x00, 0x01, 0x3F}; // 0-319
    uint8_t raset[] = {0x00, 0x00, 0x00, 0xEF}; // 0-239

    // Użyj niskopoziomowego SPI przez st7789
    st7789->command(0x2A, 4, (const char*)caset);
    st7789->command(0x2B, 4, (const char*)raset);

    // Wyślij piksele
    memset(line_buf, 0x00, sizeof(line_buf));
    st7789->command(0x2C);

    // DC=1 dla danych
    gpio_put(LCD_DC, 1);
    gpio_put(LCD_CS, 0);
    for (int y = 0; y < LCD_HEIGHT; y++) {
        spi_write_blocking(spi0, (uint8_t*)line_buf, LCD_WIDTH * 2);
    }
    gpio_put(LCD_CS, 1);
}

// --- Rysowanie prostokąta ---

void draw_rect_spi(int x1, int y1, int x2, int y2, int color) {
    if (!st7789) return;
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= LCD_WIDTH)  x2 = LCD_WIDTH  - 1;
    if (y2 >= LCD_HEIGHT) y2 = LCD_HEIGHT - 1;

    int w = x2 - x1 + 1;
    uint16_t c = (uint16_t)color;
    // Swap bytes dla ST7789 (big-endian)
    uint16_t c_be = __builtin_bswap16(c);

    for (int i = 0; i < w; i++) line_buf[i] = c_be;

    uint8_t caset[] = {
        (uint8_t)(x1 >> 8), (uint8_t)(x1 & 0xFF),
        (uint8_t)(x2 >> 8), (uint8_t)(x2 & 0xFF)
    };
    uint8_t raset[] = {
        (uint8_t)(y1 >> 8), (uint8_t)(y1 & 0xFF),
        (uint8_t)(y2 >> 8), (uint8_t)(y2 & 0xFF)
    };

    st7789->command(0x2A, 4, (const char*)caset);
    st7789->command(0x2B, 4, (const char*)raset);
    st7789->command(0x2C);

    gpio_put(LCD_DC, 1);
    gpio_put(LCD_CS, 0);
    for (int y = y1; y <= y2; y++) {
        spi_write_blocking(spi0, (uint8_t*)line_buf, w * 2);
    }
    gpio_put(LCD_CS, 1);
}

// --- Linia ---

void draw_line_spi(int x1, int y1, int x2, int y2, int color) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;
    while (1) {
        draw_rect_spi(x1, y1, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x1 += sx; }
        if (e2 <  dy) { err += dx; y1 += sy; }
    }
}

// --- Tekst ---

void lcd_set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
}

static void draw_char(int x, int y, char c, int fg, int bg) {
    int w = font_w, h = font_h;
    unsigned char *bmp = MainFont + 2 + (unsigned char)c * ((w * h + 7) / 8);

    uint16_t fg_be = __builtin_bswap16((uint16_t)fg);
    uint16_t bg_be = __builtin_bswap16((uint16_t)bg);

    // Bufor jednego znaku
    static uint16_t char_buf[16 * 16]; // max 16x16
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            int bit  = row * w + col;
            int byte = bit / 8;
            int mask = 0x80 >> (bit % 8);
            char_buf[row * w + col] = (bmp[byte] & mask) ? fg_be : bg_be;
        }
    }

    uint8_t caset[] = {
        (uint8_t)(x >> 8), (uint8_t)(x & 0xFF),
        (uint8_t)((x+w-1) >> 8), (uint8_t)((x+w-1) & 0xFF)
    };
    uint8_t raset[] = {
        (uint8_t)(y >> 8), (uint8_t)(y & 0xFF),
        (uint8_t)((y+h-1) >> 8), (uint8_t)((y+h-1) & 0xFF)
    };

    st7789->command(0x2A, 4, (const char*)caset);
    st7789->command(0x2B, 4, (const char*)raset);
    st7789->command(0x2C);

    gpio_put(LCD_DC, 1);
    gpio_put(LCD_CS, 0);
    spi_write_blocking(spi0, (uint8_t*)char_buf, w * h * 2);
    gpio_put(LCD_CS, 1);
}

void lcd_print_string_color(char *s, int fg, int bg) {
    while (*s) {
        if (*s == '\n') {
            cursor_x = 0;
            cursor_y += font_h;
        } else {
            draw_char(cursor_x, cursor_y, *s, fg, bg);
            cursor_x += font_w;
            if (cursor_x + font_w > LCD_WIDTH) {
                cursor_x = 0;
                cursor_y += font_h;
            }
        }
        s++;
    }
}

void lcd_print_string(char *s) {
    lcd_print_string_color(s, WHITE, BLACK);
}

// --- Bitmap ---

void draw_bitmap_spi(int x1, int y1, int width, int height, int scale,
                     int fc, int bc, unsigned char *bitmap) {
    uint16_t fc_be = __builtin_bswap16((uint16_t)fc);
    uint16_t bc_be = __builtin_bswap16((uint16_t)bc);

    int x2 = x1 + width  * scale - 1;
    int y2 = y1 + height * scale - 1;
    if (x2 >= LCD_WIDTH)  x2 = LCD_WIDTH  - 1;
    if (y2 >= LCD_HEIGHT) y2 = LCD_HEIGHT - 1;

    uint8_t caset[] = {
        (uint8_t)(x1 >> 8), (uint8_t)(x1 & 0xFF),
        (uint8_t)(x2 >> 8), (uint8_t)(x2 & 0xFF)
    };
    uint8_t raset[] = {
        (uint8_t)(y1 >> 8), (uint8_t)(y1 & 0xFF),
        (uint8_t)(y2 >> 8), (uint8_t)(y2 & 0xFF)
    };

    st7789->command(0x2A, 4, (const char*)caset);
    st7789->command(0x2B, 4, (const char*)raset);
    st7789->command(0x2C);

    gpio_put(LCD_DC, 1);
    gpio_put(LCD_CS, 0);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < scale; j++) {
            for (int k = 0; k < width; k++) {
                int bit  = i * width + k;
                int byte = bit / 8;
                int mask = 0x80 >> (bit % 8);
                uint16_t px = (bitmap[byte] & mask) ? fc_be : bc_be;
                for (int m = 0; m < scale; m++) {
                    spi_write_blocking(spi0, (uint8_t*)&px, 2);
                }
            }
        }
    }
    gpio_put(LCD_CS, 1);
}

// --- Ikona baterii — Display Pack nie ma baterii ---

void draw_battery_icon(int x0, int y0, int level) {
    (void)x0; (void)y0; (void)level;
}

// --- Nieużywane ---

void lcd_putc(uint8_t devn, uint8_t c) {
    (void)devn;
    char s[2] = {(char)c, 0};
    lcd_print_string(s);
}

int lcd_getc(uint8_t devn) {
    (void)devn;
    return -1;
}

// --- Niskopoziomowe (wymagane przez lcdspi.h) ---

void lcd_spi_raise_cs(void) { gpio_put(LCD_CS, 1); }
void lcd_spi_lower_cs(void) { gpio_put(LCD_CS, 0); }

void hw_send_spi(const unsigned char *buff, int cnt) {
    spi_write_blocking(spi0, buff, cnt);
}

void spi_write_command(unsigned char cmd) {
    gpio_put(LCD_DC, 0);
    gpio_put(LCD_CS, 0);
    spi_write_blocking(spi0, &cmd, 1);
    gpio_put(LCD_CS, 1);
}

void spi_write_data(unsigned char data) {
    gpio_put(LCD_DC, 1);
    gpio_put(LCD_CS, 0);
    spi_write_blocking(spi0, &data, 1);
    gpio_put(LCD_CS, 1);
}
