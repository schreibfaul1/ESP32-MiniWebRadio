/*
  ScanForMediaServers_WiFi

  This sketch scans the local network for DLNA media servers
  using ESP32 builtin WiFi and prints them.

  Last updated 2023-01-25, ThJ <yellobyte@bluewin.ch>
 */

#include <Arduino.h>
#include <WiFi.h>
#include "SoapESP32.h"

const char ssid[] = "MySSID";
const char pass[] = "MyPassword"; 

WiFiClient client;
WiFiUDP    udp;

SoapESP32 soap(&client, &udp);

void setup() {
  Serial.begin(115200);
  delay(10);

  // Connect to local network via WiFi
  Serial.println();
  Serial.print("Connecting to WiFi network ");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected successfully.");
  Serial.print("IP address: "); 
  Serial.println(WiFi.localIP());

  // scan local network for DLNA media servers
  Serial.println();
  Serial.println("Scanning local network for DLNA media servers...");
  soap.seekServer();
  Serial.print("Number of discovered servers that deliver content: ");
  Serial.println(soap.getServerCount());
  Serial.println();

  // Show connection details of all discovered, usable media servers
  soapServer_t srv;
  uint8_t srvNum = 0;

  while (soap.getServerInfo(srvNum++, &srv)) {
    // Print server details
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

  Serial.println();
  Serial.println("Sketch finished.");
}

void loop() {
  // 
}
