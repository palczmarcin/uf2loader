#ifndef UI_H
#define UI_H

#include <stdbool.h>

// Display Pack 2.8" — 320x240
// Oryginał PicoCalc miał 320x320

#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240

// Obszar UI — używaj tych zamiast oryginalnych 280x280
#undef UI_WIDTH
#undef UI_HEIGHT
#undef UI_X
#undef UI_Y
#define UI_WIDTH        300
#define UI_HEIGHT       220
#define UI_X            10
#define UI_Y            10

// FONT_HEIGHT i BAT_UPDATE_MS — NIE redefiniujemy
// text_directory_ui.h definiuje FONT_HEIGHT 12 i BAT_UPDATE_MS 60000
// zostawiamy ich wartości bez zmian

// Ile pozycji mieści się na stronie przy FONT_HEIGHT=12
// Dostępna wysokość: 220 - 20 - 14 - 14 = 172px / 14 = 12 pozycji
#define ITEMS_PER_PAGE  12

// Deklaracje funkcji SD i filesystem — zdefiniowane w main.c
extern bool sd_insert_state;
bool sd_card_inserted(void);
bool fs_init(void);
void fs_deinit(void);
void reboot(void);

#endif // UI_H
