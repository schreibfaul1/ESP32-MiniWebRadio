/*
 * Example for codepage 1252 latin
 *
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
    tft.setFont(Garamond17x21);
    tft.setTextColor(TFT_YELLOW);
    txt= "  Hør mig Vætter,\n  Hør mig Thurser,\n";
    txt+="  Hør mig alfer og dværge.\n";
    txt+="  Hør mig Aser,\n  Hør mig Vaner,\n";
    txt+="  Hør mig disedøtre og muspelsønner.\n";
    txt+="  På dette sted mrker vi marken,\n";
    txt+="  På dette sted ære vi guder,\n";
    txt+="  På dette sted holder vi blot.\n";
    tft.println(txt);
    tft.setFont(Garamond19x24);
    tft.setTextColor(TFT_WHITE);
    txt="  ¿ Æ Ê Ð Ø ß æ ç ë ï ñ ø ¡ ¢ £";
    tft.print(txt);
}
//-------------------------------------------------------------------------------------
void loop(void) {
        delay(2000);
}


