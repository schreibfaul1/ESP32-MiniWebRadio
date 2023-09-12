/*
  BrowseRoot_WiFi

  This sketch scans the local network for DLNA media servers via builtin WiFi
  and browses root of each server found.

  Last updated 2022-01-14, ThJ <yellobyte@bluewin.ch>
*/

#include <Arduino.h>
#include <WiFi.h>
#include "SoapESP32.h"

// uncomment in case you want to know
//#define SHOW_ESP32_MEMORY_STATISTICS

const char ssid[] = "MySSID";
const char pass[] = "MyPassword"; 

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

  // scan local network for DLNA media servers
  Serial.println();
  Serial.println("Scanning local network for DLNA media servers...");
  soap.seekServer();
  Serial.print("Number of discovered servers that deliver content: ");
  Serial.println(soap.getServerCount());
  Serial.println();

  // Show root content of all discovered, usable media servers
  soapObjectVect_t browseResult;      // browse results get stored here
  soapServer_t serv;                  // single server info gets stored here
  int32_t i = 0;                          // start with first entry in server list

  while (soap.getServerInfo(i, &serv)) {
    // Print some server details
    Serial.print("Server[");
    Serial.print(i);
    Serial.print("]: IP address: ");
    Serial.print(serv.ip);
    Serial.print(", port: ");
    Serial.print(serv.port);
    Serial.print(", name: ");
    Serial.println(serv.friendlyName);

    // browse root (always represented by "0" according to SOAP spec)
    if (!soap.browseServer(i, "0", &browseResult)) {
      Serial.println("error browsing server.");
    }
    else {
      Serial.print("Browsing root directory. Number of sub-directories: ");
      Serial.println(browseResult.size());

      // show each directory in root
      for (int32_t j = 0; j < browseResult.size(); j++) {
        Serial.print(" ");
        Serial.print(browseResult[j].name);
        if (browseResult[j].isDirectory) {
          Serial.print(" (child count: ");
        }
        else {
          // root shouldn't host files so it's unlikely we get here
          Serial.print(" (Item, Size: ");
        }
        if (browseResult[j].sizeMissing) {
          Serial.print("missing");
        }
        else {
          Serial.print(browseResult[j].size);
        }
        Serial.println(")");
      }
    }
    Serial.println("");
    i++;
  }

#ifdef SHOW_ESP32_MEMORY_STATISTICS
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
