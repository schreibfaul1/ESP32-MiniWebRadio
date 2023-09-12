/*
  DownloadFileExample1_WiFi

  This sketch downloads one file from a media server and writes it
  to SD card. 
	
  The parameters needed for download must be set manually further down.
  You find a snapshot in the doc directory, showing you how to use VLC 
  to find proper values.

  SD card module/shield is attached to GPIO 18, 19, 23 and GPIO 5 (CS).
    
  Last updated 2022-01-22, ThJ <yellobyte@bluewin.ch>
*/

#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>
#include "SoapESP32.h"

// uncomment in case you want to know
//#define SHOW_ESP32_MEMORY_STATISTICS

// example settings only, please change:
#define FILE_DOWNLOAD_IP   192,168,1,42
#define FILE_DOWNLOAD_PORT 8895 
#define FILE_DOWNLOAD_URI  "resource/227/MEDIA_ITEM/MP3-0/ORIGINAL"

const char ssid[] = "MySSID";
const char pass[] = "MyPassword"; 

// File download settings
#define FILE_NAME_ON_SD    "/myFile.mp3"
#define READ_BUFFER_SIZE   5000
// set a lower speed and uncomment in case you experience SD card write errors
//#define SPI_SPEED_SDCARD 2000000U     // SD library default is 4MHz
#define GPIO_SDCS   5

WiFiClient client;
WiFiUDP    udp;

SoapESP32 soap(&client, &udp);

void setup() {
  Serial.begin(115200);

  // connect to local network via WiFi
  Serial.println();
  Serial.print("Connecting to WiFi network ");
  WiFi.begin(ssid, pass);
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected successfully. IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // preparing SD card 
  Serial.print("Initializing SD card...");
#ifdef SPI_SPEED_SDCARD  
  if (!SD.begin(GPIO_SDCS, SPI, SPI_SPEED_SDCARD)) {
#else
  if (!SD.begin(GPIO_SDCS)) {
#endif
    Serial.println("failed!");
    Serial.println("Sketch finished.");
    return;
  }
  Serial.print("done. Creating file on SD ");  
  File myFile = SD.open(FILE_NAME_ON_SD, FILE_WRITE);
  if (!myFile) {
    Serial.println("failed!");
    return;
  }
  Serial.println("was successful."); 

  // memory allocation for read buffer
  uint8_t *buffer = (uint8_t *)malloc(READ_BUFFER_SIZE);
  if (!buffer) {
    Serial.println("malloc() error!");    
    return;
  }

  size_t fileSize;          // file size announced by server
  uint32_t bytesRead;       // read count
  soapObject_t object;      // holds all necessary infos for download

  object.isDirectory  = false;
  object.downloadIp   = IPAddress(FILE_DOWNLOAD_IP);
  object.downloadPort = FILE_DOWNLOAD_PORT;
  object.uri          = FILE_DOWNLOAD_URI;

  // attempting to download a file bigger than 4.2GB will fail !
  if (!soap.readStart(&object, &fileSize)) {
    // Error
    Serial.println("Error requesting file from media server.");
  }
  else {
    // request for file download was granted from server
    Serial.print("Download request granted from server, announced file size: ");
    Serial.println(fileSize);
    Serial.println("Start copying file from server to SD, please wait."); 

    bytesRead = 0;
    do {
      int32_t res = soap.read(buffer, READ_BUFFER_SIZE);
      if (res < 0) {
        // read error  
        break;
      }         
      else if (res > 0) {
        // Remark: At this point instead of writing to SD card you 
        // could write the data into a buffer/queue which feeds an 
        // audio codec (e.g. VS1053) for example
        if (!myFile.write(buffer, res)) {
          Serial.println("Error writing to SD card."); 
          break;
        }
        //
        bytesRead += res;
        Serial.print(".");
      }  
      else { 
        // res == 0, momentarily no data available
      }
    } 
    while (soap.available());
    Serial.println();

    // close connection to server
    soap.readStop();

    Serial.println();
    if (bytesRead == fileSize) {
      Serial.println("File download was successful.");     
    }
    else {
      Serial.println("Error downloading file.");
    }
  }

  free(buffer);
  myFile.close();
  Serial.println("File on SD closed.");

#ifdef SHOW_ESP32_MEMORY_STATISTICS
  Serial.println();
  Serial.println("Some ESP32 memory stats after running this sketch:");
  Serial.print(" 1) minimum ever free memory of all regions [in bytes]: ");
  Serial.println(ESP.getMinFreeHeap());
  Serial.print(" 2) minimum ever free heap size [in bytes]:             ");
  Serial.println(xPortGetMinimumEverFreeHeapSize());
  Serial.print(" 3) minimum ever stack size of this task [in bytes]:    ");
  Serial.println(uxTaskGetStackHighWaterMark(NULL)); 
#endif

  Serial.println();
  Serial.println("Sketch finished.");
}

void loop() {
  // 
}
