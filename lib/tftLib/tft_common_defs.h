#pragma once

extern __attribute__((weak)) void tft_info(const char*);

#define ANSI_ESC_RESET "\033[0m"

#define ANSI_ESC_BLACK   "\033[30m"
#define ANSI_ESC_RED     "\033[31m"
#define ANSI_ESC_GREEN   "\033[32m"
#define ANSI_ESC_YELLOW  "\033[33m"
#define ANSI_ESC_BLUE    "\033[34m"
#define ANSI_ESC_MAGENTA "\033[35m"
#define ANSI_ESC_CYAN    "\033[36m"
#define ANSI_ESC_WHITE   "\033[37m"

#define ANSI_ESC_GREY         "\033[90m"
#define ANSI_ESC_LIGHTRED     "\033[91m"
#define ANSI_ESC_LIGHTGREEN   "\033[92m"
#define ANSI_ESC_LIGHTYELLOW  "\033[93m"
#define ANSI_ESC_LIGHTBLUE    "\033[94m"
#define ANSI_ESC_LIGHTMAGENTA "\033[95m"
#define ANSI_ESC_LIGHTCYAN    "\033[96m"
#define ANSI_ESC_LIGHTGREY    "\033[97m"

#define ANSI_ESC_DARKRED     "\033[38;5;52m"
#define ANSI_ESC_DARKGREEN   "\033[38;5;22m"
#define ANSI_ESC_DARKYELLOW  "\033[38;5;136m"
#define ANSI_ESC_DARKBLUE    "\033[38;5;17m"
#define ANSI_ESC_DARKMAGENTA "\033[38;5;53m"
#define ANSI_ESC_DARKCYAN    "\033[38;5;23m"
#define ANSI_ESC_DARKGREY    "\033[38;5;240m"

#define ANSI_ESC_BROWN       "\033[38;5;130m"
#define ANSI_ESC_ORANGE      "\033[38;5;214m"
#define ANSI_ESC_DARKORANGE  "\033[38;5;166m"
#define ANSI_ESC_LIGHTORANGE "\033[38;5;215m"
#define ANSI_ESC_PURPLE      "\033[38;5;129m"
#define ANSI_ESC_PINK        "\033[38;5;213m"
#define ANSI_ESC_LIME        "\033[38;5;190m"
#define ANSI_ESC_NAVY        "\033[38;5;25m"
#define ANSI_ESC_AQUAMARINE  "\033[38;5;51m"
#define ANSI_ESC_LAVENDER    "\033[38;5;189m"

// RGB565 Color definitions            R    G    B
#define TFT_RED          0xF800 // 255,   0,   0
#define TFT_DARKRED      0x8000 // 128,   0,   0
#define TFT_LIGHTRED     0xFBEF // 255, 127, 127
#define TFT_GREEN        0x07E0 //   0, 255,   0
#define TFT_DARKGREEN    0x0400 //   0, 128,   0
#define TFT_LIGHTGREEN   0x7FE0 // 127, 255, 127
#define TFT_BLUE         0x001F //   0,   0, 255
#define TFT_DARKBLUE     0x0010 //   0,   0, 128
#define TFT_LIGHTBLUE    0x7BFF // 127, 127, 255
#define TFT_CYAN         0x07FF //   0, 255, 255
#define TFT_DARKCYAN     0x0410 //   0, 128, 128
#define TFT_LIGHTCYAN    0x7FFF // 127, 255, 255
#define TFT_MAGENTA      0xF81F // 255,   0, 255
#define TFT_DARKMAGENTA  0x8010 // 128,   0, 128
#define TFT_LIGHTMAGENTA 0xF97F // 255, 127, 255
#define TFT_YELLOW       0xFFE0 // 255, 255,   0
#define TFT_DARKYELLOW   0x8400 // 128, 128,   0
#define TFT_LIGHTYELLOW  0xFFF7 // 255, 255, 127
#define TFT_WHITE        0xFFFF // 255, 255, 255
#define TFT_BLACK        0x0000 //   0,   0,   0
#define TFT_GREY         0x8410 // 128, 128, 128
#define TFT_LIGHTGREY    0xC618 // 192, 192, 192
#define TFT_DARKGREY     0xAD55 //  64,  64,  64
#define TFT_BROWN        0xA145 // 165,  42,  42
#define TFT_DARKBROWN    0x8200 // 128,  64,   0
#define TFT_LIGHTBROWN   0xFDB2 // 254, 198, 125

#define TFT_AQUAMARINE      0x7FFA // 127, 255, 212
#define TFT_BEIGE           0xF7BB // 245, 245, 220
#define TFT_CHOCOLATE       0xD342 // 210, 105,  30
#define TFT_CORNSILK        0xFFDB // 255, 248, 220
#define TFT_DEEPSKYBLUE     0x05FF //   0, 191, 255
#define TFT_GREENYELLOW     0xAFE5 // 173, 255,  47
#define TFT_GOLD            0xFEA0 // 255, 215,   0
#define TFT_HOTPINK         0xFB56 // 255, 105, 180
#define TFT_LAVENDER        0xE73F // 230, 230, 250
#define TFT_LAWNGREEN       0x7FE0 // 124, 252,   0
#define TFT_LIME            0x07E0 //   0. 255,   0
#define TFT_MAROON          0x7800 // 128,   0,   0
#define TFT_MEDIUMVIOLETRED 0xC0B0 // 199,  21, 133
#define TFT_NAVY            0x000F //   0,   0, 128
#define TFT_OLIVE           0x7BE0 // 128, 128,   0
#define TFT_ORANGE          0xFD20 // 255, 165,   0
#define TFT_LIGHTORANGE     0xFDB8 // 255, 200, 124
#define TFT_DARKORANGE      0xFC00 // 255, 140,   0
#define TFT_PINK            0xFE19 // 255, 192, 203
#define TFT_PURPLE          0x780F // 128,   0, 128
#define TFT_SANDYBROWN      0xF52C // 244, 164,  96
#define TFT_TURQUOISE       0x471A //  64, 224, 208
#define TFT_VIOLET          0x801F // 128,   0, 255

#if TFT_FONT == 0
    #define TFT_GARAMOND
#elif TFT_FONT == 1
    #define TFT_TIMES_NEW_ROMAN
#elif TFT_FONT == 2
    #define TFT_FREE_SERIF_ITALIC
#elif TFT_FONT == 3
    #define TFT_ARIAL
#elif TFT_FONT == 4
    #define TFT_Z003
#else
    #define TFT_GARAMOND
#endif

#define TFT_ALIGN_RIGHT  (1)
#define TFT_ALIGN_LEFT   (2)
#define TFT_ALIGN_CENTER (3)
#define TFT_ALIGN_TOP    (4)
#define TFT_ALIGN_DOWN   (5)
