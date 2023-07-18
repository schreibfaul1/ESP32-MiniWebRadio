# ESP32-MiniWebRadio V2

![Display](additional_info/MiniWebRadio.jpg)

Features:
<ul>
<li>Can handle max 999 stations</li>
<li>IR remote control is optional</li>
<li>Obtains time from NTP</li>
<li>Can used as alarmclock, has sleeptimer</li>
<li>Speech the time every hour in radiomode</li>
<li>If the display has a Backlight-pin You can change the brightness</li>
<li>supports the Latin, Greek and Cyrillic character sets</li>
<li><a href="https://www.radio-browser.info/">Community Radio Browser</a> is integrated as a search engine</li>
<li>Channel lists can be exported or imported in Excel format (for data backup).</li>
</ul><br>
Required HW:
<ul>
<li>Decoder module VS1053 or external DAC (e.g. PCM5102a)</li>
<li>TFT Display with Tochpad (SPI), Controller can be ILI9341 (320x240px), HX8347D (320x240px), ILI9486 (480x320px), ILI9488 (480x320px) or ST7796 (480x320px)</li>
<li>ESP32 or ESP32-S3 Board with PSRAM</li>
<li>SD Card + adapter</li>
</ul><br>

Control is exclusively via the touchscreen or the webpage, no additional components such as switches, rotary encoders, capacitors or resistors are required

Schematic with VS1053<br>
![Schematic with VS1053](additional_info/MWR_V2_VS1053.jpg)<br>

Schematic with external DAC<br>
![Schematic with external DAC](additional_info/MWR_V2_DAC.jpg)<br>
<br>
<a href="https://github.com/schreibfaul1/ESP32-MiniWebRadio/blob/master/additional_info/MiniWebRadio%20V2%20Layout.pdf">Display (Layout)</a>

<a href="https://github.com/schreibfaul1/ESP32-MiniWebRadio/blob/master/additional_info/How%20to%20install.pdf">How to install:</a>
PlatformIO is definitely recommended as an IDE

#### New in V2:

- The audioprocess works in his own task amd must therefore be decoupled. If a VS1053 is used, it must have its own SPI bus (HSPI for VS1053 and VSPI for TFT and TP). Dropouts when drawing on the display or when the website is loading are a things of the past.
- The SD card is wired as SD_MMC to improve stability and increase speed. This means that the GPIOs cannot be chosen freely. The <a href="https://github.com/schreibfaul1/ESP32-MiniWebRadio/blob/master/additional_info/SD_Card_Adapter_for_SD_MMC_.jpg">SD card adapter</a> must not have any resistors as pull-ups or in series.
- Instead of the VS1053, it can be decoded using SW. Possible formats are mp3, aac, mp4 and flac (flac requires PSRAM). A DAC is required (e.g. UDA13348, MAX98357A, PCM5102A) connected via I2S.
- The display can now be 480x320px, the ILI9486 (SPI display from the Raspberry PI) is supported
- In the finished device, the SD card may be inaccessible. For this case, an FTP server is integrated. Here are the settings in <a href="https://github.com/schreibfaul1/ESP32-MiniWebRadio/blob/master/additional_info/Filezilla.pdf">Filezilla</a>. The username and password are 'esp32' and can be changed in 'common.h'
- supports AC101 and ES8388
- WM8978 support (TTGO audioT board)
- Entry of username and password if the server expects access data, "URL|user|pwd"
- Can process local playlists in m3u format
- The ESP32 or the ESP32-S3 can be used

<br>

Codec\Decoder| VS1053B        | PCM5102A, AC101, ES8388, WM8978 |
|----------|----------|----------|
| mp3 | y| y |
| aac | y | y |
| aacp (HLS) | y  | mono |
| wav | y | y  |
| flac | with plugin | blocksize max 8192 bytes |
| vorbis | y  | y (<=196Kbit/s)  |
| m4a | y  | y |
| opus | n  | y (celt)  |

![MWR](/additional_info/MWR.jpg)<br>
<br>

