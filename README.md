# ESP32-MiniWebRadio V2

![Display](https://github.com/schreibfaul1/ESP32-MiniWebRadio/blob/MiniWebRadio-V2/additional_info/MiniWebRadio.jpg)

Can handle max 999 stations<br>
IR remote control is optional<br>
Obtains time from NTP<br>
Can used as alarmclock, has sleeptimer<br>
Speech the time every hour in radiomode<br>
If the display has a Backlight-pin You can change the brightness<br>
supports the Latin, Greek and Cyrillic character sets
[Community Radio Browser](https://www.radio-browser.info/) is integrated as a search engine
Channel lists can be exported or imported in Excel format (for data backup).

Required HW:
Decoder module VS1053 or external DAC (e.g. PCM5102a)
TFT Display with Tochpad (SPI), Controller can be ILI9341 (320x240px), HX8347D (320x240px) or ILI9486 (480x320px)
ESP32 Board (PSRAM not necessary)
SD Card + adapter

Control is exclusively s via the touchscreen or the webpage, no additional components such as switches, rotary encoders, capacitors or resistors are required

Schematic with VS1053:

Schematic with external DAC:

How to install:
PlatformIO is definitely recommended as an IDE

[PlatformIO](https://github.com/schreibfaul1/ESP32-MiniWebRadio/blob/master/additional_info/Notes%20on%20programming%20with%20PlatformIO.pdf) or 
MiniWebRadio/blob/master/additional_info/Notes%20on%20programming%20with%20the%20Arduino%20IDE.pdf)<br>
![Display](https://github.com/schreibfaul1/ESP32-MiniWebRadio/blob/master/additional_info/MiniWebRadio1.jpg)
