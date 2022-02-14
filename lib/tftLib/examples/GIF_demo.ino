/*
 *
 * GIF Demo
 *
 * this is only experimental
 * not all gif formats are supported
 * must have fast SD card
 * SPI wires must be short
 * only ILI9341 is fast enough
 * set SD(spi) frequency as high as possible
 *
 */



#include "Arduino.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "tft.h"

#define TFT_CS        22    // do not use GPI032 or GPIO33 here
#define TFT_DC        21    // do not use GPI032 or GPIO33 here
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18
#define TP_IRQ        39
#define TP_CS         16
#define SD_CS          5


#define min(X, Y) (((X) < (Y)) ? (X) : (Y))

TFT tft(0);   //0=ILI9341, 1= HX8347D

void setup() {
    pinMode(TP_CS, OUTPUT);
    digitalWrite(TP_CS, HIGH); // disable the touchpad

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    tft.begin(TFT_CS, TFT_DC, SPI_MOSI, SPI_MISO, SPI_SCK);
    SD.begin(SD_CS,SPI,8000000); // 20000000 40000000, 80000000
}

//-------------------------------------------------------------------------------------
void loop(void) {
        tft.setRotation(0); //portait
        tft.drawGifFile(SD,"/birds.gif", 0, 0, 1);
        tft.drawGifFile(SD,"/tap.gif", 0, 0, 3);
        tft.drawGifFile(SD,"/clock.gif", 0, 0, 2);
        tft.setRotation(3);
        tft.drawGifFile(SD,"/radar.gif", 0, 0, 1);
}
