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
    const coor& winArea1;
    const coor& winArea2;
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
//  | Header                                    |       winHeader=20px
//  +-------------------------------------------+ _yName=20
//  | winArea1                                  |
//  | Logo                   StationName        |       winName=100px
//  |                                           |
//  +-------------------------------------------+ _yTitle=120
//  | winArea2                                  |
//  |              StreamTitle                  |       winSTitle=100px
//  |                                           |
//  +-------------------------------------------+ _yFooter=220
//  | Footer                                    |       winFooter=20px
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
constexpr coor winArea1 = coor().pos(0, h_footer).size(h_res, h_area).pad(0, 5, 0, 3);
constexpr coor winArea2 = coor().pos(0, y_area2).size(h_res, h_area).pad(0, 5, 0, 3);
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
//  | winArea1                                  |
//  | Logo                   StationName        |       winName=130px
//  |                                           |
//  +-------------------------------------------+ _yTitle=160
//  | winArea2                                  |
//  |              StreamTitle                  |       winSTitle=130px
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
constexpr coor winArea1 = coor().pos(0, h_footer).size(h_res, h_area).pad(0, 5, 0, 3);
constexpr coor winArea2 = coor().pos(0, y_area2).size(h_res, h_area).pad(0, 5, 0, 3);
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
//  | winArea1                                  |
//  | Logo                   StationName        |       winName=190px
//  |                                           |
//  +-------------------------------------------+ _yTitle=240
//  | winArea2                                  |
//  |              StreamTitle                  |       winSTitle=190px
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
constexpr coor winArea1 = coor().pos(0, h_footer).size(h_res, h_area).pad(0, 5, 0, 3);
constexpr coor winArea2 = coor().pos(0, y_area2).size(h_res, h_area).pad(0, 5, 0, 3);
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
//  | winArea1                                  |
//  | Logo                   StationName        |       winName=240px, area1
//  |                                           |
//  +-------------------------------------------+ _yTitle=300
//  | winArea2                                  |
//  |              StreamTitle                  |       winSTitle=240px, area2
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
constexpr coor winArea1 = coor().pos(0, h_footer).size(h_res, h_area).pad(0, 5, 0, 3);
constexpr coor winArea2 = coor().pos(0, y_area2).size(h_res, h_area).pad(0, 5, 0, 3);
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
    return {winHeader, winLogo, winName, winProgbar, winArea1, winArea2, winSTitle, winVUmeter, winFooter, winButton, winDigits, winWoHF, sdrHP,
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
    btn_RA_mute.setPicturePath("/btn/Button_Mute");
    btn_RA_prevSta.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_prevSta.setPicturePath("/btn/Button_Previous");
    btn_RA_nextSta.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_nextSta.setPicturePath("/btn/Button_Next");
    btn_RA_recorder.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_recorder.setPicturePath("/btn/Button_Recorder");
    btn_RA_staList.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_staList.setPicturePath("/btn/Button_List");
    btn_RA_player.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_player.setPicturePath("/btn/Button_Player");
    btn_RA_dlna.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_dlna.setPicturePath("/btn/Button_DLNA");
    btn_RA_clock.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_clock.setPicturePath("/btn/Button_Clock");
    btn_RA_sleep.begin(4 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_sleep.setPicturePath("/btn/Button_OffTimer");
    btn_RA_settings.begin(5 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_settings.setPicturePath("/btn/Button_Settings");
    btn_RA_bt.begin(6 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_bt.setPicturePath("/btn/Button_Bluetooth");
    btn_RA_off.begin(7 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_RA_off.setPicturePath("/btn/Button_Off");
    txt_RA_sTitle.begin(layout.winSTitle.x, layout.winSTitle.y, layout.winSTitle.w, layout.winSTitle.h, layout.winSTitle.pl, layout.winSTitle.pr, layout.winSTitle.pt, layout.winSTitle.pb);
    txt_RA_sTitle.setAlign(TFT_ALIGN_LEFT, TFT_ALIGN_CENTER);
    txt_RA_sTitle.setFont(0); // 0 -> auto
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
    btn_PL_mute.setPicturePath("/btn/Button_Mute");
    btn_PL_pause.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_pause.setPicturePath("/btn/Button_Pause");
    btn_PL_pause.setValue(false);
    btn_PL_cancel.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_cancel.setPicturePath("/btn/Button_Cancel");
    sdr_PL_volume.setTransparency(true, false);
    sdr_PL_volume.begin(5 * layout.winButton.w + 10, layout.winButton.y, displayConfig.dispWidth - (5 * layout.winButton.w + 20), layout.winButton.h, layout.winButton.pl, layout.winButton.pr,
                        layout.winButton.pt, layout.winButton.pb);
    btn_PL_prevFile.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_prevFile.setPicturePath("/btn/Button_Left");
    btn_PL_nextFile.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_nextFile.setPicturePath("/btn/Button_Right");
    btn_PL_ready.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_ready.setPicturePath("/btn/Button_Ready");
    btn_PL_playAll.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_playAll.setPicturePath("/btn/Button_PlayAll");
    btn_PL_shuffle.begin(4 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_shuffle.setPicturePath("/btn/Button_Shuffle");
    btn_PL_fileList.setTransparency(true, false);
    btn_PL_fileList.begin(5 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_fileList.setPicturePath("/btn/Button_List");
    btn_PL_radio.setTransparency(true, false);
    btn_PL_radio.begin(6 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_radio.setPicturePath("/btn/Button_Radio");
    btn_PL_off.setTransparency(true, false);
    btn_PL_off.begin(7 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_off.setPicturePath("/btn/Button_Off");
    btn_PL_playPrev.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_playPrev.setPicturePath("/btn/Button_Previous");
    btn_PL_playNext.begin(4 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_PL_playNext.setPicturePath("/btn/Button_Next");
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
    btn_DL_mute.setPicturePath("/btn/Button_Mute");
    btn_DL_pause.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_DL_pause.setPicturePath("/btn/Button_Pause");
    btn_DL_pause.setValue(false);
    btn_DL_cancel.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_DL_cancel.setPicturePath("/btn/Button_Cancel");
    btn_DL_fileList.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_DL_fileList.setPicturePath("/btn/Button_List");
    btn_DL_radio.begin(4 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_DL_radio.setPicturePath("/btn/Button_Radio");
    sdr_DL_volume.setTransparency(true, false);
    sdr_DL_volume.begin(5 * layout.winButton.w + 10, layout.winButton.y, displayConfig.dispWidth - (5 * layout.winButton.w + 20), layout.winButton.h, layout.winButton.pl, layout.winButton.pr,
                        layout.winButton.pt, layout.winButton.pb);
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
    btn_CL_alarm.setPicturePath("/btn/Button_Bell");
    btn_CL_radio.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_CL_radio.setPicturePath("/btn/Button_Radio");
    btn_CL_mute.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_CL_mute.setPicturePath("/btn/Button_Mute");
    btn_CL_off.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_CL_off.setPicturePath("/btn/Button_Off");
    sdr_CL_volume.setTransparency(false, false);
    sdr_CL_volume.begin(5 * layout.winButton.w + 10, layout.winButton.y, layout.winButton.w * 3 - 10, layout.winButton.h, layout.winButton.pl, layout.winButton.pr, layout.winButton.pt,
                        layout.winButton.pb);
    // ALARM -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    clk_AC_red.setTransparency(false, false);
    clk_AC_red.begin(layout.winDigits.x, layout.winDigits.y, layout.winDigits.w, layout.winDigits.h);
    btn_AC_left.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_AC_left.setPicturePath("/btn/Button_Left");
    btn_AC_right.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_AC_right.setPicturePath("/btn/Button_Right");
    btn_AC_up.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_AC_up.setPicturePath("/btn/Button_Up");
    btn_AC_down.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_AC_down.setPicturePath("/btn/Button_Down");
    btn_AC_ready.begin(4 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_AC_ready.setPicturePath("/btn/Button_Ready");

    // RINGING -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    pic_RI_logo.setTransparency(true, false);
    pic_RI_logo.begin(layout.winLogo.x, layout.winLogo.y, layout.winLogo.w, layout.winLogo.h, layout.winLogo.pl, layout.winLogo.pr, layout.winLogo.pt, layout.winLogo.pb);
    clk_RI_24small.setTransparency(false, false);
    clk_RI_24small.begin(layout.winName.x, layout.winName.y, layout.winName.w, layout.winName.h);
    // SLEEPTIMER --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_SL_up.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_SL_up.setPicturePath("/btn/Button_Up");
    btn_SL_down.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_SL_down.setPicturePath("/btn/Button_Down");
    btn_SL_ready.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_SL_ready.setPicturePath("/btn/Button_Ready");
    btn_SL_cancel.begin(4 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_SL_cancel.setPicturePath("/btn/Button_Cancel");
    otb_SL_stime.begin(0, layout.winFooter.h, layout.winFooter.w / 2, layout.winButton.y - layout.winHeader.h);
    pic_SL_logo.setTransparency(false, false);
    pic_SL_logo.begin(layout.winFooter.w / 2, layout.winFooter.h, layout.winFooter.w / 2, layout.winButton.y - layout.winHeader.h, 0, 0, 10, 0);
    // SETTINGS ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_SE_bright.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_SE_bright.setPicturePath("/btn/Button_Brightness");
    btn_SE_equal.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_SE_equal.setPicturePath("/btn/Button_Equalizer");
    btn_SE_wifi.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_SE_wifi.setPicturePath("/btn/Button_WiFi");
    btn_SE_radio.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_SE_radio.setPicturePath("/btn/Button_Radio");
    pic_SE_logo.setTransparency(true, false);
    pic_SE_logo.begin(layout.winLogo.x, layout.winLogo.y, layout.winLogo.w, layout.winLogo.h, layout.winLogo.pl, layout.winLogo.pr, layout.winLogo.pt, layout.winLogo.pb);
    // BRIGHTNESS --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    sdr_BR_value.setTransparency(true, true);
    sdr_BR_value.begin(2 * layout.winButton.w, layout.winButton.y, 4 * layout.winButton.w, layout.winButton.h, 0, 0, 0, 0);
    sdr_BR_value.setMinMaxVal(displayConfig.brightnessMin, displayConfig.brightnessMax);
    btn_BR_ready.begin(7 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_BR_ready.setPicturePath("/btn/Button_Ready");
    pic_BR_logo.setTransparency(false, false);
    pic_BR_logo.begin(0, layout.winWoHF.y, layout.winWoHF.w, layout.winWoHF.h, layout.winWoHF.pl, layout.winWoHF.pr, layout.winWoHF.pt, layout.winWoHF.pb);
    pic_BR_logo.setPicturePath("/common/Brightness.jpg");
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
    txt_EQ_lowPass.begin(layout.txtLP.x, layout.txtLP.y, layout.txtLP.w, layout.txtLP.h, layout.txtLP.pl, layout.txtLP.pr, layout.txtLP.pt, layout.txtLP.pb);
    txt_EQ_lowPass.setAlign(TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER);
    txt_EQ_lowPass.setFont(0); // 0 -> auto
    txt_EQ_bandPass.begin(layout.txtBP.x, layout.txtBP.y, layout.txtBP.w, layout.txtBP.h, layout.txtBP.pl, layout.txtBP.pr, layout.txtBP.pt, layout.txtBP.pb);
    txt_EQ_bandPass.setAlign(TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER);
    txt_EQ_bandPass.setFont(0); // 0 -> auto
    txt_EQ_highPass.begin(layout.txtHP.x, layout.txtHP.y, layout.txtHP.w, layout.txtHP.h, layout.txtHP.pl, layout.txtHP.pr, layout.txtHP.pt, layout.txtHP.pb);
    txt_EQ_highPass.setAlign(TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER);
    txt_EQ_highPass.setFont(0); // 0 -> auto
    txt_EQ_balance.begin(layout.txtBAL.x, layout.txtBAL.y, layout.txtBAL.w, layout.txtBAL.h, layout.txtBAL.pl, layout.txtBAL.pr, layout.txtBAL.pt, layout.txtBAL.pb);
    txt_EQ_balance.setAlign(TFT_ALIGN_RIGHT, TFT_ALIGN_CENTER);
    txt_EQ_balance.setFont(0); // 0 -> auto
    btn_EQ_lowPass.begin(layout.btnLP.x, layout.btnLP.y, layout.btnLP.w, layout.btnLP.h);
    btn_EQ_lowPass.setPicturePath("/btn/s/Button_LP");
    btn_EQ_bandPass.begin(layout.btnBP.x, layout.btnBP.y, layout.btnBP.w, layout.btnBP.h);
    btn_EQ_bandPass.setPicturePath("/btn/s/Button_BP");
    btn_EQ_highPass.begin(layout.btnHP.x, layout.btnHP.y, layout.btnHP.w, layout.btnHP.h);
    btn_EQ_highPass.setPicturePath("/btn/s/Button_HP");
    btn_EQ_balance.begin(layout.btnBAL.x, layout.btnBAL.y, layout.btnBAL.w, layout.btnBAL.h);
    btn_EQ_balance.setPicturePath("/btn/s/Button_BAL");
    btn_EQ_Radio.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_EQ_Radio.setPicturePath("/btn/Button_Radio");
    btn_EQ_Player.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_EQ_Player.setPicturePath("/btn/Button_Player");
    btn_EQ_mute.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_EQ_mute.setPicturePath("/btn/Button_Mute");
    // BLUETOOTH ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_BT_volDown.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_BT_volDown.setPicturePath("/btn/Button_Volume_Down");
    btn_BT_volUp.begin(1 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_BT_volUp.setPicturePath("/btn/Button_Volume_Up");
    btn_BT_pause.begin(2 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_BT_pause.setPicturePath("/btn/Button_Pause");
    btn_BT_mode.begin(3 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_BT_mode.setPicturePath("/btn/Button_RxTx");
    btn_BT_radio.begin(4 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_BT_radio.setPicturePath("/btn/Button_Radio");
    btn_BT_power.begin(5 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_BT_power.setPicturePath("/btn/Button_Bluetooth");
    pic_BT_mode.setTransparency(true, false);
    pic_BT_mode.begin(layout.winLogo.x, layout.winLogo.y, layout.winLogo.w, layout.winLogo.h, layout.winLogo.pl, layout.winLogo.pr, layout.winLogo.pt, layout.winLogo.pb);
    pic_BT_mode.setPicturePath("/common/BTnc.png");
    pic_BT_mode.setAlternativPicturePath("/common/BTnc.png");
    txt_BT_mode.begin(layout.winName.x, layout.winName.y, layout.winName.w, layout.winName.h, layout.winName.pl, layout.winName.pr, layout.winName.pt, layout.winName.pb);
    txt_BT_mode.setAlign(TFT_ALIGN_CENTER, TFT_ALIGN_CENTER);
    txt_BT_mode.setFont(displayConfig.fonts[5]);
    // IR_SETTINGS -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    btn_IR_radio.begin(0 * layout.winButton.w, layout.winButton.y, layout.winButton.w, layout.winButton.h);
    btn_IR_radio.setPicturePath("/btn/Button_Radio");
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
    uint8_t i = 0;
    defocusAllObjects();
next:
    i++;
    if (lr == IR_LEFT) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 7;
    }
    if (lr == IR_RIGHT) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 8) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    bool res = false;
    switch (s_ir_btn_select) {
        case 0: res = btn_RA_staList.setFocus(true); break;
        case 1: res = btn_RA_player.setFocus(true); break;
        case 2: res = btn_RA_dlna.setFocus(true); break;
        case 3: res = btn_RA_clock.setFocus(true); break;
        case 4: res = btn_RA_sleep.setFocus(true); break;
        case 5: res = btn_RA_settings.setFocus(true); break;
        case 6: res = btn_RA_bt.setFocus(true); break;
        case 7: res = btn_RA_off.setFocus(true); break;
    }
    if(i == 2) return;
    if(res == false) goto next;
}
//-------------------------------------------------------------------------------------
void set_ir_pos_PL(int lr) { // PLAYER   -100 left, +100 right, -127 reset
    uint8_t i = 0;
    if (s_subState_player == 0) {
        if (s_ir_btn_select == -1) return;
        if (s_ir_btn_select == 8) return;
        defocusAllObjects();
next0:
        i++;
        if (lr == IR_LEFT) {
            s_ir_btn_select--;
            if (s_ir_btn_select == -1) s_ir_btn_select = 7;
        }
        if (lr == IR_RIGHT) {
            s_ir_btn_select++;
            if (s_ir_btn_select == 8) s_ir_btn_select = 0;
        }
        if (lr == IR_RESET) return;
        bool res = true;
        switch (s_ir_btn_select) {
            case 0: res = btn_PL_prevFile.setFocus(true); break;
            case 1: res = btn_PL_nextFile.setFocus(true); break;
            case 2: res = btn_PL_ready.setFocus(true); break;
            case 3: res = btn_PL_playAll.setFocus(true); break;
            case 4: res = btn_PL_shuffle.setFocus(true); break;
            case 5: res = btn_PL_fileList.setFocus(true); break;
            case 6: res = btn_PL_radio.setFocus(true); break;
            case 7: res = btn_PL_off.setFocus(true); break;
        }
        if(i == 2) return;
        if(res == false) goto next0;
    }
    if (s_subState_player == 1) {
        if (s_ir_btn_select == -1) return;
        if (s_ir_btn_select == 5) return;
        defocusAllObjects();
next1:
        i++;
        if (lr == IR_LEFT) {
            s_ir_btn_select--;
            if (s_ir_btn_select == -1) s_ir_btn_select = 4;
        }
        if (lr == IR_RIGHT) {
            s_ir_btn_select++;
            if (s_ir_btn_select == 5) s_ir_btn_select = 0;
        }
        if (lr == IR_RESET) return;
        bool res = true;
        switch (s_ir_btn_select) {
            case 0: res = btn_PL_mute.setFocus(true); break;
            case 1: res = btn_PL_pause.setFocus(true); break;
            case 2: res = btn_PL_cancel.setFocus(true); break;
            case 3: res = btn_PL_playPrev.setFocus(true); break;
            case 4: res = btn_PL_playNext.setFocus(true); break;
        }
        if(i == 2) return;
        if(res == false) goto next1;
    }

}
//-------------------------------------------------------------------------------------
void set_ir_pos_DL(int lr) { // DLNA   -100 left, +100 right, -127 reset
    uint8_t i = 0;
    if (s_ir_btn_select == -1) return;
    if (s_ir_btn_select == 5) return;
    defocusAllObjects();
next:
    i++;
    if (lr == IR_LEFT) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 4;
    }
    if (lr == IR_RIGHT) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 5) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    bool res = false;
    switch (s_ir_btn_select) {
        case 0: res = btn_DL_mute.setFocus(true); break;
        case 1: res = btn_DL_pause.setFocus(true); break;
        case 2: res = btn_DL_cancel.setFocus(true); break;
        case 3: res = btn_DL_fileList.setFocus(true); break;
        case 4: res = btn_DL_radio.setFocus(true); break;
    }
    if(i == 2) return;
    if(res == false) goto next;
}
//-------------------------------------------------------------------------------------
void set_ir_pos_CL(int lr) { // CLOCK   -100 left, +100 right, -127 reset
    uint8_t i = 0;
    if (s_ir_btn_select == -1) return;
    if (s_ir_btn_select == 4) return;
    defocusAllObjects();
next:
    i++;
    if (lr == IR_LEFT) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 3;
    }
    if (lr == IR_RIGHT) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 4) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    bool res = false;
    switch (s_ir_btn_select) {
        case 0: res = btn_CL_alarm.setFocus(true); break;
        case 1: res = btn_CL_radio.setFocus(true); break;
        case 2: res = btn_CL_mute.setFocus(true); break;
        case 3: res = btn_CL_off.setFocus(true); break;
    }
    if(i == 2) return;
    if(res == false) goto next;
}
//-------------------------------------------------------------------------------------
void set_ir_pos_AC(int lr) { // ALARMCLOCK   -100 left, +100 right, -127 reset
    uint8_t i = 0;
    if (s_ir_btn_select == -1) return;
    if (s_ir_btn_select == 5) return;
    defocusAllObjects();
next:
    i++;
    if (lr == IR_LEFT) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 4;
    }
    if (lr == IR_RIGHT) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 5) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    bool res = false;
    switch (s_ir_btn_select) {
        case 0: res = btn_AC_left.setFocus(true); break;
        case 1: res = btn_AC_right.setFocus(true); break;
        case 2: res = btn_AC_up.setFocus(true); break;
        case 3: res = btn_AC_down.setFocus(true); break;
        case 4: res = btn_AC_ready.setFocus(true); break;
    }
    if(i == 2) return;
    if(res == false) goto next;
}
//-------------------------------------------------------------------------------------
void set_ir_pos_SL(int lr) { // SLEEPTIMER   -100 left, +100 right, -127 reset
    uint8_t i = 0;
    if (s_ir_btn_select == -1) return;
    if (s_ir_btn_select == 4) return;
    defocusAllObjects();
next:
    i++;
    if (lr == IR_LEFT) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 3;
    }
    if (lr == IR_RIGHT) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 4) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    bool res = false;
    switch (s_ir_btn_select) {
        case 0: res = btn_SL_up.setFocus(true); break;
        case 1: res = btn_SL_down.setFocus(true); break;
        case 2: res = btn_SL_ready.setFocus(true); break;
        case 3: res = btn_SL_cancel.setFocus(true); break;
    }
    if(i == 2) return;
    if(res == false) goto next;
}
//-------------------------------------------------------------------------------------
void set_ir_pos_SE(int lr) { // SETTINGS   -100 left, +100 right, -127 reset
    uint8_t i = 0;
    if (s_ir_btn_select == -1) return;
    if (s_ir_btn_select == 4) return;
    defocusAllObjects();
next:
    i++;
    if (lr == IR_LEFT) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 3;
    }
    if (lr == IR_RIGHT) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 4) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    bool res = false;
    switch (s_ir_btn_select) {
        case 0: res = btn_SE_bright.setFocus(true); break;
        case 1: res = btn_SE_equal.setFocus(true); break;
        case 2: res = btn_SE_wifi.setFocus(true); break;
        case 3: res = btn_SE_radio.setFocus(true); break;
    }
    if(i == 2) return;
    if(res == false) goto next;
}
//-------------------------------------------------------------------------------------
void set_ir_pos_BR(int lr) { // SETTINGS   -100 left, +100 right, -127 reset
    uint8_t i = 0;
    if (s_ir_btn_select == -1) return;
    if (s_ir_btn_select == 1) return;
    defocusAllObjects();
next:
    i++;
    if (lr == IR_LEFT) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 3;
    }
    if (lr == IR_RIGHT) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 4) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    bool res = false;
    switch (s_ir_btn_select) {
        case 0: res = btn_BR_ready.setFocus(true); break;
    }
    if(i == 2) return;
    if(res == false) goto next;
}
//-------------------------------------------------------------------------------------
void set_ir_pos_EQ(int lr) { // EQUALIZER   -1 left, +1 right
    uint8_t i = 0;
    if (s_ir_btn_select == -1) return;
    if (s_ir_btn_select > 6) return;
    defocusAllObjects();

next:
    i++;
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
    bool res = false;
    switch (s_ir_btn_select) {
        case 0: res = btn_EQ_Radio.setFocus(true); break;
        case 1: res = btn_EQ_Player.setFocus(true); break;
        case 2: res = btn_EQ_mute.setFocus(true); break;
        case 3: res = btn_EQ_balance.setFocus(true); break;
        case 4: res = btn_EQ_lowPass.setFocus(true); break;
        case 5: res = btn_EQ_bandPass.setFocus(true); break;
        case 6: res = btn_EQ_highPass.setFocus(true); break;
    }
    if(i == 2) return;
    if(res == false) goto next;
}
//-------------------------------------------------------------------------------------
void set_ir_pos_BT(int lr) { // BLUETOOTH   -1 left, +1 right
    uint8_t i = 0;
    if (s_ir_btn_select == -1) return;
    if (s_ir_btn_select == 6) return;
    defocusAllObjects();

next:
    i++;
    if (lr == IR_LEFT) {
        s_ir_btn_select--;
        if (s_ir_btn_select == -1) s_ir_btn_select = 5;
    }
    if (lr == IR_RIGHT) {
        s_ir_btn_select++;
        if (s_ir_btn_select == 6) s_ir_btn_select = 0;
    }
    if (lr == IR_RESET) return;
    bool res = false;
    switch (s_ir_btn_select) {
        case 0: res = btn_BT_volDown.setFocus(true); break;
        case 1: res = btn_BT_volUp.setFocus(true); break;
        case 2: res = btn_BT_pause.setFocus(true); break;
        case 3: res = btn_BT_mode.setFocus(true); break;
        case 4: res = btn_BT_radio.setFocus(true); break;
        case 5: res = btn_BT_power.setFocus(true); break;
    }
    if(i == 2) return;
    if(res == false) goto next;
}
//-------------------------------------------------------------------------------------