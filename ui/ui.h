#ifndef UI_H
#define UI_H

#include <stdbool.h>

// Display Pack 2.8" ma rozdzielczość 320x240
// Oryginalny PicoCalc miał 320x320 — nadpisujemy jego lokalne definicje

// Undef żeby uniknąć konfliktu z lokalnymi definicjami w text_directory_ui.c
#undef UI_WIDTH
#undef UI_HEIGHT
#undef UI_X
#undef UI_Y

#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240

// Obszar UI — pełny ekran minus marginesy
#define UI_WIDTH        300
#define UI_HEIGHT       220
#define UI_X            10
#define UI_Y            10

// Czcionka z font1.h — 8x8
#define FONT_WIDTH      8
#define FONT_HEIGHT     8
#define ENTRY_PADDING   2

// Ile pozycji mieści się na stronie
// Dostępna wysokość: 220 - 20 - 16 - 16 = 168px / (8 + 2) = 16
#define ITEMS_PER_PAGE  16

// Aktualizacja ikony baterii co N ms
// Display Pack nie ma baterii — bardzo długi interwał
#define BAT_UPDATE_MS   3600000

// Deklaracje funkcji SD — zdefiniowane w main.c
extern bool sd_insert_state;
bool sd_card_inserted(void);

// Deklaracje funkcji filesystem — zdefiniowane w main.c
bool fs_init(void);
void fs_deinit(void);
void reboot(void);

#endif // UI_H

