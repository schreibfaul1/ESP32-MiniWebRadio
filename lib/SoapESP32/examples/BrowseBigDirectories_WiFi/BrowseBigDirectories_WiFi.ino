/*
  BrowseBigDirectories_WiFi

  This sketch demonstrates how to browse directories with a large number of
  subdirectories/files by changing the starting index. We use builtin WiFi. 

  Since memory is limited, by default a maximum of only 100 entries per
  directory will be returned by browseServer(). This limit is defined in 
  "SoapESP32.h" with parameter SOAP_DEFAULT_BROWSE_MAX_COUNT. Increasing
  this parameter means using more memory.	

  If a directory contains more entries than that number, you have to browse
  that directory multiple times, each time with a higher starting index
  (0, 100, 200,...). This sketch demonstrates how to do it.
    
  Last updated 2022-01-15, ThJ <yellobyte@bluewin.ch>
*/

#include <Arduino.h>
#include <WiFi.h>
#include "SoapESP32.h"

// uncomment in case you want to know
//#define SHOW_ESP32_MEMORY_STATISTICS

// How many directory levels to browse (incl. root) at maximum. The higher this
// value and the bigger your server content the higher the memory usage!
#define BROWSE_LEVELS 3

const char ssid[] = "MySSID";
const char pass[] = "MyPassword"; 

WiFiClient client;
WiFiUDP    udp;

SoapESP32 soap(&client, &udp);

bool findBigDirectory(SoapESP32 *soap, int32_t servNum, soapObject_t *object) {
  static int32_t level;  
  soapObjectVect_t browseResult;

  if (object->id == "0") level = 0;            // root resets level counter
  if (!soap->browseServer(servNum,             // server number in list
                          object->id.c_str(),  // unique id of directory to search
                          &browseResult)) {    // pointer to vector storing directory content
    Serial.print("Error browsing server, object id: ");
    Serial.println(object->id);
    return false;
  }
  else {
    // check this level first
    for (int32_t i = 0; i < browseResult.size(); i++) {
      // go through each item in list, break if directory with more 
      // than SOAP_DEFAULT_BROWSE_MAX_COUNT items (subdirs and/or files) is found
      if (browseResult[i].isDirectory &&
          browseResult[i].size > SOAP_DEFAULT_BROWSE_MAX_COUNT) {
        // found a big directory
        *object = browseResult[i];
        Serial.print("Big directory found, name: \"");
        Serial.print(object->name);
        Serial.print("\", id: ");
        Serial.print(object->id);
        Serial.print(", size: ");
        Serial.println(object->size);
        return true;
      }
    }
    // we need to dig deeper
    for (int32_t i = 0; i < browseResult.size(); i++) {
      if (browseResult[i].isDirectory && (level + 1) < BROWSE_LEVELS) { 
        // recurse
        *object = browseResult[i];
        level++;
        if (findBigDirectory(soap, servNum, object)) {
          return true;
        }
        level--;
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

  // scan local network for DLNA media servers
  Serial.println("Scanning local network for DLNA media servers...");
  soap.seekServer();
  Serial.print("Number of discovered servers that deliver content: ");
  Serial.println(soap.getServerCount());
  Serial.println();

  // start searching all servers for a big directory
  soapObject_t     directory;
  soapObjectVect_t directoryContent;
  soapServer_t     srvInfo;
  int32_t srvNum = 0,             // start with first server in list
      startingIndex;

  while (soap.getServerInfo(srvNum, &srvInfo)) {
    // Scan each server
    Serial.print("Please be patient, searching big directory on server: ");
    Serial.println(srvInfo.friendlyName);
    
    startingIndex = 0;        // start browsing a directory with offset 0
    directory.id = "0";       // start with root ("0")
    directory.name = "root";  // only needed for printing in case of error
    
    if (findBigDirectory(&soap, srvNum, &directory)) {
      // found big directory, now print entire content 
      while (startingIndex < directory.size) {
        Serial.print("------> Browse directory with starting index: ");
        Serial.println(startingIndex);
        // browse directory with increasing starting index
        if (!soap.browseServer(srvNum, directory.id.c_str(), 
                               &directoryContent, startingIndex) ||
            directoryContent.size() == 0) {
          // function returned error or directory is empty    
          Serial.print("Error browsing directory with name: ");
          Serial.println(directory.name);
          break;
        }
        else {
          // show all entries in list
          for (int32_t i = 0; i < directoryContent.size(); i++) {
            // print object count
            Serial.print(startingIndex + i);
            Serial.print(": ");
            // print name of object
            Serial.print(directoryContent[i].name);
            if (directoryContent[i].isDirectory ) {
              // directory: append '/' to name
              Serial.println("/");
            } 
            else {
              // item: append item type and size
              Serial.print("   ");
              Serial.print("item size: ");
              if (directoryContent[i].sizeMissing) {
                Serial.print("missing");
              }
              else {
                Serial.print(directoryContent[i].size, DEC);
              }
              Serial.print(", ");
              Serial.println(soap.getFileTypeName(directoryContent[i].fileType));
            }
          }
          // increase index to get next chunk
          startingIndex += directoryContent.size();
        }  
      }
      break;
    }
    else {
      Serial.println("No big directory was found on this server.");
    }
    // try next server in list
    srvNum++;
  }

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
