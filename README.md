# ESP32-MiniWebRadio V2

![Display](https://github.com/schreibfaul1/ESP32-MiniWebRadio/blob/MiniWebRadio-V2/additional_info/MiniWebRadio.jpg)

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
</ul>
Required HW:
<ul>
<li>Decoder module VS1053 or external DAC (e.g. PCM5102a)</li>
<li>TFT Display with Tochpad (SPI), Controller can be ILI9341 (320x240px), HX8347D (320x240px) or ILI9486 (480x320px)</li>
<li>ESP32 Board (PSRAM not necessary)</li>
<li>SD Card + adapter</li>
</ul>

Control is exclusively s via the touchscreen or the webpage, no additional components such as switches, rotary encoders, capacitors or resistors are required

Schematic with VS1053:

Schematic with external DAC:

How to install:
PlatformIO is definitely recommended as an IDE

[PlatformIO](https://github.com/schreibfaul1/ESP32-MiniWebRadio/blob/master/additional_info/Notes%20on%20programming%20with%20PlatformIO.pdf) or 
MiniWebRadio/blob/master/additional_info/Notes%20on%20programming%20with%20the%20Arduino%20IDE.pdf)<br>
![Display](https://github.com/schreibfaul1/ESP32-MiniWebRadio/blob/master/additional_info/MiniWebRadio1.jpg)
