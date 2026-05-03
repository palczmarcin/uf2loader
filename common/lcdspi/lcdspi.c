/**
 * lcdspi.c — Sterownik ST7789 dla Display Pack 2.8" (320x240, SPI0)
 * Piny: MOSI=GP0, CS=GP1, SCK=GP2, DC=GP3, BL=GP20
 * Tylko SPI — zero zależności od PIO lub bibliotek Pimoroni
 */

#include <string.h>
#include <stdint.h>
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "lcdspi.h"
#include "fonts/font1.h"

// --- Piny ---
#define PIN_MOSI  0
#define PIN_CS    1
#define PIN_SCK   2
#define PIN_DC    3
#define PIN_BL    20

// --- Wymiary ---
#define W  320
#define H  240

// --- Czcionka ---
static unsigned char *Font = (unsigned char *)font1;
static int fw, fh;

// --- Stan kursora i kolorów ---
static int cur_x = 0, cur_y = 0;

// --- Niskopoziomowe SPI ---

static inline void cs_lo(void) { gpio_put(PIN_CS, 0); }
static inline void cs_hi(void) { gpio_put(PIN_CS, 1); }
static inline void dc_cmd(void) { gpio_put(PIN_DC, 0); }
static inline void dc_dat(void) { gpio_put(PIN_DC, 1); }

static void write_cmd(uint8_t cmd) {
    dc_cmd(); cs_lo();
    spi_write_blocking(spi0, &cmd, 1);
    cs_hi();
}

static void write_dat(const uint8_t *buf, size_t len) {
    dc_dat(); cs_lo();
    spi_write_blocking(spi0, buf, len);
    cs_hi();
}

static void write_dat1(uint8_t d) { write_dat(&d, 1); }

static void cmd_data(uint8_t cmd, const uint8_t *data, size_t len) {
    write_cmd(cmd);
    if (len) write_dat(data, len);
}

// --- Okno rysowania ---

static void set_window(int x1, int y1, int x2, int y2) {
    uint8_t d[4];
    d[0]=x1>>8; d[1]=x1&0xFF; d[2]=x2>>8; d[3]=x2&0xFF;
    cmd_data(0x2A, d, 4);
    d[0]=y1>>8; d[1]=y1&0xFF; d[2]=y2>>8; d[3]=y2&0xFF;
    cmd_data(0x2B, d, 4);
    write_cmd(0x2C);
    dc_dat(); cs_lo();
    // CS zostaje low — dane pikseli są wysyłane po tym wywołaniu
}

static void end_write(void) { cs_hi(); }

// --- Inicjalizacja ST7789 ---

static void st7789_init(void) {
    // Software reset
    write_cmd(0x01); sleep_ms(150);

    // Sleep out
    write_cmd(0x11); sleep_ms(10);

    // COLMOD: 16-bit RGB565
    cmd_data(0x3A, (uint8_t[]){0x05}, 1);

    // MADCTL: MX + BGR dla Display Pack 2.8" (landscape 320x240)
    cmd_data(0x36, (uint8_t[]){0x00}, 1);

    // PORCTRL
    cmd_data(0xB2, (uint8_t[]){0x0C,0x0C,0x00,0x33,0x33}, 5);

    // GCTRL
    cmd_data(0xB7, (uint8_t[]){0x35}, 1);

    // VCOMS
    cmd_data(0xBB, (uint8_t[]){0x19}, 1);

    // LCMCTRL
    cmd_data(0xC0, (uint8_t[]){0x2C}, 1);

    // VDVVRHEN
    cmd_data(0xC2, (uint8_t[]){0x01}, 1);

    // VRHS
    cmd_data(0xC3, (uint8_t[]){0x12}, 1);

    // VDVS
    cmd_data(0xC4, (uint8_t[]){0x20}, 1);

    // FRCTRL2
    cmd_data(0xC6, (uint8_t[]){0x0F}, 1);

    // PWCTRL1
    cmd_data(0xD0, (uint8_t[]){0xA4,0xA1}, 2);

    // RAMCTRL — naprawia zielone paski
    cmd_data(0xB0, (uint8_t[]){0x00,0xC0}, 2);

    // Gamma positive
    cmd_data(0xE0, (uint8_t[]){
        0xD0,0x08,0x11,0x08,0x0C,0x15,0x39,0x33,
        0x50,0x36,0x13,0x14,0x29,0x2D
    }, 14);

    // Gamma negative
    cmd_data(0xE1, (uint8_t[]){
        0xD0,0x08,0x10,0x08,0x06,0x06,0x39,0x44,
        0x51,0x0B,0x16,0x14,0x2F,0x31
    }, 14);

    // Inversion on (wymagane przez ST7789)
    write_cmd(0x21);

    // Normal display mode on
    write_cmd(0x13); sleep_ms(10);

    // Display on
    write_cmd(0x29); sleep_ms(100);

    // CASET 0-319, RASET 0-239
    cmd_data(0x2A, (uint8_t[]){0x00,0x00,0x01,0x3F}, 4);
    cmd_data(0x2B, (uint8_t[]){0x00,0x00,0x00,0xEF}, 4);
}

// --- API publiczne ---

void lcd_init(void) {
    // SPI0 @ 40MHz
    spi_init(spi0, 40 * 1000 * 1000);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS); gpio_set_dir(PIN_CS, GPIO_OUT); gpio_put(PIN_CS, 1);
    gpio_init(PIN_DC); gpio_set_dir(PIN_DC, GPIO_OUT); gpio_put(PIN_DC, 1);
    gpio_init(PIN_BL); gpio_set_dir(PIN_BL, GPIO_OUT); gpio_put(PIN_BL, 0);

    sleep_ms(10);

    st7789_init();
    lcd_clear();

    // Podświetlenie ON po inicjalizacji
    gpio_put(PIN_BL, 1);

    fw = Font[0];
    fh = Font[1];
}

void lcd_clear(void) {
    set_window(0, 0, W-1, H-1);
    // Wyślij 320*240*2 = 153600 bajtów zera (czarny)
    static uint8_t zero[320*2];
    memset(zero, 0, sizeof(zero));
    for (int y = 0; y < H; y++) {
        spi_write_blocking(spi0, zero, sizeof(zero));
    }
    end_write();
}

void draw_rect_spi(int x1, int y1, int x2, int y2, int color) {
    if (x1>x2){int t=x1;x1=x2;x2=t;}
    if (y1>y2){int t=y1;y1=y2;y2=t;}
    if (x1<0) x1=0; if (y1<0) y1=0;
    if (x2>=W) x2=W-1; if (y2>=H) y2=H-1;

    int n = (x2-x1+1);
    uint16_t c = __builtin_bswap16((uint16_t)color);
    static uint8_t row[320*2];
    for (int i=0; i<n; i++) {
        row[i*2]   = c>>8;
        row[i*2+1] = c&0xFF;
    }
    set_window(x1, y1, x2, y2);
    for (int y=y1; y<=y2; y++) {
        spi_write_blocking(spi0, row, n*2);
    }
    end_write();
}

void draw_line_spi(int x1, int y1, int x2, int y2, int color) {
    int dx=abs(x2-x1), sx=x1<x2?1:-1;
    int dy=abs(y2-y1), sy=y1<y2?1:-1;
    int err=(dx>dy?dx:-dy)/2, e2;
    while(1){
        draw_rect_spi(x1,y1,x1,y1,color);
        if(x1==x2&&y1==y2) break;
        e2=err;
        if(e2>-dx){err-=dy;x1+=sx;}
        if(e2< dy){err+=dx;y1+=sy;}
    }
}

void lcd_set_cursor(int x, int y) { cur_x=x; cur_y=y; }

static void draw_char(int x, int y, char c, int fg, int bg) {
    unsigned char *bmp = Font + 2 + (uint8_t)c * ((fw*fh+7)/8);
    uint16_t fg_be = __builtin_bswap16((uint16_t)fg);
    uint16_t bg_be = __builtin_bswap16((uint16_t)bg);
    set_window(x, y, x+fw-1, y+fh-1);
    for (int r=0; r<fh; r++) {
        for (int col=0; col<fw; col++) {
            int bit=r*fw+col;
            uint16_t px = (bmp[bit/8]&(0x80>>(bit%8))) ? fg_be : bg_be;
            spi_write_blocking(spi0, (uint8_t*)&px, 2);
        }
    }
    end_write();
}

void lcd_print_string_color(char *s, int fg, int bg) {
    while (*s) {
        if (*s=='\n') { cur_x=0; cur_y+=fh; }
        else {
            draw_char(cur_x, cur_y, *s, fg, bg);
            cur_x+=fw;
            if (cur_x+fw>W) { cur_x=0; cur_y+=fh; }
        }
        s++;
    }
}

void lcd_print_string(char *s) {
    lcd_print_string_color(s, WHITE, BLACK);
}

void draw_bitmap_spi(int x1, int y1, int width, int height, int scale,
                     int fc, int bc, unsigned char *bitmap) {
    uint16_t fc_be = __builtin_bswap16((uint16_t)fc);
    uint16_t bc_be = __builtin_bswap16((uint16_t)bc);
    int x2 = x1+width*scale-1;
    int y2 = y1+height*scale-1;
    if (x2>=W) x2=W-1;
    if (y2>=H) y2=H-1;
    set_window(x1, y1, x2, y2);
    for (int i=0; i<height; i++) {
        for (int j=0; j<scale; j++) {
            for (int k=0; k<width; k++) {
                int bit=i*width+k;
                uint16_t px=(bitmap[bit/8]&(0x80>>(bit%8)))?fc_be:bc_be;
                for (int m=0; m<scale; m++) {
                    spi_write_blocking(spi0,(uint8_t*)&px,2);
                }
            }
        }
    }
    end_write();
}

void draw_battery_icon(int x0, int y0, int level) {
    (void)x0; (void)y0; (void)level;
}

void lcd_putc(uint8_t devn, uint8_t c) {
    (void)devn;
    char s[2]={(char)c,0};
    lcd_print_string(s);
}

int lcd_getc(uint8_t devn) { (void)devn; return -1; }

void lcd_spi_raise_cs(void) { cs_hi(); }
void lcd_spi_lower_cs(void) { cs_lo(); }

void hw_send_spi(const unsigned char *buff, int cnt) {
    spi_write_blocking(spi0, buff, cnt);
}

void spi_write_command(unsigned char cmd) { write_cmd(cmd); }
void spi_write_data(unsigned char data)   { write_dat1(data); }
