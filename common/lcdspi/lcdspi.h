#ifndef LCDSPI_H
#define LCDSPI_H

#include <stdint.h>

// Piny Display Pack 2.8" — zdefiniowane w lcdspi.c
// Nie definiujemy ich tutaj żeby uniknąć konfliktów z config.h

// Kolory RGB565 (big-endian dla ST7789)
#define RGB(r,g,b) ((uint16_t)(((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3)))
#define WHITE    0xFFFF
#define BLACK    0x0000
#define RED      0xF800
#define GREEN    0x07E0
#define BLUE     0x001F
#define YELLOW   0xFFE0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define GRAY     0x8410
#define LITEGRAY 0xC618
#define ORANGE   0xFD20

// API publiczne
void lcd_init(void);
void lcd_clear(void);
void lcd_set_cursor(int x, int y);
void lcd_print_string(char *s);
void lcd_print_string_color(char *s, int fg, int bg);
void draw_rect_spi(int x1, int y1, int x2, int y2, int color);
void draw_line_spi(int x1, int y1, int x2, int y2, int color);
void draw_bitmap_spi(int x1, int y1, int width, int height, int scale,
                     int fc, int bc, unsigned char *bitmap);
void draw_battery_icon(int x0, int y0, int level);
void lcd_putc(uint8_t devn, uint8_t c);
int  lcd_getc(uint8_t devn);
void lcd_spi_raise_cs(void);
void lcd_spi_lower_cs(void);
void hw_send_spi(const unsigned char *buff, int cnt);
void spi_write_command(unsigned char cmd);
void spi_write_data(unsigned char data);

#endif // LCDSPI_H
