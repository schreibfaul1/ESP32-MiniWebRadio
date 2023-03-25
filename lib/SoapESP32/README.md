# SoapESP32

This Arduino library provides basic UPnP/SOAP functionality, enabling an ESP32 device to scan the local network for DLNA media servers, browse their content and finally download files.

Motivation for writing it was the missing capability of an existing ESP32-Radio (based on Ed Smallenburg's code) to play audio content stored on the various NAS devices in the local network at home (most of them running a DLNA server).

The library has been successfully tested so far with the following DLNA media servers: **DiXim**, **Twonky**, **UMS** (Universal Media Server), **Jellyfin**, **Emby**, **Kodi**, **Plex**, **Serviio**, **Subsonic**, **MinimServer**, **QNAP-DLNA**, **Mezzmo** and **Windows Media Player**.

Integrating this library into your ESP32 Arduino projects is easy. For detailed infos have a look at the many [examples](https://github.com/yellobyte/soapESP32/blob/main/examples) included. Below the basics for searching and printing media servers in your local network. 
```c
...
#include "SoapESP32.h"

WiFiClient client;
WiFiUDP    udp;

SoapESP32    soap(&client, &udp);
soapServer_t srv;
uint8_t      srvNum = 0;

setup() {
  // setting up Wifi, serial output, etc.
  ...
  // searching local network for media servers
  soap.seekServer();
  // printing details of all discovered servers
  while (soap.getServerInfo(srvNum++, &srv)) {
    Serial.print("Server name: ");
    Serial.println(srv.friendlyName);
    ...
  }
}
...
```

## :zap: Application notes

To install the library into your **IDE** open the **Library Manager**, search for **SoapESP32** and choose **install**, **Add to Project** or similar.  

Always make sure you have one of the latest versions of **Arduino core for ESP32** installed. Older versions might produce build errors with some examples.

Most DLNA media servers I tested the library with showed some oddities. All compatibility issues I ran across have been fixed. Please note the following:

- Streams/podcasts: Some media servers (e.g. Fritzbox, Serviio) do **not** provide a size for items (media content) located in their Web/Online/InternetRadio folders. Thanks to Github user KiloOscarRomeo for drawing my attention to this fact. In contrast, UMS (Universal Media Server) always provides a fixed size of 9223372034707292159 (0x7FFFFFFF7FFFFFFF) for items in directory Web (incl. subdirectories Radio, Podcasts, etc.). 

- Empty files: Some media servers (MS MediaPlayer/Kodi/Jellyfin) show empty files, others don't. This library by default ignores files with reported size zero. They will not show up in browse results. You can change this behaviour with build option `SHOW_EMPTY_FILES`.

- Empty directories: They always show up in browse results.

- Missing attribute size: Media servers often show items without telling their size. That applies to all item types: streams, video files, audio files, image files, etc. In this case the library will return them in browse results with *size=0* and *sizeMissing=true*. 

- Missing attributes "size" (means child count in case of directories) & "searchable" (for directories): Some servers report containers (directories) with size (child count) = 0 or even missing attribute size when in fact they are not empty. Same applies to attribute searchable. This is very annoying for it forces you to dig into each (sub)directory not to overlook anything.

- Parent-ID Mismatch: The id of a directory and the parent id of it's content should match. Sometimes it does not (e.g. Subsonic). As of V1.1.1 this mismatch is ignored by default. You can go back to strict behaviour with build option `PARENT_ID_MUST_MATCH`

- Downloading big files/reading streams: Files with reported size bigger than 4.2GB (SIZE_MAX) will be shown in browse results but an attempt to download them with *readStart()/read()/readEnd()* will fail. If you want to download them or read endless streams you will have to do it outside this library in your own code.

- IP & port for file download can be different from the media server's IP & port! So always evaluate *downloadIp* & *downloadPort* in media server objects returned by *browseServer()* when a download is intended.
	
If you run into trouble with your particular DLNA media server or NAS, increase `CORE_DEBUG_LEVEL` and it gives you an indication where the problem is. Tracing the communication with Wireshark can help as well.

### :heavy_exclamation_mark: Using W5x00 Ethernet shield/boards instead of builtin WiFi (optional)

Using a Wiznet W5x00 board and the standard Arduino Ethernet lib for communication produced some sporadic issues. Especially client.read() calls returned corrupted data every now and then, esp. with other threads using the SPI bus simultaneously.
This problem is mentioned a few times in forums on the internet, so it seems to be a known ESP32 Arduino SPI library issue.

The only remedy I found was to wrap all function calls that use SPI with a global/project wide mutex lock (realized within this library with the aid of *claimSPI()/releaseSPI()*). This completely wiped all those problems. See example [*UsingMutexLocks_Ethernet.ino*](https://github.com/yellobyte/SoapESP32/tree/main/examples/UsingMutexLocks_Ethernet/UsingMutexLocks_Ethernet.ino) for more details.

Of course, the ESP32 Arduino SPI library already uses locks (SPI_MUTEX_LOCK/SPI_MUTEX_UNLOCK) but doesn't seem to be 100% thread proof though. Please correct me if I'm wrong or you do find a better solution.

####  Ethernet SSDP multicast issue:

SSDP M-SEARCH multicast packets carry the destination ip 239.255.255.250 and port 1900. However, some NAS devices do **not** reply to such packets when the **source** port is 1900 as well. Unfortunately when using the standard Arduino Ethernet library, all SSDP multicast packets carry identical destination & source ports, in our case 1900. There are two solutions to this problem:

a) use *addServer()* to manually add a server to the server list, which I recommend **or**

b) modify file *socket.cpp* in Arduino Ethernet library to use a source port between 49152 & 65535. Which is not only a dirty solution, it puts you under risk to forget about it and then lose those changes with a later Ethernet lib update.  

```c
uint8_t EthernetClass::socketBeginMulticast(uint8_t protocol, IPAddress ip, uint16_t port)
{
  ...
  if (port > 0 && (protocol != (SnMR::UDP | SnMR::MULTI))) {	 // <------ modification 
    W5100.writeSnPORT(s, port);
  } else {
    // if don't set the source port, set local_port number.
    if (++local_port < 49152) local_port = 49152;
    W5100.writeSnPORT(s, local_port);
  }
  ...
```

### :hammer_and_wrench: Setting compiler/build options:

All examples were build & tested with various versions of ArduinoIDE and VSCode/PlatformIO.

If possible, always set project wide preprocessor option `__GNU_VISIBLE` which enables usage of strcasestr() provided in _string.h_. However, if `__GNU_VISIBLE` is not defined, a quick and dirty version of strcasestr() defined in _SoapESP32.cpp_ will be used instead.

If you use an Ethernet module/shield instead of builtin WiFi you must set the preprocessor option `USE_ETHERNET`. Otherwise the build will fail.

#### Building with Arduino IDE:

Unfortunately we can't set project wide build options in *.ino sketches. So the easiest way is to uncomment the line **//#define USE_ETHERNET** in _SoapESP32.h_. Alternatively you could add any needed build options to line _compiler.cpreprocessor.flags_ in your Arduino IDE file _platform.txt_.  On my PC for example I find this file in directory:  	
* _C:\Users\tj\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.4_

Be reminded, those options will stay permanent until you delete them!  

#### Building with VSCode/PlatformIO:

Simply add needed build options to your _platformio.ini_ project file, e.g.:  build_flags = `-D__GNU_VISIBLE, -DUSE_ETHERNET`
	
## :mag: How to find correct server parameters needed in some examples

Four examples (_BrowseRecursively_Ethernet.ino_, _BrowseRecursively_WiFi.ino_, _DownloadFileExample1_Ethernet.ino_ and _DownloadFileExample1_WiFi.ino_) require some parameters (that apply to your specific DLNA media server) be defined manually. The three provided snapshots (_Using_VLC_to_find_....._parameter.JPG_) in folder [**Doc**](https://github.com/yellobyte/soapESP32/blob/main/doc) show you how to use the open source **VLC** media player to find the right values.

## :file_folder: Documentation

Folder [**Doc**](https://github.com/yellobyte/soapESP32/blob/main/doc) contains a big collection of files to help you implement this library into your projects:
* The platformio.ini file I used when testing examples with VSCode/PlatformIO
* Build log
* Plenty of log files (serial monitor output) of all examples, mostly with different core debug levels
* Schematic diagram (wiring) plus picture of the test set used
* VLC snapshots to help you find the right server parameters needed for file download examples

## :tada: Example of an implementation: ESP32-Radio utilizing SoapESP32

An example of a real world implementation can be found here: [ESP32-Radio project](https://github.com/yellobyte/ESP32-Webradio-PlusDLNA). No fiddling around with SD cards anymore. Using the rotary switch encoder is all it needs to browse through the content of a DLNA media server in the local network and select an audio file. After selecting the file, the radio simply reads the data from the server and writes it into the queue which feeds the audio codec VS1053B.  

![github](https://github.com/yellobyte/SoapESP32/raw/main/doc/ESP32-Radio-DLNA.jpg)

Alternatively have a look at the short clip _ESP32-Radio-DLNA.mp4_ in folder **Doc** to see the final implementation in action. To watch now, click [Here](https://github.com/yellobyte/SoapESP32/raw/main/doc/ESP32-Radio-DLNA.mp4)

## :relaxed: Postscript

If you run into trouble with your mediaserver or have suggestions how to improve the lib, feel free to contact me.  

