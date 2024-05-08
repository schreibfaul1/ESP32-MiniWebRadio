# ESP32-MiniWebRadio V3.1

![Display](docs/MiniWebRadio.jpg)

MiniWebRadio Features:
<ul>
<li>User interfaces: TFT touchscreen display, web browser and FTP</li>
<li>Functions: WiFi Radio, Digital Clock, MP3 player, Alarm, Sleep timer, adjust screen brightness, EQ settings and Volume, web browser User Interface, access SD card via FTP (e.g. FileZilla), IR remote controller support</li>
<li>Up to 999 pre-set stations can be held in stations.csv file on SD card (can edit using web UI)</li>
<li>Each station can display its own station icon (when saved to SD card)</li>
<li>Time is obtained via Network Time Protocol (NTP) from internet. Local Time Zone can be set from web UI</li>
<li>Web UI - MiniWebRadio can be accessed via any web browser (e.g. IE/Edge/Chrome/FireFox)</li>
<li>Internal SD card can be accessed via FTP (e.g. FileZilla)</li>
<li>Your home WiFi router SSID and password can be set using the browser on your smart phone (only required on first boot) or edit the networks.csv file on SD card</li>
<li>Play audio files on SD card or on DLNA home network (via web UI)</li>
<li>Use Infra-Red (IR) remote controller (38kHz NEC-encoded - e.g. arduino or mp3 remote). The web UI allows you to configure the buttons if required</li>
<li>One Alarm time can be pre-set using display (choose days, Monday-Sunday)</li>
<li>A Sleep timer can be set using display (switches off sound and screen after a pre-set time - max. 6 hours)</li>
<li>Can announce the time each hour when in radio mode (set via web UI)</li>
<li>Screen brightness can be adjusted using display (if the display has a backlight-pin)</li>
<li>Supports the Latin, Greek and Cyrillic character sets</li>
<li><a href="https://www.radio-browser.info/">Community Radio Browser</a> is integrated as a search engine. User can find new stations and then add them to the station list via web UI (with station icon if available) and then save the list and station icon file to the SD card</li>
<li>Channel lists can be exported or imported in Excel format (for data backup).</li>
</ul><br>
Required HW:
<ul>
<li>ESP32 or ESP32-S3 board <b>with PSRAM</b></li>
<li>External DAC (e.g. PCM5102a, CS4344, PT8211, AC101, ES8388, WM8978 ...)</li>
<li>TFT Display with Touchpad (SPI), Display controller can be ILI9341 (320x240px), ILI9486 (480x320px), ILI9488 (480x320px) or ST7796 (480x320px)</li>
<li>SD Card (FAT32) + SD adapter (can use SD slot on back of TFT display if available)</li>
</ul>
Optional HW:
<ul>
<li>IR receiver + IR remote controller according to the NFC protocol</li>
<li>KCX_BT_EMITTER V1.7, for connecting external Bluetooth devices in the sending or receiving direction, a connection with voice assistants such as ALEXA is possible</li>
</ul><br>

Control is via the display touchscreen or a web page in a browser, no additional components such as switches, rotary encoders, capacitors or resistors are required.

Schematic<br>
![Schematic with external DAC](docs/MiniWebRadioV3_schematic.jpg)<br>
<br>
[Display Layout](docs/MiniWebRadio%20V3%20Layout.pdf)

[How to install](docs/How%20to%20install.pdf) : PlatformIO is definitely recommended as the IDE.

#### Some features:

- The audioprocess works in its own task and is decoupled. If a VS1053 is used, it must have its own SPI bus (VS1053 uses HSPI - TFT uses VSPI). This prevents dropouts when drawing on the display or when the website is loading.
- The SD card is wired as SD_MMC to improve stability and increase speed. This means that the GPIOs cannot be chosen freely. The [SD card adapter](docs/SD_Card_Adapter_for_SD_MMC_.jpg) must not have any resistors in series. For best display update speed, use 40MHz frequency for SD card if possible (SDMMC_FREQUENCY 40000000 in common.h).
- Audio can be decoded using software and a DAC instead of VS1053 decoder board. Possible formats are mp3, aac, mp4 and flac (flac requires PSRAM). DAC (e.g. UDA13348, MAX98357A, PCM5102A) is connected via I2S. AC101, ES8388 and WM8978 (TTGO audioT board) audio decoder boards are also supported
- 480x320px display supported. The ILI9486 (SPI display from the Raspberry PI) is also supported
- The SD card files can be accessed via FTP. See settings for [Filezilla](docs/Filezilla.pdf). The username and password are 'esp32' (this can be changed in 'common.h')
- Access Point SSID/password can be set using mobile phone browser - no need to modify source code or networks.csv file on SD card
- Stations URLs support entry of username and password if the server expects access data, "URL|user|pwd"
- Can process local playlists in m3u format
- Either the ESP32 or the ESP32-S3 can be used (PSRAM is highly recommended)
- IR remote button codes can be changed by user using web UI
- Improved web UI reliability
- VU meter added to display
- Timezone can be set using web UI
- Play media files on home network DLNA (uPNP/DLNA app on smart phone, router, etc.)
- Prevent clicks when changing radio stations

<br>

|Codec|                          |
|-----|--------------------------|
| mp3 | y |
| aac | y |
| aacp (HLS) | mono |
| wav | y |
| flac | blocksize max 8192 bytes |
| vorbis | y (<=196Kbit/s)  |
| m4a | y |
| opus |  y (celt)  |

***
<br>

[self-made devices of the users](https://github.com/schreibfaul1/ESP32-MiniWebRadio/wiki/User-devices)<br>

***

## Known problems
### SD Card
In the simplest case, the SD card is connected directly to the ESP32
<br>
![SD Card Pinout](docs/SD_Card_Pinout.jpg)<br>
Some SD card adapters for displays use series resistors. These are useless and in many cases harmful. Therefore, it is better to remove them and replace them with solder bridges.<br>
![Display Resistors](docs/Display_resistors.jpg)<br>
If an ESP32 is used, any existing pull-up resistor at pin D0 must be removed (ESP32 - bootstrap pin). This will be added again later via SW. This is not necessary with the ESP32-S3.
(Photo from the <a href="https://forum.espuino.de/"> ESPuino </a>forum)![SD Card Adapter ESP32](docs/ESP32_SD_Card_PullUp.jpg)<br>

### Display
Many displays can be used without any problems. If the touchpad does not work, it may be that the TFT controller does not enable the SPI bus. This is the case with my ILI9488 display. Then MISO of the TFT controller must not be connected.<br>
![ILI9488 Display](docs/ILI9488_pins.jpg)<br>

### DAC
On some PCM5102 boards the solder bridges are missing on the back.<br>
![PCM5102A Board](docs/PCM5102A.png)<br>
This is how the DAC CS4344 is connected:<br>
![CS4344 Board](docs/DAC_CS434.jpg)<br>
If the DAC PT8211 is used, the *I2S_COMM_FMT* must be changed in common.h. This DAC requires Japanese LSBJ (Least Significant Bit Justified) format

### KCX_BT_EMITTER
The RT pin is not part of the soldering strip, but is located in the middle of the right side.<br>
![PCM5102A Board](docs/KCX_BT_EMITTER_pins.jpg)<br>

<br>
___________________________________________________________
<br>

![MWR](/docs/MWR.jpg)<br>
<br>



