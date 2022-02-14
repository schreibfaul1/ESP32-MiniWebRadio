/*
 * Example, font "Old_English_Text"
 *
 *
 * uncomment #fonts/Old_English_Text_MT.h" in tft.h
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


TFT tft(1); //0=ILI9341, 1= HX8347D

void setup() {
    String txt="";
    pinMode(TP_CS, OUTPUT);
    digitalWrite(TP_CS, HIGH); // disable the touchpad

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    tft.begin(TFT_CS, TFT_DC, SPI_MOSI, SPI_MISO, SPI_SCK);
    tft.fillScreen(TFT_BLACK);
    tft.setRotation(1);
    tft.setCursor(0, 0);
    tft.setFont(Old_English_Text_MT26x26);
    tft.setTextColor(TFT_WHITE);
    txt= "    O rose, thou art sick!\n    The invisible worm,\n";
    txt+="    That flies in the nightn\n    In the howling storm.\n";
    txt+="    Has found out thy bed\n    Of crimson joy,\n";
    txt+="    And his dark secret love,\n    Does thy life destroy";
    tft.print(txt);

}
//-------------------------------------------------------------------------------------
void loop(void) {
        delay(2000);
}


