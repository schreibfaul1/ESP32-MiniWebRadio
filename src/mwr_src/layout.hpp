#include "../common.h"

#pragma once

struct coor {
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t w = 0;
    uint16_t h = 0;
    uint8_t  pl = 0;
    uint8_t  pr = 0;
    uint8_t  pt = 0;
    uint8_t  pb = 0;

    // builder-methods:
    constexpr coor& pos(uint16_t X, uint16_t Y) {
        x = X;
        y = Y;
        return *this;
    }
    constexpr coor& size(uint16_t W, uint16_t H) {
        w = W;
        h = H;
        return *this;
    }
    constexpr coor& pad(uint8_t L, uint8_t R, uint8_t T, uint8_t B) {
        pl = L;
        pr = R;
        pt = T;
        pb = B;
        return *this;
    }
};

struct Layout {
    const coor& winHeader;
    const coor& winLogo;
    const coor& winName;
    const coor& winProgbar;
    const coor& winTitle;
    const coor& winSTitle;
    const coor& winVUmeter;
    const coor& winFooter;
    const coor& winButton;
    const coor& winDigits;
    const coor& winWoHF;
    const coor& sdrHP;
    const coor& sdrBP;
    const coor& sdrLP;
    const coor& sdrBAL;
    const coor& btnHP;
    const coor& btnBP;
    const coor& btnLP;
    const coor& btnBAL;
    const coor& txtHP;
    const coor& txtBP;
    const coor& txtLP;
    const coor& txtBAL;
};

struct DisplayConfig {
    const uint8_t* fonts; // Pointer auf Array mit Schriftgrößen
    uint8_t        listFontSize;
    uint8_t        headerFontSize;
    uint8_t        footerFontSize;
    uint8_t        bigNumbersFontSize;
    uint8_t        fileNumberFontSize;

    uint16_t sleeptimeXPos[5];
    uint16_t sleeptimeYPos;

    uint16_t dispWidth;
    uint16_t dispHeight;
    uint8_t  brightnessMin;
    uint8_t  brightnessMax;

    const char* tftSize; // "s", "m", "l", "xl"
};
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

namespace layout_320x240 {
//
//  Display 320x240
//  +-------------------------------------------+ _yHeader=0
//  | Header                                    |       _hHeader=20px
//  +-------------------------------------------+ _yName=20
//  |                                           |
//  | Logo                   StationName        |       _hName=100px
//  |                                           |
//  +-------------------------------------------+ _yTitle=120
//  |                                           |
//  |              StreamTitle                  |       _hTitle=100px
//  |                                           |
//  +-------------------------------------------+ _yFooter=220
//  | Footer                                    |       _hFooter=20px
//  +-------------------------------------------+ 240
//                                             320

constexpr uint16_t h_res = 320, v_res = 240; // horizontal - vertical resolution
constexpr uint16_t h_footer = 20;            // footer height
constexpr uint16_t hw_btn = 40 + 0;          // 40x40 + padding, normal buttons
constexpr uint16_t hw_btn_s = 32 + 2;        // 32x32 + padding, small buttons
constexpr uint16_t w_vuMeter = 24;           // width vuMeter

constexpr uint16_t h_area = (v_res - 2 * h_footer) / 2;                      // 100, height area1 and  area2
constexpr uint16_t y_area2 = v_res - h_footer - h_area;                      // 240 - 20 - 100, yPos area2
constexpr uint16_t y_btn = y_area2 + h_area / 2 + (h_area / 4 - hw_btn / 2); // center in the lower half of area2
constexpr uint16_t h_progBar = (h_area / 2);                                 // height progressBar and volumeSlider = half of y_area - 20%
constexpr uint16_t y_progbar = y_area2;                                      // y_area2 + 10%
constexpr uint16_t h_EQ = (2 * h_area - hw_btn) / 4;

// -----------------------------------------------------------------------------------
// window definitions .pos(x, y) .size(w, h) .padding(l, r, t, b)
// -----------------------------------------------------------------------------------

constexpr coor winHeader = coor().pos(0, 0).size(h_res, h_footer);
constexpr coor winLogo = coor().pos(0, h_footer).size(h_area, h_area).pad(1, 1, 1, 1);
constexpr coor winName = coor().pos(h_area, h_footer).size(h_res - h_area, h_area).pad(1, 1, 0, 0); // StationName
constexpr coor winProgbar = coor().pos(0, y_progbar).size(h_res, h_progBar).pad(5, 5, 0, 0);        // or volume slider
constexpr coor winTitle = coor().pos(0, y_area2).size(h_res - w_vuMeter, h_area).pad(0, 5, 0, 3);
constexpr coor winSTitle = coor().pos(0, y_area2).size(h_res - w_vuMeter, h_area).pad(0, 4, 0, 3);
constexpr coor winVUmeter = coor().pos(h_res - w_vuMeter, y_area2).size(w_vuMeter, h_area);
constexpr coor winFooter = coor().pos(0, v_res - h_footer).size(h_res, h_footer);
constexpr coor winButton = coor().pos(0, y_btn).size(hw_btn, hw_btn);
constexpr coor winDigits = coor().pos(0, h_footer).size(h_res, y_btn - h_footer); // clock24, alarmclock
constexpr coor winWoHF = coor().pos(0, h_footer).size(h_res, 2 * h_area);

// -----------------------------------------------------------------------------------
// window derived (calculated from others)
// -----------------------------------------------------------------------------------

constexpr coor btnHP = coor().pos(10, h_footer + 0 * h_EQ).size(hw_btn_s, hw_btn_s);
constexpr coor btnBP = coor().pos(10, h_footer + 1 * h_EQ).size(hw_btn_s, hw_btn_s);
constexpr coor btnLP = coor().pos(10, h_footer + 2 * h_EQ).size(hw_btn_s, hw_btn_s);
constexpr coor btnBAL = coor().pos(10, h_footer + 3 * h_EQ).size(hw_btn_s, hw_btn_s);

constexpr coor sdrHP = coor().pos(55, h_footer + 0 * h_EQ).size(150, h_EQ).pad(0, 0, 8, 8);
constexpr coor sdrBP = coor().pos(55, h_footer + 1 * h_EQ).size(150, h_EQ).pad(0, 0, 8, 8);
constexpr coor sdrLP = coor().pos(55, h_footer + 2 * h_EQ).size(150, h_EQ).pad(0, 0, 8, 8);
constexpr coor sdrBAL = coor().pos(55, h_footer + 3 * h_EQ).size(150, h_EQ).pad(0, 0, 8, 8);

constexpr coor txtHP = coor().pos(210, h_footer + 0 * h_EQ).size(105, h_EQ).pad(0, 0, 1, 1);
constexpr coor txtBP = coor().pos(210, h_footer + 1 * h_EQ).size(105, h_EQ).pad(0, 0, 1, 1);
constexpr coor txtLP = coor().pos(210, h_footer + 2 * h_EQ).size(105, h_EQ).pad(0, 0, 1, 1);
constexpr coor txtBAL = coor().pos(210, h_footer + 3 * h_EQ).size(105, h_EQ).pad(0, 0, 1, 1);

inline constexpr uint8_t fonts[13] = {15, 16, 18, 21, 25, 27, 34, 38, 43, 56, 66, 81, 96};

inline constexpr DisplayConfig config = {
    fonts,
    16,                  // listFontSize
    0,                   // headerFontSize, 0 -> autoSize
    0,                   // footerFontSize, 0 -> autoSize
    156,                 // bigNumbersFontSize
    15,                  // fileNumberFontSize
    {5, 77, 129, 57, 0}, // sleeptimeXPos[5]
    48,                  // sleeptimeYPos
    320,                 // width
    240,                 // height
    5,                   // brightnessMin
    255,                 // brightnessMax
    "s"                  // size code
};
} // namespace layout_320x240
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

namespace layout_480x320 {
//
//  Display 480x320
//  +-------------------------------------------+ _yHeader=0
//  | Header                                    |       winHeader=30px
//  +-------------------------------------------+ _yName=30
//  |                                           |
//  | Logo                   StationName        |       winName=130px
//  |                                           |
//  +-------------------------------------------+ _yTitle=160
//  |                                           |
//  |              StreamTitle                  |       winTitle=130px
//  |                                           |
//  +-------------------------------------------+ _yFooter=290
//  | Footer                                    |       winFooter=30px
//  +-------------------------------------------+ 320
//                                             480

constexpr uint16_t h_res = 480, v_res = 320; // horizontal - vertical resolution
constexpr uint16_t h_footer = 30;            // footer height
constexpr uint16_t hw_btn = 56 + 2;          // 56x56 + padding, normal buttons
constexpr uint16_t hw_btn_s = 44 + 2;        // 44x45 + padding, small buttons
constexpr uint16_t w_vuMeter = 32;           // width vuMeter

constexpr uint16_t h_area = (v_res - 2 * h_footer) / 2;                      // 130, height area1 and  area2
constexpr uint16_t y_area2 = v_res - h_footer - h_area;                      // 320 - 30 - 130, yPos area2
constexpr uint16_t y_btn = y_area2 + h_area / 2 + (h_area / 4 - hw_btn / 2); // center in the lower half of area2
constexpr uint16_t h_progBar = (h_area / 2);                                 // height progressBar and volumeSlider = half of y_area
constexpr uint16_t y_progbar = y_area2;                                      // y_area2
constexpr uint16_t h_EQ = (2 * h_area - hw_btn) / 4;

// -----------------------------------------------------------------------------------
// window definitions .pos(x, y) .size(w, h) .padding(l, r, t, b)
// -----------------------------------------------------------------------------------
constexpr coor winHeader = coor().pos(0, 0).size(h_res, h_footer);
constexpr coor winLogo = coor().pos(0, h_footer).size(h_area, h_area).pad(1, 1, 1, 1);
constexpr coor winName = coor().pos(h_area, h_footer).size(h_res - h_area, h_area).pad(1, 1, 0, 0); // StationName
constexpr coor winProgbar = coor().pos(0, y_progbar).size(h_res, h_progBar).pad(5, 5, 0, 0);        // or volume slider
constexpr coor winTitle = coor().pos(0, y_area2).size(h_res - w_vuMeter, h_area).pad(0, 5, 0, 3);
constexpr coor winSTitle = coor().pos(0, y_area2).size(h_res - w_vuMeter, h_area).pad(0, 4, 0, 3);
constexpr coor winVUmeter = coor().pos(h_res - w_vuMeter, y_area2).size(w_vuMeter, h_area);
constexpr coor winFooter = coor().pos(0, v_res - h_footer).size(h_res, h_footer);
constexpr coor winButton = coor().pos(0, y_btn).size(hw_btn, hw_btn);
constexpr coor winDigits = coor().pos(0, h_footer).size(h_res, y_btn - h_footer); // clock24, alarmclock
constexpr coor winWoHF = coor().pos(0, h_footer).size(h_res, 2 * h_area);         // window without header and footer

// -----------------------------------------------------------------------------------
// window derived (calculated from others)
// -----------------------------------------------------------------------------------

constexpr coor btnHP = coor().pos(40, h_footer + 0 * h_EQ).size(hw_btn_s, hw_btn_s);
constexpr coor btnBP = coor().pos(40, h_footer + 1 * h_EQ).size(hw_btn_s, hw_btn_s);
constexpr coor btnLP = coor().pos(40, h_footer + 2 * h_EQ).size(hw_btn_s, hw_btn_s);
constexpr coor btnBAL = coor().pos(40, h_footer + 3 * h_EQ).size(hw_btn_s, hw_btn_s);

constexpr coor sdrHP = coor().pos(110, h_footer + 0 * h_EQ).size(200, h_EQ).pad(0, 0, 8, 8);
constexpr coor sdrBP = coor().pos(110, h_footer + 1 * h_EQ).size(200, h_EQ).pad(0, 0, 8, 8);
constexpr coor sdrLP = coor().pos(110, h_footer + 2 * h_EQ).size(200, h_EQ).pad(0, 0, 8, 8);
constexpr coor sdrBAL = coor().pos(110, h_footer + 3 * h_EQ).size(200, h_EQ).pad(0, 0, 8, 8);

constexpr coor txtHP = coor().pos(320, h_footer + 0 * h_EQ).size(150, h_EQ).pad(0, 0, 3, 3);
constexpr coor txtBP = coor().pos(320, h_footer + 1 * h_EQ).size(150, h_EQ).pad(0, 0, 3, 3);
constexpr coor txtLP = coor().pos(320, h_footer + 2 * h_EQ).size(150, h_EQ).pad(0, 0, 3, 3);
constexpr coor txtBAL = coor().pos(320, h_footer + 3 * h_EQ).size(150, h_EQ).pad(0, 0, 3, 3);

inline constexpr uint8_t fonts[13] = {15, 16, 18, 21, 25, 27, 34, 38, 43, 56, 66, 81, 96};

inline constexpr DisplayConfig config = {
    fonts,
    21,                   // listFontSize
    0,                    // headerFontSize, 0 -> autoSize
    0,                    // footerFontSize, 0 -> autoSize
    156,                  // bigNumbersFontSize
    21,                   // fileNumberFontSize
    {5, 107, 175, 73, 0}, // sleeptimeXPos[5]
    48,                   // sleeptimeYPos
    480,                  // width
    320,                  // height
    5,                    // brightnessMin
    255,                  // brightnessMax
    "m"                   // size code
};
} // namespace layout_480x320
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

namespace layout_800x480 {
//
//  Display 800x480
//  +-------------------------------------------+ _yHeader=0
//  | Header                                    |       winHeader=50px
//  +-------------------------------------------+ _yName=50
//  |                                           |
//  | Logo                   StationName        |       winName=190px
//  |                                           |
//  +-------------------------------------------+ _yTitle=240
//  |                                           |
//  |              StreamTitle                  |       winTitle=190px
//  |                                           |
//  +-------------------------------------------+ _yFooter=430
//  | Footer                                    |       winFooter=50px
//  +-------------------------------------------+ 480
//                                             800

constexpr uint16_t h_res = 800, v_res = 480; // horizontal - vertical resolution
constexpr uint16_t h_footer = 50;            // footer height
constexpr uint16_t hw_btn = 76 + 2;          // 76x76 + padding, normal buttons
constexpr uint16_t hw_btn_s = 65 + 2;        // 65x65 + padding, small buttons
constexpr uint16_t w_vuMeter = 40;           // width vuMeter

constexpr uint16_t h_area = (v_res - 2 * h_footer) / 2;                      // 130, height area1 and  area2
constexpr uint16_t y_area2 = v_res - h_footer - h_area;                      // 320 - 30 - 130, yPos area2
constexpr uint16_t y_btn = y_area2 + h_area / 2 + (h_area / 4 - hw_btn / 2); // center in the lower half of area2
constexpr uint16_t h_progBar = (h_area / 2);                                 // height progressBar and volumeSlider = half of y_area
constexpr uint16_t y_progbar = y_area2;                                      // y_area2
constexpr uint16_t h_EQ = (2 * h_area - hw_btn) / 4;

// -----------------------------------------------------------------------------------
// window definitions .pos(x, y) .size(w, h) .padding(l, r, t, b)
// -----------------------------------------------------------------------------------

constexpr coor winHeader = coor().pos(0, 0).size(h_res, h_footer);
constexpr coor winLogo = coor().pos(0, h_footer).size(h_area, h_area).pad(4, 4, 4, 4);
constexpr coor winName = coor().pos(h_area, h_footer).size(h_res - h_area, h_area).pad(15, 5, 0, 0); // StationName
constexpr coor winProgbar = coor().pos(0, y_progbar).size(h_res, h_progBar).pad(15, 15, 0, 0);       // or volume slider
constexpr coor winTitle = coor().pos(0, y_area2).size(h_res - w_vuMeter, h_area).pad(0, 5, 0, 3);
constexpr coor winSTitle = coor().pos(0, y_area2).size(h_res - w_vuMeter, h_area).pad(10, 5, 2, 2);
constexpr coor winVUmeter = coor().pos(h_res - w_vuMeter, y_area2).size(w_vuMeter, h_area);
constexpr coor winFooter = coor().pos(0, v_res - h_footer).size(h_res, h_footer);
constexpr coor winButton = coor().pos(0, y_btn).size(hw_btn, hw_btn);
constexpr coor winDigits = coor().pos(0, h_footer).size(h_res, y_btn - h_footer); // clock24, alarmclock
constexpr coor winWoHF = coor().pos(0, h_footer).size(h_res, 2 * h_area);

// -----------------------------------------------------------------------------------
// window derived (calculated from others)
// -----------------------------------------------------------------------------------
constexpr coor btnHP = coor().pos(100, h_footer + 0 * h_EQ).size(hw_btn_s, hw_btn_s);
constexpr coor btnBP = coor().pos(100, h_footer + 1 * h_EQ).size(hw_btn_s, hw_btn_s);
constexpr coor btnLP = coor().pos(100, h_footer + 2 * h_EQ).size(hw_btn_s, hw_btn_s);
constexpr coor btnBAL = coor().pos(100, h_footer + 3 * h_EQ).size(hw_btn_s, hw_btn_s);

constexpr coor sdrHP = coor().pos(180, h_footer + 0 * h_EQ).size(380, h_EQ).pad(0, 0, 8, 8);
constexpr coor sdrBP = coor().pos(180, h_footer + 1 * h_EQ).size(380, h_EQ).pad(0, 0, 8, 8);
constexpr coor sdrLP = coor().pos(180, h_footer + 2 * h_EQ).size(380, h_EQ).pad(0, 0, 8, 8);
constexpr coor sdrBAL = coor().pos(180, h_footer + 3 * h_EQ).size(380, h_EQ).pad(0, 0, 8, 8);

constexpr coor txtHP = coor().pos(570, h_footer + 0 * h_EQ).size(200, h_EQ).pad(0, 0, 6, 6);
constexpr coor txtBP = coor().pos(570, h_footer + 1 * h_EQ).size(200, h_EQ).pad(0, 0, 6, 6);
constexpr coor txtLP = coor().pos(570, h_footer + 2 * h_EQ).size(200, h_EQ).pad(0, 0, 6, 6);
constexpr coor txtBAL = coor().pos(570, h_footer + 3 * h_EQ).size(200, h_EQ).pad(0, 0, 6, 6);

inline constexpr uint8_t fonts[13] = {15, 16, 18, 21, 25, 27, 34, 38, 43, 56, 66, 81, 96};

inline constexpr DisplayConfig config = {
    fonts,
    27,                     // listFontSize
    38,                     // headerFontSize
    38,                     // footerFontSize
    156,                    // bigNumbersFontSize
    34,                     // fileNumberFontSize
    {20, 137, 223, 106, 0}, // sleeptimeXPos[5]
    112,                    // sleeptimeYPos
    800,                    // width
    480,                    // height
    30,                     // brightnessMin
    255,                    // brightnessMax
    "l"                     // size code
};
} // namespace layout_800x480
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
namespace layout_1024x600 {
//
//  Display 1024x600
//  +-------------------------------------------+ _yHeader=0
//  | Header                                    |       winHeader=60px
//  +-------------------------------------------+ _yName=60
//  |                                           |
//  | Logo                   StationName        |       winName=240px, area1
//  |                                           |
//  +-------------------------------------------+ _yTitle=300
//  |                                           |
//  |              StreamTitle                  |       winTitle=240px, area2
//  |                                           |
//  +-------------------------------------------+ _yFooter=540
//  | Footer                                    |       winFooter=60px
//  +-------------------------------------------+ 600
//                                            1024

constexpr uint16_t h_res = 1024, v_res = 600; // horizontal - vertical resolution
constexpr uint16_t h_footer = 60;             // footer height
constexpr uint16_t hw_btn = 96 + 4;           // 96x96 + padding, normal buttons
constexpr uint16_t hw_btn_s = 80 + 2;         // 80x80 + padding, small buttons
constexpr uint16_t w_vuMeter = 50;            // width vuMeter

constexpr uint16_t h_area = (v_res - 2 * h_footer) / 2;                      // 240, height area1 and  area2
constexpr uint16_t y_area2 = v_res - h_footer - h_area;                      // 600 - 60 - 240, yPos area2
constexpr uint16_t y_btn = y_area2 + h_area / 2 + (h_area / 4 - hw_btn / 2); // center in the lower half of area2
constexpr uint16_t h_progBar = (h_area / 2);                                 // height progressBar and volumeSlider = half of y_area
constexpr uint16_t y_progbar = y_area2;                                      // y_area2
constexpr uint16_t h_EQ = (2 * h_area - hw_btn) / 4;

// -----------------------------------------------------------------------------------
// window definitions .pos(x, y) .size(w, h) .padding(l, r, t, b)
// -----------------------------------------------------------------------------------

constexpr coor winHeader = coor().pos(0, 0).size(h_res, h_footer);
constexpr coor winLogo = coor().pos(0, h_footer).size(h_area, h_area).pad(4, 4, 4, 4);
constexpr coor winName = coor().pos(h_area, h_footer).size(h_res - h_area, h_area).pad(15, 5, 0, 0); // StationName
constexpr coor winProgbar = coor().pos(0, y_progbar).size(h_res, h_progBar).pad(5, 5, 20, 20);       // or volume slider
constexpr coor winTitle = coor().pos(0, y_area2).size(h_res - w_vuMeter, h_area).pad(0, 5, 0, 3);
constexpr coor winSTitle = coor().pos(0, y_area2).size(h_res - w_vuMeter, h_area).pad(10, 5, 2, 2);
constexpr coor winVUmeter = coor().pos(h_res - w_vuMeter, y_area2).size(w_vuMeter, h_area).pad(5, 5, 5, 5);
constexpr coor winFooter = coor().pos(0, v_res - h_footer).size(h_res, h_footer);
constexpr coor winButton = coor().pos(0, y_btn).size(hw_btn, hw_btn);
constexpr coor winDigits = coor().pos(0, h_footer).size(h_res, y_btn - h_footer); // clock24, alarmclock
constexpr coor winWoHF = coor().pos(0, h_footer).size(h_res, 2 * h_area);

// -----------------------------------------------------------------------------------
// window derived (calculated from others)
// -----------------------------------------------------------------------------------

constexpr coor btnHP = coor().pos(100, h_footer + 0 * h_EQ).size(hw_btn_s, hw_btn_s);
constexpr coor btnBP = coor().pos(100, h_footer + 1 * h_EQ).size(hw_btn_s, hw_btn_s);
constexpr coor btnLP = coor().pos(100, h_footer + 2 * h_EQ).size(hw_btn_s, hw_btn_s);
constexpr coor btnBAL = coor().pos(100, h_footer + 3 * h_EQ).size(hw_btn_s, hw_btn_s);

constexpr coor sdrHP = coor().pos(220, h_footer + 0 * h_EQ).size(500, h_EQ).pad(0, 0, 8, 8);
constexpr coor sdrBP = coor().pos(220, h_footer + 1 * h_EQ).size(500, h_EQ).pad(0, 0, 8, 8);
constexpr coor sdrLP = coor().pos(220, h_footer + 2 * h_EQ).size(500, h_EQ).pad(0, 0, 8, 8);
constexpr coor sdrBAL = coor().pos(220, h_footer + 3 * h_EQ).size(500, h_EQ).pad(0, 0, 8, 8);

constexpr coor txtHP = coor().pos(730, h_footer + 0 * h_EQ).size(260, h_EQ).pad(0, 0, 4, 4);
constexpr coor txtBP = coor().pos(730, h_footer + 1 * h_EQ).size(260, h_EQ).pad(0, 0, 4, 4);
constexpr coor txtLP = coor().pos(730, h_footer + 2 * h_EQ).size(260, h_EQ).pad(0, 0, 4, 4);
constexpr coor txtBAL = coor().pos(730, h_footer + 3 * h_EQ).size(260, h_EQ).pad(0, 0, 4, 4);

inline constexpr uint8_t fonts[13] = {15, 16, 18, 21, 25, 27, 34, 38, 43, 56, 66, 81, 96};

inline constexpr DisplayConfig config = {
    fonts,
    38,                     // listFontSize
    38,                     // headerFontSize
    38,                     // footerFontSize
    156,                    // bigNumbersFontSize
    34,                     // fileNumberFontSize
    {20, 137, 223, 106, 0}, // sleeptimeXPos[5]
    112,                    // sleeptimeYPos
    h_res,                  // width
    v_res,                  // height
    1,                      // brightnessMin
    255,                    // brightnessMax
    "xl"                    // size code
};
} // namespace layout_1024x600
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

// Factory-Funktion (Compile-Time)
inline Layout makeLayout() {
#ifdef TFT_LAYOUT_S
    using namespace layout_320x240;
#elifdef TFT_LAYOUT_M
    using namespace layout_480x320;
#elifdef TFT_LAYOUT_L
    using namespace layout_800x480;
#elifdef TFT_LAYOUT_XL
    using namespace layout_1024x600;
#else
    printf()"Unsupported TFT_LAYOUT\n"
#endif
    return {winHeader, winLogo, winName, winProgbar, winTitle, winSTitle, winVUmeter, winFooter, winButton, winDigits, winWoHF, sdrHP,
            sdrBP,     sdrLP,   sdrBAL,  btnHP,      btnBP,    btnLP,     btnBAL,     txtHP,     txtBP,     txtLP,     txtBAL};
}

// global constant - finished initialized
inline const Layout layout = makeLayout();
//----------------------------------------------------------------------------------------------------------------------------------------------------
inline DisplayConfig makeDisplayConfig() {
#ifdef TFT_LAYOUT_S
    using namespace layout_320x240;
#elifdef TFT_LAYOUT_M
    using namespace layout_480x320;
#elifdef TFT_LAYOUT_L
    using namespace layout_800x480;
#elifdef TFT_LAYOUT_XL
    using namespace layout_1024x600;
#else
    printf("Unsupported TFT_LAYOUT\n");
#endif
    return config;
}

inline const DisplayConfig displayConfig = makeDisplayConfig();
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/*         ╔═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗
           ║                                                                                  M E N U E / B U T T O N S                                                                  ║
           ╚═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝   */

displayHeader dispHeader("dispHeader", displayConfig.headerFontSize); // 0 -> autoSize
displayFooter dispFooter("dispFooter", displayConfig.footerFontSize); // 0 -> autoSize
numbersBox    volBox("volBox");
uniList       myList("myList");
// RADIO
button2state btn_RA_mute("btn_RA_mute");
button2state btn_RA_recorder("btn_RA_recorder");
button1state btn_RA_prevSta("btn_RA_prevSta"), btn_RA_nextSta("btn_RA_nextSta");
button1state btn_RA_staList("btn_RA_staList"), btn_RA_player("btn_RA_player"), btn_RA_dlna("btn_RA_dlna"), btn_RA_clock("btn_RA_clock");
button1state btn_RA_sleep("btn_RA_sleep"), btn_RA_bt("btn_RA_bt");
button1state btn_RA_off("btn_RA_off"), btn_RA_settings("btn_RA_settings");
pictureBox   pic_RA_logo("pic_RA_logo");
textbox      txt_RA_sTitle("txt_RA_sTitle"), txt_RA_staName("txt_RA_staName"), txt_RA_irNum("txt_RA_irNum");
vuMeter      VUmeter_RA("VUmeter_RA");
slider       sdr_RA_volume("sdr_RA_volume");
numbersBox   nbr_RA_staBox("nbr_RA_staBox");
// STATIONSLIST
stationsList lst_RADIO("lst_RADIO");
// PLAYER
button2state btn_PL_mute("btn_PL_mute"), btn_PL_pause("btn_PL_pause");
button1state btn_PL_ready("btn_PL_ready"), btn_PL_shuffle("btn_PL_shuffle");
button1state btn_PL_playAll("btn_PL_playAll"), btn_PL_fileList("btn_PL_fileList"), btn_PL_radio("btn_PL_radio"), btn_PL_cancel("btn_PL_cancel");
button1state btn_PL_prevFile("btn_PL_prevFile"), btn_PL_nextFile("btn_PL_nextFile"), btn_PL_off("btn_PL_off");
button1state btn_PL_playNext("btn_PL_playNext"), btn_PL_playPrev("btn_PL_playPrev");
textbox      txt_PL_fName("txt_PL_fName");
slider       sdr_PL_volume("sdr_PL_volume");
pictureBox   pic_PL_logo("pic_PL_logo");
progressbar  pgb_PL_progress("pgb_PL_progress");
// AUDIOFILESLIST
fileList lst_PLAYER("lst_PLAYER");
// DLNA
button2state btn_DL_mute("btn_DL_mute"), btn_DL_pause("btn_DL_pause");
button1state btn_DL_radio("btn_DL_radio"), btn_DL_fileList("btn_DL_fileList"), btn_DL_cancel("btn_DL_cancel");
textbox      txt_DL_fName("txt_DL_fName");
slider       sdr_DL_volume("sdr_DL_volume");
pictureBox   pic_DL_logo("pic_DL_logo");
progressbar  pgb_DL_progress("pgb_DL_progress");
// DLNAITEMSLIST
dlnaList lst_DLNA("lst_DLNA");
// CLOCK
imgClock24   clk_CL_24("clk_CL_24");
button2state btn_CL_mute("btn_CL_mute");
button1state btn_CL_alarm("btn_CL_alarm"), btn_CL_radio("btn_CL_radio"), btn_CL_off("btn_CL_off");
slider       sdr_CL_volume("sdr_CL_volume");
// ALARMCLOCK
alarmClock   clk_AC_red("clk_AC_red");
button1state btn_AC_left("btn_AC_left"), btn_AC_right("btn_AC_right"), btn_AC_up("btn_AC_up"), btn_AC_down("btn_AC_down");
button1state btn_AC_ready("btn_AC_ready");
// RINGING
pictureBox      pic_RI_logo("pic_RI_logo");
imgClock24small clk_RI_24small("clk_RI_24small");
// SETTINGS
pictureBox   pic_SE_logo("pic_SE_logo");
button1state btn_SE_bright("btn_SE_bright"), btn_SE_equal("btn_SE_equal"), btn_SE_wifi("btn_SE_wifi"), btn_SE_radio("btn_SE_radio");
// BRIGHTNESS
button1state btn_BR_ready("btn_BR_ready");
pictureBox   pic_BR_logo("pic_BR_logo");
slider       sdr_BR_value("sdr_BR_value");
textbox      txt_BR_value("txt_BR_value");
// SLEEPTIMER
button1state btn_SL_up("btn_SL_up"), btn_SL_down("btn_SL_down"), btn_SL_ready("btn_SL_ready"), btn_SL_cancel("btn_SL_cancel");
offTimerBox  otb_SL_stime("otb_SL_stime");
pictureBox   pic_SL_logo("pic_SL_logo");
// EQUALIZER
slider       sdr_EQ_lowPass("sdr_EQ_LP"), sdr_EQ_bandPass("sdr_EQ_BP"), sdr_EQ_highPass("sdr_EQ_HP"), sdr_EQ_balance("sdr_EQ_BAL");
textbox      txt_EQ_lowPass("txt_EQ_LP"), txt_EQ_bandPass("txt_EQ_BP"), txt_EQ_highPass("txt_EQ_HP"), txt_EQ_balance("txt_EQ_BAL");
button1state btn_EQ_lowPass("btn_EQ_LP");
button1state btn_EQ_bandPass("btn_EQ_BP"), btn_EQ_highPass("btn_EQ_HP"), btn_EQ_balance("btn_EQ_BAL");
button1state btn_EQ_Radio("btn_EQ_Radio"), btn_EQ_Player("btn_EQ_Player");
button2state btn_EQ_mute("btn_EQ_mute");
// BLUETOOTH
button2state btn_BT_pause("btn_BT_pause"), btn_BT_power("btn_BT_power");
button1state btn_BT_volDown("btn_BT_volDown"), btn_BT_volUp("btn_BT_volUp"), btn_BT_radio("btn_BT_radio"), btn_BT_mode("btn_BT_mode");
pictureBox   pic_BT_mode("pic_BT_mode");
textbox      txt_BT_mode("txt_BT_mode");
// IR_SETTINGS
button1state btn_IR_radio("btn_IR_radio");
// WIFI_SETTINGS
wifiSettings cls_wifiSettings("wifiSettings", 2);
// ALL_STATE
messageBox msg_box("messagebox");
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void placingGraphicObjects() { // and initialize them
    // ALL STATE
    dispHeader.begin(layout.winHeader.x, layout.winHeader.y, layout.winHeader.w, layout.winHeader.h);
    dispHeader.setTimeColor(TFT_LIGHTGREEN);
    dispFooter.begin(layout.winFooter.x, layout.winFooter.y, layout.winFooter.w, layout.winFooter.h);
    volBox.begin(layout.winName.x, layout.winName.y, layout.winName.w, layout.winName.h);
    myList.begin(layout.winWoHF.x, layout.winWoHF.y, layout.winWoHF.w, layout.winWoHF.h, displayConfig.fonts[0]);
    // RADIO -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    sdr_RA_volume.setTransparency(true, false);
    sdr_RA_volume.begin(layout.winProgbar.x, layout.winProgbar.y, layout.winProgbar.w, layout.winProgbar.h, layout.winProgbar.pl, layout.winProgbar.pr, layout.winProgbar.pt, layout.winProgbar.pb);
    btn_RA_mute.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_mute.setOffPicturePath("/btn/Button_Mute_Off_Green.png");
    btn_RA_mute.setOnPicturePath("/btn/Button_Mute_On_Red.png");
    btn_RA_mute.setClickedOffPicturePath("/btn/Button_Mute_Off_Yellow.png");
    btn_RA_mute.setClickedOnPicturePath("/btn/Button_Mute_On_Yellow.png");
    btn_RA_prevSta.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_prevSta.setDefaultPicturePath("/btn/Button_Previous_Green.png");
    btn_RA_prevSta.setClickedPicturePath("/btn/Button_Previous_Yellow.png");
    btn_RA_nextSta.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_nextSta.setDefaultPicturePath("/btn/Button_Next_Green.png");
    btn_RA_nextSta.setClickedPicturePath("/btn/Button_Next_Yellow.png");
    btn_RA_recorder.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_recorder.setOffPicturePath("/btn/Button_Recorder_Blue.png");
    btn_RA_recorder.setOnPicturePath("/btn/Button_Recorder_Red.png");
    btn_RA_recorder.setClickedOffPicturePath("/btn/Button_Recorder_Yellow.png");
    btn_RA_recorder.setClickedOnPicturePath("/btn/Button_Recorder_Yellow.png");
    btn_RA_staList.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_staList.setDefaultPicturePath("/btn/Button_List_Green.png");
    btn_RA_staList.setClickedPicturePath("/btn/Button_List_Yellow.png");
    btn_RA_staList.setAlternativePicturePath("/btn/Button_List_Magenta.png");
    btn_RA_player.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_player.setDefaultPicturePath("/btn/Button_Player_Green.png");
    btn_RA_player.setClickedPicturePath("/btn/Button_Player_Yellow.png");
    btn_RA_player.setAlternativePicturePath("/btn/Button_Player_Magenta.png");
    btn_RA_dlna.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_dlna.setDefaultPicturePath("/btn/Button_DLNA_Green.png");
    btn_RA_dlna.setClickedPicturePath("/btn/Button_DLNA_Yellow.png");
    btn_RA_dlna.setAlternativePicturePath("/btn/Button_DLNA_Magenta.png");
    btn_RA_clock.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_clock.setDefaultPicturePath("/btn/Button_Clock_Green.png");
    btn_RA_clock.setClickedPicturePath("/btn/Button_Clock_Yellow.png");
    btn_RA_clock.setAlternativePicturePath("/btn/Button_Clock_Magenta.png");
    btn_RA_sleep.begin(4 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_sleep.setDefaultPicturePath("/btn/Button_OffTimer_Green.png");
    btn_RA_sleep.setClickedPicturePath("/btn/Button_OffTimer_Yellow.png");
    btn_RA_sleep.setAlternativePicturePath("/btn/Button_OffTimer_Magenta.png");
    btn_RA_settings.begin(5 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_settings.setDefaultPicturePath("/btn/Button_Settings_Green.png");
    btn_RA_settings.setClickedPicturePath("/btn/Button_Settings_Yellow.png");
    btn_RA_settings.setAlternativePicturePath("/btn/Button_Settings_Magenta.png");
    btn_RA_bt.begin(6 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_bt.setDefaultPicturePath("/btn/Button_Bluetooth_Green.png");
    btn_RA_bt.setClickedPicturePath("/btn/Button_Bluetooth_Yellow.png");
    btn_RA_bt.setAlternativePicturePath("/btn/Button_Bluetooth_Magenta.png");
    btn_RA_bt.setInactivePicturePath("/btn/Button_Bluetooth_Grey.png");
    btn_RA_off.begin(7 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_off.setDefaultPicturePath("/btn/Button_Off_Red.png");
    btn_RA_off.setClickedPicturePath("/btn/Button_Off_Yellow.png");
    btn_RA_off.setAlternativePicturePath("/btn/Button_Off_Magenta.png");
    txt_RA_sTitle.setTransparency(true, false);
    txt_RA_sTitle.begin(layout.winSTitle.x, layout.winSTitle.y, layout.winSTitle.w, layout.winSTitle.h, layout.winSTitle.pl, layout.winSTitle.pr, layout.winSTitle.pt, layout.winSTitle.pb);
    txt_RA_sTitle.setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
    txt_RA_sTitle.setFont(0); // 0 -> auto
    txt_RA_staName.setTransparency(true, false);
    txt_RA_staName.begin(layout.winName.x, layout.winName.y, layout.winName.w, layout.winName.h, layout.winName.pl, layout.winName.pr, layout.winName.pt, layout.winName.pb);
    txt_RA_staName.setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_TOP);
    txt_RA_staName.setFont(0); // 0 -> auto
    txt_RA_irNum.begin(layout.winWoHF.x, layout.winWoHF.y, layout.winWoHF.w, layout.winWoHF.h, layout.winWoHF.pl, layout.winWoHF.pr, layout.winWoHF.pt, layout.winWoHF.pb);
    txt_RA_irNum.setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
    txt_RA_irNum.setTextColor(TFT_GOLD);
    txt_RA_irNum.setFont(displayConfig.bigNumbersFontSize);
    pic_RA_logo.setTransparency(true, false);
    pic_RA_logo.begin(layout.winLogo.x, layout.winLogo.y, layout.winLogo.w, layout.winLogo.h, layout.winLogo.pl, layout.winLogo.pr, layout.winLogo.pt, layout.winLogo.pb);
    VUmeter_RA.setTransparency(true, false);
    VUmeter_RA.begin(layout.winVUmeter.x, layout.winVUmeter.y, layout.winVUmeter.w, layout.winVUmeter.h, layout.winVUmeter.pl, layout.winVUmeter.pr, layout.winVUmeter.pt, layout.winVUmeter.pb);
    nbr_RA_staBox.begin(layout.winName.x, layout.winName.y, layout.winName.w, layout.winName.h);
    // STATIONSLIST ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    lst_RADIO.begin(layout.winWoHF.x, layout.winWoHF.y, layout.winWoHF.w, layout.winWoHF.h, displayConfig.tftSize, displayConfig.listFontSize);
    // PLAYER-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_PL_mute.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_mute.setOffPicturePath("/btn/Button_Mute_Off_Green.png");
    btn_PL_mute.setOnPicturePath("/btn/Button_Mute_On_Red.png");
    btn_PL_mute.setClickedOffPicturePath("/btn/Button_Mute_Off_Yellow.png");
    btn_PL_mute.setClickedOnPicturePath("/btn/Button_Mute_On_Yellow.png");
    btn_PL_mute.setAlternativeOffPicturePath("/btn/Button_Mute_Off_Magenta.png");
    btn_PL_mute.setAlternativeOnPicturePath("/btn/Button_Mute_On_Magenta.png");
    btn_PL_pause.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_pause.setOffPicturePath("/btn/Button_Pause_Blue.png");
    btn_PL_pause.setOnPicturePath("/btn/Button_Play_Blue.png");
    btn_PL_pause.setClickedOffPicturePath("/btn/Button_Pause_Yellow.png");
    btn_PL_pause.setClickedOnPicturePath("/btn/Button_Play_Yellow.png");
    btn_PL_pause.setAlternativeOffPicturePath("/btn/Button_Pause_Magenta.png");
    btn_PL_pause.setAlternativeOnPicturePath("/btn/Button_Play_Magenta.png");
    btn_PL_pause.setValue(false);
    btn_PL_cancel.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_cancel.setDefaultPicturePath("/btn/Button_Cancel_Red.png");
    btn_PL_cancel.setClickedPicturePath("/btn/Button_Cancel_Yellow.png");
    btn_PL_cancel.setAlternativePicturePath("/btn/Button_Cancel_Magenta.png");
    sdr_PL_volume.setTransparency(true, false);
    sdr_PL_volume.begin(5 * layout.winButton.w + 10, layout.winButton.y, displayConfig.dispWidth - (5 * layout.winButton.w + 20), layout.winButton.h, layout.winButton.pl, layout.winButton.pr,
                        layout.winButton.pt, layout.winButton.pb);
    btn_PL_prevFile.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_prevFile.setDefaultPicturePath("/btn/Button_Left_Blue.png");
    btn_PL_prevFile.setClickedPicturePath("/btn/Button_Left_Yellow.png");
    btn_PL_prevFile.setAlternativePicturePath("/btn/Button_Left_Magenta.png");
    btn_PL_nextFile.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_nextFile.setDefaultPicturePath("/btn/Button_Right_Blue.png");
    btn_PL_nextFile.setClickedPicturePath("/btn/Button_Right_Yellow.png");
    btn_PL_nextFile.setAlternativePicturePath("/btn/Button_Right_Magenta.png");
    btn_PL_ready.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_ready.setDefaultPicturePath("/btn/Button_Ready_Blue.png");
    btn_PL_ready.setClickedPicturePath("/btn/Button_Ready_Yellow.png");
    btn_PL_ready.setAlternativePicturePath("/btn/Button_Ready_Magenta.png");
    btn_PL_playAll.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_playAll.setDefaultPicturePath("/btn/Button_PlayAll_Blue.png");
    btn_PL_playAll.setClickedPicturePath("/btn/Button_PlayAll_Yellow.png");
    btn_PL_playAll.setAlternativePicturePath("/btn/Button_PlayAll_Magenta.png");
    btn_PL_shuffle.begin(4 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_shuffle.setDefaultPicturePath("/btn/Button_Shuffle_Blue.png");
    btn_PL_shuffle.setClickedPicturePath("/btn/Button_Shuffle_Yellow.png");
    btn_PL_shuffle.setAlternativePicturePath("/btn/Button_Shuffle_Magenta.png");
    btn_PL_fileList.setTransparency(true, false);
    btn_PL_fileList.begin(5 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_fileList.setDefaultPicturePath("/btn/Button_List_Green.png");
    btn_PL_fileList.setClickedPicturePath("/btn/Button_List_Yellow.png");
    btn_PL_fileList.setAlternativePicturePath("/btn/Button_List_Magenta.png");
    btn_PL_radio.setTransparency(true, false);
    btn_PL_radio.begin(6 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_radio.setDefaultPicturePath("/btn/Button_Radio_Green.png");
    btn_PL_radio.setClickedPicturePath("/btn/Button_Radio_Yellow.png");
    btn_PL_radio.setAlternativePicturePath("/btn/Button_Radio_Magenta.png");
    btn_PL_off.setTransparency(true, false);
    btn_PL_off.begin(7 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_off.setDefaultPicturePath("/btn/Button_Off_Red.png");
    btn_PL_off.setClickedPicturePath("/btn/Button_Off_Yellow.png");
    btn_PL_off.setAlternativePicturePath("/btn/Button_Off_Magenta.png");
    btn_PL_playPrev.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_playPrev.setDefaultPicturePath("/btn/Button_Previous_Blue.png");
    btn_PL_playPrev.setClickedPicturePath("/btn/Button_Previous_Yellow.png");
    btn_PL_playPrev.setAlternativePicturePath("/btn/Button_Previous_Magenta.png");
    btn_PL_playNext.begin(4 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_playNext.setDefaultPicturePath("/btn/Button_Next_Blue.png");
    btn_PL_playNext.setClickedPicturePath("/btn/Button_Next_Yellow.png");
    btn_PL_playNext.setAlternativePicturePath("/btn/Button_Next_Magenta.png");
    txt_PL_fName.setTransparency(true, false);
    txt_PL_fName.begin(layout.winName.x, layout.winName.y, layout.winName.w, layout.winName.h, layout.winName.pl, layout.winName.pr, layout.winName.pt, layout.winName.pb);
    txt_PL_fName.setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
    txt_PL_fName.setFont(0); // 0 -> auto
    pic_PL_logo.setTransparency(true, false);
    pic_PL_logo.begin(layout.winLogo.x, layout.winLogo.y, layout.winLogo.w, layout.winLogo.h, layout.winLogo.pl, layout.winLogo.pr, layout.winLogo.pt, layout.winLogo.pb);
    pgb_PL_progress.setTransparency(true, false);
    pgb_PL_progress.begin(layout.winProgbar.x, layout.winProgbar.y, layout.winProgbar.w, layout.winProgbar.h, layout.winProgbar.pl, layout.winProgbar.pr, layout.winProgbar.pt, layout.winProgbar.pb, 0,
                          30);
    pgb_PL_progress.setValue(0);
    // AUDIOFILESLIST-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    lst_PLAYER.begin(layout.winWoHF.x, layout.winWoHF.y, layout.winWoHF.w, layout.winWoHF.h, displayConfig.tftSize, displayConfig.listFontSize);
    // DLNA --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_DL_mute.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_DL_mute.setOffPicturePath("/btn/Button_Mute_Off_Green.png");
    btn_DL_mute.setOnPicturePath("/btn/Button_Mute_On_Red.png");
    btn_DL_mute.setClickedOffPicturePath("/btn/Button_Mute_Off_Yellow.png");
    btn_DL_mute.setClickedOnPicturePath("/btn/Button_Mute_On_Yellow.png");
    btn_DL_mute.setAlternativeOffPicturePath("/btn/Button_Mute_Off_Magenta.png");
    btn_DL_mute.setAlternativeOnPicturePath("/btn/Button_Mute_On_Magenta.png");
    btn_DL_pause.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_DL_pause.setOffPicturePath("/btn/Button_Pause_Blue.png");
    btn_DL_pause.setOnPicturePath("/btn/Button_Play_Blue.png");
    btn_DL_pause.setClickedOffPicturePath("/btn/Button_Pause_Yellow.png");
    btn_DL_pause.setClickedOnPicturePath("/btn/Button_Play_Yellow.png");
    btn_DL_pause.setInactivePicturePath("/btn/Button_Pause_Grey.png");
    btn_DL_pause.setAlternativeOffPicturePath("/btn/Button_Pause_Magenta.png");
    btn_DL_pause.setAlternativeOnPicturePath("/btn/Button_Play_Magenta.png");
    btn_DL_pause.setValue(false);
    btn_DL_cancel.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_DL_cancel.setDefaultPicturePath("/btn/Button_Cancel_Red.png");
    btn_DL_cancel.setClickedPicturePath("/btn/Button_Cancel_Yellow.png");
    btn_DL_cancel.setAlternativePicturePath("/btn/Button_Cancel_Magenta.png");
    btn_DL_fileList.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_DL_fileList.setDefaultPicturePath("/btn/Button_List_Green.png");
    btn_DL_fileList.setClickedPicturePath("/btn/Button_List_Yellow.png");
    btn_DL_fileList.setAlternativePicturePath("/btn/Button_List_Magenta.png");
    btn_DL_radio.begin(4 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_DL_radio.setDefaultPicturePath("/btn/Button_Radio_Green.png");
    btn_DL_radio.setClickedPicturePath("/btn/Button_Radio_Yellow.png");
    btn_DL_radio.setAlternativePicturePath("/btn/Button_Radio_Magenta.png");
    sdr_DL_volume.setTransparency(true, false);
    sdr_DL_volume.begin(5 * layout.winButton.w + 10, layout.winButton.y, displayConfig.dispWidth - (5 * layout.winButton.w + 20), layout.winButton.h, layout.winButton.pl, layout.winButton.pr,
                        layout.winButton.pt, layout.winButton.pb);
    txt_DL_fName.setTransparency(true, false);
    txt_DL_fName.begin(layout.winName.x, layout.winName.y, layout.winName.w, layout.winName.h, layout.winName.pl, layout.winName.pr, layout.winName.pt, layout.winName.pb);
    txt_DL_fName.setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
    txt_DL_fName.setFont(0); // 0 -> auto)
    pic_DL_logo.setTransparency(true, false);
    pic_DL_logo.begin(layout.winLogo.x, layout.winLogo.y, layout.winLogo.w, layout.winLogo.h, layout.winLogo.pl, layout.winLogo.pr, layout.winLogo.pt, layout.winLogo.pb);
    pgb_DL_progress.setTransparency(true, false);
    pgb_DL_progress.begin(layout.winProgbar.x, layout.winProgbar.y, layout.winProgbar.w, layout.winProgbar.h, layout.winProgbar.pl, layout.winProgbar.pr, layout.winProgbar.pt, layout.winProgbar.pb, 0,
                          30);
    pgb_DL_progress.setValue(0);
    // DLNAITEMSLIST -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    lst_DLNA.begin(layout.winWoHF.x, layout.winWoHF.y, layout.winWoHF.w, layout.winWoHF.h, displayConfig.tftSize, displayConfig.listFontSize);
    // CLOCK -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    clk_CL_24.setTransparency(false, false);
    clk_CL_24.begin(layout.winDigits.x, layout.winDigits.y, layout.winDigits.w, layout.winDigits.h);
    btn_CL_alarm.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_CL_alarm.setDefaultPicturePath("/btn/Button_Bell_Green.png");
    btn_CL_alarm.setClickedPicturePath("/btn/Button_Bell_Yellow.png");
    btn_CL_alarm.setAlternativePicturePath("/btn/Button_Bell_Magenta.png");
    btn_CL_radio.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_CL_radio.setDefaultPicturePath("/btn/Button_Radio_Green.png");
    btn_CL_radio.setClickedPicturePath("/btn/Button_Radio_Yellow.png");
    btn_CL_radio.setAlternativePicturePath("/btn/Button_Radio_Magenta.png");
    btn_CL_mute.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_CL_mute.setOffPicturePath("/btn/Button_Mute_Off_Green.png");
    btn_CL_mute.setOnPicturePath("/btn/Button_Mute_On_Red.png");
    btn_CL_mute.setClickedOffPicturePath("/btn/Button_Mute_Off_Yellow.png");
    btn_CL_mute.setClickedOnPicturePath("/btn/Button_Mute_On_Yellow.png");
    btn_CL_mute.setAlternativeOffPicturePath("/btn/Button_Mute_Off_Magenta.png");
    btn_CL_mute.setAlternativeOnPicturePath("/btn/Button_Mute_On_Magenta.png");
    btn_CL_off.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_CL_off.setDefaultPicturePath("/btn/Button_Off_Red.png");
    btn_CL_off.setClickedPicturePath("/btn/Button_Off_Yellow.png");
    btn_CL_off.setAlternativePicturePath("/btn/Button_Off_Magenta.png");
    sdr_CL_volume.setTransparency(false, false);
    sdr_CL_volume.begin(5 * layout.winButton.w + 10, layout.winButton.y, layout.winButton.w * 3 - 10, layout.winButton.h, layout.winButton.pl, layout.winButton.pr, layout.winButton.pt,
                        layout.winButton.pb);
    // ALARM -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    clk_AC_red.setTransparency(false, false);
    clk_AC_red.begin(layout.winDigits.x, layout.winDigits.y, layout.winDigits.w, layout.winDigits.h);
    btn_AC_left.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_AC_left.setDefaultPicturePath("/btn/Button_Left_Blue.png");
    btn_AC_left.setClickedPicturePath("/btn/Button_Left_Yellow.png");
    btn_AC_left.setAlternativePicturePath("/btn/Button_Left_Magenta.png");
    btn_AC_right.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_AC_right.setDefaultPicturePath("/btn/Button_Right_Blue.png");
    btn_AC_right.setClickedPicturePath("/btn/Button_Right_Yellow.png");
    btn_AC_right.setAlternativePicturePath("/btn/Button_Right_Magenta.png");
    btn_AC_up.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_AC_up.setDefaultPicturePath("/btn/Button_Up_Blue.png");
    btn_AC_up.setClickedPicturePath("/btn/Button_Up_Yellow.png");
    btn_AC_up.setAlternativePicturePath("/btn/Button_Up_Magenta.png");
    btn_AC_down.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_AC_down.setDefaultPicturePath("/btn/Button_Down_Blue.png");
    btn_AC_down.setClickedPicturePath("/btn/Button_Down_Yellow.png");
    btn_AC_down.setAlternativePicturePath("/btn/Button_Down_Magenta.png");
    btn_AC_ready.begin(4 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_AC_ready.setDefaultPicturePath("/btn/Button_Ready_Blue.png");
    btn_AC_ready.setClickedPicturePath("/btn/Button_Ready_Yellow.png");
    btn_AC_ready.setAlternativePicturePath("/btn/Button_Ready_Magenta.png");
    // RINGING -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    pic_RI_logo.setTransparency(true, false);
    pic_RI_logo.begin(layout.winLogo.x, layout.winLogo.y, layout.winLogo.w, layout.winLogo.h, layout.winLogo.pl, layout.winLogo.pr, layout.winLogo.pt, layout.winLogo.pb);
    clk_RI_24small.setTransparency(false, false);
    clk_RI_24small.begin(layout.winName.x, layout.winName.y, layout.winName.w, layout.winName.h);
    // SLEEPTIMER --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_SL_up.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_SL_up.setDefaultPicturePath("/btn/Button_Up_Blue.png");
    btn_SL_up.setClickedPicturePath("/btn/Button_Up_Yellow.png");
    btn_SL_up.setAlternativePicturePath("/btn/Button_Up_Magenta.png");
    btn_SL_down.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_SL_down.setDefaultPicturePath("/btn/Button_Down_Blue.png");
    btn_SL_down.setClickedPicturePath("/btn/Button_Down_Yellow.png");
    btn_SL_down.setAlternativePicturePath("/btn/Button_Down_Magenta.png");
    btn_SL_ready.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_SL_ready.setDefaultPicturePath("/btn/Button_Ready_Blue.png");
    btn_SL_ready.setClickedPicturePath("/btn/Button_Ready_Yellow.png");
    btn_SL_ready.setAlternativePicturePath("/btn/Button_Ready_Magenta.png");
    btn_SL_cancel.begin(4 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_SL_cancel.setDefaultPicturePath("/btn/Button_Cancel_Blue.png");
    btn_SL_cancel.setClickedPicturePath("/btn/Button_Cancel_Yellow.png");
    btn_SL_cancel.setAlternativePicturePath("/btn/Button_Cancel_Magenta.png");
    otb_SL_stime.begin(0, layout.winFooter.h, layout.winFooter.w / 2, layout.winButton.y - layout.winHeader.h);
    pic_SL_logo.setTransparency(false, false);
    pic_SL_logo.begin(layout.winFooter.w / 2, layout.winFooter.h, layout.winFooter.w / 2, layout.winButton.y - layout.winHeader.h, 0, 0, 10, 0);
    // SETTINGS ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_SE_bright.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_SE_bright.setDefaultPicturePath("/btn/Button_Brightness_Green.png");
    btn_SE_bright.setClickedPicturePath("/btn/Button_Brightness_Yellow.png");
    btn_SE_bright.setAlternativePicturePath("/btn/Button_Brightness_Magenta.png");
    btn_SE_bright.setInactivePicturePath("/btn/Button_Brightness_Grey.png");
    btn_SE_equal.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_SE_equal.setDefaultPicturePath("/btn/Button_Equalizer_Green.png");
    btn_SE_equal.setClickedPicturePath("/btn/Button_Equalizer_Yellow.png");
    btn_SE_equal.setAlternativePicturePath("/btn/Button_Equalizer_Magenta.png");
    btn_SE_wifi.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_SE_wifi.setDefaultPicturePath("/btn/Button_WiFi_Green.png");
    btn_SE_wifi.setClickedPicturePath("/btn/Button_WiFi_Yellow.png");
    btn_SE_wifi.setAlternativePicturePath("/btn/Button_WiFi_Magenta.png");
    btn_SE_radio.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_SE_radio.setDefaultPicturePath("/btn/Button_Radio_Green.png");
    btn_SE_radio.setClickedPicturePath("/btn/Button_Radio_Yellow.png");
    btn_SE_radio.setAlternativePicturePath("/btn/Button_Radio_Magenta.png");
    pic_SE_logo.setTransparency(true, false);
    pic_SE_logo.begin(layout.winLogo.x, layout.winLogo.y, layout.winLogo.w, layout.winLogo.h, layout.winLogo.pl, layout.winLogo.pr, layout.winLogo.pt, layout.winLogo.pb);
    // BRIGHTNESS --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    sdr_BR_value.setTransparency(true, true);
    sdr_BR_value.begin(2 * layout.winButton.w, layout.winButton.y, 4 * layout.winButton.w, layout.winButton.h, 0, 0, 0, 0);
    sdr_BR_value.setMinMaxVal(displayConfig.brightnessMin, displayConfig.brightnessMax);
    btn_BR_ready.begin(7 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_BR_ready.setDefaultPicturePath("/btn/Button_Ready_Blue.png");
    btn_BR_ready.setClickedPicturePath("/btn/Button_Ready_Yellow.png");
    btn_BR_ready.setAlternativePicturePath("/btn/Button_Ready_Magenta.png");
    pic_BR_logo.setTransparency(false, false);
    pic_BR_logo.begin(0, layout.winWoHF.y, layout.winWoHF.w, layout.winWoHF.h, layout.winWoHF.pl, layout.winWoHF.pr, layout.winWoHF.pt, layout.winWoHF.pb);
    pic_BR_logo.setPicturePath("/common/Brightness.jpg");
    txt_BR_value.setTransparency(true, true);
    txt_BR_value.begin(0, layout.winButton.y, layout.winButton.w * 2, layout.winButton.h, layout.winButton.pl, layout.winButton.pr, layout.winButton.pt, layout.winButton.pb);
    txt_BR_value.setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
    txt_BR_value.setFont(displayConfig.fonts[4]);
    // EQUALIZER ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    sdr_EQ_lowPass.setTransparency(true, false);
    sdr_EQ_lowPass.begin(layout.sdrLP.x, layout.sdrLP.y, layout.sdrLP.w, layout.sdrLP.h, layout.sdrLP.pl, layout.sdrLP.pr, layout.sdrLP.pt, layout.sdrLP.pb);
    sdr_EQ_lowPass.setMinMaxVal(-12, 12);
    sdr_EQ_bandPass.setTransparency(true, false);
    sdr_EQ_bandPass.begin(layout.sdrBP.x, layout.sdrBP.y, layout.sdrBP.w, layout.sdrBP.h, layout.sdrBP.pl, layout.sdrBP.pr, layout.sdrBP.pt, layout.sdrBP.pb);
    sdr_EQ_bandPass.setMinMaxVal(-12, 12);
    sdr_EQ_highPass.setTransparency(true, false);
    sdr_EQ_highPass.begin(layout.sdrHP.x, layout.sdrHP.y, layout.sdrHP.w, layout.sdrHP.h, layout.sdrHP.pl, layout.sdrHP.pr, layout.sdrHP.pt, layout.sdrHP.pb);
    sdr_EQ_highPass.setMinMaxVal(-12, 12);
    sdr_EQ_balance.setTransparency(true, false);
    sdr_EQ_balance.begin(layout.sdrBAL.x, layout.sdrBAL.y, layout.sdrBAL.w, layout.sdrBAL.h, layout.sdrBAL.pl, layout.sdrBAL.pr, layout.sdrBAL.pt, layout.sdrBAL.pb);
    sdr_EQ_balance.setMinMaxVal(-16, 16);
    txt_EQ_lowPass.setTransparency(true, false);
    txt_EQ_lowPass.begin(layout.txtLP.x, layout.txtLP.y, layout.txtLP.w, layout.txtLP.h, layout.txtLP.pl, layout.txtLP.pr, layout.txtLP.pt, layout.txtLP.pb);
    txt_EQ_lowPass.setAlign(TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER);
    txt_EQ_lowPass.setFont(0); // 0 -> auto
    txt_EQ_bandPass.setTransparency(true, false);
    txt_EQ_bandPass.begin(layout.txtBP.x, layout.txtBP.y, layout.txtBP.w, layout.txtBP.h, layout.txtBP.pl, layout.txtBP.pr, layout.txtBP.pt, layout.txtBP.pb);
    txt_EQ_bandPass.setAlign(TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER);
    txt_EQ_bandPass.setFont(0); // 0 -> auto
    txt_EQ_highPass.setTransparency(true, false);
    txt_EQ_highPass.begin(layout.txtHP.x, layout.txtHP.y, layout.txtHP.w, layout.txtHP.h, layout.txtHP.pl, layout.txtHP.pr, layout.txtHP.pt, layout.txtHP.pb);
    txt_EQ_highPass.setAlign(TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER);
    txt_EQ_highPass.setFont(0); // 0 -> auto
    txt_EQ_balance.setTransparency(true, false);
    txt_EQ_balance.begin(layout.txtBAL.x, layout.txtBAL.y, layout.txtBAL.w, layout.txtBAL.h, layout.txtBAL.pl, layout.txtBAL.pr, layout.txtBAL.pt, layout.txtBAL.pb);
    txt_EQ_balance.setAlign(TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER);
    txt_EQ_balance.setFont(0); // 0 -> auto
    btn_EQ_lowPass.begin(layout.btnLP.x, layout.btnLP.y, layout.btnLP.w, layout.btnLP.h);
    btn_EQ_lowPass.setDefaultPicturePath("/btn/s/Button_LP_Green.png");
    btn_EQ_lowPass.setClickedPicturePath("/btn/s/Button_LP_Yellow.png");
    btn_EQ_lowPass.setAlternativePicturePath("/btn/s/Button_LP_Magenta.png");
    btn_EQ_bandPass.begin(layout.btnBP.x, layout.btnBP.y, layout.btnBP.w, layout.btnBP.h);
    btn_EQ_bandPass.setDefaultPicturePath("/btn/s/Button_BP_Green.png");
    btn_EQ_bandPass.setClickedPicturePath("/btn/s/Button_BP_Yellow.png");
    btn_EQ_bandPass.setAlternativePicturePath("/btn/s/Button_BP_Magenta.png");
    btn_EQ_highPass.begin(layout.btnHP.x, layout.btnHP.y, layout.btnHP.w, layout.btnHP.h);
    btn_EQ_highPass.setDefaultPicturePath("/btn/s/Button_HP_Green.png");
    btn_EQ_highPass.setClickedPicturePath("/btn/s/Button_HP_Yellow.png");
    btn_EQ_highPass.setAlternativePicturePath("/btn/s/Button_HP_Magenta.png");
    btn_EQ_balance.begin(layout.btnBAL.x, layout.btnBAL.y, layout.btnBAL.w, layout.btnBAL.h);
    btn_EQ_balance.setDefaultPicturePath("/btn/s/Button_BAL_Green.png");
    btn_EQ_balance.setClickedPicturePath("/btn/s/Button_BAL_Yellow.png");
    btn_EQ_balance.setAlternativePicturePath("/btn/s/Button_BAL_Magenta.png");
    btn_EQ_Radio.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_EQ_Radio.setDefaultPicturePath("/btn/Button_Radio_Green.png");
    btn_EQ_Radio.setClickedPicturePath("/btn/Button_Radio_Yellow.png");
    btn_EQ_Radio.setAlternativePicturePath("/btn/Button_Radio_Magenta.png");
    btn_EQ_Player.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_EQ_Player.setDefaultPicturePath("/btn/Button_Player_Green.png");
    btn_EQ_Player.setClickedPicturePath("/btn/Button_Player_Yellow.png");
    btn_EQ_Player.setAlternativePicturePath("/btn/Button_Player_Magenta.png");
    btn_EQ_mute.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_EQ_mute.setOffPicturePath("/btn/Button_Mute_Off_Green.png");
    btn_EQ_mute.setOnPicturePath("/btn/Button_Mute_On_Red.png");
    btn_EQ_mute.setClickedOffPicturePath("/btn/Button_Mute_Off_Yellow.png");
    btn_EQ_mute.setClickedOnPicturePath("/btn/Button_Mute_On_Yellow.png");
    btn_EQ_mute.setAlternativeOffPicturePath("/btn/Button_Mute_Off_Magenta.png");
    btn_EQ_mute.setAlternativeOnPicturePath("/btn/Button_Mute_On_Magenta.png");
    // BLUETOOTH ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_BT_volDown.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_BT_volDown.setDefaultPicturePath("/btn/Button_Volume_Down_Blue.png");
    btn_BT_volDown.setClickedPicturePath("/btn/Button_Volume_Down_Yellow.png");
    btn_BT_volDown.setAlternativePicturePath("/btn/Button_Volume_Down_Magenta.png");
    btn_BT_volUp.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_BT_volUp.setDefaultPicturePath("/btn/Button_Volume_Up_Blue.png");
    btn_BT_volUp.setClickedPicturePath("/btn/Button_Volume_Up_Yellow.png");
    btn_BT_volUp.setAlternativePicturePath("/btn/Button_Volume_Up_Magenta.png");
    btn_BT_pause.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_BT_pause.setOffPicturePath("/btn/Button_Pause_Blue.png");
    btn_BT_pause.setOnPicturePath("/btn/Button_Play_Blue.png");
    btn_BT_pause.setClickedOffPicturePath("/btn/Button_Pause_Yellow.png");
    btn_BT_pause.setClickedOnPicturePath("/btn/Button_Play_Yellow.png");
    btn_BT_pause.setAlternativeOffPicturePath("/btn/Button_Pause_Magenta.png");
    btn_BT_pause.setAlternativeOnPicturePath("/btn/Button_Play_Magenta.png");
    btn_BT_mode.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_BT_mode.setDefaultPicturePath("/btn/Button_RxTx_Blue.png");
    btn_BT_mode.setClickedPicturePath("/btn/Button_RxTx_Yellow.png");
    btn_BT_mode.setAlternativePicturePath("/btn/Button_RxTx_Magenta.png");
    btn_BT_radio.begin(4 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_BT_radio.setDefaultPicturePath("/btn/Button_Radio_Green.png");
    btn_BT_radio.setClickedPicturePath("/btn/Button_Radio_Yellow.png");
    btn_BT_radio.setAlternativePicturePath("/btn/Button_Radio_Magenta.png");
    btn_BT_power.begin(5 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_BT_power.setOffPicturePath("/btn/Button_Bluetooth_Red.png");
    btn_BT_power.setOnPicturePath("/btn/Button_Bluetooth_Blue.png");
    btn_BT_power.setClickedOffPicturePath("/btn/Button_Bluetooth_Yellow.png");
    btn_BT_power.setClickedOnPicturePath("/btn/Button_Bluetooth_Yellow.png");
    btn_BT_power.setAlternativeOffPicturePath("/btn/Button_Bluetooth_Magenta.png");
    btn_BT_power.setAlternativeOnPicturePath("/btn/Button_Bluetooth_Magenta.png");
    pic_BT_mode.setTransparency(true, false);
    pic_BT_mode.begin(layout.winLogo.x, layout.winLogo.y, layout.winLogo.w, layout.winLogo.h, layout.winLogo.pl, layout.winLogo.pr, layout.winLogo.pt, layout.winLogo.pb);
    pic_BT_mode.setPicturePath("/common/BTnc.png");
    pic_BT_mode.setAlternativPicturePath("/common/BTnc.png");
    txt_BT_mode.setTransparency(true, false);
    txt_BT_mode.begin(layout.winName.x, layout.winName.y, layout.winName.w, layout.winName.h, layout.winName.pl, layout.winName.pr, layout.winName.pt, layout.winName.pb);
    txt_BT_mode.setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
    txt_BT_mode.setFont(displayConfig.fonts[5]);
    // IR_SETTINGS -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_IR_radio.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_IR_radio.setDefaultPicturePath("/btn/Button_Radio_Green.png");
    btn_IR_radio.setClickedPicturePath("/btn/Button_Radio_Yellow.png");
    // WIFI_SETTINGS -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    cls_wifiSettings.setTransparency(false, false);
    cls_wifiSettings.begin(layout.winWoHF.x, layout.winWoHF.y, layout.winWoHF.w, layout.winWoHF.h, layout.winWoHF.pl, layout.winWoHF.pr, layout.winWoHF.pt, layout.winWoHF.pb);
    // ALL_STATE ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    msg_box.setTransparency(false, false);
    msg_box.begin(-1, -1, -1, -1);
}
// —————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
extern int8_t s_ir_btn_select;
extern int8_t s_subState_player;

void set_ir_pos_RA(int lr) { // RADIO   -100 left, +100 right, -127 reset
    switch (s_ir_btn_select) {
        case 0: btn_RA_staList.show(); break;
        case 1: btn_RA_player.show(); break;
        case 2: btn_RA_dlna.show(); break;
        case 3: btn_RA_clock.show(); break;
        case 4: btn_RA_sleep.show(); break;
        case 5: btn_RA_settings.show(); break;
        case 6: btn_RA_bt.show(); break;
        case 7: btn_RA_off.show(); break;
    }
    if (lr == IR_LEFT) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 7;
    }
    if (lr == IR_RIGHT) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 8) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    switch (s_ir_btn_select) {
        case 0: btn_RA_staList.showAlternativePic(); break;
        case 1: btn_RA_player.showAlternativePic(); break;
        case 2: btn_RA_dlna.showAlternativePic(); break;
        case 3: btn_RA_clock.showAlternativePic(); break;
        case 4: btn_RA_sleep.showAlternativePic(); break;
        case 5: btn_RA_settings.showAlternativePic(); break;
        case 6: btn_RA_bt.showAlternativePic(); break;
        case 7: btn_RA_off.showAlternativePic(); break;
    }
}
//-------------------------------------------------------------------------------------
void set_ir_pos_PL(int lr) { // PLAYER   -100 left, +100 right, -127 reset
    if (s_subState_player == 0) {
        if (s_ir_btn_select == -1) return;
        if (s_ir_btn_select == 8) return;
        switch (s_ir_btn_select) {
            case 0: btn_PL_prevFile.show(); break;
            case 1: btn_PL_nextFile.show(); break;
            case 2: btn_PL_ready.show(); break;
            case 3: btn_PL_playAll.show(); break;
            case 4: btn_PL_shuffle.show(); break;
            case 5: btn_PL_fileList.show(); break;
            case 6: btn_PL_radio.show(); break;
            case 7: btn_PL_off.show(); break;
        }
        if (lr == IR_LEFT) {
            s_ir_btn_select--;
            if (s_ir_btn_select == -1) s_ir_btn_select = 7;
        }
        if (lr == IR_RIGHT) {
            s_ir_btn_select++;
            if (s_ir_btn_select == 8) s_ir_btn_select = 0;
        }
        if (lr == IR_RESET) return;
        switch (s_ir_btn_select) {
            case 0: btn_PL_prevFile.showAlternativePic(); break;
            case 1: btn_PL_nextFile.showAlternativePic(); break;
            case 2: btn_PL_ready.showAlternativePic(); break;
            case 3: btn_PL_playAll.showAlternativePic(); break;
            case 4: btn_PL_shuffle.showAlternativePic(); break;
            case 5: btn_PL_fileList.showAlternativePic(); break;
            case 6: btn_PL_radio.showAlternativePic(); break;
            case 7: btn_PL_off.showAlternativePic(); break;
        }
    }
    if (s_subState_player == 1) {
        if (s_ir_btn_select == -1) return;
        if (s_ir_btn_select == 5) return;
        switch (s_ir_btn_select) {
            case 0: btn_PL_mute.show(); break;
            case 1: btn_PL_pause.show(); break;
            case 2: btn_PL_cancel.show(); break;
            case 3: btn_PL_playPrev.show(); break;
            case 4: btn_PL_playNext.show(); break;
        }
        if (lr == IR_LEFT) {
            s_ir_btn_select--;
            if (s_ir_btn_select == -1) s_ir_btn_select = 4;
        }
        if (lr == IR_RIGHT) {
            s_ir_btn_select++;
            if (s_ir_btn_select == 5) s_ir_btn_select = 0;
        }
        if (lr == IR_RESET) return;
        switch (s_ir_btn_select) {
            case 0: btn_PL_mute.showAlternativePic(); break;
            case 1: btn_PL_pause.showAlternativePic(); break;
            case 2: btn_PL_cancel.showAlternativePic(); break;
            case 3: btn_PL_playPrev.showAlternativePic(); break;
            case 4: btn_PL_playNext.showAlternativePic(); break;
        }
    }
}
//-------------------------------------------------------------------------------------
void set_ir_pos_DL(int lr) { // DLNA   -100 left, +100 right, -127 reset
    if (s_ir_btn_select == -1) return;
    if (s_ir_btn_select == 5) return;
    switch (s_ir_btn_select) {
        case 0: btn_DL_mute.show(); break;
        case 1: btn_DL_pause.show(); break;
        case 2: btn_DL_cancel.show(); break;
        case 3: btn_DL_fileList.show(); break;
        case 4: btn_DL_radio.show(); break;
    }
    if (lr == IR_LEFT) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 4;
    }
    if (lr == IR_RIGHT) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 5) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    switch (s_ir_btn_select) {
        case 0: btn_DL_mute.showAlternativePic(); break;
        case 1: btn_DL_pause.showAlternativePic(); break;
        case 2: btn_DL_cancel.showAlternativePic(); break;
        case 3: btn_DL_fileList.showAlternativePic(); break;
        case 4: btn_DL_radio.showAlternativePic(); break;
    }
}
//-------------------------------------------------------------------------------------
void set_ir_pos_CL(int lr) { // CLOCK   -100 left, +100 right, -127 reset
    if (s_ir_btn_select == -1) return;
    if (s_ir_btn_select == 4) return;
    switch (s_ir_btn_select) {
        case 0: btn_CL_alarm.show(); break;
        case 1: btn_CL_radio.show(); break;
        case 2: btn_CL_mute.show(); break;
        case 3: btn_CL_off.show(); break;
    }
    if (lr == IR_LEFT) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 3;
    }
    if (lr == IR_RIGHT) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 4) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    switch (s_ir_btn_select) {
        case 0: btn_CL_alarm.showAlternativePic(); break;
        case 1: btn_CL_radio.showAlternativePic(); break;
        case 2: btn_CL_mute.showAlternativePic(); break;
        case 3: btn_CL_off.showAlternativePic(); break;
    }
}
//-------------------------------------------------------------------------------------
void set_ir_pos_AC(int lr) { // ALARMCLOCK   -100 left, +100 right, -127 reset
    if (s_ir_btn_select == -1) return;
    if (s_ir_btn_select == 5) return;
    switch (s_ir_btn_select) {
        case 0: btn_AC_left.show(); break;
        case 1: btn_AC_right.show(); break;
        case 2: btn_AC_up.show(); break;
        case 3: btn_AC_down.show(); break;
        case 4: btn_AC_ready.show(); break;
    }
    if (lr == IR_LEFT) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 4;
    }
    if (lr == IR_RIGHT) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 5) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    switch (s_ir_btn_select) {
        case 0: btn_AC_left.showAlternativePic(); break;
        case 1: btn_AC_right.showAlternativePic(); break;
        case 2: btn_AC_up.showAlternativePic(); break;
        case 3: btn_AC_down.showAlternativePic(); break;
        case 4: btn_AC_ready.showAlternativePic(); break;
    }
}
//-------------------------------------------------------------------------------------
void set_ir_pos_SL(int lr) { // SLEEPTIMER   -100 left, +100 right, -127 reset
    if (s_ir_btn_select == -1) return;
    if (s_ir_btn_select == 4) return;
    switch (s_ir_btn_select) {
        case 0: btn_SL_up.show(); break;
        case 1: btn_SL_down.show(); break;
        case 2: btn_SL_ready.show(); break;
        case 3: btn_SL_cancel.show(); break;
    }
    if (lr == IR_LEFT) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 3;
    }
    if (lr == IR_RIGHT) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 4) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    switch (s_ir_btn_select) {
        case 0: btn_SL_up.showAlternativePic(); break;
        case 1: btn_SL_down.showAlternativePic(); break;
        case 2: btn_SL_ready.showAlternativePic(); break;
        case 3: btn_SL_cancel.showAlternativePic(); break;
    }
}
//-------------------------------------------------------------------------------------
void set_ir_pos_SE(int lr) { // SETTINGS   -100 left, +100 right, -127 reset
    if (s_ir_btn_select == -1) return;
    if (s_ir_btn_select == 4) return;
    switch (s_ir_btn_select) {
        case 0: btn_SE_bright.show(); break;
        case 1: btn_SE_equal.show(); break;
        case 2: btn_SE_wifi.show(); break;
        case 3: btn_SE_radio.show(); break;
    }
    if (lr == IR_LEFT) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 3;
    }
    if (lr == IR_RIGHT) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 4) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    switch (s_ir_btn_select) {
        case 0: btn_SE_bright.showAlternativePic(); break;
        case 1: btn_SE_equal.showAlternativePic(); break;
        case 2: btn_SE_wifi.showAlternativePic(); break;
        case 3: btn_SE_radio.showAlternativePic(); break;
    }
}
//-------------------------------------------------------------------------------------
void set_ir_pos_BR(int lr) { // SETTINGS   -100 left, +100 right, -127 reset
    if (s_ir_btn_select == -1) return;
    if (s_ir_btn_select == 1) return;
    switch (s_ir_btn_select) {
        case 0: btn_BR_ready.show(); break;
    }
    if (lr == IR_LEFT) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 3;
    }
    if (lr == IR_RIGHT) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 4) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    switch (s_ir_btn_select) {
        case 0: btn_BR_ready.showAlternativePic(); break;
    }
}
//-------------------------------------------------------------------------------------
void set_ir_pos_EQ(int lr) { // EQUALIZER   -1 left, +1 right
    if (s_ir_btn_select == -1) return;
    if (s_ir_btn_select > 6) return;
    switch (s_ir_btn_select) {
        case 0: btn_EQ_Radio.show(); break;
        case 1: btn_EQ_Player.show(); break;
        case 2: btn_EQ_mute.show(); break;
        case 3: btn_EQ_balance.show(); break;
        case 4: btn_EQ_lowPass.show(); break;
        case 5: btn_EQ_bandPass.show(); break;
        case 6: btn_EQ_highPass.show(); break;
    }
    if (lr == IR_LEFT && s_ir_btn_select < 3) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 2;
    }
    if (lr == IR_RIGHT && s_ir_btn_select < 3) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 3) s_ir_btn_select = 0;
    }
    if (lr == IR_UP) {
        if (s_ir_btn_select < 3)
            s_ir_btn_select = 3;
        else {
            s_ir_btn_select++;
            if (s_ir_btn_select == 7) s_ir_btn_select = 3;
        }
    }
    if (lr == IR_DOWN) {
        s_ir_btn_select--;
        if (s_ir_btn_select == 2) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    switch (s_ir_btn_select) {
        case 0: btn_EQ_Radio.showAlternativePic(); break;
        case 1: btn_EQ_Player.showAlternativePic(); break;
        case 2: btn_EQ_mute.showAlternativePic(); break;
        case 3: btn_EQ_balance.showAlternativePic(); break;
        case 4: btn_EQ_lowPass.showAlternativePic(); break;
        case 5: btn_EQ_bandPass.showAlternativePic(); break;
        case 6: btn_EQ_highPass.showAlternativePic(); break;
    }
}
//-------------------------------------------------------------------------------------
void set_ir_pos_BT(int lr) { // BLUETOOTH   -1 left, +1 right
    if (s_ir_btn_select == -1) return;
    if (s_ir_btn_select == 6) return;
    switch (s_ir_btn_select) {
        case 0: btn_BT_volDown.show(); break;
        case 1: btn_BT_volUp.show(); break;
        case 2: btn_BT_pause.show(); break;
        case 3: btn_BT_mode.show(); break;
        case 4: btn_BT_radio.show(); break;
        case 5: btn_BT_power.show(); break;
    }
    if (lr == IR_LEFT) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 5;
    }
    if (lr == IR_RIGHT) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 6) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    switch (s_ir_btn_select) {
        case 0: btn_BT_volDown.showAlternativePic(); break;
        case 1: btn_BT_volUp.showAlternativePic(); break;
        case 2: btn_BT_pause.showAlternativePic(); break;
        case 3: btn_BT_mode.showAlternativePic(); break;
        case 4: btn_BT_radio.showAlternativePic(); break;
        case 5: btn_BT_power.showAlternativePic(); break;
    }
}
//-------------------------------------------------------------------------------------