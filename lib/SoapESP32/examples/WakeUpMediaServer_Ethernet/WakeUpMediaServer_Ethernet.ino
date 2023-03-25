/*
  WakeUpMediaServer_Ethernet

  This sketch shows how to use the soapESP32 class for broadcasting
  a WOL (Wake On LAN) message in the local network. Necessary when you 
  have to wake up the NAS that is hosting your DLNA media server. 
	
  We use a Wiznet W5x00 Ethernet module/shield instead of builtin WiFi.
  It's connected to ESP32 GPIO 18, 19, 23 and GPIO 25 (Chip Select).

  Last updated 2022-01-23, ThJ <yellobyte@bluewin.ch>
*/

#include <Arduino.h>
#include <Ethernet.h>
#include "SoapESP32.h"

// == IMPORTANT ==
// We use Ethernet module/shield instead of builtin WiFi, so you must do one of the following:
// 1) uncomment line "//#define USE_ETHERNET" in SoapESP32.h OR
// 2) add -DUSE_ETHERNET to compiler.cpreprocessor.flags in platform.txt (ArduinoIDE) OR
// 3) add -DUSE_ETHERNET to your build_flags in platformio.ini (VSCode/PlatformIO)

// uncomment in case you want to know
//#define SHOW_ESP32_MEMORY_STATISTICS

// example WOL settings only, please change:
#define WAKE_UP_MAC  "10:6F:3F:21:EE:23"   // MAC of device you want to wake up
#define WAKE_UP_TIME 120                   // time in seconds the device needs to wake up fully

// Ethernet module/shield settings
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
#define GPIO_ETHCS 25

EthernetClient client;
EthernetUDP    udp;

SoapESP32 soap(&client, &udp);

// scan local network for media servers and print them
void showServer(SoapESP32 *soap)
{
  soapServer_t srv;
  uint8_t srvNum = 0;

  // scan local network for DLNA media servers
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

  Ethernet.init(GPIO_ETHCS);
  Serial.print("\nInitializing Ethernet...");

  if (Ethernet.begin(mac)) {
    Serial.println("DHCP ok.");
  }
  else {
    Serial.println("DHCP error !");
    while (true) {
      // no point to continue
    }
  }

  Serial.print("Local IP: ");
  Serial.println(Ethernet.localIP());

  // scan for and print media servers,
  // your server (if sleeping) shouldn't be detected at this stage !
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

  // again scan for and print media servers  
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
