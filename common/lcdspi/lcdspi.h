#ifndef LCDSPI_H
#define LCDSPI_H

#include "pico/multicore.h"
#include <hardware/spi.h>
#include "config.h"

// ST7789 — Display Pack 2.8" (320x240)
#define LCD_WIDTH   320
#define LCD_HEIGHT  240
#define LCD_SPI_SPEED 40000000

// ST7789 commands
#define ST7789_NOP        0x00
#define ST7789_SWRESET    0x01
#define ST7789_SLPOUT     0x11
#define ST7789_NORON      0x13
#define ST7789_INVOFF     0x20
#define ST7789_INVON      0x21
#define ST7789_DISPOFF    0x28
#define ST7789_DISPON     0x29
#define ST7789_CASET      0x2A
#define ST7789_RASET      0x2B
#define ST7789_RAMWR      0x2C
#define ST7789_MADCTL     0x36
#define ST7789_COLMOD     0x3A
#define ST7789_PORCTRL    0xB2
#define ST7789_GCTRL      0xB7
#define ST7789_VCOMS      0xBB
#define ST7789_LCMCTRL    0xC0
#define ST7789_VDVVRHEN   0xC2
#define ST7789_VRHS       0xC3
#define ST7789_VDVS       0xC4
#define ST7789_FRCTRL2    0xC6
#define ST7789_PWCTRL1    0xD0
#define ST7789_PVGAMCTRL  0xE0
#define ST7789_NVGAMCTRL  0xE1

// MADCTL bits
#define MADCTL_MY   0x80
#define MADCTL_MX   0x40
#define MADCTL_MV   0x20
#define MADCTL_BGR  0x08

// Kolory RGB888 -> RGB565
#define RGB565(r,g,b) ((((r)&0xF8)<<8)|(((g)&0xFC)<<3)|((b)>>3))
#define BLACK       RGB565(0,   0,   0)
#define WHITE       RGB565(255, 255, 255)
#define RED         RGB565(255, 0,   0)
#define GREEN       RGB565(0,   255, 0)
#define BLUE        RGB565(0,   0,   255)
#define YELLOW      RGB565(255, 255, 0)
#define CYAN        RGB565(0,   255, 255)
#define MAGENTA     RGB565(255, 0,   255)
#define GRAY        RGB565(128, 128, 128)
#define LITEGRAY    RGB565(210, 210, 210)
#define ORANGE      RGB565(255, 165, 0)

#define Pico_LCD_SPI_MOD spi0

// Publiczne API (zachowane dla kompatybilności z resztą uf2loadera)
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

// Niskopoziomowe SPI
void lcd_spi_raise_cs(void);
void lcd_spi_lower_cs(void);
void hw_send_spi(const unsigned char *buff, int cnt);
void spi_write_command(unsigned char cmd);
void spi_write_data(unsigned char data);

#endif // LCDSPI_H
