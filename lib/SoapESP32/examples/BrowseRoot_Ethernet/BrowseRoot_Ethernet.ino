/*
  BrowseRoot_Ethernet

  This sketch scans the local network for DLNA media servers and browses root
  of each server found.

  We use a Wiznet W5x00 Ethernet module/shield instead of builtin WiFi. 
  It's attached to GPIO 18, 19, 23 and GPIO 25 (Chip Select).

  Last updated 2022-01-23, ThJ <yellobyte@bluewin.ch>
 */

#include <Arduino.h>
#include <Ethernet.h>
#include "SoapESP32.h"

// == IMPORTANT ==
// We use Ethernet module/shield instead of WiFi, so you must do one of the following:
// 1) uncomment line "//#define USE_ETHERNET" in SoapESP32.h OR
// 2) add -DUSE_ETHERNET to compiler.cpreprocessor.flags in platform.txt (ArduinoIDE) OR
// 3) add -DUSE_ETHERNET to your build_flags in platformio.ini (VSCode/PlatformIO)

// uncomment in case you want to know
//#define SHOW_ESP32_MEMORY_STATISTICS

// Ethernet module/shield settings
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
#define GPIO_ETHCS 25

EthernetClient client;
EthernetUDP    udp;

SoapESP32 soap(&client, &udp);

void setup() {
  Serial.begin(115200);

  Ethernet.init(GPIO_ETHCS);
  Serial.print("\nInitializing Ethernet...");

  if (Ethernet.begin(mac))
  {
    Serial.println("DHCP ok.");
  }
  else
  {
    Serial.println("DHCP error !");
    while (true) {
      // no point to continue
    }
  }

  Serial.print("Local IP: ");
  Serial.println(Ethernet.localIP());

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
