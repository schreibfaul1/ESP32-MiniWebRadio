# ESP32-TFT-Library-ILI9431-HX8347D
TFT Library for controller ILI9341 and HX8347D

Displays working with this library:
![Display](https://github.com/schreibfaul1/ESP32-TFT-Library-ILI9341-HX8347D/blob/master/additional%20info/tested%20displays.jpg)

Create new fonts with MikroElektronika GLCD Font Creator and insert the new font in fonts.h
You can also display bitmaps, touchpadcontroller XPT2046 is included

Use the touchpad, if the display have one
```` c++
#include "Arduino.h"
#include "SPI.h"
#include "tft.h"

#define TP_IRQ        39
#define TP_CS         16

TFT tft(1); // (0) ILI9341 Display, (1) Waveshare 2.8 TFT with TP
TP tp(TP_CS, TP_IRQ);

uint16_t tp_x, tp_y;

void setup() {
    SPI.begin();
    tft.begin();
    tft.setRotation(3); // Use landscape format
    tp.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREENYELLOW);
    tft.setTextSize(2);
}

//-------------------------------------------------------------------------------------
void loop(void) {
    tp.loop();
}
//-------------------------------------------------------------------------------------

// Event from TouchPad
void tp_pressed(uint16_t x, uint16_t y){
    tp_x=x;  tp_y=y;
}
void tp_released(){
    tft.fillRect(100, 100, 80, 40, TFT_BLACK);
    tft.setCursor(100, 100);
    tft.print("PosX="); tft.println(tp_x);
    tft.print("PosY="); tft.println(tp_y);
}
````
Display a bitmap, GIF or JPEG
```` c++
#include "Arduino.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "tft.h"

TFT tft(1); // (0) ILI 9341 (1) HX8347

void setup() {
    SPI.begin();
    tft.begin();
    SD.begin();
    tft.setRotation(3); //landscape
    tft.drawBmpFile(SD, "/tiger.bmp", 0, 0);
    delay(3000);
    tft.drawJpgFile(SD, "/Wallpaper 320x240.jpg");
    delay(3000);
    tft.setRotation(0);
    tft.drawGifFile(SD, "/tap.gif", 0, 0, 4); // xpos, ypos, repeat 0= no animation
}

//-------------------------------------------------------------------------------------
void loop(void) {
        delay(3000);
}
//-------------------------------------------------------------------------------------
````

![Display](https://github.com/schreibfaul1/ESP32-TFT-Library-ILI9431-HX8347D/blob/master/additional%20info/Tiger.jpg)
