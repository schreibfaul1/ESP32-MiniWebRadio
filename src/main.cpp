/**************************CrowPanel ESP32 HMI Display Example Code************************
Version     :	1.1
Suitable for:	CrowPanel ESP32 HMI Display
Product link:	https://www.elecrow.com/esp32-display-series-hmi-touch-screen.html
Code	  link:	https://github.com/Elecrow-RD/CrowPanel-ESP32-Display-Course-File
Lesson	link:	https://www.youtube.com/watch?v=WHfPH-Kr9XU
Description	:	The code is currently available based on the course on YouTube, 
				        if you have any questions, please refer to the course video: Introduction 
				        to ask questions or feedback.
**************************************************************/




#include <Wire.h>
#include <SPI.h>


/*******************************************************************************
   Config the display panel and touch panel in gfx_conf.h
 ******************************************************************************/
#include "gfx_conf.h"


void setup()
{
  Serial.begin(9600);

  //Display Prepare
  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(7);
  delay(100);

  tft.fillScreen(TFT_BLUE);
  delay(1000);
  tft.fillScreen(TFT_YELLOW);
  delay(1000);
  tft.fillScreen(TFT_GREEN);
  delay(1000);
  tft.fillScreen(TFT_WHITE);
  delay(1000);
  tft.fillScreen(TFT_BLACK);
  tft.fillCircle    ( 100, 100      , 50, TFT_YELLOW);
  tft.setCursor(200, 240);
  tft.print("Hello, Elecrow");
  Serial.println( "Hello, my Display" );

}

void loop()
{
    delay(5);
}