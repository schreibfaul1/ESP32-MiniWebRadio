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

TFT tft(1);   //0=ILI9341, 1= HX8347D

void setup() {
    pinMode(TP_CS, OUTPUT);
    digitalWrite(TP_CS, HIGH); // disable the touchpad
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    tft.begin(TFT_CS, TFT_DC, SPI_MOSI, SPI_MISO, SPI_SCK);
    SD.begin(SD_CS);
    //SD.begin(SD_CS,SPI,16000000); // faster speed
}

//-------------------------------------------------------------------------------------
void loop(void) {
        tft.setRotation(0); //portait
        tft.drawBmpFile(SD, "/Wall_e.bmp", 0, 0);
        delay(2000);
        tft.setRotation(3); //landscape
        tft.drawJpgFile(SD,"/Wallpaper 320x240.jpg", 0, 0);
        delay(2000);
        tft.drawJpgFile(SD,"/Delaware 320x240.jpg", 0, 0);
        delay(2000);

}
