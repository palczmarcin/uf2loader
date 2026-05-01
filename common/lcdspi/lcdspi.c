#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"

#include "lcdspi.h"
#include "fonts/font1.h"

unsigned char *MainFont = (unsigned char *) font1;

static int gui_font_width;
static int gui_font_height;
static short current_x = 0, current_y = 0;
static short hres = LCD_WIDTH;
static short vres = LCD_HEIGHT;

// --- Niskopoziomowe SPI ---

void lcd_spi_raise_cs(void) {
    gpio_put(LCD_CS_PIN, 1);
}

void lcd_spi_lower_cs(void) {
    gpio_put(LCD_CS_PIN, 0);
}

void hw_send_spi(const unsigned char *buff, int cnt) {
    spi_write_blocking(spi0, buff, cnt);
}

void spi_write_command(unsigned char cmd) {
    gpio_put(LCD_DC_PIN, 0);
    lcd_spi_lower_cs();
    spi_write_blocking(spi0, &cmd, 1);
    lcd_spi_raise_cs();
}

void spi_write_data(unsigned char data) {
    gpio_put(LCD_DC_PIN, 1);
    lcd_spi_lower_cs();
    spi_write_blocking(spi0, &data, 1);
    lcd_spi_raise_cs();
}

static void write_cmd_data(uint8_t cmd, const uint8_t *data, int len) {
    spi_write_command(cmd);
    if (len > 0 && data) {
        gpio_put(LCD_DC_PIN, 1);
        lcd_spi_lower_cs();
        spi_write_blocking(spi0, data, len);
        lcd_spi_raise_cs();
    }
}

// --- Inicjalizacja ST7789 ---

static void st7789_set_window(int x1, int y1, int x2, int y2) {
    uint8_t data[4];

    data[0] = x1 >> 8; data[1] = x1 & 0xFF;
    data[2] = x2 >> 8; data[3] = x2 & 0xFF;
    write_cmd_data(ST7789_CASET, data, 4);

    data[0] = y1 >> 8; data[1] = y1 & 0xFF;
    data[2] = y2 >> 8; data[3] = y2 & 0xFF;
    write_cmd_data(ST7789_RASET, data, 4);

    spi_write_command(ST7789_RAMWR);
    gpio_put(LCD_DC_PIN, 1);
    lcd_spi_lower_cs();
}

static void st7789_init_sequence(void) {
    spi_write_command(ST7789_SWRESET);
    sleep_ms(150);

    spi_write_command(ST7789_SLPOUT);
    sleep_ms(10);

    write_cmd_data(ST7789_COLMOD,   (uint8_t[]){0x55}, 1); // 16-bit RGB565
    write_cmd_data(ST7789_MADCTL,   (uint8_t[]){MADCTL_MX | MADCTL_BGR}, 1);
    write_cmd_data(ST7789_PORCTRL,  (uint8_t[]){0x0C, 0x0C, 0x00, 0x33, 0x33}, 5);
    write_cmd_data(ST7789_GCTRL,    (uint8_t[]){0x35}, 1);
    write_cmd_data(ST7789_VCOMS,    (uint8_t[]){0x19}, 1);
    write_cmd_data(ST7789_LCMCTRL, (uint8_t[]){0x2C}, 1);
    write_cmd_data(ST7789_VDVVRHEN,(uint8_t[]){0x01}, 1);
    write_cmd_data(ST7789_VRHS,    (uint8_t[]){0x12}, 1);
    write_cmd_data(ST7789_VDVS,    (uint8_t[]){0x20}, 1);
    write_cmd_data(ST7789_FRCTRL2, (uint8_t[]){0x0F}, 1);
    write_cmd_data(ST7789_PWCTRL1, (uint8_t[]){0xA4, 0xA1}, 2);

    write_cmd_data(ST7789_PVGAMCTRL, (uint8_t[]){
        0xD0,0x04,0x0D,0x11,0x13,0x2B,0x3F,0x54,0x4C,0x18,0x0D,0x0B,0x1F,0x23
    }, 14);
    write_cmd_data(ST7789_NVGAMCTRL, (uint8_t[]){
        0xD0,0x04,0x0C,0x11,0x13,0x2C,0x3F,0x44,0x51,0x2F,0x1F,0x1F,0x20,0x23
    }, 14);

    spi_write_command(ST7789_INVON);
    sleep_ms(10);
    spi_write_command(ST7789_NORON);
    sleep_ms(10);
    spi_write_command(ST7789_DISPON);
    sleep_ms(10);
}

void lcd_spi_init(void) {
    spi_init(spi0, LCD_SPI_SPEED);
    gpio_set_function(LCD_SCK_PIN,  GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI_PIN, GPIO_FUNC_SPI);

    gpio_init(LCD_CS_PIN);
    gpio_init(LCD_DC_PIN);
    gpio_init(LCD_BL_PIN);

    gpio_set_dir(LCD_CS_PIN,  GPIO_OUT);
    gpio_set_dir(LCD_DC_PIN,  GPIO_OUT);
    gpio_set_dir(LCD_BL_PIN,  GPIO_OUT);

    lcd_spi_raise_cs();
    gpio_put(LCD_BL_PIN, 1);  // podświetlenie ON
}

void lcd_init(void) {
    lcd_spi_init();
    st7789_init_sequence();

    gui_font_width  = MainFont[0];
    gui_font_height = MainFont[1];
}

// --- Rysowanie ---

void lcd_clear(void) {
    int total = LCD_WIDTH * LCD_HEIGHT;
    st7789_set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    uint8_t px[2] = {0x00, 0x00};  // czarny RGB565
    for (int i = 0; i < total; i++) {
        spi_write_blocking(spi0, px, 2);
    }
    lcd_spi_raise_cs();
}

void draw_rect_spi(int x1, int y1, int x2, int y2, int color) {
    if (x1 > x2) { int t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { int t = y1; y1 = y2; y2 = t; }
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= hres) x2 = hres - 1;
    if (y2 >= vres) y2 = vres - 1;

    int total = (x2 - x1 + 1) * (y2 - y1 + 1);
    uint8_t hi = (color >> 8) & 0xFF;
    uint8_t lo = color & 0xFF;

    st7789_set_window(x1, y1, x2, y2);
    for (int i = 0; i < total; i++) {
        spi_write_blocking(spi0, &hi, 1);
        spi_write_blocking(spi0, &lo, 1);
    }
    lcd_spi_raise_cs();
}

void draw_line_spi(int x1, int y1, int x2, int y2, int color) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;
    while (1) {
        draw_rect_spi(x1, y1, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x1 += sx; }
        if (e2 < dy)  { err += dx; y1 += sy; }
    }
}

// --- Tekst ---

void lcd_set_cursor(int x, int y) {
    current_x = x;
    current_y = y;
}

static void draw_char(int x, int y, char c, int fc, int bc) {
    int w = gui_font_width;
    int h = gui_font_height;
    unsigned char *bmp = MainFont + 2 + (unsigned char)c * ((w * h + 7) / 8);

    st7789_set_window(x, y, x + w - 1, y + h - 1);

    uint8_t fhi = (fc >> 8) & 0xFF, flo = fc & 0xFF;
    uint8_t bhi = (bc >> 8) & 0xFF, blo = bc & 0xFF;

    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            int bit = row * w + col;
            int byte = bit / 8;
            int mask = 0x80 >> (bit % 8);
            if (bmp[byte] & mask) {
                spi_write_blocking(spi0, &fhi, 1);
                spi_write_blocking(spi0, &flo, 1);
            } else {
                spi_write_blocking(spi0, &bhi, 1);
                spi_write_blocking(spi0, &blo, 1);
            }
        }
    }
    lcd_spi_raise_cs();
}

void lcd_print_string_color(char *s, int fg, int bg) {
    while (*s) {
        if (*s == '\n') {
            current_x = 0;
            current_y += gui_font_height;
        } else {
            draw_char(current_x, current_y, *s, fg, bg);
            current_x += gui_font_width;
            if (current_x + gui_font_width > hres) {
                current_x = 0;
                current_y += gui_font_height;
            }
        }
        s++;
    }
}

void lcd_print_string(char *s) {
    lcd_print_string_color(s, WHITE, BLACK);
}

void draw_bitmap_spi(int x1, int y1, int width, int height, int scale,
                     int fc, int bc, unsigned char *bitmap) {
    int XEnd = x1 + width * scale - 1;
    int YEnd = y1 + height * scale - 1;
    if (XEnd >= hres) XEnd = hres - 1;
    if (YEnd >= vres) YEnd = vres - 1;

    st7789_set_window(x1, y1, XEnd, YEnd);

    uint8_t fhi = (fc >> 8) & 0xFF, flo = fc & 0xFF;
    uint8_t bhi = (bc >> 8) & 0xFF, blo = bc & 0xFF;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < scale; j++) {
            for (int k = 0; k < width; k++) {
                int bit = i * width + k;
                int byte = bit / 8;
                int mask = 0x80 >> (bit % 8);
                uint8_t hi, lo;
                if (bitmap[byte] & mask) { hi = fhi; lo = flo; }
                else                     { hi = bhi; lo = blo; }
                for (int m = 0; m < scale; m++) {
                    spi_write_blocking(spi0, &hi, 1);
                    spi_write_blocking(spi0, &lo, 1);
                }
            }
        }
    }
    lcd_spi_raise_cs();
}

// --- Ikona baterii — zastąpiona pustą funkcją (Display Pack nie ma baterii) ---

void draw_battery_icon(int x0, int y0, int level) {
    // Display Pack 2.8" nie ma wbudowanej baterii — nic nie rysujemy
    (void)x0; (void)y0; (void)level;
}

// --- Klawiatura (nieużywane w bootloaderze — obsługa przez key_event) ---

void lcd_putc(uint8_t devn, uint8_t c) {
    (void)devn;
    char s[2] = {c, 0};
    lcd_print_string(s);
}

int lcd_getc(uint8_t devn) {
    (void)devn;
    return -1;
}
