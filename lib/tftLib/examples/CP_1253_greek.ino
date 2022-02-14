/*
 * Example for codepage 1253 greek
 *
 *
 * uncomment #include "fonts/Garamond_greek.h" in tft.h
 */

#include "Arduino.h"
#include "SPI.h"
#include "tft.h"

#define TFT_CS        22    // do not use GPI032 or GPIO33 here
#define TFT_DC        21    // do not use GPI032 or GPIO33 here
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18
#define TP_CS         16


TFT tft(1);  //0=ILI9341, 1= HX8347D

void setup() {
    String txt="";
    pinMode(TP_CS, OUTPUT);
    digitalWrite(TP_CS, HIGH); // disable the touchpad
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    tft.begin(TFT_CS, TFT_DC, SPI_MOSI, SPI_MISO, SPI_SCK);
    tft.fillScreen(TFT_BLACK);
    tft.setRotation(1);
    tft.setCursor(0, 0);
    tft.setFont(Garamond27x25greek);
    tft.setTextColor(TFT_YELLOW);
    txt="Ο Παπάς πρώτα τα γένια του ευλογεί.";
    tft.print(txt);
    tft.setTextColor(TFT_WHITE);
    txt=" The pastor blesses his own beard first\n";
    tft.print(txt);
    tft.setTextColor(TFT_YELLOW);
    txt="Ο τρελός είδε τον μεθυσμένο και φοβήθηκε.";
    tft.print(txt);
    tft.setTextColor(TFT_WHITE);
    txt= " A drunk is more dangerous than a madman";
    tft.print(txt);

}
//-------------------------------------------------------------------------------------
void loop(void) {
        delay(2000);
}



