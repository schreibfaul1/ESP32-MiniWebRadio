/*
  DownloadFileExample2_WiFi

  This sketch scans the local network for media servers and if found, 
  browses them for a small audio file. If found, the file is copied
  to SD card. 

  SD card module/shield is attached to GPIO 18, 19, 23 and GPIO 5 (CS).
    
  Last updated 2022-01-20, ThJ <yellobyte@bluewin.ch>
*/

#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>
#include "SoapESP32.h"

// uncomment in case you want to know
//#define SHOW_ESP32_MEMORY_STATISTICS

// set IP and uncomment if you want to browse only a single system
//#define THIS_IP_ONLY 192,168,1,42

// How many sub-directory levels to browse (incl. root) at maximum in 
// search for a file. 
#define BROWSE_LEVELS 3

// File download settings
#define FILE_MAX_SIZE    8000000
#define FILE_MIN_SIZE    500000         // set to 0 in case your server doesn't provide a size
#define FILE_NAME_ON_SD  "/myFile.mp3"
#define READ_BUFFER_SIZE 5000
// set a lower speed and uncomment in case you experience SD card write errors
//#define SPI_SPEED_SDCARD 2000000U     // SD library default is 4MHz
#define GPIO_SDCS   5

const char ssid[] = "MySSID";
const char pass[] = "MyPassword"; 

WiFiClient client;
WiFiUDP    udp;

SoapESP32 soap(&client, &udp);
File myFile;

// browse a server recursively until an audio file is found.
// parameter "object": 
//  - when entering the function it contains the directory to browse
//  - when function returns true it contains the file info
bool findAudioFile(SoapESP32 *soap, int servNum, soapObject_t *object) {
  static int level;  
  soapObjectVect_t browseResult;

  if (object->id == "0") level = 0;            // root resets level marker
  if (!soap->browseServer(servNum,             // server number in list
                          object->id.c_str(),  // id of directory to browse
                          &browseResult)) {    // pointer to vector storing directory content
    Serial.print("Error browsing server, object id: ");
    Serial.println(object->id);
    return false;
  }
  else {
    for (int i = 0; i < browseResult.size(); i++) {
      // go through each object in list and recurse for directories or
      // break if we find an audio file
      if (browseResult[i].isDirectory ) {
        if ((level + 1) < BROWSE_LEVELS) { 
          // recurse
          *object = browseResult[i];
          level++;
          if (findAudioFile(soap, servNum, object)) {
            return true;
          }
          level--;
        }  
      } 
      else {
        if (browseResult[i].fileType == fileTypeAudio &&
            browseResult[i].size <= FILE_MAX_SIZE &&
            browseResult[i].size >= FILE_MIN_SIZE) {
          // object is an audio file with a size between 0.5-8MB
          *object = browseResult[i];
          Serial.println("Audio file was found:");
          Serial.print(" name: \"");
          Serial.print(object->name);
          Serial.print("\", id: ");
          Serial.print(object->id);
          Serial.print(", size: ");
          Serial.println(object->size);
          Serial.print(" URI: ");
          Serial.println(object->uri);
          return true;
        }
      }
    }
  }
  return false;
}

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
  Serial.print("done. Opening file on SD ");  
  myFile = SD.open(FILE_NAME_ON_SD, FILE_WRITE);
  if (!myFile) {
    Serial.println("failed!");
    return;
  }
  Serial.println("was successful."); 

  // scan local network for DLNA media servers
  Serial.println("Scanning local network for DLNA media servers...");
  soap.seekServer();
  Serial.print("Number of discovered servers that deliver content: ");
  Serial.println(soap.getServerCount());
  Serial.println();

  // now browse all servers for an audio file & if we find one, copy it to SD
  soapServer_t srvInfo;       // single server info
  uint8_t srvNum;             // server number in list

  for (srvNum = 0; soap.getServerInfo(srvNum, &srvInfo); srvNum++) {
#ifdef THIS_IP_ONLY
    if (srvInfo.ip != IPAddress(THIS_IP_ONLY)) continue;
#endif
    Serial.print("Please be patient, searching audio file on server: ");
    Serial.println(srvInfo.friendlyName);

    // browse server beginning with root ("0")
    size_t fileSize;          // will store size of file
    soapObject_t object;

    object.id = "0";          // we start with root
    object.name = "root";     // only needed for printing in case of error
    
    if (findAudioFile(&soap, srvNum, &object)) {

      // file has been found
      uint32_t bytesRead = 0;     // read count
      uint8_t *buffer = (uint8_t *)malloc(READ_BUFFER_SIZE);

      if (!buffer) {
        Serial.println("malloc() error!");    
        break;
      }

      // request file download from media server
      if (!soap.readStart(&object, &fileSize)) {
        // request rejected or communication error
        Serial.println("Error requesting file from media server.");
      }
      else {
        // request was granted
        Serial.print("Download request granted from server, announced file size: ");
        Serial.println(fileSize);
        Serial.println("Start copying file from server to SD, please wait."); 

        do {
          int res = soap.read(buffer, READ_BUFFER_SIZE);
          if (res < 0) {
            // read error or timeout 
            Serial.println("Error reading from media server."); 
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
      if (buffer) free(buffer);
      break;
    }
  }

  Serial.println("Closing file on SD.");
  myFile.close();

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

