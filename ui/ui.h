#ifndef UI_H
#define UI_H

// Display Pack 2.8" ma rozdzielczość 320x240
// Oryginalny PicoCalc miał 320x320

#define SCREEN_WIDTH    320
#define SCREEN_HEIGHT   240

// Obszar UI — pełny ekran minus marginesy
#define UI_WIDTH        300
#define UI_HEIGHT       220

// Czcionka z font1.h — 8x8
#define FONT_WIDTH      8
#define FONT_HEIGHT     8
#define ENTRY_PADDING   2

// Ile pozycji mieści się na stronie
// Dostępna wysokość: UI_HEIGHT - HEADER_TITLE_HEIGHT - PATH_HEADER_HEIGHT - STATUS_BAR_HEIGHT
// = 220 - 20 - 16 - 16 = 168px
// 168 / (8 + 2) = 16 pozycji
#define ITEMS_PER_PAGE  16

// Aktualizacja ikony baterii co N ms
// Display Pack nie ma baterii — ustawiamy bardzo długi interwał
#define BAT_UPDATE_MS   3600000

#endif // UI_H
