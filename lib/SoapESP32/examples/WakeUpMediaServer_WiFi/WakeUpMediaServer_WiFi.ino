/*
  WakeUpMediaServer_WiFi

  This sketch shows how to use the soapESP32 class for broadcasting 
  a WOL (Wake On LAN) message in the local network. Necessary when you 
  have to wake up the NAS that is hosting your DLNA media server.

  Last updated 2021-02-02, ThJ <yellobyte@bluewin.ch>
*/

#include <Arduino.h>
#include <WiFi.h>
#include "SoapESP32.h"

// uncomment in case you want to know
//#define SHOW_ESP32_MEMORY_STATISTICS

// example settings only, please change:
#define WAKE_UP_MAC  "10:6F:3F:21:EE:23"   // MAC of device you want to wake up
#define WAKE_UP_TIME 120                   // time in seconds the device needs to wake up fully

const char ssid[] = "MySSID";
const char pass[] = "MyPassword"; 

WiFiClient client;
WiFiUDP    udp;

SoapESP32 soap(&client, &udp);

// scan local network for media servers and print them
void showServer(SoapESP32 *soap)
{
  soapServer_t srv;
  uint8_t srvNum = 0;

  // scan local network for DLNA media servers,
  // your server (if sleeping) shouldn't be detected at this stage !
  Serial.println();
  Serial.println("Scanning local network for DLNA media servers...");
  soap->seekServer();
  Serial.print("Number of discovered servers that deliver content: ");
  Serial.println(soap->getServerCount());
  Serial.println();
  
  // show connection details of all discovered, usable media servers
  while (soap->getServerInfo(srvNum++, &srv)) {
    // print server details
    Serial.print("Server[");
    Serial.print(srvNum);
    Serial.print("]: IP address: ");
    Serial.print(srv.ip);
    Serial.print(", port: ");
    Serial.print(srv.port);
    Serial.print(", name: ");
    Serial.println(srv.friendlyName);
    Serial.print("  -> controlURL: ");
    Serial.println(srv.controlURL);
  }
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
  Serial.println("Connected successfully.");
  Serial.print("IP address: "); 
  Serial.println(WiFi.localIP());

  // scan for and print media servers 
  showServer(&soap);

  // send WOL message to target device
  Serial.println();
  Serial.print("Send WOL message to device with MAC: ");
  Serial.println(WAKE_UP_MAC);
  soap.wakeUpServer(WAKE_UP_MAC);
  Serial.println("Now waiting a few seconds before scanning local network again.");

  // give target device some time to wake up
  for (int sec = 0; sec < WAKE_UP_TIME; sec += 2) {
    sleep(2);
    Serial.print('.');
  }

  // scan for and print media servers  
  showServer(&soap);

#ifdef SHOW_ESP32_MEMORY_STATISTICS
  Serial.println("");
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
