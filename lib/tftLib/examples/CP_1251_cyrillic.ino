/*
 * Example for codepage 1251 cyrillic
 *
 *
 * uncomment #include "fonts/Garamond_cyrillic.h" in tft.h
 */



#include "Arduino.h"
#include "SPI.h"
#include "tft.h"

#define TFT_DC   21    /* Data or Command, do not use GPI032 or GPIO33 here*/
#define TFT_CS   22    /* SPI Chip select, do not use GPI032 or GPIO33 here*/
#define TFT_BL   17    /* BackLight */
#define SPI_SCK  18
#define SPI_MISO 19
#define SPI_MOSI 23
#define TFT_RST  16    /* Reset */
#define TP_IRQ   39
#define TP_CS    16
#define SD_CS     5


TFT tft(0);  //0=ILI9341, 1= HX8347D

void setup() {
    String txt="";
    pinMode(TP_CS, OUTPUT);
    digitalWrite(TP_CS, HIGH); // disable the touchpad
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH); // disable SDcard
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    tft.begin(TFT_CS, TFT_DC, SPI_MOSI, SPI_MISO, SPI_SCK);
    tft.fillScreen(TFT_BLACK);
    tft.setRotation(1);
    tft.setCursor(0, 0);
    tft.setFont(Garamond23x24cyrillic); // uncomment this font in tft,h
    tft.setTextColor(TFT_WHITE);
    txt="Парус – a poem by M.Lermontow\n";
    tft.println(txt);
    tft.setTextColor(TFT_YELLOW);
    txt= "Белеет парус одинокий\n";
    txt+="В тумане моря голубом!..\n";
    txt+="Что ищет он в стране далекой?,\n";
    txt+="Что кинул он в краю родном?..\n";
    tft.println(txt);
}
//-------------------------------------------------------------------------------------
void loop(void) {
        delay(2000);
}
