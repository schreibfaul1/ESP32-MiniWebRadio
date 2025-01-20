// first release on 09/2019
// updated on Jan 20 2025

#include "tft.h"
#include "Arduino.h"
#include "pins_arduino.h"

#define LCD_SPI
// define RGB_HMI

SPIClass*   SPItransfer;

#define __malloc_heap_psram(size) heap_caps_malloc_prefer(size, 2, MALLOC_CAP_DEFAULT | MALLOC_CAP_SPIRAM, MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL)

#define ILI9341_MADCTL_MY  0x80
#define ILI9341_MADCTL_MX  0x40
#define ILI9341_MADCTL_MV  0x20
#define ILI9341_MADCTL_ML  0x10
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH  0x04
#define ILI9341_SLPOUT     0x11 // Sleep Out
#define ILI9341_INVOFF     0x20 // Display Invert Off
#define ILI9341_INVON      0x21 // Display Invert On
#define ILI9341_DISPON     0x29 // Display On
#define ILI9341_CASET      0x2A // Column Address Set
#define ILI9341_RASET      0x2B // Row Address Set
#define ILI9341_RAMWR      0x2C // Memory Write
#define ILI9341_RAMRD      0x2E // Memory Read
#define ILI9341_MADCTL     0x36 // Memory Data Access Control
#define ILI9341_VSCRSADD   0x37 // Vertical Scrolling Start Address
//----------------------------------------------------------------------------------------------------------------------------------------------------
#define ILI9486_INVOFF     0x20 // Display Inversion OFF
#define ILI9486_INVON      0x21 // Display Inversion ON
#define ILI9486_CASET      0x2A // Display On
#define ILI9486_PASET      0x2B // Page Address Set
#define ILI9486_RAMWR      0x2C // Memory Write
#define ILI9486_RAMRD      0x2E // Memory Read
#define ILI9486_MADCTL     0x36 // Memory Data Access Control
#define ILI9486_MADCTL_MY  0x80 // Bit 7 Parameter MADCTL
#define ILI9486_MADCTL_MX  0x40 // Bit 5 Parameter MADCTL
#define ILI9486_MADCTL_MV  0x20 // Bit 5 Parameter MADCTL
#define ILI9486_MADCTL_ML  0x10 // Bit 4 Parameter MADCTL
#define ILI9486_MADCTL_BGR 0x08 // Bit 3 Parameter MADCTL
#define ILI9486_MADCTL_MH  0x04 // Bit 2 Parameter MADCTL
#define ILI9486_WDBVAL     0x51 // Write Display Brightness Value
#define ILI9486_CDBVAL     0x53 // Write Control Display Value
//----------------------------------------------------------------------------------------------------------------------------------------------------
#define ILI9488_SLPOUT     0x11 // Sleep OUT
#define ILI9488_INVOFF     0x20 // Display Inversion OFF
#define ILI9488_INVON      0x21 // Display Inversion ON
#define ILI9488_DISPOFF    0x28 // Display OFF
#define ILI9488_DISPON     0x29 // Display ON
#define ILI9488_CASET      0x2A // Column Address Set
#define ILI9488_PASET      0x2B // Page Address Set
#define ILI9488_MADCTL     0x36 // Memory Access Control
#define ILI9488_COLMOD     0x3A // Interface Pixel Format
#define ILI9488_IFMODE     0xB0 // Interface Mode Control
#define ILI9488_FRMCTR1    0xB1 // Frame Rate Control
#define ILI9488_FRMCTR2    0xB2 // Frame Rate Control
#define ILI9488_INVTR      0xB4 // Display Inversion Control
#define ILI9488_DISCTRL    0xB6 // Display Function Control
#define ILI9488_ETMOD      0xB7 // Entry Mode Set
#define ILI9488_RAMWR      0x2C // Write_memory_start
#define ILI9488_RAMRD      0x2E // Read_memory_start
#define ILI9488_PWCTR1     0xC0 // Panel Driving Setting
#define ILI9488_PWCTR2     0xC1 // Display_Timing_Setting for Normal Mode
#define ILI9488_VMCTR1     0xC5 // Frame Rate and Inversion Control
#define ILI9488_PGAMCTRL   0xE0 // NV Memory Write
#define ILI9488_NGAMCTRL   0xE1 // NV Memory Control
#define ILI9488_MADCTL_MY  0x80
#define ILI9488_MADCTL_MX  0x40
#define ILI9488_MADCTL_MV  0x20
#define ILI9488_MADCTL_ML  0x10
#define ILI9488_MADCTL_RGB 0x00
#define ILI9488_MADCTL_BGR 0x08
#define ILI9488_MADCTL_MH  0x04
#define ILI9488_MADCTL_SS  0x02
#define ILI9488_MADCTL_GS  0x01
//----------------------------------------------------------------------------------------------------------------------------------------------------
#define ST7796_NOP        0x00 // No operation
#define ST7796_SWRESET    0x01 // Software reset
#define ST7796_SLPIN      0x10 // Sleep in
#define ST7796_SLPOUT     0x11 // Sleep Out
#define ST7796_NORON      0x13 // Normal Display Mode On
#define ST7796_INVOFF     0x20 // Display Inversion OFF
#define ST7796_INVON      0x21 // Display Inversion ON
#define ST7796_GAMSET     0x26 // Gamma set
#define ST7796_DISPOFF    0x28 // Display Off
#define ST7796_DISPON     0x29 // Display On
#define ST7796_CASET      0x2A // Column Address Set
#define ST7796_RASET      0x2B // Row Address Set
#define ST7796_RAMWR      0x2C // Memory Write
#define ST7796_RAMRD      0x2E // Memory Read
#define ST7796_MADCTL     0x36 // Memory Data Access Control
#define ST7796_COLMOD     0x3A // Interface Pixel Format
#define ST7796_IFMODE     0xB0 // RAM control
#define ST7796_FRMCTR1    0xB1 // RGB Interface Control
#define ST7796_FRMCTR2    0xB2 // Porch control
#define ST7796_FRMCTR3    0xB3 // Frame Rate Control 1 (In partial mode/ idle colors)
#define ST7796_DIC        0xB4 // Display Inversion Control
#define ST7796_BPC        0xB5 // Blanking Porch Control
#define ST7796_DFC        0xB6 // Display Function Control
#define ST7796_EM         0xB7 // Entry Mode Set
#define ST7796_VCOMS      0xBB // VCOMS setting
#define ST7796_PWR1       0xC0 // Power Control 1
#define ST7796_PWR2       0xC1 // Power Control 2
#define ST7796_PWR3       0xC2 // Power Control 3
#define ST7796_VCMPCTL    0xC5 // VCOM Control
#define ST7796_VCM        0xC6 // Vcom Offset Register
#define ST7796_NVMADW     0xD0 // NVM Address/Data Write
#define ST7796_PGC        0xE0 // Positive Gamma Control
#define ST7796_NGC        0xE1 // Negative Gamma Control
#define ST7796_DOCA       0xE8 // Display Output Ctrl Adjust
#define ST7796_CSCON      0xF0 // Command Set Control
#define ST7796_MADCTL_MY  0x80 // Bit 7 Parameter MADCTL
#define ST7796_MADCTL_MX  0x40 // Bit 6 Parameter MADCTL
#define ST7796_MADCTL_MV  0x20 // Bit 5 Parameter MADCTL
#define ST7796_MADCTL_ML  0x10 // Bit 4 Parameter MADCTL
#define ST7796_MADCTL_RGB 0x00 // Bit 3 Parameter MADCTL
#define ST7796_MADCTL_BGR 0x08 // Bit 3 Parameter MADCTL
#define ST7796_MADCTL_MH  0x04 // Bit 2 Parameter MADCTL
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::init() {
    startWrite();
    if(_TFTcontroller == ILI9341) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ILI9341");
        writeCommand(0xCB); // POWERA
        spi_TFT->write(0x39); spi_TFT->write(0x2C); spi_TFT->write(0x00); spi_TFT->write(0x34);
        spi_TFT->write(0x02);
        writeCommand(0xCF); // POWERB
        spi_TFT->write(0x00); spi_TFT->write(0xC1); spi_TFT->write(0x30);
        writeCommand(0xE8); // DTCA
        spi_TFT->write(0x85); spi_TFT->write(0x00); spi_TFT->write(0x78);
        writeCommand(0xEA); // DTCB
        spi_TFT->write(0x00); spi_TFT->write(0x00);
        writeCommand(0xED); // POWER_SEQ
        spi_TFT->write(0x64); spi_TFT->write(0x03); spi_TFT->write(0X12); spi_TFT->write(0X81);
        writeCommand(0xF7); // PRC
        spi_TFT->write(0x20);
        writeCommand(0xC0);   // Power control
        spi_TFT->write(0x23); // VRH[5:0]
        writeCommand(0xC1);   // Power control
        spi_TFT->write(0x10); // SAP[2:0];BT[3:0]
        writeCommand(0xC5); // VCM control
        spi_TFT->write(0x3e); spi_TFT->write(0x28);
        writeCommand(0xC7); // VCM control2
        spi_TFT->write(0x86);
        writeCommand(0x36);   // Memory Access Control
        spi_TFT->write(0x48); // 88
        writeCommand(0x3A); // PIXEL_FORMAT
        spi_TFT->write(0x55);
        writeCommand(0xB1); // FRC
        spi_TFT->write(0x00); spi_TFT->write(0x18);
        writeCommand(0xB6); // Display Function Control
        spi_TFT->write(0x08); spi_TFT->write(0x82); spi_TFT->write(0x27);
        writeCommand(0xF2); // 3Gamma Function Disable
        spi_TFT->write(0x00);
        writeCommand(0x2A); // COLUMN_ADDR
        spi_TFT->write(0x00); spi_TFT->write(0x00); spi_TFT->write(0x00); spi_TFT->write(0xEF);
        writeCommand(0x2A); // PAGE_ADDR
        spi_TFT->write(0x00); spi_TFT->write(0x00); spi_TFT->write(0x01); spi_TFT->write(0x3F);
        writeCommand(0x26); // Gamma curve selected
        spi_TFT->write(0x01);
        writeCommand(0xE0); // Set Gamma
        spi_TFT->write(0x0F); spi_TFT->write(0x31); spi_TFT->write(0x2B); spi_TFT->write(0x0C); spi_TFT->write(0x0E); spi_TFT->write(0x08);
        spi_TFT->write(0x4E); spi_TFT->write(0xF1); spi_TFT->write(0x37); spi_TFT->write(0x07); spi_TFT->write(0x10); spi_TFT->write(0x03);
        spi_TFT->write(0x0E); spi_TFT->write(0x09); spi_TFT->write(0x00);
        writeCommand(0xE1); // Set Gamma
        spi_TFT->write(0x00); spi_TFT->write(0x0E); spi_TFT->write(0x14); spi_TFT->write(0x03); spi_TFT->write(0x11); spi_TFT->write(0x07);
        spi_TFT->write(0x31); spi_TFT->write(0xC1); spi_TFT->write(0x48); spi_TFT->write(0x08); spi_TFT->write(0x0F); spi_TFT->write(0x0C);
        spi_TFT->write(0x31); spi_TFT->write(0x36); spi_TFT->write(0x0F);
        if(_displayInversion == 0) {
            writeCommand(ILI9341_INVOFF); // Display Inversion OFF, normal mode
        }
        else {
            writeCommand(ILI9341_INVON); // Display Inversion ON
        }
        writeCommand(0x11); // Sleep out
        delay(120);
        writeCommand(0x2c);
        writeCommand(0x29); // Display on
        writeCommand(0x2c);
    }
    if(_TFTcontroller == HX8347D) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "HX8347D");
        // Driving ability Setting
        writeCommand(0xEA); spi_TFT->write(0x00); // PTBA[15:8]
        writeCommand(0xEB); spi_TFT->write(0x20); // PTBA[7:0]
        writeCommand(0xEC); spi_TFT->write(0x0C); // STBA[15:8]
        writeCommand(0xED); spi_TFT->write(0xC4); // STBA[7:0]
        writeCommand(0xE8); spi_TFT->write(0x40); // OPON[7:0]
        writeCommand(0xE9); spi_TFT->write(0x38); // OPON1[7:0]
        writeCommand(0xF1); spi_TFT->write(0x01); // OTPS1B
        writeCommand(0xF2); spi_TFT->write(0x10); // GEN
        writeCommand(0x27); spi_TFT->write(0xA3); // Display control 2 register

        // Gamma 2.2 Setting
        writeCommand(0x40); spi_TFT->write(0x01); // Gamma control 1 register
        writeCommand(0x41); spi_TFT->write(0x00); // Gamma control 2 register
        writeCommand(0x42); spi_TFT->write(0x00); // Gamma control 3 register
        writeCommand(0x43); spi_TFT->write(0x10); // Gamma control 4 register
        writeCommand(0x44); spi_TFT->write(0x0E); // Gamma control 5 register
        writeCommand(0x45); spi_TFT->write(0x24); // Gamma control 6 register
        writeCommand(0x46); spi_TFT->write(0x04); // Gamma control 7 register
        writeCommand(0x47); spi_TFT->write(0x50); // Gamma control 8 register
        writeCommand(0x48); spi_TFT->write(0x02); // Gamma control 9 register
        writeCommand(0x49); spi_TFT->write(0x13); // Gamma control 10 register
        writeCommand(0x4A); spi_TFT->write(0x19); // Gamma control 11 register
        writeCommand(0x4B); spi_TFT->write(0x19); // Gamma control 12 register
        writeCommand(0x4C); spi_TFT->write(0x16); // Gamma control 13 register
        writeCommand(0x50); spi_TFT->write(0x1B); // Gamma control 14 register
        writeCommand(0x51); spi_TFT->write(0x31); // Gamma control 15 register
        writeCommand(0x52); spi_TFT->write(0x2F); // Gamma control 16 register
        writeCommand(0x53); spi_TFT->write(0x3F); // Gamma control 17 register
        writeCommand(0x54); spi_TFT->write(0x3F); // Gamma control 18 register
        writeCommand(0x55); spi_TFT->write(0x3E); // Gamma control 19 register
        writeCommand(0x56); spi_TFT->write(0x2F); // Gamma control 20 register
        writeCommand(0x57); spi_TFT->write(0x7B); // Gamma control 21 register
        writeCommand(0x58); spi_TFT->write(0x09); // Gamma control 22 register
        writeCommand(0x59); spi_TFT->write(0x06); // Gamma control 23 register
        writeCommand(0x5A); spi_TFT->write(0x06); // Gamma control 24 register
        writeCommand(0x5B); spi_TFT->write(0x0C); // Gamma control 25 register
        writeCommand(0x5C); spi_TFT->write(0x1D); // Gamma control 26 register
        writeCommand(0x5D); spi_TFT->write(0xCC); // Gamma control 27 register

        // Power Voltage Setting
        writeCommand(0x1B); spi_TFT->write(0x1B); // VRH=4.65V
        writeCommand(0x1A); spi_TFT->write(0x01); // BT (VGH~15V,VGL~-10V,DDVDH~5V)
        writeCommand(0x24); spi_TFT->write(0x15); // VMH(VCOM High voltage ~3.2V)
        writeCommand(0x25); spi_TFT->write(0x50); // VML(VCOM Low voltage -1.2V)
        writeCommand(0x23); spi_TFT->write(0x88); // for Flicker adjust //can reload from OTP

        // Power on Setting
        writeCommand(0x18); spi_TFT->write(0x36); // I/P_RADJ,N/P_RADJ, Normal mode 60Hz
        writeCommand(0x19); spi_TFT->write(0x01); // OSC_EN='1', start Osc

        if(_displayInversion == 0) { writeCommand(0x01); spi_TFT->write(0x00);} // DP_STB='0', out deep sleep
        else {                       writeCommand(0x01); spi_TFT->write(0x02);} // DP_STB='0', out deep sleep, invon = 1

        writeCommand(0x1F); spi_TFT->write(0x88); // GAS=1, VOMG=00, PON=0, DK=1, XDK=0, DVDH_TRI=0, STB=0
        delay(5);
        writeCommand(0x1F); spi_TFT->write(0x80); // GAS=1, VOMG=00, PON=0, DK=0, XDK=0, DVDH_TRI=0, STB=0
        delay(5);
        writeCommand(0x1F); spi_TFT->write(0x90); // GAS=1, VOMG=00, PON=1, DK=0, XDK=0, DVDH_TRI=0, STB=0
        delay(5);
        writeCommand(0x1F); spi_TFT->write(0xD0); // GAS=1, VOMG=10, PON=1, DK=0, XDK=0, DDVDH_TRI=0, STB=0
        delay(5);
        // 262k/65k color selection
        writeCommand(0x17); spi_TFT->write(0x05); // default 0x06 262k color // 0x05 65k color
        // SET PANEL
        writeCommand(0x36); spi_TFT->write(0x00); // SS_P, GS_P,REV_P,BGR_P
        // Display ON Setting
        writeCommand(0x28); spi_TFT->write(0x38); // GON=1, DTE=1, D=1000
        delay(40);
        writeCommand(0x28); spi_TFT->write(0x3C); // GON=1, DTE=1, D=1100

        writeCommand(0x16); spi_TFT->write(0x08); // MY=0, MX=0, MV=0, BGR=1
        // Set GRAM Area
        writeCommand(0x02); spi_TFT->write(0x00); // Column address start register upper byte
        writeCommand(0x03); spi_TFT->write(0x00); // Column address start register low byte
        writeCommand(0x04); spi_TFT->write(0x00);
        writeCommand(0x05); spi_TFT->write(0xEF); // Column End
        writeCommand(0x06); spi_TFT->write(0x00);
        writeCommand(0x07); spi_TFT->write(0x00); // Row Start
        writeCommand(0x08); spi_TFT->write(0x01);
        writeCommand(0x09); spi_TFT->write(0x3F); // Row End
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ILI9486");

        // Driving ability Setting
        writeCommand(0x11); // Sleep out, also SW reset
        delay(120);

        writeCommand(0x3A); // Interface Pixel Format
        spi_TFT->write16(0x55);

        writeCommand(0xC2); // Power Control 3 (For Normal Mode)
        spi_TFT->write16(0x44);

        writeCommand(0xC5); // VCOM Control
        spi_TFT->write16(0x00); spi_TFT->write16(0x00); spi_TFT->write16(0x00); spi_TFT->write16(0x00);

        if(_TFTcontroller == ILI9486a) {
            writeCommand(0xE0); // PGAMCTRL(Positive Gamma Control)
            spi_TFT->write16(0x00); spi_TFT->write16(0x04); spi_TFT->write16(0x0E); spi_TFT->write16(0x08); spi_TFT->write16(0x17); spi_TFT->write16(0x0A);
            spi_TFT->write16(0x40); spi_TFT->write16(0x79); spi_TFT->write16(0x4D); spi_TFT->write16(0x07); spi_TFT->write16(0x0E); spi_TFT->write16(0x0A);
            spi_TFT->write16(0x1A); spi_TFT->write16(0x1D); spi_TFT->write16(0x0F);
            writeCommand(0xE1); // NGAMCTRL (Negative Gamma Correction)
            spi_TFT->write16(0x00); spi_TFT->write16(0x1B); spi_TFT->write16(0x1F); spi_TFT->write16(0x02); spi_TFT->write16(0x10); spi_TFT->write16(0x05);
            spi_TFT->write16(0x32); spi_TFT->write16(0x34); spi_TFT->write16(0x43); spi_TFT->write16(0x02); spi_TFT->write16(0x0A); spi_TFT->write16(0x09);
            spi_TFT->write16(0x33); spi_TFT->write16(0x37); spi_TFT->write16(0x0F);
        }

        if(_TFTcontroller == ILI9486b) {
            writeCommand(0xE0); // PGAMCTRL(alternative Positive Gamma Control)
            spi_TFT->write16(0x0F);
            spi_TFT->write16(0x1F);
            spi_TFT->write16(0x1C);
            spi_TFT->write16(0x0C);
            spi_TFT->write16(0x0F);
            spi_TFT->write16(0x08);
            spi_TFT->write16(0x48);
            spi_TFT->write16(0x98);
            spi_TFT->write16(0x37);
            spi_TFT->write16(0x0A);
            spi_TFT->write16(0x13);
            spi_TFT->write16(0x04);
            spi_TFT->write16(0x11);
            spi_TFT->write16(0x0D);
            spi_TFT->write16(0x00);

            writeCommand(0xE1); // NGAMCTRL (alternative Negative Gamma Correction)
            spi_TFT->write16(0x0F);
            spi_TFT->write16(0x32);
            spi_TFT->write16(0x2E);
            spi_TFT->write16(0x0B);
            spi_TFT->write16(0x0D);
            spi_TFT->write16(0x05);
            spi_TFT->write16(0x47);
            spi_TFT->write16(0x75);
            spi_TFT->write16(0x37);
            spi_TFT->write16(0x06);
            spi_TFT->write16(0x10);
            spi_TFT->write16(0x03);
            spi_TFT->write16(0x24);
            spi_TFT->write16(0x20);
            spi_TFT->write16(0x00);
        }

        if(_displayInversion == 0) {
            writeCommand(ILI9486_INVOFF); // Display Inversion OFF, normal mode   RPi LCD (A)
        }
        else {
            writeCommand(ILI9486_INVON); // Display Inversion ON,                RPi LCD (B)
        }

        writeCommand(0x36); // Memory Access Control
        spi_TFT->write16(0x48);

        writeCommand(0x29); // Display ON
        delay(150);
    }
    //==========================================
    if(_TFTcontroller == ILI9488) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ILI9488");
        writeCommand(ILI9488_PGAMCTRL); // PGAMCTRL(Positive Gamma Control)
        spi_TFT->write(0x00);
        spi_TFT->write(0x03);
        spi_TFT->write(0x09);
        spi_TFT->write(0x08);
        spi_TFT->write(0x16);
        spi_TFT->write(0x0A);
        spi_TFT->write(0x3F);
        spi_TFT->write(0x78);
        spi_TFT->write(0x4C);
        spi_TFT->write(0x09);
        spi_TFT->write(0x0A);
        spi_TFT->write(0x08);
        spi_TFT->write(0x16);
        spi_TFT->write(0x1A);
        spi_TFT->write(0x0F);
        writeCommand(ILI9488_NGAMCTRL); // NGAMCTRL (Negative Gamma Correction)
        spi_TFT->write(0x00);
        spi_TFT->write(0x16);
        spi_TFT->write(0x19);
        spi_TFT->write(0x03);
        spi_TFT->write(0x0F);
        spi_TFT->write(0x05);
        spi_TFT->write(0x32);
        spi_TFT->write(0x45);
        spi_TFT->write(0x46);
        spi_TFT->write(0x04);
        spi_TFT->write(0x0E);
        spi_TFT->write(0x0D);
        spi_TFT->write(0x35);
        spi_TFT->write(0x37);
        spi_TFT->write(0x0F);

        if(_displayInversion == 0) {
            writeCommand(ILI9488_INVOFF); // Display Inversion OFF, normal mode
        }
        else {
            writeCommand(ILI9488_INVON); // Display Inversion ON
        }

        writeCommand(ILI9488_PWCTR1); // Power Control 1
        spi_TFT->write(0x17); spi_TFT->write(0x15);
        writeCommand(ILI9488_PWCTR2); // Power Control 2
        spi_TFT->write(0x41);
        writeCommand(ILI9488_VMCTR1); // VCOM Control
        spi_TFT->write(0x00); spi_TFT->write(0x12); spi_TFT->write(0x80);
        writeCommand(ILI9488_MADCTL); // Memory Access Control
        spi_TFT->write(0x48);
        writeCommand(ILI9488_COLMOD); // Pixel Interface Format
        spi_TFT->write(0x66);
        writeCommand(ILI9488_IFMODE); // Interface Mode Control
        spi_TFT->write(0x00);
        writeCommand(ILI9488_FRMCTR1); // Frame Rate Control
        spi_TFT->write(0xA0);
        writeCommand(ILI9488_INVTR); // Display Inversion Control
        spi_TFT->write(0x02);
        writeCommand(ILI9488_DISCTRL); // Display Function Control
        spi_TFT->write(0x02); spi_TFT->write(0x02); spi_TFT->write(0x3B);
        writeCommand(ILI9488_ETMOD); // Entry Mode Set
        spi_TFT->write(0xC6);
        writeCommand(0xF7); // Adjust Control 3
        spi_TFT->write(0xA9); spi_TFT->write(0x51); spi_TFT->write(0x2C); spi_TFT->write(0x82);
        writeCommand(ILI9488_SLPOUT); // Exit Sleep
        delay(120);
        writeCommand(ILI9488_DISPON); // Display on
        delay(25);
    }
    //==========================================
    if(_TFTcontroller == ST7796) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ST7796");
        writeCommand(ST7796_SWRESET);
        delay(120);
        writeCommand(ST7796_SLPOUT); // Sleep Out
        delay(120);
        writeCommand(ST7796_MADCTL); // Memory Data Access Control
        spi_TFT->write(0x40);
        writeCommand(ST7796_CSCON); // Command Set Control
        spi_TFT->write(0xC3);       // Enable extension command 2 partI
        writeCommand(ST7796_CSCON); // Command Set Control
        spi_TFT->write(0x96);       // Enable extension command 2 partII
        writeCommand(ST7796_DIC); // Display Inversion Control
        spi_TFT->write(0x00);
        writeCommand(ST7796_IFMODE); // RAM control
        spi_TFT->write(0x00);
        writeCommand(ST7796_BPC); // Blanking Porch Control
        spi_TFT->write(0x08); spi_TFT->write(0x08); spi_TFT->write(0x00); spi_TFT->write(0x64);
        writeCommand(ST7796_PWR1); // Power Control 1
        spi_TFT->write(0xF0); spi_TFT->write(0x17);
        writeCommand(ST7796_PWR2); // Power Control 2
        spi_TFT->write(0x14);      //
        writeCommand(ST7796_PWR3); // Power Control 3
        spi_TFT->write(0xA7);
        writeCommand(ST7796_VCMPCTL); // VCOM Control
        spi_TFT->write(0x20);
        writeCommand(ST7796_DOCA); // Display Output Ctrl Adjust
        spi_TFT->write(0x40); spi_TFT->write(0x8A); spi_TFT->write(0x00); spi_TFT->write(0x00);
        spi_TFT->write(0x29); spi_TFT->write(0x01); spi_TFT->write(0xBF); spi_TFT->write(0x33);

        //--------------------------------ST7789V gamma setting---------------------------------------//
        writeCommand(ST7796_PGC); // PGAMCTRL(Positive Gamma Control)
        spi_TFT->write(0xF0);
        spi_TFT->write(0x0B);
        spi_TFT->write(0x11);
        spi_TFT->write(0x0B);
        spi_TFT->write(0x0A);
        spi_TFT->write(0x27);
        spi_TFT->write(0x3C);
        spi_TFT->write(0x55);
        spi_TFT->write(0x51);
        spi_TFT->write(0x37);
        spi_TFT->write(0x15);
        spi_TFT->write(0x17);
        spi_TFT->write(0x31);
        spi_TFT->write(0x35);

        writeCommand(ST7796_NGC); // NGAMCTRL (Negative Gamma Correction)
        spi_TFT->write(0x4E);
        spi_TFT->write(0x15);
        spi_TFT->write(0x19);
        spi_TFT->write(0x0B);
        spi_TFT->write(0x09);
        spi_TFT->write(0x27);
        spi_TFT->write(0x34);
        spi_TFT->write(0x32);
        spi_TFT->write(0x46);
        spi_TFT->write(0x38);
        spi_TFT->write(0x14);
        spi_TFT->write(0x16);
        spi_TFT->write(0x26);
        spi_TFT->write(0x2A);

        writeCommand(ST7796_CSCON); // Command Set Control
        spi_TFT->write(0x3C);       // Enable extension command 2 partI

        writeCommand(ST7796_CSCON); // Command Set Control
        spi_TFT->write(0x69);       // Enable extension command 2 partII

        if(_displayInversion == 0) {
            writeCommand(ST7796_INVOFF); // Display Inversion OFF, normal mode
        }
        else {
            writeCommand(ST7796_INVON); // Display Inversion ON
        }

        writeCommand(ST7796_DISPON); // Display on
        delay(25);
    } //===============================================================================

    if(_TFTcontroller == ST7796RPI) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ST7796_RPI");
        writeCommand(ST7796_SWRESET);
        delay(120);

        writeCommand(ST7796_SLPOUT); // Sleep Out
        delay(120);

        writeCommand(ST7796_CSCON); // Command Set Control
        spi_TFT->write16(0xC3);     // Enable extension command 2 partI

        writeCommand(ST7796_CSCON); // Command Set Control
        spi_TFT->write16(0x96);     // Enable extension command 2 partII

        writeCommand(ST7796_MADCTL); // Memory Data Access Control
        spi_TFT->write16(0x48);

        writeCommand(ST7796_COLMOD); // Memory Data Access Control MX, MY, RGB mode
        spi_TFT->write16(0x55);

        writeCommand(ST7796_DIC); // Display Inversion Control
        spi_TFT->write16(0x00);

        writeCommand(ST7796_IFMODE); // RAM control
        spi_TFT->write16(0x00);

        writeCommand(ST7796_BPC); // Blanking Porch Control
        spi_TFT->write16(0x08); spi_TFT->write16(0x08); spi_TFT->write16(0x00); spi_TFT->write16(0x64);
        writeCommand(ST7796_PWR1); // Power Control 1
        spi_TFT->write16(0xF0); spi_TFT->write16(0x17);
        writeCommand(ST7796_PWR2); // Power Control 2
        spi_TFT->write16(0x14);    //
        writeCommand(ST7796_PWR3); // Power Control 3
        spi_TFT->write16(0xA7);
        writeCommand(ST7796_VCMPCTL); // VCOM Control
        spi_TFT->write16(0x20);
        writeCommand(ST7796_DOCA); // Display Output Ctrl Adjust
        spi_TFT->write16(0x40); spi_TFT->write16(0x8A); spi_TFT->write16(0x00); spi_TFT->write16(0x00);
        spi_TFT->write16(0x29); spi_TFT->write16(0x01); spi_TFT->write16(0xBF); spi_TFT->write16(0x33);

        //--------------------------------ST7789V gamma setting---------------------------------------//
        writeCommand(ST7796_PGC); // PGAMCTRL(Positive Gamma Control)
        spi_TFT->write16(0xF0); spi_TFT->write16(0x0B); spi_TFT->write16(0x11); spi_TFT->write16(0x0B);
        spi_TFT->write16(0x0A); spi_TFT->write16(0x27); spi_TFT->write16(0x3C); spi_TFT->write16(0x55);
        spi_TFT->write16(0x51); spi_TFT->write16(0x37); spi_TFT->write16(0x15); spi_TFT->write16(0x17);
        spi_TFT->write16(0x31); spi_TFT->write16(0x35);

        writeCommand(ST7796_NGC); // NGAMCTRL (Negative Gamma Correction)
        spi_TFT->write16(0x4E); spi_TFT->write16(0x15); spi_TFT->write16(0x19); spi_TFT->write16(0x0B);
        spi_TFT->write16(0x09); spi_TFT->write16(0x27); spi_TFT->write16(0x34); spi_TFT->write16(0x32);
        spi_TFT->write16(0x46); spi_TFT->write16(0x38); spi_TFT->write16(0x14); spi_TFT->write16(0x16);
        spi_TFT->write16(0x26); spi_TFT->write16(0x2A);
        writeCommand(ST7796_CSCON); // Command Set Control
        spi_TFT->write16(0x3C);     // Enable extension command 2 partI
        writeCommand(ST7796_CSCON); // Command Set Control
        spi_TFT->write16(0x69);     // Enable extension command 2 partII
        if(_displayInversion == 0) {
            writeCommand(ST7796_INVOFF); // Display Inversion OFF, normal mode
        }
        else {
            writeCommand(ST7796_INVON); // Display Inversion ON
        }
        writeCommand(ST7796_DISPON); // Display on
        delay(25);
    }
    endWrite();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

TFT::TFT(uint8_t TFTcontroller, uint8_t dispInv) {
    _TFTcontroller = TFTcontroller; // 0=ILI9341, 1=HX8347D, 2=ILI9486(a), 3=ILI9486(b), 4= ILI9488, 5=ST7796
    _displayInversion = dispInv;    // 0=off default, 1=on

    if(_TFTcontroller == ILI9341) {
        m_h_res = 320;
        m_v_res = 240;
        _rotation = 0;
    }
    if(_TFTcontroller == HX8347D) {
        m_h_res = 320;
        m_v_res = 240;
        _rotation = 0;
    }
    if(_TFTcontroller == ILI9486a) {
        m_h_res = 480;
        m_v_res = 320;
        _rotation = 0;
    }
    if(_TFTcontroller == ILI9486b) { // Waveshare
        m_h_res = 480;
        m_v_res = 320;
        _rotation = 0;
    }
    if(_TFTcontroller == ILI9488) {
        m_h_res = 480;
        m_v_res = 320;
        _rotation = 0;
    }
    if(_TFTcontroller == ST7796 || _TFTcontroller == ST7796RPI) {
        m_h_res = 480;
        m_v_res = 320;
        _rotation = 0;
    }
    _freq = 20000000;
    spi_TFT = &SPI;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::setFrequency(uint32_t f) {
    if(f > 80000000) f = 80000000;
    _freq = f; // overwrite default
    spi_TFT->setFrequency(_freq);
    TFT_SPI = SPISettings(_freq, MSBFIRST, SPI_MODE0);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::startWrite(void) {
    spi_TFT->beginTransaction(TFT_SPI);
    TFT_CS_LOW();
}

void TFT::endWrite(void) {
    TFT_CS_HIGH();
    spi_TFT->endTransaction();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::writeCommand(uint16_t cmd) {
    TFT_DC_LOW();
    if(_TFTcontroller == ILI9341 || _TFTcontroller == HX8347D || _TFTcontroller == ILI9488 || _TFTcontroller == ST7796) spi_TFT->write(cmd);

    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b || _TFTcontroller == ST7796RPI) spi_TFT->write16(cmd);
    TFT_DC_HIGH();
}
// Return the size of the display (per current rotation)
int16_t TFT::width(void) const { return m_h_res; }
int16_t TFT::height(void) const { return m_v_res; }
uint8_t TFT::getRotation(void) const { return _rotation; }

uint16_t TFT::readCommand() {
    uint16_t ret = 0;
    TFT_DC_LOW();
    if(_TFTcontroller == ILI9341 || _TFTcontroller == HX8347D || _TFTcontroller == ILI9488 || _TFTcontroller == ST7796) ret = spi_TFT->transfer(0);

    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b || _TFTcontroller == ST7796RPI) ret = spi_TFT->transfer16(0);
    TFT_DC_HIGH();
    return ret;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::begin(uint8_t CS, uint8_t DC, uint8_t spi, uint8_t mosi, uint8_t miso, uint8_t sclk) { // LCD_SPI
    spi_TFT = new SPIClass(spi);
    spi_TFT->begin(sclk, miso, mosi, -1);
    spi_TFT->setFrequency(_freq);
    SPItransfer = spi_TFT;
    TFT_SPI = SPISettings(_freq, MSBFIRST, SPI_MODE0);
    String info = "";
    _TFT_CS = CS;
    _TFT_DC = DC;

    pinMode(_TFT_DC, OUTPUT);
    digitalWrite(_TFT_DC, LOW);
    pinMode(_TFT_CS, OUTPUT);
    digitalWrite(_TFT_CS, HIGH);
    init(); //
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::begin(const Pins& newPins, const Timing& newTiming){ // RGB_HMI
    m_pins = newPins;
    m_timing = newTiming;

    esp_lcd_rgb_panel_config_t panel_config;
    panel_config.clk_src = LCD_CLK_SRC_PLL160M;

    panel_config.timings.pclk_hz = m_timing.pixel_clock_hz;
    panel_config.timings.h_res = m_timing.h_res;
    panel_config.timings.v_res = m_timing.v_res;
    panel_config.timings.hsync_pulse_width = m_timing.hsync_pulse_width;
    panel_config.timings.hsync_back_porch = m_timing.hsync_back_porch;
    panel_config.timings.hsync_front_porch = m_timing.hsync_front_porch;
    panel_config.timings.vsync_pulse_width = m_timing.vsync_pulse_width;
    panel_config.timings.vsync_back_porch = m_timing.vsync_back_porch;
    panel_config.timings.vsync_front_porch = m_timing.vsync_front_porch;
    panel_config.timings.flags.hsync_idle_low = false;
    panel_config.timings.flags.vsync_idle_low = false;
    panel_config.timings.flags.de_idle_high = false;
    panel_config.timings.flags.pclk_active_neg = true;
    panel_config.timings.flags.pclk_idle_high = true;

    panel_config.data_width = 16; // RGB565
    panel_config.bits_per_pixel = 16;
    panel_config.num_fbs = 1;
    panel_config.bounce_buffer_size_px = 0;
    panel_config.psram_trans_align = 16;
    panel_config.dma_burst_size = 0;
    panel_config.hsync_gpio_num = m_pins.hsync;
    panel_config.vsync_gpio_num = m_pins.vsync;
    panel_config.de_gpio_num = m_pins.de;
    panel_config.pclk_gpio_num = m_pins.pclk;
    panel_config.disp_gpio_num = m_pins.bl;

    int8_t pinArr[16] = {               // Daten-Pins für RGB565
        m_pins.b0, m_pins.b1, m_pins.b2, m_pins.b3, m_pins.b4,
        m_pins.g0, m_pins.g1, m_pins.g2, m_pins.g3, m_pins.g4, m_pins.g5,
        m_pins.r0, m_pins.r1, m_pins.r2, m_pins.r3, m_pins.r4
    };

    for (int i = 0; i < 16; ++i) {
        log_i("i %i. pin %i", i, pinArr[i]);
        panel_config.data_gpio_nums[i] = pinArr[i];
    }
    panel_config.flags.disp_active_low = false;
    panel_config.flags.refresh_on_demand = false;
    panel_config.flags.fb_in_psram = true;
    panel_config.flags.double_fb = false;
    panel_config.flags.no_fb = false;
    panel_config.flags.bb_invalidate_cache = false;

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, &m_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(m_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(m_panel));
    log_i("Display initialisiert.");
    esp_lcd_panel_mirror(m_panel, true, true);

    // Hintergrundbeleuchtung einschalten
    gpio_set_direction((gpio_num_t)m_pins.bl, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)m_pins.bl, 1); // Hintergrundbeleuchtung aktivieren

    m_h_res = m_timing.h_res;
    m_v_res = m_timing.v_res;

    void *fb0;
    esp_lcd_rgb_panel_get_frame_buffer(m_panel, 1, &fb0);
    m_framebuffer = (uint16_t*)fb0;

    log_e("m_h_res: %d, m_v_res: %d, m_framebuffer %i", m_h_res, m_v_res, m_framebuffer);
    memset(m_framebuffer, 0x00, m_h_res * m_v_res * 2);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
typedef struct {
    uint8_t  madctl;
    uint8_t  bmpctl;
    uint16_t width;
    uint16_t height;
} rotation_data_t;

const rotation_data_t ili9341_rotations[4] = {
    {(ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR), (ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR), ILI9341_WIDTH, ILI9341_HEIGHT},
    {(ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR), (ILI9341_MADCTL_MV | ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR), ILI9341_HEIGHT, ILI9341_WIDTH},
    {(ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR), (ILI9341_MADCTL_BGR), ILI9341_WIDTH, ILI9341_HEIGHT},
    {(ILI9341_MADCTL_MX | ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR), (ILI9341_MADCTL_MY | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR), ILI9341_HEIGHT, ILI9341_WIDTH}};
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::setRotation(uint8_t m) {
    _rotation = m % 4; // can't be higher than 3

    if(_TFTcontroller == HX8347D) { //"HX8347D"
        startWrite();
        if(_rotation == 0) {
            writeCommand(0x16);
            spi_TFT->write(0x08); // 0
            writeCommand(0x04);
            spi_TFT->write(0x00);
            writeCommand(0x05);
            spi_TFT->write(0xEF);
            writeCommand(0x08);
            spi_TFT->write(0x01);
            writeCommand(0x09);
            spi_TFT->write(0x3F);
            m_h_res = HX8347D_WIDTH;
            m_v_res = HX8347D_HEIGHT;
        }
        if(_rotation == 1) {
            writeCommand(0x16);
            spi_TFT->write(0x68); // 90
            writeCommand(0x04);
            spi_TFT->write(0x01);
            writeCommand(0x05);
            spi_TFT->write(0x3F);
            writeCommand(0x08);
            spi_TFT->write(0x00);
            writeCommand(0x09);
            spi_TFT->write(0xEF);
            m_v_res = HX8347D_WIDTH;
            m_h_res = HX8347D_HEIGHT;
        }
        if(_rotation == 2) {
            writeCommand(0x16); spi_TFT->write(0xC8); // 180
            writeCommand(0x04); spi_TFT->write(0x00);
            writeCommand(0x05); spi_TFT->write(0xEF);
            writeCommand(0x08); spi_TFT->write(0x01);
            writeCommand(0x09); spi_TFT->write(0x3F);
            m_h_res = HX8347D_WIDTH;
            m_v_res = HX8347D_HEIGHT;
        }
        if(_rotation == 3) {
            writeCommand(0x16); spi_TFT->write(0xA8); // 270
            writeCommand(0x04); spi_TFT->write(0x01);
            writeCommand(0x05); spi_TFT->write(0x3F);
            writeCommand(0x08); spi_TFT->write(0x00);
            writeCommand(0x09); spi_TFT->write(0xEF);
            m_v_res = HX8347D_WIDTH;
            m_h_res = HX8347D_HEIGHT;
        }
        endWrite();
    }
    if(_TFTcontroller == ILI9341) { // ILI9341
        m = ili9341_rotations[_rotation].madctl;
        m_v_res = ili9341_rotations[_rotation].width;
        m_h_res = ili9341_rotations[_rotation].height;
        startWrite();
        writeCommand(ILI9341_MADCTL);
        spi_TFT->write(m);
        endWrite();
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) {
        _rotation = m % 4; // can't be higher than 3
        startWrite();
        writeCommand(ILI9486_MADCTL);
        switch(_rotation) {
            case 0:
                spi_TFT->write16(ILI9486_MADCTL_MX | ILI9486_MADCTL_BGR);
                m_h_res = ILI9486_WIDTH;
                m_v_res = ILI9486_HEIGHT;
                break;
            case 1:
                spi_TFT->write16(ILI9486_MADCTL_MV | ILI9486_MADCTL_BGR);
                m_v_res = ILI9486_WIDTH;
                m_h_res = ILI9486_HEIGHT;
                break;
            case 2:
                spi_TFT->write16(ILI9486_MADCTL_MY | ILI9486_MADCTL_BGR);
                m_h_res = ILI9486_WIDTH;
                m_v_res = ILI9486_HEIGHT;
                break;
            case 3:
                spi_TFT->write16(ILI9486_MADCTL_MX | ILI9486_MADCTL_MY | ILI9486_MADCTL_MV | ILI9486_MADCTL_BGR);
                m_v_res = ILI9486_WIDTH;
                m_h_res = ILI9486_HEIGHT;
                break;
        }
        endWrite();
    }
    if(_TFTcontroller == ILI9488) {
        _rotation = m % 4; // can't be higher than 3
        startWrite();
        writeCommand(ILI9488_MADCTL);
        switch(_rotation) {
            case 0:
                spi_TFT->write(ILI9488_MADCTL_MX | ILI9488_MADCTL_BGR);
                m_h_res = ILI9488_WIDTH;
                m_v_res = ILI9488_HEIGHT;
                break;
            case 1:
                spi_TFT->write(ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR);
                m_v_res = ILI9488_WIDTH;
                m_h_res = ILI9488_HEIGHT;
                break;
            case 2:
                spi_TFT->write(ILI9488_MADCTL_MY | ILI9488_MADCTL_BGR);
                m_h_res = ILI9488_WIDTH;
                m_v_res = ILI9488_HEIGHT;
                break;
            case 3:
                spi_TFT->write(ILI9488_MADCTL_MX | ILI9488_MADCTL_MY | ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR);
                m_v_res = ILI9488_WIDTH;
                m_h_res = ILI9488_HEIGHT;
                break;
        }
        endWrite();
    }
    if(_TFTcontroller == ST7796) {
        _rotation = m % 4; // can't be higher than 3
        startWrite();
        writeCommand(ST7796_MADCTL);
        switch(_rotation) {
            case 0:
                spi_TFT->write(ST7796_MADCTL_MX | ST7796_MADCTL_BGR);
                m_h_res = ST7796_WIDTH;
                m_v_res = ST7796_HEIGHT;
                break;
            case 1:
                spi_TFT->write(ST7796_MADCTL_MV | ST7796_MADCTL_BGR);
                m_v_res = ST7796_WIDTH;
                m_h_res = ST7796_HEIGHT;
                break;
            case 2:
                spi_TFT->write(ST7796_MADCTL_MY | ST7796_MADCTL_BGR);
                m_h_res = ST7796_WIDTH;
                m_v_res = ST7796_HEIGHT;
                break;
            case 3:
                spi_TFT->write(ST7796_MADCTL_MX | ST7796_MADCTL_MY | ST7796_MADCTL_MV | ST7796_MADCTL_BGR);
                m_v_res = ST7796_WIDTH;
                m_h_res = ST7796_HEIGHT;
                break;
        }
        endWrite();
    }
    if(_TFTcontroller == ST7796RPI) {
        _rotation = m % 4; // can't be higher than 3
        startWrite();
        writeCommand(ST7796_MADCTL);
        switch(_rotation) {
            case 0:
                spi_TFT->write16(ST7796_MADCTL_MX | ST7796_MADCTL_BGR);
                m_h_res = ST7796_WIDTH;
                m_v_res = ST7796_HEIGHT;
                break;
            case 1:
                spi_TFT->write16(ST7796_MADCTL_MV | ST7796_MADCTL_BGR);
                m_v_res = ST7796_WIDTH;
                m_h_res = ST7796_HEIGHT;
                break;
            case 2:
                spi_TFT->write16(ST7796_MADCTL_MY | ST7796_MADCTL_BGR);
                m_h_res = ST7796_WIDTH;
                m_v_res = ST7796_HEIGHT;
                break;
            case 3:
                spi_TFT->write16(ST7796_MADCTL_MX | ST7796_MADCTL_MY | ST7796_MADCTL_MV | ST7796_MADCTL_BGR);
                m_v_res = ST7796_WIDTH;
                m_h_res = ST7796_HEIGHT;
                break;
        }
        endWrite();
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::invertDisplay(bool i) {
    startWrite();
    if(_TFTcontroller == ILI9341) { writeCommand(i ? ILI9341_INVON : ILI9341_INVOFF); }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) { writeCommand(i ? ILI9486_INVON : ILI9486_INVOFF); }
    if(_TFTcontroller == ILI9488) { writeCommand(i ? ILI9488_INVON : ILI9488_INVOFF); }
    if(_TFTcontroller == ST7796 || _TFTcontroller == ST7796RPI) { writeCommand(i ? ST7796_INVON : ST7796_INVOFF); }
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::scrollTo(uint16_t y) {
    if(_TFTcontroller != ILI9341) return;
    startWrite();
    writeCommand(ILI9341_VSCRSADD);
    spi_TFT->write16(y);
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    if(_TFTcontroller == ILI9341) { // ILI9341
        uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
        uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);
        writeCommand(ILI9341_CASET);
        spi_TFT->write32(xa);
        writeCommand(ILI9341_RASET);
        spi_TFT->write32(ya);
        writeCommand(ILI9341_RAMWR);
    }
    if(_TFTcontroller == HX8347D) { // HX8347D
        writeCommand(0x02);
        spi_TFT->write(x >> 8);
        writeCommand(0x03);
        spi_TFT->write(x & 0xFF); // Column Start
        writeCommand(0x04);
        spi_TFT->write((x + w - 1) >> 8);
        writeCommand(0x05);
        spi_TFT->write((x + w - 1) & 0xFF); // Column End
        writeCommand(0x06);
        spi_TFT->write(y >> 8);
        writeCommand(0x07);
        spi_TFT->write(y & 0xFF); // Row Start
        writeCommand(0x08);
        spi_TFT->write((y + h - 1) >> 8);
        writeCommand(0x09);
        spi_TFT->write((y + h - 1) & 0xFF); // Row End
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) {
        writeCommand(ILI9486_CASET); // Column addr set
        spi_TFT->write16(x >> 8);
        spi_TFT->write16(x & 0xFF); // XSTART
        w = x + w - 1;
        spi_TFT->write16(w >> 8);
        spi_TFT->write16(w & 0xFF);  // XEND
        writeCommand(ILI9486_PASET); // Row addr set
        spi_TFT->write16(y >> 8);
        spi_TFT->write16(y & 0xFF); // YSTART
        h = y + h - 1;
        spi_TFT->write16(h >> 8);
        spi_TFT->write16(h & 0xFF); // YEND
        writeCommand(ILI9486_RAMWR);
    }
    if(_TFTcontroller == ILI9488) {
        writeCommand(ILI9488_CASET); // Column addr set
        spi_TFT->write(x >> 8);
        spi_TFT->write(x & 0xFF); // XSTART
        w = x + w - 1;
        spi_TFT->write(w >> 8);
        spi_TFT->write(w & 0xFF);    // XEND
        writeCommand(ILI9488_PASET); // Row addr set
        spi_TFT->write(y >> 8);
        spi_TFT->write(y & 0xFF); // YSTART
        h = y + h - 1;
        spi_TFT->write(h >> 8);
        spi_TFT->write(h & 0xFF); // YEND
        writeCommand(ILI9488_RAMWR);
    }
    if(_TFTcontroller == ST7796) {
        writeCommand(ST7796_CASET); // Column addr set
        spi_TFT->write(x >> 8);
        spi_TFT->write(x & 0xFF); // XSTART
        w = x + w - 1;
        spi_TFT->write(w >> 8);
        spi_TFT->write(w & 0xFF);   // XEND
        writeCommand(ST7796_RASET); // Row addr set
        spi_TFT->write(y >> 8);
        spi_TFT->write(y & 0xFF); // YSTART
        h = y + h - 1;
        spi_TFT->write(h >> 8);
        spi_TFT->write(h & 0xFF); // YEND
        writeCommand(ST7796_RAMWR);
    }
    if(_TFTcontroller == ST7796RPI) {
        writeCommand(ST7796_CASET); // Column addr set
        spi_TFT->write16(x >> 8);
        spi_TFT->write16(x & 0xFF); // XSTART
        w = x + w - 1;
        spi_TFT->write16(w >> 8);
        spi_TFT->write16(w & 0xFF); // XEND
        writeCommand(ST7796_RASET); // Row addr set
        spi_TFT->write16(y >> 8);
        spi_TFT->write16(y & 0xFF); // YSTART
        h = y + h - 1;
        spi_TFT->write16(h >> 8);
        spi_TFT->write16(h & 0xFF); // YEND
        writeCommand(ST7796_RAMWR);
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::readAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    if(_TFTcontroller == ILI9341) { // ILI9341
        uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
        uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);
        writeCommand(ILI9341_CASET);
        spi_TFT->write32(xa);
        writeCommand(ILI9341_RASET);
        spi_TFT->write32(ya);
        writeCommand(ILI9341_RAMRD);
    }
    if(_TFTcontroller == HX8347D) { // HX8347D
        writeCommand(0x02);
        spi_TFT->write(x >> 8);
        writeCommand(0x03);
        spi_TFT->write(x & 0xFF); // Column Start
        writeCommand(0x04);
        spi_TFT->write((x + w - 1) >> 8);
        writeCommand(0x05);
        spi_TFT->write((x + w - 1) & 0xFF); // Column End
        writeCommand(0x06);
        spi_TFT->write(y >> 8);
        writeCommand(0x07);
        spi_TFT->write(y & 0xFF); // Row Start
        writeCommand(0x08);
        spi_TFT->write((y + h - 1) >> 8);
        writeCommand(0x09);
        spi_TFT->write((y + h - 1) & 0xFF); // Row End
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) {
        writeCommand(ILI9486_CASET); // Column addr set
        spi_TFT->write16(x >> 8);
        spi_TFT->write16(x & 0xFF); // XSTART
        w = x + w - 1;
        spi_TFT->write16(w >> 8);
        spi_TFT->write16(w & 0xFF);  // XEND
        writeCommand(ILI9486_PASET); // Row addr set
        spi_TFT->write16(y >> 8);
        spi_TFT->write16(y & 0xFF); // YSTART
        h = y + h - 1;
        spi_TFT->write16(h >> 8);
        spi_TFT->write16(h & 0xFF); // YEND
        writeCommand(ILI9486_RAMRD);
    }
    if(_TFTcontroller == ILI9488) {
        writeCommand(ILI9488_CASET); // Column addr set
        spi_TFT->write(x >> 8);
        spi_TFT->write(x & 0xFF); // XSTART
        w = x + w - 1;
        spi_TFT->write(w >> 8);
        spi_TFT->write(w & 0xFF);    // XEND
        writeCommand(ILI9488_PASET); // Row addr set
        spi_TFT->write(y >> 8);
        spi_TFT->write(y & 0xFF); // YSTART
        h = y + h - 1;
        spi_TFT->write(h >> 8);
        spi_TFT->write(h & 0xFF); // YEND
        writeCommand(ILI9488_RAMRD);
    }
    if(_TFTcontroller == ST7796) {
        writeCommand(ST7796_CASET); // Column addr set
        spi_TFT->write(x >> 8);
        spi_TFT->write(x & 0xFF); // XSTART
        w = x + w - 1;
        spi_TFT->write(w >> 8);
        spi_TFT->write(w & 0xFF);   // XEND
        writeCommand(ST7796_RASET); // Row addr set
        spi_TFT->write(y >> 8);
        spi_TFT->write(y & 0xFF); // YSTART
        h = y + h - 1;
        spi_TFT->write(h >> 8);
        spi_TFT->write(h & 0xFF); // YEND
        writeCommand(ST7796_RAMRD);
    }
    if(_TFTcontroller == ST7796RPI) {
        writeCommand(ST7796_CASET); // Column addr set
        spi_TFT->write16(x >> 8);
        spi_TFT->write16(x & 0xFF); // XSTART
        w = x + w - 1;
        spi_TFT->write16(w >> 8);
        spi_TFT->write16(w & 0xFF); // XEND
        writeCommand(ST7796_RASET); // Row addr set
        spi_TFT->write16(y >> 8);
        spi_TFT->write16(y & 0xFF); // YSTART
        h = y + h - 1;
        spi_TFT->write16(h >> 8);
        spi_TFT->write16(h & 0xFF); // YEND
        writeCommand(ST7796_RAMRD);
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::startBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {

    startWrite();
    if(_TFTcontroller == ILI9341) { // ILI9341
        writeCommand(ILI9341_MADCTL);
        spi_TFT->write(ili9341_rotations[_rotation].bmpctl);
    }
    setAddrWindow(x, m_v_res - y - h, w, h);
    if(_TFTcontroller == HX8347D) { // HX8347D
        if(_rotation == 0) {
            writeCommand(0x16);
            spi_TFT->write(0x88);
        } // 0
        if(_rotation == 1) {
            writeCommand(0x16);
            spi_TFT->write(0x38);
        } // 90
        if(_rotation == 2) {
            writeCommand(0x16);
            spi_TFT->write(0x48);
        } // 180
        if(_rotation == 3) {
            writeCommand(0x16);
            spi_TFT->write(0xE8);
        } // 270
        writeCommand(0x22);
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) {
        writeCommand(ILI9486_MADCTL);
        if(_rotation == 0) { spi_TFT->write16(ILI9486_MADCTL_MX | ILI9486_MADCTL_MY | ILI9486_MADCTL_ML | ILI9486_MADCTL_BGR); }
        if(_rotation == 1) { spi_TFT->write16(ILI9486_MADCTL_MH | ILI9486_MADCTL_MV | ILI9486_MADCTL_MX | ILI9486_MADCTL_BGR); }
        if(_rotation == 2) { spi_TFT->write16(ILI9486_MADCTL_MH | ILI9486_MADCTL_BGR); }
        if(_rotation == 3) { spi_TFT->write16(ILI9486_MADCTL_MV | ILI9486_MADCTL_MY | ILI9486_MADCTL_BGR); }
        writeCommand(ILI9486_RAMWR);
    }
    if(_TFTcontroller == ILI9488) {
        writeCommand(ILI9488_MADCTL);
        if(_rotation == 0) { spi_TFT->write(ILI9488_MADCTL_MX | ILI9488_MADCTL_MY | ILI9488_MADCTL_ML | ILI9488_MADCTL_BGR); }
        if(_rotation == 1) { spi_TFT->write(ILI9488_MADCTL_MH | ILI9488_MADCTL_MV | ILI9488_MADCTL_MX | ILI9488_MADCTL_BGR); }
        if(_rotation == 2) { spi_TFT->write(ILI9488_MADCTL_MH | ILI9488_MADCTL_BGR); }
        if(_rotation == 3) { spi_TFT->write(ILI9488_MADCTL_MV | ILI9488_MADCTL_MY | ILI9488_MADCTL_BGR); }
        writeCommand(ILI9488_RAMWR);
    }
    if(_TFTcontroller == ST7796) {
        writeCommand(ST7796_MADCTL);
        if(_rotation == 0) { spi_TFT->write(ST7796_MADCTL_MX | ST7796_MADCTL_MY | ST7796_MADCTL_ML | ST7796_MADCTL_BGR); }
        if(_rotation == 1) { spi_TFT->write(ST7796_MADCTL_MH | ST7796_MADCTL_MV | ST7796_MADCTL_MX | ST7796_MADCTL_BGR); }
        if(_rotation == 2) { spi_TFT->write(ST7796_MADCTL_MH | ST7796_MADCTL_BGR); }
        if(_rotation == 3) { spi_TFT->write(ST7796_MADCTL_MV | ST7796_MADCTL_MY | ST7796_MADCTL_BGR); }
        writeCommand(ST7796_RAMWR);
    }
    if(_TFTcontroller == ST7796RPI) {
        writeCommand(ST7796_MADCTL);
        if(_rotation == 0) { spi_TFT->write16(ST7796_MADCTL_MX | ST7796_MADCTL_MY | ST7796_MADCTL_ML | ST7796_MADCTL_BGR); }
        if(_rotation == 1) { spi_TFT->write16(ST7796_MADCTL_MH | ST7796_MADCTL_MV | ST7796_MADCTL_MX | ST7796_MADCTL_BGR); }
        if(_rotation == 2) { spi_TFT->write16(ST7796_MADCTL_MH | ST7796_MADCTL_BGR); }
        if(_rotation == 3) { spi_TFT->write16(ST7796_MADCTL_MV | ST7796_MADCTL_MY | ST7796_MADCTL_BGR); }
        writeCommand(ST7796_RAMWR);
    }
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::endBitmap() {
    if(_TFTcontroller == ILI9341) { // ILI9341
        startWrite();
        writeCommand(ILI9341_MADCTL);
        spi_TFT->write(ili9341_rotations[_rotation].madctl);
        // setAddrWindow(x, m_h_res - y - h, w, h);
        endWrite();
    }
    if(_TFTcontroller == HX8347D) { // HX8347D
        setRotation(_rotation);     // return to old values
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) { setRotation(_rotation); }
    if(_TFTcontroller == ILI9488) { setRotation(_rotation); }
    if(_TFTcontroller == ST7796 || _TFTcontroller == ST7796RPI) { setRotation(_rotation); }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::startJpeg() {
    startWrite();
    if(_TFTcontroller == ILI9341) { // ILI9341
        writeCommand(ILI9341_MADCTL);
    }
    if(_TFTcontroller == HX8347D) { // HX8347D
        // nothing to do
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) {
        writeCommand(ILI9486_MADCTL);
        if(_rotation == 0) { spi_TFT->write16(ILI9486_MADCTL_MH | ILI9486_MADCTL_MX | ILI9486_MADCTL_BGR); }
        if(_rotation == 1) { spi_TFT->write16(ILI9486_MADCTL_MV | ILI9486_MADCTL_BGR); }
        if(_rotation == 2) { spi_TFT->write16(ILI9486_MADCTL_MY | ILI9486_MADCTL_BGR); }
        if(_rotation == 3) { spi_TFT->write16(ILI9486_MADCTL_MV | ILI9486_MADCTL_MY | ILI9486_MADCTL_MX | ILI9486_MADCTL_BGR); }
    }
    if(_TFTcontroller == ILI9488) {
        writeCommand(ILI9488_MADCTL);
        if(_rotation == 0) { spi_TFT->write(ILI9488_MADCTL_MH | ILI9488_MADCTL_MX | ILI9488_MADCTL_BGR); }
        if(_rotation == 1) { spi_TFT->write(ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR); }
        if(_rotation == 2) { spi_TFT->write(ILI9488_MADCTL_MY | ILI9488_MADCTL_BGR); }
        if(_rotation == 3) { spi_TFT->write(ILI9488_MADCTL_MV | ILI9488_MADCTL_MY | ILI9488_MADCTL_MX | ILI9488_MADCTL_BGR); }
    }
    if(_TFTcontroller == ST7796) {
        writeCommand(ST7796_MADCTL);
        if(_rotation == 0) { spi_TFT->write(ST7796_MADCTL_MH | ST7796_MADCTL_MX | ST7796_MADCTL_BGR); }
        if(_rotation == 1) { spi_TFT->write(ST7796_MADCTL_MV | ST7796_MADCTL_BGR); }
        if(_rotation == 2) { spi_TFT->write(ST7796_MADCTL_MY | ST7796_MADCTL_BGR); }
        if(_rotation == 3) { spi_TFT->write(ST7796_MADCTL_MV | ST7796_MADCTL_MY | ST7796_MADCTL_MX | ST7796_MADCTL_BGR); }
    }
    if(_TFTcontroller == ST7796RPI) {
        writeCommand(ST7796_MADCTL);
        if(_rotation == 0) { spi_TFT->write16(ST7796_MADCTL_MH | ST7796_MADCTL_MX | ST7796_MADCTL_BGR); }
        if(_rotation == 1) { spi_TFT->write16(ST7796_MADCTL_MV | ST7796_MADCTL_BGR); }
        if(_rotation == 2) { spi_TFT->write16(ST7796_MADCTL_MY | ST7796_MADCTL_BGR); }
        if(_rotation == 3) { spi_TFT->write16(ST7796_MADCTL_MV | ST7796_MADCTL_MY | ST7796_MADCTL_MX | ST7796_MADCTL_BGR); }
    }
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::endJpeg() { setRotation(_rotation); }
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::writePixels(uint16_t* colors, uint32_t len) {
    if((_TFTcontroller == ILI9488) || (_TFTcontroller == ST7796)) {
        uint32_t i = 0;
        while(len) {
            write24BitColor(*(colors + i));
            i++;
            len--;
        }
    }
    else { spi_TFT->writePixels((uint8_t*)colors, len * 2); }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::writeColor(uint16_t color, uint32_t len) {
    if((_TFTcontroller == ILI9488) || (_TFTcontroller == ST7796)) {
        uint8_t r = (color & 0xF800) >> 8;
        uint8_t g = (color & 0x07E0) >> 3;
        uint8_t b = (color & 0x001F) << 3;
        uint8_t c[3] = {r, g, b};
        spi_TFT->writePattern(c, 3, len);
    }
    else {
        uint8_t c[2];
        c[0] = (color & 0xFF00) >> 8;
        c[1] = color & 0x00FF;
        spi_TFT->writePattern(c, 2, len);
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::write24BitColor(uint16_t color) {
    spi_TFT->write((color & 0xF800) >> 8); // r
    spi_TFT->write((color & 0x07E0) >> 3); // g
    spi_TFT->write((color & 0x001F) << 3); // b
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::writePixel(int16_t x, int16_t y, uint16_t color) {
    if((x < 0) || (x >= m_v_res) || (y < 0) || (y >= m_h_res)) return;
    setAddrWindow(x, y, 1, 1);
    switch(_TFTcontroller) {
        case ILI9341: spi_TFT->write16(color); break;
        case HX8347D:
            writeCommand(0x22); spi_TFT->write16(color);
            break;
        case ILI9486a:
            writeCommand(ILI9486_RAMWR); spi_TFT->write16(color);
            break;
        case ILI9486b:
            writeCommand(ILI9486_RAMWR); spi_TFT->write16(color);
            break;
        case ILI9488:
            writeCommand(ILI9488_RAMWR); write24BitColor(color);
            break;
        case ST7796:
            writeCommand(ST7796_RAMWR); write24BitColor(color);
            break;
        case ST7796RPI:
            writeCommand(ST7796_RAMWR); spi_TFT->write16(color);
            break;
        default:
            if(tft_info) tft_info("unknown tft controller");
            break;
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if((x >= m_h_res) || (y >= m_v_res)) return;
    int16_t x2 = x + w - 1, y2 = y + h - 1;
    if((x2 < 0) || (y2 < 0)) return;

    // Clip left/top
    if(x < 0) {
        x = 0;
        w = x2 + 1;
    }
    if(y < 0) {
        y = 0;
        h = y2 + 1;
    }

    // Clip right/bottom
    if(x2 >= m_h_res) w = m_h_res - x;
    if(y2 >= m_v_res) h = m_v_res - y;

    int32_t len = (int32_t)w * h;
    setAddrWindow(x, y, w, h);
    if(_TFTcontroller == HX8347D) writeCommand(0x22);
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) writeCommand(ILI9486_RAMWR);
    if(_TFTcontroller == ILI9488) writeCommand(ILI9488_RAMWR);
    if(_TFTcontroller == ST7796 || _TFTcontroller == ST7796RPI) writeCommand(ST7796_RAMWR);
    writeColor(color, len);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) { writeFillRect(x, y, 1, h, color); }
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) { writeFillRect(x, y, w, 1, color); }
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT::readcommand8(uint8_t c, uint8_t index) {
    uint32_t freq = _freq;
    if(_freq > 24000000) { _freq = 24000000; }
    startWrite();
    writeCommand(0xD9); // woo sekret command?
    spi_TFT->write(0x10 + index);
    writeCommand(c);
    uint8_t r = spi_TFT->transfer(0);
    endWrite();
    _freq = freq;
    return r;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::drawPixel(int16_t x, int16_t y, uint16_t color) {
    startWrite();
    writePixel(x, y, color);
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT::color565(uint8_t r, uint8_t g, uint8_t b) { return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3); }
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    startWrite();
    writeFastVLine(x, y, h, color);
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    startWrite();
    writeFastHLine(x, y, w, color);
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
#ifdef RGB_HMI
    // Calculate differences
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);

    // Determine directions
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;

    int16_t err = dx - dy; // Fehlerterm

    while (true) {
        // Set point in framebuffer if within bounds
        if (x0 >= 0 && x0 < m_h_res && y0 >= 0 && y0 < m_v_res) {
            m_framebuffer[y0 * m_h_res + x0] = color;
        }

        // Wenn Endpunkt erreicht, beenden
        if (x0 == x1 && y0 == y1) break;

        // Add error term twice and correct errors
        int16_t e2 = err * 2;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }

    // Update on the display (only the drawn area)
    int16_t update_x0 = std::min(x0, x1);
    int16_t update_y0 = std::min(y0, y1);
    int16_t update_x1 = std::max(x0, x1) + 1;
    int16_t update_y1 = std::max(y0, y1) + 1;
    esp_lcd_panel_draw_bitmap(m_panel, update_x0, update_y0, update_x1, update_y1, m_framebuffer);
#endif
#ifdef LCD_SPI
    // Bresenham's algorithm - thx wikipedia - speed enhanced by Bodmer to use
    // an efficient FastH/V Line draw routine for line segments of 2 pixels or more
    int16_t t;
    bool    steep = abs(y1 - y0) > abs(x1 - x0);
    if(steep) {
        t = x0;
        x0 = y0;
        y0 = t; // swap (x0, y0);
        t = x1;
        x1 = y1;
        y1 = t; // swap(x1, y1);
    }
    if(x0 > x1) {
        t = x0;
        x0 = x1;
        x1 = t; // swap(x0, x1);
        t = y0;
        y0 = y1;
        y1 = t; // swap(y0, y1);
    }
    int16_t dx = x1 - x0, dy = abs(y1 - y0);
    ;
    int16_t err = dx >> 1, ystep = -1, xs = x0, dlen = 0;

    if(y0 < y1) ystep = 1;
    startWrite();
    // Split into steep and not steep for FastH/V separation
    if(steep) {
        for(; x0 <= x1; x0++) {
            dlen++;
            err -= dy;
            if(err < 0) {
                err += dx;
                if(dlen == 1) writePixel(y0, xs, color);
                else writeFastVLine(y0, xs, dlen, color);
                dlen = 0;
                y0 += ystep;
                xs = x0 + 1;
            }
        }
        if(dlen) writeFastVLine(y0, xs, dlen, color);
    }
    else {
        for(; x0 <= x1; x0++) {
            dlen++;
            err -= dy;
            if(err < 0) {
                err += dx;
                if(dlen == 1) writePixel(xs, y0, color);
                else writeFastHLine(xs, y0, dlen, color);
                dlen = 0;
                y0 += ystep;
                xs = x0 + 1;
            }
        }
        if(dlen) writeFastHLine(xs, y0, dlen, color);
    }
    endWrite();
#endif
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::fillScreen(uint16_t color) {
    fillRect(0, 0, m_h_res, m_v_res, color);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
#ifdef RGB_HMI
    // Clipping: Rechteck-Koordinaten auf den Framebuffer-Bereich beschränken
    int16_t x0 = max((int16_t)0, x);
    int16_t y0 = max((int16_t)0, y);
    int16_t x1 = min((int)m_h_res, x + w); // Rechte Grenze
    int16_t y1 = min((int)m_v_res, y + h); // Untere Grenze
    // Zeichnen des Rechtecks nur im gültigen Bereich
    for (int16_t j = y0; j < y1; ++j) { // Zeilen iterieren
        for (int16_t i = x0; i < x1; ++i) { // Spalten iterieren
            m_framebuffer[j * m_h_res + i] = color;
        }
    }
    esp_lcd_panel_draw_bitmap(m_panel, x0, y0, x1, y1, m_framebuffer);
#endif
#ifdef LCD_SPI
    startWrite();
    writeFillRect(x, y, w, h, color);
    endWrite();
#endif
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
#ifdef RGB_HMI
    auto drawLine = [](int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, uint16_t* m_framebuffer, uint16_t m_h_res) {
        // Bresenham-Algorithmus für Linien
        int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int16_t err = dx + dy, e2; // Fehlerwert

        while (true) {
            m_framebuffer[y0 * m_h_res + x0] = color; // Pixel setzen
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    };


    // Zeichne die drei Linien des Dreiecks
    drawLine(x0, y0, x1, y1, color, m_framebuffer, m_h_res); // Linie von Punkt 0 nach Punkt 1
    drawLine(x1, y1, x2, y2, color, m_framebuffer, m_h_res); // Linie von Punkt 1 nach Punkt 2
    drawLine(x2, y2, x0, y0, color, m_framebuffer, m_h_res); // Linie von Punkt 2 nach Punkt 0

    // Aktualisierung des gezeichneten Bereichs
    int16_t x_min = std::min({x0, x1, x2});
    int16_t y_min = std::min({y0, y1, y2});
    int16_t x_max = std::max({x0, x1, x2});
    int16_t y_max = std::max({y0, y1, y2});
    esp_lcd_panel_draw_bitmap(m_panel, x_min, y_min, x_max + 1, y_max + 1, m_framebuffer);
#endif
#ifdef LCD_SPI
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
#endif
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
#ifdef RGB_HMI
 // Helferfunktion zum Zeichnen einer horizontalen Linie
    auto drawHorizontalLine = [&](int16_t x_start, int16_t x_end, int16_t y) {
        if (y >= 0 && y < m_v_res) { // Clipping in y-Richtung
            if (x_start > x_end) std::swap(x_start, x_end);
            x_start = std::max((int16_t)0, x_start); // Clipping in x-Richtung
            x_end = std::min((int16_t)(m_h_res - 1), x_end);
            for (int16_t x = x_start; x <= x_end; ++x) {
                m_framebuffer[y * m_h_res + x] = color;
            }
        }
    };

    // Punkte nach ihrer y-Koordinate sortieren
    if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }
    if (y1 > y2) { std::swap(y1, y2); std::swap(x1, x2); }
    if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }

    // Variablen zur Begrenzung des aktualisierten Bereichs
    int16_t x_min = std::min({x0, x1, x2});
    int16_t x_max = std::max({x0, x1, x2});
    int16_t y_min = std::min({y0, y1, y2});
    int16_t y_max = std::max({y0, y1, y2});

    // Clipping auf Framebuffer-Grenzen
    x_min = std::max((int16_t)0, x_min);
    x_max = std::min((int16_t)(m_h_res - 1), x_max);
    y_min = std::max((int16_t)0, y_min);
    y_max = std::min((int16_t)(m_v_res - 1), y_max);

    // Dreieck in zwei Teile zerlegen (oben und unten)
    if (y1 == y2) { // Sonderfall: flaches unteres Dreieck
        for (int16_t y = y0; y <= y1; ++y) {
            int16_t x_start = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            int16_t x_end = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            drawHorizontalLine(x_start, x_end, y);
        }
    } else if (y0 == y1) { // Sonderfall: flaches oberes Dreieck
        for (int16_t y = y0; y <= y2; ++y) {
            int16_t x_start = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            int16_t x_end = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
            drawHorizontalLine(x_start, x_end, y);
        }
    } else { // Allgemeiner Fall: Dreieck wird in zwei Teile aufgeteilt
        for (int16_t y = y0; y <= y1; ++y) { // Unterer Teil
            int16_t x_start = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            int16_t x_end = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            drawHorizontalLine(x_start, x_end, y);
        }
        for (int16_t y = y1; y <= y2; ++y) { // Oberer Teil
            int16_t x_start = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
            int16_t x_end = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            drawHorizontalLine(x_start, x_end, y);
        }
    }

    // Aktualisierung nur des geänderten Bereichs
    esp_lcd_panel_draw_bitmap(m_panel, x_min, y_min, x_max + 1, y_max + 1, m_framebuffer);
#endif
#ifdef LCD_SPI
    int16_t a, b, y, last;
    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if(y0 > y1) {
        _swap_int16_t(y0, y1);
        _swap_int16_t(x0, x1);
    }
    if(y1 > y2) {
        _swap_int16_t(y2, y1);
        _swap_int16_t(x2, x1);
    }
    if(y0 > y1) {
        _swap_int16_t(y0, y1);
        _swap_int16_t(x0, x1);
    }
    startWrite();
    if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if(x1 < a) a = x1;
        else if(x1 > b) b = x1;
        if(x2 < a) a = x2;
        else if(x2 > b) b = x2;
        writeFastHLine(a, y0, b - a + 1, color);
        endWrite();
        return;
    }
    int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0, dx12 = x2 - x1, dy12 = y2 - y1;
    int32_t sa = 0, sb = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if(y1 == y2) last = y1; // Include y1 scanline
    else last = y1 - 1;     // Skip it

    for(y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
         a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
         b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
         */
        if(a > b) _swap_int16_t(a, b);
        writeFastHLine(a, y, b - a + 1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = (int32_t)dx12 * (y - y1);
    sb = (int32_t)dx02 * (y - y0);
    for(; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
         a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
         b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
         */
        if(a > b) _swap_int16_t(a, b);
        writeFastHLine(a, y, b - a + 1, color);
    }
    endWrite();
#endif
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::drawRect(int16_t Xpos, int16_t Ypos, uint16_t Width, uint16_t Height, uint16_t Color) {
#ifdef RGB_HMI
    auto drawLine = [](int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, uint16_t* m_framebuffer, uint16_t m_h_res) {
        // Bresenham-Algorithmus für Linien
        int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int16_t err = dx + dy, e2; // Fehlerwert

        while (true) {
            m_framebuffer[y0 * m_h_res + x0] = color; // Pixel setzen
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    };

    // Zeichne die vier Linien des Rechtecks
    drawLine(Xpos, Ypos, Xpos + Width, Ypos, Color, m_framebuffer, m_h_res); // Oben
    drawLine(Xpos + Width, Ypos, Xpos + Width, Ypos + Height, Color, m_framebuffer, m_h_res); // Rechts
    drawLine(Xpos + Width, Ypos + Height, Xpos, Ypos + Height, Color,  m_framebuffer, m_h_res); // Unten
    drawLine(Xpos, Ypos + Height, Xpos, Ypos, Color,  m_framebuffer, m_h_res); // Links

    // Aktualisierung des gezeichneten Bereichs
    int16_t x_min = std::min((int)Xpos, Xpos + Width);
    int16_t y_min = std::min((int)Ypos, Ypos + Height);
    int16_t x_max = std::max((int)Xpos, Xpos + Width);
    int16_t y_max = std::max((int)Ypos, Ypos + Height);

    esp_lcd_panel_draw_bitmap(m_panel, x_min, y_min, x_max + 1, y_max + 1, m_framebuffer);
#endif
#ifdef LCD_SPI
    if(Width < 1 || Height < 1) return;
    startWrite();
    writeFastHLine(Xpos, Ypos, Width, Color);
    writeFastHLine(Xpos, Ypos + Height - 1, Width, Color);
    writeFastVLine(Xpos, Ypos, Height, Color);
    writeFastVLine(Xpos + Width - 1, Ypos, Height, Color);
    endWrite();
#endif
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
#ifdef RGB_HMI
    // Helferfunktion: Kreislinie für die Ecken berechnen
    auto drawCircleQuadrant = [&](int16_t cx, int16_t cy, int16_t r, uint8_t quadrant) {
        int16_t f = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x = 0;
        int16_t y = r;

        while (x <= y) {
            if (quadrant & 0x1) m_framebuffer[(cy - y) * m_h_res + (cx + x)] = color; // oben rechts
            if (quadrant & 0x2) m_framebuffer[(cy + y) * m_h_res + (cx + x)] = color; // unten rechts
            if (quadrant & 0x4) m_framebuffer[(cy + y) * m_h_res + (cx - x)] = color; // unten links
            if (quadrant & 0x8) m_framebuffer[(cy - y) * m_h_res + (cx - x)] = color; // oben links

            if (quadrant & 0x10) m_framebuffer[(cy - x) * m_h_res + (cx + y)] = color; // oben rechts (90° gedreht)
            if (quadrant & 0x20) m_framebuffer[(cy + x) * m_h_res + (cx + y)] = color; // unten rechts (90° gedreht)
            if (quadrant & 0x40) m_framebuffer[(cy + x) * m_h_res + (cx - y)] = color; // unten links (90° gedreht)
            if (quadrant & 0x80) m_framebuffer[(cy - x) * m_h_res + (cx - y)] = color; // oben links (90° gedreht)

            if (f >= 0) {
                y--;
                ddF_y += 2;
                f += ddF_y;
            }
            x++;
            ddF_x += 2;
            f += ddF_x;
        }
    };

    // Rechteckseiten zeichnen (ohne die abgerundeten Ecken)
    for (int16_t i = x + r; i < x + w - r; i++) { // Obere und untere horizontale Linien
        m_framebuffer[y * m_h_res + i] = color; // Oben
        m_framebuffer[(y + h - 1) * m_h_res + i] = color; // Unten
    }
    for (int16_t i = y + r; i < y + h - r; i++) { // Linke und rechte vertikale Linien
        m_framebuffer[i * m_h_res + x] = color; // Links
        m_framebuffer[i * m_h_res + (x + w - 1)] = color; // Rechts
    }

    // Abgerundete Ecken zeichnen
    drawCircleQuadrant(x + w - r - 1, y + r, r, 0x1 | 0x10); // Oben rechts
    drawCircleQuadrant(x + w - r - 1, y + h - r - 1, r, 0x2 | 0x20); // Unten rechts
    drawCircleQuadrant(x + r, y + h - r - 1, r, 0x4 | 0x40); // Unten links
    drawCircleQuadrant(x + r, y + r, r, 0x8 | 0x80); // Oben links

    // Aktualisierung des gezeichneten Bereichs
    esp_lcd_panel_draw_bitmap(m_panel, x, y, x + w, y + h, m_framebuffer);
#endif
#ifdef LCD_SPI
    // smarter version
    startWrite();
    writeFastHLine(x + r, y, w - 2 * r, color);         // Top
    writeFastHLine(x + r, y + h - 1, w - 2 * r, color); // Bottom
    writeFastVLine(x, y + r, h - 2 * r, color);         // Left
    writeFastVLine(x + w - 1, y + r, h - 2 * r, color); // Right
    // draw four corners
    drawCircleHelper(x + r, y + r, r, 1, color);
    drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
    drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
    drawCircleHelper(x + r, y + h - r - 1, r, 8, color);
    endWrite();
#endif
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
#ifdef RGB_HMI
    // Helferfunktion: Kreisfüllung für die Ecken berechnen
    auto fillCircleQuadrant = [&](int16_t cx, int16_t cy, int16_t r, uint8_t quadrant) {
        int16_t f = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x = 0;
        int16_t y = r;

        while (x <= y) {
            for (int16_t i = 0; i <= x; i++) {
                if (quadrant & 0x1) m_framebuffer[(cy - y) * m_h_res + (cx + i)] = color; // oben rechts
                if (quadrant & 0x2) m_framebuffer[(cy + y) * m_h_res + (cx + i)] = color; // unten rechts
                if (quadrant & 0x4) m_framebuffer[(cy + y) * m_h_res + (cx - i)] = color; // unten links
                if (quadrant & 0x8) m_framebuffer[(cy - y) * m_h_res + (cx - i)] = color; // oben links
            }
            for (int16_t i = 0; i <= y; i++) {
                if (quadrant & 0x10) m_framebuffer[(cy - x) * m_h_res + (cx + i)] = color; // oben rechts (gedreht)
                if (quadrant & 0x20) m_framebuffer[(cy + x) * m_h_res + (cx + i)] = color; // unten rechts (gedreht)
                if (quadrant & 0x40) m_framebuffer[(cy + x) * m_h_res + (cx - i)] = color; // unten links (gedreht)
                if (quadrant & 0x80) m_framebuffer[(cy - x) * m_h_res + (cx - i)] = color; // oben links (gedreht)
            }

            if (f >= 0) {
                y--;
                ddF_y += 2;
                f += ddF_y;
            }
            x++;
            ddF_x += 2;
            f += ddF_x;
        }
    };

    // Horizontale Bereiche zwischen den oberen und unteren Viertelkreisen füllen
    for (int16_t i = y; i < y + r; i++) { // Bereich oberhalb der Viertelkreise
        for (int16_t j = x + r; j < x + w - r; j++) {
            m_framebuffer[i * m_h_res + j] = color;
        }
    }
    for (int16_t i = y + h - r; i < y + h; i++) { // Bereich unterhalb der Viertelkreise
        for (int16_t j = x + r; j < x + w - r; j++) {
            m_framebuffer[i * m_h_res + j] = color;
        }
    }

    // Vertikaler Bereich zwischen den Viertelkreisen füllen
    for (int16_t i = y + r; i < y + h - r; i++) { // Vertikaler Bereich
        for (int16_t j = x; j < x + w; j++) { // Horizontaler Bereich
            m_framebuffer[i * m_h_res + j] = color;
        }
    }

    // Viertelkreise in den Ecken füllen
    fillCircleQuadrant(x + w - r - 1, y + r, r, 0x1 | 0x10); // Oben rechts
    fillCircleQuadrant(x + w - r - 1, y + h - r - 1, r, 0x2 | 0x20); // Unten rechts
    fillCircleQuadrant(x + r, y + h - r - 1, r, 0x4 | 0x40); // Unten links
    fillCircleQuadrant(x + r, y + r, r, 0x8 | 0x80); // Oben links

    // Aktualisierung des gezeichneten Bereichs
    esp_lcd_panel_draw_bitmap(m_panel, x, y, x + w, y + h, m_framebuffer);
#endif
#ifdef LCD_SPI
    // smarter version
    startWrite();
    writeFillRect(x + r, y, w - 2 * r, h, color);

    // draw four corners
    fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
    fillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
    endWrite();
#endif
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
#ifdef RGB_HMI
    // Bresenham-Algorithmus für Kreise
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    // Setze die Anfangspunkte (Symmetrieachsen)
    m_framebuffer[(cy + r) * m_h_res + cx] = color; // Oben
    m_framebuffer[(cy - r) * m_h_res + cx] = color; // Unten
    m_framebuffer[cy * m_h_res + (cx + r)] = color; // Rechts
    m_framebuffer[cy * m_h_res + (cx - r)] = color; // Links

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        // Punkte in den acht Symmetrieachsen zeichnen
        m_framebuffer[(cy + y) * m_h_res + (cx + x)] = color; // Quadrant 1
        m_framebuffer[(cy + y) * m_h_res + (cx - x)] = color; // Quadrant 2
        m_framebuffer[(cy - y) * m_h_res + (cx + x)] = color; // Quadrant 3
        m_framebuffer[(cy - y) * m_h_res + (cx - x)] = color; // Quadrant 4
        m_framebuffer[(cy + x) * m_h_res + (cx + y)] = color; // Quadrant 5
        m_framebuffer[(cy + x) * m_h_res + (cx - y)] = color; // Quadrant 6
        m_framebuffer[(cy - x) * m_h_res + (cx + y)] = color; // Quadrant 7
        m_framebuffer[(cy - x) * m_h_res + (cx - y)] = color; // Quadrant 8
    }

    // Aktualisierung des gezeichneten Bereichs
    esp_lcd_panel_draw_bitmap(m_panel, cx - r, cy - r, cx + r + 1, cy + r + 1, m_framebuffer);
#endif
#ifdef LCD_SPI
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    startWrite();
    writePixel(x0, y0 + r, color);
    writePixel(x0, y0 - r, color);
    writePixel(x0 + r, y0, color);
    writePixel(x0 - r, y0, color);

    while(x < y) {
        if(f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        writePixel(x0 + x, y0 + y, color);
        writePixel(x0 - x, y0 + y, color);
        writePixel(x0 + x, y0 - y, color);
        writePixel(x0 - x, y0 - y, color);
        writePixel(x0 + y, y0 + x, color);
        writePixel(x0 - y, y0 + x, color);
        writePixel(x0 + y, y0 - x, color);
        writePixel(x0 - y, y0 - x, color);
    }
    endWrite();
#endif
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::fillCircle(int16_t  Xm,    // specify x position.
                     int16_t  Ym,    // specify y position.
                     uint16_t r,     // specify the radius of the circle.
                     uint16_t color) // specify the color of the circle.
{
#ifdef RGB_HMI
//void TFT::drawFilledCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color) {
    // Bresenham-Algorithmus für Kreise
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    // Fülle die erste vertikale Linie durch den Mittelpunkt
    for (int16_t i = Ym - r; i <= Ym + r; i++) {
        m_framebuffer[i * m_h_res + Xm] = color;
    }

    while (x <= y) {
        // Fülle horizontale Linien für alle acht Symmetrieachsen
        for (int16_t i = Xm - x; i <= Xm + x; i++) {
            m_framebuffer[(Ym + y) * m_h_res + i] = color; // Unten +y
            m_framebuffer[(Ym - y) * m_h_res + i] = color; // Oben -y
        }
        for (int16_t i = Xm - y; i <= Xm + y; i++) {
            m_framebuffer[(Ym + x) * m_h_res + i] = color; // Rechts +x
            m_framebuffer[(Ym - x) * m_h_res + i] = color; // Links -x
        }

        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
    }

    // Aktualisierung des gezeichneten Bereichs
    esp_lcd_panel_draw_bitmap(m_panel, Xm - r, Ym - r, Xm + r + 1, Ym + r + 1, m_framebuffer);
#endif
#ifdef LCD_SPI
    int32_t f = 1 - r, ddF_x = 1, ddF_y = 0 - (2 * r), x = 0, y = r;
    startWrite();
    writeFastVLine(Xm, Ym - r, 2 * r, color);

    while(x < y) {
        if(f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }

        x++;
        ddF_x += 2;
        f += ddF_x;

        writeFastVLine(Xm + x, Ym - y, 2 * y, color);
        writeFastVLine(Xm - x, Ym - y, 2 * y, color);
        writeFastVLine(Xm + y, Ym - x, 2 * x, color);
        writeFastVLine(Xm - y, Ym - x, 2 * x, color);
    }
    endWrite();
#endif
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while(x < y) {
        if(f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        if(cornername & 0x4) {
            writePixel(x0 + x, y0 + y, color);
            writePixel(x0 + y, y0 + x, color);
        }
        if(cornername & 0x2) {
            writePixel(x0 + x, y0 - y, color);
            writePixel(x0 + y, y0 - x, color);
        }
        if(cornername & 0x8) {
            writePixel(x0 - y, y0 + x, color);
            writePixel(x0 - x, y0 + y, color);
        }
        if(cornername & 0x1) {
            writePixel(x0 - y, y0 - x, color);
            writePixel(x0 - x, y0 - y, color);
        }
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color) {

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while(x < y) {
        if(f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if(cornername & 0x1) {
            writeFastVLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
            writeFastVLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
        }
        if(cornername & 0x2) {
            writeFastVLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
            writeFastVLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
        }
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* data) {

    uint16_t color = 0;

    uint32_t dataSize = w * h;
    uint32_t counter = 0;
    startWrite();
    readAddrWindow(x, y, w, h);
    readCommand(); // Dummy read to throw away don't care value

    // Read window pixel 24-bit RGB values
    uint8_t r, g, b;

    while(dataSize--) {
        if(_TFTcontroller == ILI9488) {
            // The 6 colour bits are in MS 6 bits of each byte but we do not include the extra clock pulse so we use a trick
            // and mask the middle 6 bits of the byte, then only shift 1 place left
            r = (readCommand() & 0x7E) << 1;
            g = (readCommand() & 0x7E) << 1;
            b = (readCommand() & 0x7E) << 1;
            color = color565(r, g, b);
        }
        else {
            // Read the 3 RGB bytes, colour is actually only in the top 6 bits of each byte as the TFT stores colours as 18 bits
            r = readCommand();
            g = readCommand();
            b = readCommand();
            color = color565(r, g, b);
        }
        data[counter] = color;
        counter++;
    }
    endWrite();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::setFont(uint16_t font) {

#ifdef TFT_TIMES_NEW_ROMAN
    switch(font) {
        case 15:
            _current_font.cmaps = cmaps_Times15;
            _current_font.glyph_bitmap = glyph_bitmap_Times15;
            _current_font.glyph_dsc = glyph_dsc_Times15;
            _current_font.range_start = cmaps_Times15->range_start;
            _current_font.range_length = cmaps_Times15->range_length;
            _current_font.line_height = cmaps_Times15->line_height;
            _current_font.font_height = cmaps_Times15->font_height;
            _current_font.base_line = cmaps_Times15->base_line;
            _current_font.lookup_table = cmaps_Times15->lookup_table;
            break;
        case 16:
            _current_font.cmaps = cmaps_Times16;
            _current_font.glyph_bitmap = glyph_bitmap_Times16;
            _current_font.glyph_dsc = glyph_dsc_Times16;
            _current_font.range_start = cmaps_Times16->range_start;
            _current_font.range_length = cmaps_Times16->range_length;
            _current_font.line_height = cmaps_Times16->line_height;
            _current_font.font_height = cmaps_Times16->font_height;
            _current_font.base_line = cmaps_Times16->base_line;
            _current_font.lookup_table = cmaps_Times16->lookup_table;
            break;
        case 18:
            _current_font.cmaps = cmaps_Times18;
            _current_font.glyph_bitmap = glyph_bitmap_Times18;
            _current_font.glyph_dsc = glyph_dsc_Times18;
            _current_font.range_start = cmaps_Times18->range_start;
            _current_font.range_length = cmaps_Times18->range_length;
            _current_font.line_height = cmaps_Times18->line_height;
            _current_font.font_height = cmaps_Times18->font_height;
            _current_font.base_line = cmaps_Times18->base_line;
            _current_font.lookup_table = cmaps_Times18->lookup_table;
            break;
        case 21:
            _current_font.cmaps = cmaps_Times21;
            _current_font.glyph_bitmap = glyph_bitmap_Times21;
            _current_font.glyph_dsc = glyph_dsc_Times21;
            _current_font.range_start = cmaps_Times21->range_start;
            _current_font.range_length = cmaps_Times21->range_length;
            _current_font.line_height = cmaps_Times21->line_height;
            _current_font.font_height = cmaps_Times21->font_height;
            _current_font.base_line = cmaps_Times21->base_line;
            _current_font.lookup_table = cmaps_Times21->lookup_table;
            break;
        case 25:
            _current_font.cmaps = cmaps_Times25;
            _current_font.glyph_bitmap = glyph_bitmap_Times25;
            _current_font.glyph_dsc = glyph_dsc_Times25;
            _current_font.range_start = cmaps_Times25->range_start;
            _current_font.range_length = cmaps_Times25->range_length;
            _current_font.line_height = cmaps_Times25->line_height;
            _current_font.font_height = cmaps_Times25->font_height;
            _current_font.base_line = cmaps_Times25->base_line;
            _current_font.lookup_table = cmaps_Times15->lookup_table;
            break;
        case 27:
            _current_font.cmaps = cmaps_Times27;
            _current_font.glyph_bitmap = glyph_bitmap_Times27;
            _current_font.glyph_dsc = glyph_dsc_Times27;
            _current_font.range_start = cmaps_Times27->range_start;
            _current_font.range_length = cmaps_Times27->range_length;
            _current_font.line_height = cmaps_Times27->line_height;
            _current_font.font_height = cmaps_Times27->font_height;
            _current_font.base_line = cmaps_Times27->base_line;
            _current_font.lookup_table = cmaps_Times27->lookup_table;
            break;
        case 34:
            _current_font.cmaps = cmaps_Times34;
            _current_font.glyph_bitmap = glyph_bitmap_Times34;
            _current_font.glyph_dsc = glyph_dsc_Times34;
            _current_font.range_start = cmaps_Times34->range_start;
            _current_font.range_length = cmaps_Times34->range_length;
            _current_font.line_height = cmaps_Times34->line_height;
            _current_font.font_height = cmaps_Times34->font_height;
            _current_font.base_line = cmaps_Times34->base_line;
            _current_font.lookup_table = cmaps_Times34->lookup_table;
            break;
        case 38:
            _current_font.cmaps = cmaps_Times38;
            _current_font.glyph_bitmap = glyph_bitmap_Times38;
            _current_font.glyph_dsc = glyph_dsc_Times38;
            _current_font.range_start = cmaps_Times38->range_start;
            _current_font.range_length = cmaps_Times38->range_length;
            _current_font.line_height = cmaps_Times38->line_height;
            _current_font.font_height = cmaps_Times38->font_height;
            _current_font.base_line = cmaps_Times38->base_line;
            _current_font.lookup_table = cmaps_Times38->lookup_table;
            break;
        case 43:
            _current_font.cmaps = cmaps_Times43;
            _current_font.glyph_bitmap = glyph_bitmap_Times43;
            _current_font.glyph_dsc = glyph_dsc_Times43;
            _current_font.range_start = cmaps_Times43->range_start;
            _current_font.range_length = cmaps_Times43->range_length;
            _current_font.line_height = cmaps_Times43->line_height;
            _current_font.font_height = cmaps_Times43->font_height;
            _current_font.base_line = cmaps_Times43->base_line;
            _current_font.lookup_table = cmaps_Times43->lookup_table;
            break;
        case 56:
            _current_font.cmaps = cmaps_Times56;
            _current_font.glyph_bitmap = glyph_bitmap_Times56;
            _current_font.glyph_dsc = glyph_dsc_Times56;
            _current_font.range_start = cmaps_Times56->range_start;
            _current_font.range_length = cmaps_Times56->range_length;
            _current_font.line_height = cmaps_Times56->line_height;
            _current_font.font_height = cmaps_Times56->font_height;
            _current_font.base_line = cmaps_Times56->base_line;
            _current_font.lookup_table = cmaps_Times56->lookup_table;
            break;
        case 66:
            _current_font.cmaps = cmaps_Times66;
            _current_font.glyph_bitmap = glyph_bitmap_Times66;
            _current_font.glyph_dsc = glyph_dsc_Times66;
            _current_font.range_start = cmaps_Times66->range_start;
            _current_font.range_length = cmaps_Times66->range_length;
            _current_font.line_height = cmaps_Times66->line_height;
            _current_font.font_height = cmaps_Times66->font_height;
            _current_font.base_line = cmaps_Times66->base_line;
            _current_font.lookup_table = cmaps_Times66->lookup_table;
            break;
        case 156:
            _current_font.cmaps = cmaps_BigNumbers;
            _current_font.glyph_bitmap = glyph_bitmap_BiGNumbers;
            _current_font.glyph_dsc = glyph_dsc_BigNumbers;
            _current_font.range_start = cmaps_BigNumbers->range_start;
            _current_font.range_length = cmaps_BigNumbers->range_length;
            _current_font.line_height = cmaps_BigNumbers->line_height;
            _current_font.font_height = cmaps_BigNumbers->font_height;
            _current_font.base_line = cmaps_BigNumbers->base_line;
            _current_font.lookup_table = cmaps_BigNumbers->lookup_table;
            break;
        default: log_e("unknown font size for Times New Roman, size is %i", font); break;
    }
#endif

#ifdef TFT_GARAMOND
    switch(font) {
        case 15:
            _current_font.cmaps = cmaps_Garamond15;
            _current_font.glyph_bitmap = glyph_bitmap_Garamond15;
            _current_font.glyph_dsc = glyph_dsc_Garamond15;
            _current_font.range_start = cmaps_Garamond15->range_start;
            _current_font.range_length = cmaps_Garamond15->range_length;
            _current_font.line_height = cmaps_Garamond15->line_height;
            _current_font.font_height = cmaps_Garamond15->font_height;
            _current_font.base_line = cmaps_Garamond15->base_line;
            _current_font.lookup_table = cmaps_Garamond15->lookup_table;
            break;
        case 16:
            _current_font.cmaps = cmaps_Garamond16;
            _current_font.glyph_bitmap = glyph_bitmap_Garamond16;
            _current_font.glyph_dsc = glyph_dsc_Garamond16;
            _current_font.range_start = cmaps_Garamond16->range_start;
            _current_font.range_length = cmaps_Garamond16->range_length;
            _current_font.line_height = cmaps_Garamond16->line_height;
            _current_font.font_height = cmaps_Garamond16->font_height;
            _current_font.base_line = cmaps_Garamond16->base_line;
            _current_font.lookup_table = cmaps_Garamond16->lookup_table;
            break;
        case 18:
            _current_font.cmaps = cmaps_Garamond18;
            _current_font.glyph_bitmap = glyph_bitmap_Garamond18;
            _current_font.glyph_dsc = glyph_dsc_Garamond18;
            _current_font.range_start = cmaps_Garamond18->range_start;
            _current_font.range_length = cmaps_Garamond18->range_length;
            _current_font.line_height = cmaps_Garamond18->line_height;
            _current_font.font_height = cmaps_Garamond18->font_height;
            _current_font.base_line = cmaps_Garamond18->base_line;
            _current_font.lookup_table = cmaps_Garamond18->lookup_table;
            break;
        case 21:
            _current_font.cmaps = cmaps_Garamond21;
            _current_font.glyph_bitmap = glyph_bitmap_Garamond21;
            _current_font.glyph_dsc = glyph_dsc_Garamond21;
            _current_font.range_start = cmaps_Garamond21->range_start;
            _current_font.range_length = cmaps_Garamond21->range_length;
            _current_font.line_height = cmaps_Garamond21->line_height;
            _current_font.font_height = cmaps_Garamond21->font_height;
            _current_font.base_line = cmaps_Garamond21->base_line;
            _current_font.lookup_table = cmaps_Garamond21->lookup_table;
            break;
        case 25:
            _current_font.cmaps = cmaps_Garamond25;
            _current_font.glyph_bitmap = glyph_bitmap_Garamond25;
            _current_font.glyph_dsc = glyph_dsc_Garamond25;
            _current_font.range_start = cmaps_Garamond25->range_start;
            _current_font.range_length = cmaps_Garamond25->range_length;
            _current_font.line_height = cmaps_Garamond25->line_height;
            _current_font.font_height = cmaps_Garamond25->font_height;
            _current_font.base_line = cmaps_Garamond25->base_line;
            _current_font.lookup_table = cmaps_Garamond25->lookup_table;
            break;
        case 27:
            _current_font.cmaps = cmaps_Garamond27;
            _current_font.glyph_bitmap = glyph_bitmap_Garamond27;
            _current_font.glyph_dsc = glyph_dsc_Garamond27;
            _current_font.range_start = cmaps_Garamond27->range_start;
            _current_font.range_length = cmaps_Garamond27->range_length;
            _current_font.line_height = cmaps_Garamond27->line_height;
            _current_font.font_height = cmaps_Garamond27->font_height;
            _current_font.base_line = cmaps_Garamond27->base_line;
            _current_font.lookup_table = cmaps_Garamond27->lookup_table;
            break;
        case 34:
            _current_font.cmaps = cmaps_Garamond34;
            _current_font.glyph_bitmap = glyph_bitmap_Garamond34;
            _current_font.glyph_dsc = glyph_dsc_Garamond34;
            _current_font.range_start = cmaps_Garamond34->range_start;
            _current_font.range_length = cmaps_Garamond34->range_length;
            _current_font.line_height = cmaps_Garamond34->line_height;
            _current_font.font_height = cmaps_Garamond34->font_height;
            _current_font.base_line = cmaps_Garamond34->base_line;
            _current_font.lookup_table = cmaps_Garamond34->lookup_table;
            break;
        case 38:
            _current_font.cmaps = cmaps_Garamond38;
            _current_font.glyph_bitmap = glyph_bitmap_Garamond38;
            _current_font.glyph_dsc = glyph_dsc_Garamond38;
            _current_font.range_start = cmaps_Garamond38->range_start;
            _current_font.range_length = cmaps_Garamond38->range_length;
            _current_font.line_height = cmaps_Garamond38->line_height;
            _current_font.font_height = cmaps_Garamond38->font_height;
            _current_font.base_line = cmaps_Garamond38->base_line;
            _current_font.lookup_table = cmaps_Garamond38->lookup_table;
            break;
        case 43:
            _current_font.cmaps = cmaps_Garamond43;
            _current_font.glyph_bitmap = glyph_bitmap_Garamond43;
            _current_font.glyph_dsc = glyph_dsc_Garamond43;
            _current_font.range_start = cmaps_Garamond43->range_start;
            _current_font.range_length = cmaps_Garamond43->range_length;
            _current_font.line_height = cmaps_Garamond43->line_height;
            _current_font.font_height = cmaps_Garamond43->font_height;
            _current_font.base_line = cmaps_Garamond43->base_line;
            _current_font.lookup_table = cmaps_Garamond43->lookup_table;
            break;
        case 56:
            _current_font.cmaps = cmaps_Garamond56;
            _current_font.glyph_bitmap = glyph_bitmap_Garamond56;
            _current_font.glyph_dsc = glyph_dsc_Garamond56;
            _current_font.range_start = cmaps_Garamond56->range_start;
            _current_font.range_length = cmaps_Garamond56->range_length;
            _current_font.line_height = cmaps_Garamond56->line_height;
            _current_font.font_height = cmaps_Garamond56->font_height;
            _current_font.base_line = cmaps_Garamond56->base_line;
            _current_font.lookup_table = cmaps_Garamond56->lookup_table;
            break;
        case 66:
            _current_font.cmaps = cmaps_Garamond66;
            _current_font.glyph_bitmap = glyph_bitmap_Garamond66;
            _current_font.glyph_dsc = glyph_dsc_Garamond66;
            _current_font.range_start = cmaps_Garamond66->range_start;
            _current_font.range_length = cmaps_Garamond66->range_length;
            _current_font.line_height = cmaps_Garamond66->line_height;
            _current_font.font_height = cmaps_Garamond66->font_height;
            _current_font.base_line = cmaps_Garamond66->base_line;
            _current_font.lookup_table = cmaps_Garamond66->lookup_table;
            break;
        case 156:
            _current_font.cmaps = cmaps_BigNumbers;
            _current_font.glyph_bitmap = glyph_bitmap_BiGNumbers;
            _current_font.glyph_dsc = glyph_dsc_BigNumbers;
            _current_font.range_start = cmaps_BigNumbers->range_start;
            _current_font.range_length = cmaps_BigNumbers->range_length;
            _current_font.line_height = cmaps_BigNumbers->line_height;
            _current_font.font_height = cmaps_BigNumbers->font_height;
            _current_font.base_line = cmaps_BigNumbers->base_line;
            _current_font.lookup_table = cmaps_BigNumbers->lookup_table;
            break;
        default: break;
    }
#endif

#ifdef TFT_FREE_SERIF_ITALIC
    switch(font) {
        case 15:
            _current_font.cmaps = cmaps_FreeSerifItalic15;
            _current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic15;
            _current_font.glyph_dsc = glyph_dsc_FreeSerifItalic15;
            _current_font.range_start = cmaps_FreeSerifItalic15->range_start;
            _current_font.range_length = cmaps_FreeSerifItalic15->range_length;
            _current_font.line_height = cmaps_FreeSerifItalic15->line_height;
            _current_font.font_height = cmaps_FreeSerifItalic15->font_height;
            _current_font.base_line = cmaps_FreeSerifItalic15->base_line;
            _current_font.lookup_table = cmaps_FreeSerifItalic15->lookup_table;
            break;
        case 16:
            _current_font.cmaps = cmaps_FreeSerifItalic16;
            _current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic16;
            _current_font.glyph_dsc = glyph_dsc_FreeSerifItalic16;
            _current_font.range_start = cmaps_FreeSerifItalic16->range_start;
            _current_font.range_length = cmaps_FreeSerifItalic16->range_length;
            _current_font.line_height = cmaps_FreeSerifItalic16->line_height;
            _current_font.font_height = cmaps_FreeSerifItalic16->font_height;
            _current_font.base_line = cmaps_FreeSerifItalic16->base_line;
            _current_font.lookup_table = cmaps_FreeSerifItalic16->lookup_table;
            break;
        case 18:
            _current_font.cmaps = cmaps_FreeSerifItalic18;
            _current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic18;
            _current_font.glyph_dsc = glyph_dsc_FreeSerifItalic18;
            _current_font.range_start = cmaps_FreeSerifItalic18->range_start;
            _current_font.range_length = cmaps_FreeSerifItalic18->range_length;
            _current_font.line_height = cmaps_FreeSerifItalic18->line_height;
            _current_font.font_height = cmaps_FreeSerifItalic18->font_height;
            _current_font.base_line = cmaps_FreeSerifItalic18->base_line;
            _current_font.lookup_table = cmaps_FreeSerifItalic18->lookup_table;
            break;
        case 21:
            _current_font.cmaps = cmaps_FreeSerifItalic21;
            _current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic21;
            _current_font.glyph_dsc = glyph_dsc_FreeSerifItalic21;
            _current_font.range_start = cmaps_FreeSerifItalic21->range_start;
            _current_font.range_length = cmaps_FreeSerifItalic21->range_length;
            _current_font.line_height = cmaps_FreeSerifItalic21->line_height;
            _current_font.font_height = cmaps_FreeSerifItalic21->font_height;
            _current_font.base_line = cmaps_FreeSerifItalic21->base_line;
            _current_font.lookup_table = cmaps_FreeSerifItalic21->lookup_table;
            break;
        case 25:
            _current_font.cmaps = cmaps_FreeSerifItalic25;
            _current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic25;
            _current_font.glyph_dsc = glyph_dsc_FreeSerifItalic25;
            _current_font.range_start = cmaps_FreeSerifItalic25->range_start;
            _current_font.range_length = cmaps_FreeSerifItalic25->range_length;
            _current_font.line_height = cmaps_FreeSerifItalic25->line_height;
            _current_font.font_height = cmaps_FreeSerifItalic25->font_height;
            _current_font.base_line = cmaps_FreeSerifItalic25->base_line;
            _current_font.lookup_table = cmaps_FreeSerifItalic25->lookup_table;
            break;
        case 27:
            _current_font.cmaps = cmaps_FreeSerifItalic27;
            _current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic27;
            _current_font.glyph_dsc = glyph_dsc_FreeSerifItalic27;
            _current_font.range_start = cmaps_FreeSerifItalic27->range_start;
            _current_font.range_length = cmaps_FreeSerifItalic27->range_length;
            _current_font.line_height = cmaps_FreeSerifItalic27->line_height;
            _current_font.font_height = cmaps_FreeSerifItalic27->font_height;
            _current_font.base_line = cmaps_FreeSerifItalic27->base_line;
            _current_font.lookup_table = cmaps_FreeSerifItalic27->lookup_table;
            break;
        case 34:
            _current_font.cmaps = cmaps_FreeSerifItalic34;
            _current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic34;
            _current_font.glyph_dsc = glyph_dsc_FreeSerifItalic34;
            _current_font.range_start = cmaps_FreeSerifItalic34->range_start;
            _current_font.range_length = cmaps_FreeSerifItalic34->range_length;
            _current_font.line_height = cmaps_FreeSerifItalic34->line_height;
            _current_font.font_height = cmaps_FreeSerifItalic34->font_height;
            _current_font.base_line = cmaps_FreeSerifItalic34->base_line;
            _current_font.lookup_table = cmaps_FreeSerifItalic34->lookup_table;
            break;
        case 38:
            _current_font.cmaps = cmaps_FreeSerifItalic38;
            _current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic38;
            _current_font.glyph_dsc = glyph_dsc_FreeSerifItalic38;
            _current_font.range_start = cmaps_FreeSerifItalic38->range_start;
            _current_font.range_length = cmaps_FreeSerifItalic38->range_length;
            _current_font.line_height = cmaps_FreeSerifItalic38->line_height;
            _current_font.font_height = cmaps_FreeSerifItalic38->font_height;
            _current_font.base_line = cmaps_FreeSerifItalic38->base_line;
            _current_font.lookup_table = cmaps_FreeSerifItalic38->lookup_table;
            break;
        case 43:
            _current_font.cmaps = cmaps_FreeSerifItalic43;
            _current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic43;
            _current_font.glyph_dsc = glyph_dsc_FreeSerifItalic43;
            _current_font.range_start = cmaps_FreeSerifItalic43->range_start;
            _current_font.range_length = cmaps_FreeSerifItalic43->range_length;
            _current_font.line_height = cmaps_FreeSerifItalic43->line_height;
            _current_font.font_height = cmaps_FreeSerifItalic43->font_height;
            _current_font.base_line = cmaps_FreeSerifItalic43->base_line;
            _current_font.lookup_table = cmaps_FreeSerifItalic43->lookup_table;
            break;
        case 56:
            _current_font.cmaps = cmaps_FreeSerifItalic56;
            _current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic56;
            _current_font.glyph_dsc = glyph_dsc_FreeSerifItalic56;
            _current_font.range_start = cmaps_FreeSerifItalic56->range_start;
            _current_font.range_length = cmaps_FreeSerifItalic56->range_length;
            _current_font.line_height = cmaps_FreeSerifItalic56->line_height;
            _current_font.font_height = cmaps_FreeSerifItalic56->font_height;
            _current_font.base_line = cmaps_FreeSerifItalic56->base_line;
            _current_font.lookup_table = cmaps_FreeSerifItalic56->lookup_table;
            break;
        case 66:
            _current_font.cmaps = cmaps_FreeSerifItalic66;
            _current_font.glyph_bitmap = glyph_bitmap_FreeSerifItalic66;
            _current_font.glyph_dsc = glyph_dsc_FreeSerifItalic66;
            _current_font.range_start = cmaps_FreeSerifItalic66->range_start;
            _current_font.range_length = cmaps_FreeSerifItalic66->range_length;
            _current_font.line_height = cmaps_FreeSerifItalic66->line_height;
            _current_font.font_height = cmaps_FreeSerifItalic66->font_height;
            _current_font.base_line = cmaps_FreeSerifItalic66->base_line;
            _current_font.lookup_table = cmaps_FreeSerifItalic66->lookup_table;
            break;
        case 156:
            _current_font.cmaps = cmaps_BigNumbers;
            _current_font.glyph_bitmap = glyph_bitmap_BiGNumbers;
            _current_font.glyph_dsc = glyph_dsc_BigNumbers;
            _current_font.range_start = cmaps_BigNumbers->range_start;
            _current_font.range_length = cmaps_BigNumbers->range_length;
            _current_font.line_height = cmaps_BigNumbers->line_height;
            _current_font.font_height = cmaps_BigNumbers->font_height;
            _current_font.base_line = cmaps_BigNumbers->base_line;
            _current_font.lookup_table = cmaps_BigNumbers->lookup_table;
            break;
        default: break;
    }

#endif

#ifdef TFT_ARIAL
    switch(font) {
        case 15:
            _current_font.cmaps = cmaps_Arial15;
            _current_font.glyph_bitmap = glyph_bitmap_Arial15;
            _current_font.glyph_dsc = glyph_dsc_Arial15;
            _current_font.range_start = cmaps_Arial15->range_start;
            _current_font.range_length = cmaps_Arial15->range_length;
            _current_font.line_height = cmaps_Arial15->line_height;
            _current_font.font_height = cmaps_Arial15->font_height;
            _current_font.base_line = cmaps_Arial15->base_line;
            _current_font.lookup_table = cmaps_Arial15->lookup_table;
            break;
        case 16:
            _current_font.cmaps = cmaps_Arial16;
            _current_font.glyph_bitmap = glyph_bitmap_Arial16;
            _current_font.glyph_dsc = glyph_dsc_Arial16;
            _current_font.range_start = cmaps_Arial16->range_start;
            _current_font.range_length = cmaps_Arial16->range_length;
            _current_font.line_height = cmaps_Arial16->line_height;
            _current_font.font_height = cmaps_Arial16->font_height;
            _current_font.base_line = cmaps_Arial16->base_line;
            _current_font.lookup_table = cmaps_Arial16->lookup_table;
            break;
        case 18:
            _current_font.cmaps = cmaps_Arial18;
            _current_font.glyph_bitmap = glyph_bitmap_Arial18;
            _current_font.glyph_dsc = glyph_dsc_Arial18;
            _current_font.range_start = cmaps_Arial18->range_start;
            _current_font.range_length = cmaps_Arial18->range_length;
            _current_font.line_height = cmaps_Arial18->line_height;
            _current_font.font_height = cmaps_Arial18->font_height;
            _current_font.base_line = cmaps_Arial18->base_line;
            _current_font.lookup_table = cmaps_Arial18->lookup_table;
            break;
        case 21:
            _current_font.cmaps = cmaps_Arial21;
            _current_font.glyph_bitmap = glyph_bitmap_Arial21;
            _current_font.glyph_dsc = glyph_dsc_Arial21;
            _current_font.range_start = cmaps_Arial21->range_start;
            _current_font.range_length = cmaps_Arial21->range_length;
            _current_font.line_height = cmaps_Arial21->line_height;
            _current_font.font_height = cmaps_Arial21->font_height;
            _current_font.base_line = cmaps_Arial21->base_line;
            _current_font.lookup_table = cmaps_Arial21->lookup_table;
            break;
        case 25:
            _current_font.cmaps = cmaps_Arial25;
            _current_font.glyph_bitmap = glyph_bitmap_Arial25;
            _current_font.glyph_dsc = glyph_dsc_Arial25;
            _current_font.range_start = cmaps_Arial25->range_start;
            _current_font.range_length = cmaps_Arial25->range_length;
            _current_font.line_height = cmaps_Arial25->line_height;
            _current_font.font_height = cmaps_Arial25->font_height;
            _current_font.base_line = cmaps_Arial25->base_line;
            _current_font.lookup_table = cmaps_Arial25->lookup_table;
            break;
        case 27:
            _current_font.cmaps = cmaps_Arial27;
            _current_font.glyph_bitmap = glyph_bitmap_Arial27;
            _current_font.glyph_dsc = glyph_dsc_Arial27;
            _current_font.range_start = cmaps_Arial27->range_start;
            _current_font.range_length = cmaps_Arial27->range_length;
            _current_font.line_height = cmaps_Arial27->line_height;
            _current_font.font_height = cmaps_Arial27->font_height;
            _current_font.base_line = cmaps_Arial27->base_line;
            _current_font.lookup_table = cmaps_Arial27->lookup_table;
            break;
        case 34:
            _current_font.cmaps = cmaps_Arial34;
            _current_font.glyph_bitmap = glyph_bitmap_Arial34;
            _current_font.glyph_dsc = glyph_dsc_Arial34;
            _current_font.range_start = cmaps_Arial34->range_start;
            _current_font.range_length = cmaps_Arial34->range_length;
            _current_font.line_height = cmaps_Arial34->line_height;
            _current_font.font_height = cmaps_Arial34->font_height;
            _current_font.base_line = cmaps_Arial34->base_line;
            _current_font.lookup_table = cmaps_Arial34->lookup_table;
            break;
        case 38:
            _current_font.cmaps = cmaps_Arial38;
            _current_font.glyph_bitmap = glyph_bitmap_Arial38;
            _current_font.glyph_dsc = glyph_dsc_Arial38;
            _current_font.range_start = cmaps_Arial38->range_start;
            _current_font.range_length = cmaps_Arial38->range_length;
            _current_font.line_height = cmaps_Arial38->line_height;
            _current_font.font_height = cmaps_Arial38->font_height;
            _current_font.base_line = cmaps_Arial38->base_line;
            _current_font.lookup_table = cmaps_Arial38->lookup_table;
            break;
        case 43:
            _current_font.cmaps = cmaps_Arial43;
            _current_font.glyph_bitmap = glyph_bitmap_Arial43;
            _current_font.glyph_dsc = glyph_dsc_Arial43;
            _current_font.range_start = cmaps_Arial43->range_start;
            _current_font.range_length = cmaps_Arial43->range_length;
            _current_font.line_height = cmaps_Arial43->line_height;
            _current_font.font_height = cmaps_Arial43->font_height;
            _current_font.base_line = cmaps_Arial43->base_line;
            _current_font.lookup_table = cmaps_Arial43->lookup_table;
            break;
        case 56:
            _current_font.cmaps = cmaps_Arial56;
            _current_font.glyph_bitmap = glyph_bitmap_Arial56;
            _current_font.glyph_dsc = glyph_dsc_Arial56;
            _current_font.range_start = cmaps_Arial56->range_start;
            _current_font.range_length = cmaps_Arial56->range_length;
            _current_font.line_height = cmaps_Arial56->line_height;
            _current_font.font_height = cmaps_Arial56->font_height;
            _current_font.base_line = cmaps_Arial56->base_line;
            _current_font.lookup_table = cmaps_Arial56->lookup_table;
            break;
        case 66:
            _current_font.cmaps = cmaps_Arial66;
            _current_font.glyph_bitmap = glyph_bitmap_Arial66;
            _current_font.glyph_dsc = glyph_dsc_Arial66;
            _current_font.range_start = cmaps_Arial66->range_start;
            _current_font.range_length = cmaps_Arial66->range_length;
            _current_font.line_height = cmaps_Arial66->line_height;
            _current_font.font_height = cmaps_Arial66->font_height;
            _current_font.base_line = cmaps_Arial66->base_line;
            _current_font.lookup_table = cmaps_Arial66->lookup_table;
            break;
        case 156:
            _current_font.cmaps = cmaps_BigNumbers;
            _current_font.glyph_bitmap = glyph_bitmap_BiGNumbers;
            _current_font.glyph_dsc = glyph_dsc_BigNumbers;
            _current_font.range_start = cmaps_BigNumbers->range_start;
            _current_font.range_length = cmaps_BigNumbers->range_length;
            _current_font.line_height = cmaps_BigNumbers->line_height;
            _current_font.font_height = cmaps_BigNumbers->font_height;
            _current_font.base_line = cmaps_BigNumbers->base_line;
            _current_font.lookup_table = cmaps_BigNumbers->lookup_table;
            break;
        default: break;
    }
#endif

#ifdef TFT_Z300
    switch(font) {
        case 15:
            _current_font.cmaps = cmaps_Z300_15;
            _current_font.glyph_bitmap = glyph_bitmap_Z300_15;
            _current_font.glyph_dsc = glyph_dsc_Z300_15;
            _current_font.range_start = cmaps_Z300_15->range_start;
            _current_font.range_length = cmaps_Z300_15->range_length;
            _current_font.line_height = cmaps_Z300_15->line_height;
            _current_font.font_height = cmaps_Z300_15->font_height;
            _current_font.base_line = cmaps_Z300_15->base_line;
            _current_font.lookup_table = cmaps_Z300_15->lookup_table;
            break;
        case 16:
            _current_font.cmaps = cmaps_Z300_16;
            _current_font.glyph_bitmap = glyph_bitmap_Z300_16;
            _current_font.glyph_dsc = glyph_dsc_Z300_16;
            _current_font.range_start = cmaps_Z300_16->range_start;
            _current_font.range_length = cmaps_Z300_16->range_length;
            _current_font.line_height = cmaps_Z300_16->line_height;
            _current_font.font_height = cmaps_Z300_16->font_height;
            _current_font.base_line = cmaps_Z300_16->base_line;
            _current_font.lookup_table = cmaps_Z300_16->lookup_table;
            break;
        case 18:
            _current_font.cmaps = cmaps_Z300_18;
            _current_font.glyph_bitmap = glyph_bitmap_Z300_18;
            _current_font.glyph_dsc = glyph_dsc_Z300_18;
            _current_font.range_start = cmaps_Z300_18->range_start;
            _current_font.range_length = cmaps_Z300_18->range_length;
            _current_font.line_height = cmaps_Z300_18->line_height;
            _current_font.font_height = cmaps_Z300_18->font_height;
            _current_font.base_line = cmaps_Z300_18->base_line;
            _current_font.lookup_table = cmaps_Z300_18->lookup_table;
            break;
        case 21:
            _current_font.cmaps = cmaps_Z300_21;
            _current_font.glyph_bitmap = glyph_bitmap_Z300_21;
            _current_font.glyph_dsc = glyph_dsc_Z300_21;
            _current_font.range_start = cmaps_Z300_21->range_start;
            _current_font.range_length = cmaps_Z300_21->range_length;
            _current_font.line_height = cmaps_Z300_21->line_height;
            _current_font.font_height = cmaps_Z300_21->font_height;
            _current_font.base_line = cmaps_Z300_21->base_line;
            _current_font.lookup_table = cmaps_Z300_21->lookup_table;
            break;
        case 25:
            _current_font.cmaps = cmaps_Z300_25;
            _current_font.glyph_bitmap = glyph_bitmap_Z300_25;
            _current_font.glyph_dsc = glyph_dsc_Z300_25;
            _current_font.range_start = cmaps_Z300_25->range_start;
            _current_font.range_length = cmaps_Z300_25->range_length;
            _current_font.line_height = cmaps_Z300_25->line_height;
            _current_font.font_height = cmaps_Z300_25->font_height;
            _current_font.base_line = cmaps_Z300_25->base_line;
            _current_font.lookup_table = cmaps_Z300_25->lookup_table;
            break;
        case 27:
            _current_font.cmaps = cmaps_Z300_27;
            _current_font.glyph_bitmap = glyph_bitmap_Z300_27;
            _current_font.glyph_dsc = glyph_dsc_Z300_27;
            _current_font.range_start = cmaps_Z300_27->range_start;
            _current_font.range_length = cmaps_Z300_27->range_length;
            _current_font.line_height = cmaps_Z300_27->line_height;
            _current_font.font_height = cmaps_Z300_27->font_height;
            _current_font.base_line = cmaps_Z300_27->base_line;
            _current_font.lookup_table = cmaps_Z300_27->lookup_table;
            break;
        case 34:
            _current_font.cmaps = cmaps_Z300_34;
            _current_font.glyph_bitmap = glyph_bitmap_Z300_34;
            _current_font.glyph_dsc = glyph_dsc_Z300_34;
            _current_font.range_start = cmaps_Z300_34->range_start;
            _current_font.range_length = cmaps_Z300_34->range_length;
            _current_font.line_height = cmaps_Z300_34->line_height;
            _current_font.font_height = cmaps_Z300_34->font_height;
            _current_font.base_line = cmaps_Z300_34->base_line;
            _current_font.lookup_table = cmaps_Z300_34->lookup_table;
            break;
        case 38:
            _current_font.cmaps = cmaps_Z300_38;
            _current_font.glyph_bitmap = glyph_bitmap_Z300_38;
            _current_font.glyph_dsc = glyph_dsc_Z300_38;
            _current_font.range_start = cmaps_Z300_38->range_start;
            _current_font.range_length = cmaps_Z300_38->range_length;
            _current_font.line_height = cmaps_Z300_38->line_height;
            _current_font.font_height = cmaps_Z300_38->font_height;
            _current_font.base_line = cmaps_Z300_38->base_line;
            _current_font.lookup_table = cmaps_Z300_38->lookup_table;
            break;
        case 43:
            _current_font.cmaps = cmaps_Z300_43;
            _current_font.glyph_bitmap = glyph_bitmap_Z300_43;
            _current_font.glyph_dsc = glyph_dsc_Z300_43;
            _current_font.range_start = cmaps_Z300_43->range_start;
            _current_font.range_length = cmaps_Z300_43->range_length;
            _current_font.line_height = cmaps_Z300_43->line_height;
            _current_font.font_height = cmaps_Z300_43->font_height;
            _current_font.base_line = cmaps_Z300_43->base_line;
            _current_font.lookup_table = cmaps_Z300_43->lookup_table;
            break;
        case 56:
            _current_font.cmaps = cmaps_Z300_56;
            _current_font.glyph_bitmap = glyph_bitmap_Z300_56;
            _current_font.glyph_dsc = glyph_dsc_Z300_56;
            _current_font.range_start = cmaps_Z300_56->range_start;
            _current_font.range_length = cmaps_Z300_56->range_length;
            _current_font.line_height = cmaps_Z300_56->line_height;
            _current_font.font_height = cmaps_Z300_56->font_height;
            _current_font.base_line = cmaps_Z300_56->base_line;
            _current_font.lookup_table = cmaps_Z300_56->lookup_table;
            break;
        case 66:
            _current_font.cmaps = cmaps_Z300_66;
            _current_font.glyph_bitmap = glyph_bitmap_Z300_66;
            _current_font.glyph_dsc = glyph_dsc_Z300_66;
            _current_font.range_start = cmaps_Z300_66->range_start;
            _current_font.range_length = cmaps_Z300_66->range_length;
            _current_font.line_height = cmaps_Z300_66->line_height;
            _current_font.font_height = cmaps_Z300_66->font_height;
            _current_font.base_line = cmaps_Z300_66->base_line;
            _current_font.lookup_table = cmaps_Z300_66->lookup_table;
            break;
        case 156:
            _current_font.cmaps = cmaps_BigNumbers;
            _current_font.glyph_bitmap = glyph_bitmap_BiGNumbers;
            _current_font.glyph_dsc = glyph_dsc_BigNumbers;
            _current_font.range_start = cmaps_BigNumbers->range_start;
            _current_font.range_length = cmaps_BigNumbers->range_length;
            _current_font.line_height = cmaps_BigNumbers->line_height;
            _current_font.font_height = cmaps_BigNumbers->font_height;
            _current_font.base_line = cmaps_BigNumbers->base_line;
            _current_font.lookup_table = cmaps_BigNumbers->lookup_table;
            break;
        default: break;
    }
#endif
}
/*******************************************************************************************************************************************************************************************************
 *                                                                                                                                                                                                     *
 *        ⏫⏫⏫⏫⏫⏫                                       W R I T E    T E X T    R E L A T E D    F U N C T I O N S                                                      ⏫⏫⏫⏫⏫⏫             *
 *                                                                                                                                                                                                     *
 * *****************************************************************************************************************************************************************************************************
 */
void TFT::writeInAddrWindow(const uint8_t* bmi, uint16_t posX, uint16_t poxY, uint16_t width, uint16_t height) {

    uint16_t nrOfPixels = width * height;

    auto bitreader = [&](const uint8_t* bm) { // lambda
        static uint16_t       bmi = 0;
        static uint8_t        idx = 0;
        static const uint8_t* bitmap = NULL;
        if(bm) {
            bitmap = bm;
            idx = 0x80;
            bmi = 0;
            return (uint16_t)0;
        }
        bool bit = *(bitmap + bmi) & idx;
        idx >>= 1;
        if(idx == 0) {
            bmi++;
            idx = 0x80;
        }
        if(bit) { return _textColor; }
        return _backGroundColor;
    };

    bitreader(bmi);

    startWrite();
    setAddrWindow(posX, poxY, width, height);
    if(_TFTcontroller == ILI9341) {
        writeCommand(ILI9341_RAMWR);                        // ILI9341
        while(nrOfPixels--) spi_TFT->write16(bitreader(0)); // Send to TFT 16 bits at a time
    }
    else if(_TFTcontroller == HX8347D) {
        writeCommand(0x22);
        while(nrOfPixels--) spi_TFT->write16(bitreader(0)); // Send to TFT 16 bits at a time
    }
    else if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) {
        writeCommand(ILI9486_RAMWR);
        while(nrOfPixels--) spi_TFT->write16(bitreader(0)); // Send to TFT 16 bits at a time
    }
    else if(_TFTcontroller == ILI9488) {
        writeCommand(ILI9488_RAMWR);
        while(nrOfPixels--) write24BitColor(bitreader(0)); // Send to TFT 16 bits at a time
    }
    else if(_TFTcontroller == ST7796 || _TFTcontroller == ST7796RPI) {
        writeCommand(ST7796_RAMWR);
        while(nrOfPixels--) write24BitColor(bitreader(0)); // Send to TFT 16 bits at a time
    }
    endWrite();
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// The function is passed a string and two arrays of length strlen(str + 1). This is definitely enough, since ANSI sequences or non-ASCII UTF-8 characters are always greater than 1.
// For each printable character found in the LookUp table, the codepoint is written to the next position in the charr. The number of printable characters is increased by one.
// If an ANSI sequence is found, the color found is written into ansiArr at the position of the current character. The return value is the number of printable character.
uint16_t TFT::validCharsInString(const char* str, uint16_t* chArr, int8_t* ansiArr) {
    int16_t  codePoint = -1;
    uint16_t idx = 0;
    uint16_t chLen = 0;
    while((uint8_t)str[idx] != 0) {
        switch((uint8_t)str[idx]) {
            case '\033': // ANSI sequence
                if(strncmp(str + idx, "\033[30m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 1;  break;} // ANSI_ESC_BLACK
                if(strncmp(str + idx, "\033[31m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 2;  break;} // ANSI_ESC_RED
                if(strncmp(str + idx, "\033[32m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 3;  break;} // ANSI_ESC_GREEN
                if(strncmp(str + idx, "\033[33m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 4;  break;} // ANSI_ESC_YELLOW
                if(strncmp(str + idx, "\033[34m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 5;  break;} // ANSI_ESC_BLUE
                if(strncmp(str + idx, "\033[35m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 6;  break;} // ANSI_ESC_MAGENTA
                if(strncmp(str + idx, "\033[36m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 7;  break;} // ANSI_ESC_CYAN
                if(strncmp(str + idx, "\033[37m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 8;  break;} // ANSI_ESC_WHITE
                if(strncmp(str + idx, "\033[38;5;130m", 11) == 0)           {idx += 11; ansiArr[chLen] = 9;  break;} // ANSI_ESC_BROWN
                if(strncmp(str + idx, "\033[38;5;214m", 11) == 0)           {idx += 11; ansiArr[chLen] = 10; break;} // ANSI_ESC_ORANGE
                if(strncmp(str + idx, "\033[90m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 11; break;} // ANSI_ESC_GREY
                if(strncmp(str + idx, "\033[91m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 12; break;} // ANSI_ESC_LIGHTRED
                if(strncmp(str + idx, "\033[92m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 13; break;} // ANSI_ESC_LIGHTGREEN
                if(strncmp(str + idx, "\033[93m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 14; break;} // ANSI_ESC_LIGHTYELLOW
                if(strncmp(str + idx, "\033[94m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 15; break;} // ANSI_ESC_LIGHTBLUE
                if(strncmp(str + idx, "\033[95m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 16; break;} // ANSI_ESC_LIGHTMAGENTA
                if(strncmp(str + idx, "\033[96m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 17; break;} // ANSI_ESC_LIGHTCYAN
                if(strncmp(str + idx, "\033[97m", 5) == 0)                  {idx += 5;  ansiArr[chLen] = 18; break;} // ANSI_ESC_LIGHTGREY
                if(strncmp(str + idx, "\033[38;5;52m", 10) == 0)            {idx += 10; ansiArr[chLen] = 19; break;} // ANSI_ESC_DARKRED
                if(strncmp(str + idx, "\033[38;5;22m", 10) == 0)            {idx += 10; ansiArr[chLen] = 20; break;} // ANSI_ESC_DARKGREEN
                if(strncmp(str + idx, "\033[38;5;136m", 11) == 0)           {idx += 11; ansiArr[chLen] = 21; break;} // ANSI_ESC_DARKYELLOW
                if(strncmp(str + idx, "\033[38;5;17m", 10) == 0)            {idx += 10; ansiArr[chLen] = 22; break;} // ANSI_ESC_DARKBLUE
                if(strncmp(str + idx, "\033[38;5;53m", 10) == 0)            {idx += 10; ansiArr[chLen] = 23; break;} // ANSI_ESC_DARKMAGENTA
                if(strncmp(str + idx, "\033[38;5;23m", 10) == 0)            {idx += 10; ansiArr[chLen] = 24; break;} // ANSI_ESC_DARKCYAN
                if(strncmp(str + idx, "\033[38;5;240m", 11) == 0)           {idx += 11; ansiArr[chLen] = 25; break;} // ANSI_ESC_DARKGREY
                if(strncmp(str + idx, "\033[38;5;166m", 11) == 0)           {idx += 11; ansiArr[chLen] = 26; break;} // ANSI_ESC_DARKORANGE
                if(strncmp(str + idx, "\033[38;5;215m", 11) == 0)           {idx += 11; ansiArr[chLen] = 27; break;} // ANSI_ESC_LIGHTORANGE
                if(strncmp(str + idx, "\033[38;5;129m", 11) == 0)           {idx += 11; ansiArr[chLen] = 28; break;} // ANSI_ESC_PURPLE
                if(strncmp(str + idx, "\033[38;5;213m", 11) == 0)           {idx += 11; ansiArr[chLen] = 29; break;} // ANSI_ESC_PINK
                if(strncmp(str + idx, "\033[38;5;190m", 11) == 0)           {idx += 11; ansiArr[chLen] = 30; break;} // ANSI_ESC_LIME
                if(strncmp(str + idx, "\033[38;5;25m", 10) == 0)            {idx += 10; ansiArr[chLen] = 31; break;} // ANSI_ESC_NAVY
                if(strncmp(str + idx, "\033[38;5;51m", 10) == 0)            {idx += 10; ansiArr[chLen] = 32; break;} // ANSI_ESC_AQUAMARINE
                if(strncmp(str + idx, "\033[38;5;189m", 11) == 0)           {idx += 11; ansiArr[chLen] = 33; break;} // ANSI_ESC_LAVENDER
                if(strncmp(str + idx, "\033[38;2;210;180;140m", 19) == 0)   {idx += 19; ansiArr[chLen] = 34; break;} // ANSI_ESC_LIGHTBROWN
                if(strncmp(str + idx, "\033[0m", 4) == 0)                   {idx += 4;  ansiArr[chLen] = -1; break;} // ANSI_ESC_RESET       unused
                if(strncmp(str + idx, "\033[1m", 4) == 0)                   {idx += 4;  ansiArr[chLen] = -1; break;} // ANSI_ESC_BOLD        unused
                if(strncmp(str + idx, "\033[2m", 4) == 0)                   {idx += 4;  ansiArr[chLen] = -1; break;} // ANSI_ESC_FAINT       unused
                if(strncmp(str + idx, "\033[3m", 4) == 0)                   {idx += 4;  ansiArr[chLen] = -1; break;} // ANSI_ESC_ITALIC      unused
                if(strncmp(str + idx, "\033[4m", 4) == 0)                   {idx += 4;  ansiArr[chLen] = -1; break;} // ANSI_ESC_UNDERLINE   unused
                log_w("unknown ANSI ESC SEQUENCE");                         {idx += 4;  ansiArr[chLen] = -1; break;} // unknown
                break;
            case 0x20 ... 0x7F:                   // is ASCII
                chArr[chLen] = (uint8_t)str[idx]; // codepoint
                idx += 1;
                chLen += 1;
                break;
            case 0xC2 ... 0xD1:
                codePoint = ((uint8_t)str[idx] - 0xC2) * 0x40 + (uint8_t)str[idx + 1]; // codepoint
                if(_current_font.lookup_table[codePoint] != 0) {                       // is invalid UTF8 char
                    chArr[chLen] = codePoint;
                    chLen += 1;
                }
                else { log_w("character 0x%02X%02X is not in table", str[idx], str[idx + 1]); }
                idx += 2;
                break;
            case 0xD2 ... 0xDF:
                log_w("character 0x%02X%02X  is not in table", str[idx], str[idx + 1]);
                idx += 2;
                break;
            case 0xE0:
                if((uint8_t)str[idx + 1] == 0x80 && (uint8_t)str[idx + 2] == 0x99) {
                    codePoint = 0xA4;
                    chLen += 1;
                } // special sign 0xe28099 (general punctuation)
                else log_w("character %02X%02X  is not in table", str[idx], str[idx + 1]);
                idx += 3;
                break;
            case 0xE1 ... 0xEF: idx += 3; break;
            case 0xF0 ... 0xFF: idx += 4; break;
            case 0x0A: idx+=1; break; // is '/n'
            default: log_w("char is not printable 0x%02X", (uint8_t)str[idx]); idx += 1;
        }
    }
    return chLen;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint16_t TFT::fitinline(uint16_t* cpArr, uint16_t chLength, uint16_t begin, int16_t win_W, uint16_t* usedPxLength, bool narrow, bool noWrap){
    // cpArr contains the CodePoints of all printable characters in the string. chLength is the length of cpArr. From a starting position, the characters that fit into a width of win_W are determined.
    // The ending of a word is defined by a space or an \n. The number of characters that can be written is returned. For later alignment of the characters, the length is passed in usedPxLength.
    // narrow: no x offsets are taken into account
    // noWrap: The last possible character is written, spaces and line ends are not taken into account.
    uint16_t idx = begin;
    uint16_t pxLength = 0;
    uint16_t lastSpacePos = 0;
    uint16_t drawableChars = 0;
    uint16_t lastUsedPxLength = 0;
    uint16_t glyphPos = 0;
    while(cpArr[idx] != 0) {
        *usedPxLength = pxLength;
        if(cpArr[idx] == 0x20 || cpArr[idx - 1] == '-') {
            lastSpacePos = drawableChars;
            lastUsedPxLength = pxLength;
        }
        glyphPos = _current_font.lookup_table[cpArr[idx]];
        pxLength += _current_font.glyph_dsc[glyphPos].adv_w / 16;
        if(!narrow) pxLength += _current_font.glyph_dsc[glyphPos].ofs_x;
        if(pxLength > win_W || cpArr[idx] == '\n') { // force wrap
            if(noWrap) { return drawableChars; }
            if(lastSpacePos) {
                *usedPxLength = lastUsedPxLength;
                return lastSpacePos;
            }
            else return drawableChars;
        }
        idx++;
        drawableChars++;
        *usedPxLength = pxLength;
    }
    return drawableChars;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT::fitInAddrWindow(uint16_t* cpArr, uint16_t chLength, int16_t win_W, int16_t win_H, bool narrow, bool noWrap){
    // First, the largest character set is used to check whether a given string str fits into a window of size win_W - winH.
    // If this is not the case, the next smaller character set is selected and checked again.
    // The largest possible character set (in px) is used; if nothing fits, the smallest character set is used. Then parts of the string will not be able to be written.
    // cpArr contains the codepoints of the str, chLength determines th Length of cpArr, returns the number of lines used
    uint8_t nrOfFonts = sizeof(fontSizes);
    uint8_t currentFontSize = 0;
    uint16_t usedPxLength = 0;
    uint16_t drawableCharsTotal = 0;
    uint16_t drawableCharsinline = 0;
    uint16_t startPos = 0;
    uint8_t  nrOfLines = 0;
    while(true){
        currentFontSize = fontSizes[nrOfFonts - 1];
        if(currentFontSize == 0) break;
        setFont(currentFontSize);
        drawableCharsTotal = 0;
        startPos = 0;
        nrOfLines = 1;
        int16_t win_H_remain = win_H;
        while(true){
            if(win_H_remain < _current_font.line_height) {break;}
            drawableCharsinline = fitinline(cpArr, chLength, startPos, win_W, &usedPxLength, narrow, noWrap);
            win_H_remain -= _current_font.line_height;
        //    log_i("drawableCharsinline  %i,chLength  %i, currentFontSize %i", drawableCharsinline, chLength, currentFontSize);
            drawableCharsTotal += drawableCharsinline;
            startPos += drawableCharsinline;
            if(drawableCharsinline == 0) break;
            if(drawableCharsTotal == chLength) goto exit;
            nrOfLines++;
        }
        if(drawableCharsTotal == chLength) goto exit;
        if(nrOfFonts == 0) break;
        nrOfFonts--;
    }
exit:
//  log_w("nrOfLines  %i", nrOfLines);
    return nrOfLines;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
size_t TFT::writeText(const char* str, uint16_t win_X, uint16_t win_Y, int16_t win_W, int16_t win_H, uint8_t h_align, uint8_t v_align, bool narrow, bool noWrap, bool autoSize) {
    // autoSize choose the biggest possible font
    uint16_t idx = 0;
    uint16_t utfPosArr[strlen(str) + 1] = {0};
    int8_t   ansiArr[strlen(str) + 1] = {0};
    uint16_t strChLength = 0; // nr. of chars
    uint8_t  nrOfLines = 1;

    //-------------------------------------------------------------------------------------------------------------------
    auto drawChar = [&](uint16_t idx, uint16_t x, uint16_t y) { // lambda
        uint16_t glyphPos = _current_font.lookup_table[utfPosArr[idx]];
        uint16_t adv_w = _current_font.glyph_dsc[glyphPos].adv_w / 16;
        uint32_t bitmap_index = _current_font.glyph_dsc[glyphPos].bitmap_index;
        uint16_t box_w = _current_font.glyph_dsc[glyphPos].box_w;
        uint16_t box_h = _current_font.glyph_dsc[glyphPos].box_h;
        int16_t  ofs_x = _current_font.glyph_dsc[glyphPos].ofs_x;
        int16_t  ofs_y = _current_font.glyph_dsc[glyphPos].ofs_y;
        x += ofs_x;
        y = y + (_current_font.line_height - _current_font.base_line - 1) - box_h - ofs_y;
        writeInAddrWindow(_current_font.glyph_bitmap + bitmap_index, x, y, box_w, box_h);
        if(!narrow) adv_w += ofs_x;
        return adv_w;
    };
    //-------------------------------------------------------------------------------------------------------------------

    strChLength = validCharsInString(str, utfPosArr, ansiArr); // fill utfPosArr,
    if(autoSize) {nrOfLines = fitInAddrWindow(utfPosArr, strChLength, win_W, win_H, narrow, noWrap);}  // choose perfect fontsize
    if(!strChLength) return 0;
    //----------------------------------------------------------------------
    if((win_X + win_W) > m_h_res) { win_W = m_h_res - win_X; }   // Limit, right edge of the display
    if((win_Y + win_H) > m_v_res) { win_H = m_v_res - win_Y; }   // Limit, bottom of the display
    if(win_W < 0) { win_X = 0; }                                 // Limit, left edge of the display
    if(win_H < 0) { win_Y = 0; }                                 // Limit, top of the display
    idx = 0;
    uint16_t pX = win_X;
    uint16_t pY = win_Y;
    int16_t  pH = win_H;
    int16_t  pW = win_W;

    if(v_align == TFT_ALIGN_TOP){
        ; // nothing to do, is default
    }
    if(v_align == TFT_ALIGN_CENTER){
        int offset = (win_H - (nrOfLines * _current_font.line_height)) / 2;
        pY = pY + offset;
    }
    if(v_align == TFT_ALIGN_DOWN){
        int offset = (win_H - (nrOfLines * _current_font.line_height));
        pY = pY + offset;
    }

    uint16_t charsToDraw = 0;
    uint16_t usedPxLength = 0;
    uint16_t charsDrawn = 0;
    while(true) { // outer while
        if(noWrap && idx) goto exit;
        if(pH < _current_font.line_height) { goto exit; }
        //charsToDraw = fitinline(idx, pW, &usedPxLength);
        charsToDraw = fitinline(utfPosArr, strChLength, idx, pW, &usedPxLength, narrow, noWrap);

        if(h_align == TFT_ALIGN_RIGHT)  { pX += win_W - (usedPxLength + 1); }
        if(h_align == TFT_ALIGN_CENTER) { pX += (win_W - usedPxLength) / 2; }
        uint16_t cnt = 0;
        while(true) {               // inner while
            if(ansiArr[idx] != 0) { //
                if(ansiArr[idx] ==  1) setTextColor(TFT_BLACK);
                if(ansiArr[idx] ==  2) setTextColor(TFT_RED);
                if(ansiArr[idx] ==  3) setTextColor(TFT_GREEN);
                if(ansiArr[idx] ==  4) setTextColor(TFT_YELLOW);
                if(ansiArr[idx] ==  5) setTextColor(TFT_BLUE);
                if(ansiArr[idx] ==  6) setTextColor(TFT_MAGENTA);
                if(ansiArr[idx] ==  7) setTextColor(TFT_CYAN);
                if(ansiArr[idx] ==  8) setTextColor(TFT_WHITE);
                if(ansiArr[idx] ==  9) setTextColor(TFT_BROWN);
                if(ansiArr[idx] == 10) setTextColor(TFT_ORANGE);
                if(ansiArr[idx] == 11) setTextColor(TFT_GREY);
                if(ansiArr[idx] == 12) setTextColor(TFT_LIGHTRED);
                if(ansiArr[idx] == 13) setTextColor(TFT_LIGHTGREEN);
                if(ansiArr[idx] == 14) setTextColor(TFT_LIGHTYELLOW);
                if(ansiArr[idx] == 15) setTextColor(TFT_LIGHTBLUE);
                if(ansiArr[idx] == 16) setTextColor(TFT_LIGHTMAGENTA);
                if(ansiArr[idx] == 17) setTextColor(TFT_LIGHTCYAN);
                if(ansiArr[idx] == 18) setTextColor(TFT_LIGHTGREY);
                if(ansiArr[idx] == 19) setTextColor(TFT_DARKRED);
                if(ansiArr[idx] == 20) setTextColor(TFT_DARKGREEN);
                if(ansiArr[idx] == 21) setTextColor(TFT_DARKYELLOW);
                if(ansiArr[idx] == 22) setTextColor(TFT_DARKBLUE);
                if(ansiArr[idx] == 23) setTextColor(TFT_DARKMAGENTA);
                if(ansiArr[idx] == 24) setTextColor(TFT_DARKCYAN);
                if(ansiArr[idx] == 25) setTextColor(TFT_DARKGREY);
                if(ansiArr[idx] == 26) setTextColor(TFT_DARKORANGE);
                if(ansiArr[idx] == 27) setTextColor(TFT_LIGHTORANGE);
                if(ansiArr[idx] == 28) setTextColor(TFT_PURPLE);
                if(ansiArr[idx] == 29) setTextColor(TFT_PINK);
                if(ansiArr[idx] == 30) setTextColor(TFT_LIME);
                if(ansiArr[idx] == 31) setTextColor(TFT_NAVY);
                if(ansiArr[idx] == 32) setTextColor(TFT_AQUAMARINE);
                if(ansiArr[idx] == 33) setTextColor(TFT_LAVENDER);
                if(ansiArr[idx] == 34) setTextColor(TFT_LIGHTBROWN);
            }
            if(cnt == 0 && utfPosArr[idx] == 0x20) {
                idx++;
                charsDrawn++;
                continue;
            } // skip leading spaces
            uint16_t res = drawChar(idx, pX, pY);
            pX += res;
            pW -= res;
            idx++;
            cnt++;
            charsDrawn++;
            if(idx == strChLength) goto exit;
            if(cnt == charsToDraw) break;
        } // inner while
        pH -= _current_font.line_height;
        pY += _current_font.line_height;
        pX = win_X;
        pW = win_W;
    } // outer while
exit:
    return charsDrawn;
}

/*******************************************************************************************************************************************************************************************************
 *                                                                                                                                                                                                     *
 *      ⏫⏫⏫⏫⏫⏫                                                           B I T M A P                                                                                  ⏫⏫⏫⏫⏫⏫              *
 *                                                                                                                                                                                                     *
 *******************************************************************************************************************************************************************************************************
 */

#define bmpRead32(d, o) (d[o] | (uint16_t)(d[(o) + 1]) << 8 | (uint32_t)(d[(o) + 2]) << 16 | (uint32_t)(d[(o) + 3]) << 24)
#define bmpRead16(d, o) (d[o] | (uint16_t)(d[(o) + 1]) << 8)

#define bmpColor8(c)  (((uint16_t)(((uint8_t*)(c))[0] & 0xE0) << 8) | ((uint16_t)(((uint8_t*)(c))[0] & 0x1C) << 6) | ((((uint8_t*)(c))[0] & 0x3) << 3))
#define bmpColor16(c) ((((uint8_t*)(c))[0] | ((uint16_t)((uint8_t*)(c))[1]) << 8))
#define bmpColor24(c) (((uint16_t)(((uint8_t*)(c))[2] & 0xF8) << 8) | ((uint16_t)(((uint8_t*)(c))[1] & 0xFC) << 3) | ((((uint8_t*)(c))[0] & 0xF8) >> 3))
#define bmpColor32(c) (((uint16_t)(((uint8_t*)(c))[3] & 0xF8) << 8) | ((uint16_t)(((uint8_t*)(c))[2] & 0xFC) << 3) | ((((uint8_t*)(c))[1] & 0xF8) >> 3))
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::bmpSkipPixels(fs::File& file, uint8_t bitsPerPixel, size_t len) {
    size_t bytesToSkip = (len * bitsPerPixel) / 8;
    file.seek(bytesToSkip, SeekCur);
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::bmpAddPixels(fs::File& file, uint8_t bitsPerPixel, size_t len) {
    size_t   bytesPerTransaction = bitsPerPixel * 4;
    uint8_t  transBuf[bytesPerTransaction];
    uint16_t pixBuf[32];
    uint8_t* tBuf;
    uint8_t  pixIndex = 0;
    size_t   wIndex = 0, pixNow, bytesNow;
    while(wIndex < len) {
        pixNow = len - wIndex;
        if(pixNow > 32) { pixNow = 32; }
        bytesNow = (pixNow * bitsPerPixel) / 8;
        file.read(transBuf, bytesNow);
        tBuf = transBuf;
        for(pixIndex = 0; pixIndex < pixNow; pixIndex++) {
            if(bitsPerPixel == 32) {
                pixBuf[pixIndex] = (bmpColor32(tBuf));
                tBuf += 4;
            }
            else if(bitsPerPixel == 24) {
                pixBuf[pixIndex] = (bmpColor24(tBuf));
                tBuf += 3;
            }
            else if(bitsPerPixel == 16) {
                pixBuf[pixIndex] = (bmpColor16(tBuf));
                tBuf += 2;
            }
            else if(bitsPerPixel == 8) {
                pixBuf[pixIndex] = (bmpColor8(tBuf));
                tBuf += 1;
            }
            else if(bitsPerPixel == 4) {
                uint16_t g = tBuf[0] & 0xF;
                if(pixIndex & 1) { tBuf += 1; }
                else { g = tBuf[0] >> 4; }
                pixBuf[pixIndex] = ((g << 12) | (g << 7) | (g << 1));
            }
        }

        startWrite();
        writePixels(pixBuf, pixNow);
        endWrite();
        wIndex += pixNow;
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT::drawBmpFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight, uint16_t offX, uint16_t offY) {
    if((x + maxWidth) > m_h_res || (y + maxHeight) > m_v_res) {
        log_e("Bad dimensions given");
        return false;
    }

    if(!maxWidth) { maxWidth = m_h_res- x; }
    if(!maxHeight) { maxHeight = m_v_res - y; }
    // log_i("maxWidth=%i, maxHeight=%i", maxWidth, maxHeight);
    if(!fs.exists(path)) {
        log_e("file %s not exists", path);
        return false;
    }

    File bmp_file = fs.open(path);
    if(!bmp_file) {
        log_e("Failed to open file for reading %s", path);
        return false;
    }
    size_t  headerLen = 0x22;
    size_t  fileSize = bmp_file.size();
    uint8_t headerBuf[headerLen];
    if(fileSize < headerLen || bmp_file.read(headerBuf, headerLen) < headerLen) {
        log_e("Failed to read the file's header");
        bmp_file.close();
        return false;
    }

    if(headerBuf[0] != 'B' || headerBuf[1] != 'M') {
        log_e("Wrong file format");
        bmp_file.close();
        return false;
    }

    // size_t bmpSize = bmpRead32(headerBuf, 0x02);
    uint32_t dataOffset = bmpRead32(headerBuf, 0x0A);
    int32_t  bmpWidthI = bmpRead32(headerBuf, 0x12);
    int32_t  bmpHeightI = bmpRead32(headerBuf, 0x16);
    uint16_t bitsPerPixel = bmpRead16(headerBuf, 0x1C);

    size_t bmpWidth = abs(bmpWidthI);
    size_t bmpHeight = abs(bmpHeightI);

    if(offX >= bmpWidth || offY >= bmpHeight) {
        log_e("Offset Outside of bitmap size");
        bmp_file.close();
        return false;
    }

    size_t bmpMaxWidth = bmpWidth - offX;
    size_t bmpMaxHeight = bmpHeight - offY;
    size_t outWidth = (bmpMaxWidth > maxWidth) ? maxWidth : bmpMaxWidth;
    size_t outHeight = (bmpMaxHeight > maxHeight) ? maxHeight : bmpMaxHeight;
    size_t ovfWidth = bmpMaxWidth - outWidth;
    size_t ovfHeight = bmpMaxHeight - outHeight;

    bmp_file.seek(dataOffset);
    startBitmap(x, y, outWidth, outHeight);

    if(ovfHeight) { bmpSkipPixels(bmp_file, bitsPerPixel, ovfHeight * bmpWidth); }
    if(!offX && !ovfWidth) { bmpAddPixels(bmp_file, bitsPerPixel, outWidth * outHeight); }
    else {
        size_t ih;
        for(ih = 0; ih < outHeight; ih++) {
            if(offX) { bmpSkipPixels(bmp_file, bitsPerPixel, offX); }
            bmpAddPixels(bmp_file, bitsPerPixel, outWidth);
            if(ovfWidth) { bmpSkipPixels(bmp_file, bitsPerPixel, ovfWidth); }
        }
    }

    endBitmap();
    bmp_file.close();
    return true;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫   G I F   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT::drawGifFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint8_t repeat) {
    // debug=true;
    int32_t iterations = repeat;

    gif_next.clear();
    gif_next.shrink_to_fit();
    gif_vals.clear();
    gif_vals.shrink_to_fit();
    gif_stack.clear();
    gif_stack.shrink_to_fit();
    gif_GlobalColorTable.clear();
    gif_GlobalColorTable.shrink_to_fit();
    gif_LocalColorTable.clear();
    gif_LocalColorTable.shrink_to_fit();

    gif_decodeSdFile_firstread = false;
    gif_GlobalColorTableFlag = false;
    gif_LocalColorTableFlag = false;
    gif_SortFlag = false;
    gif_TransparentColorFlag = false;
    gif_UserInputFlag = false;
    gif_ZeroDataBlock = 0;
    gif_InterlaceFlag = false;

    do { // repeat this gif
        gif_file = fs.open(path);
        if(!gif_file) {
            if(tft_info) tft_info("Failed to open file for reading");
            return false;
        }
        GIF_readGifItems();

        // check it's a gif
        if(!gif_GifHeader.startsWith("GIF")) {
            if(tft_info) tft_info("File is not a gif");
            return false;
        }
        // check dimensions
        if(debug) { log_i("Width: %i, Height: %i,", gif_LogicalScreenWidth, gif_LogicalScreenHeight); }
        if(gif_LogicalScreenWidth * gif_LogicalScreenHeight > 155000) {
            if(tft_info) tft_info("!Image is too big!!");
            return false;
        }

        while(GIF_decodeGif(x, y) == true) {
            if(iterations == 0) break;
        } // endwhile
        iterations--;
        gif_file.close();
    } while(iterations > 0);

    GIF_freeMemory();
    // log_i("freeHeap= %i", ESP.getFreeHeap());
    return true;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::GIF_readHeader() {

    //      7 6 5 4 3 2 1 0        Field Name                    Type
    //     +---------------+
    //   0 |               |       Signature                     3 Bytes
    //     +-             -+
    //   1 |               |
    //     +-             -+
    //   2 |               |
    //     +---------------+
    //   3 |               |       Version                       3 Bytes
    //     +-             -+
    //   4 |               |
    //     +-             -+
    //   5 |               |
    //     +---------------+
    //
    //  i) Signature - Identifies the GIF Data Stream. This field contains
    //     the fixed value 'GIF'.
    //
    // ii) Version - Version number used to format the data stream.
    //     Identifies the minimum set of capabilities necessary to a decoder
    //     to fully process the contents of the Data Stream.
    //
    //     Version Numbers as of 10 July 1990 :       "87a" - May 1987
    //                                                "89a" - July 1989
    //
    gif_file.readBytes(gif_buffer, 6); // Header
    gif_buffer[6] = 0;
    gif_GifHeader = gif_buffer;
    if(debug) { log_i("GifHeader: %s", gif_GifHeader.c_str()); }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::GIF_readLogicalScreenDescriptor() {

    //    Logical Screen Descriptor
    //
    //    7 6 5 4 3 2 1 0        Field Name                    Type
    //    +---------------+
    // 0  |               |       Logical Screen Width          Unsigned
    //    +-             -+
    // 1  |               |
    //    +---------------+
    // 2  |               |       Logical Screen Height         Unsigned
    //    +-             -+
    // 3  |               |
    //    +---------------+
    // 4  | |     | |     |       <Packed Fields>               See below
    //    +---------------+
    // 5  |               |       Background Color Index        Byte
    //    +---------------+
    // 6  |               |       Pixel Aspect Ratio            Byte
    //    +---------------+
    //
    gif_file.readBytes(gif_buffer, 7); // Logical Screen Descriptor
    gif_LogicalScreenWidth = gif_buffer[0] + 256 * gif_buffer[1];
    gif_LogicalScreenHeight = gif_buffer[2] + 256 * gif_buffer[3];
    gif_PackedFields = gif_buffer[4];
    gif_GlobalColorTableFlag = (gif_PackedFields & 0x80);
    gif_ColorResulution = ((gif_PackedFields & 0x70) >> 3) + 1;
    gif_SortFlag = (gif_PackedFields & 0x08);
    gif_SizeOfGlobalColorTable = (gif_PackedFields & 0x07);
    gif_SizeOfGlobalColorTable = (1 << (gif_SizeOfGlobalColorTable + 1));
    gif_BackgroundColorIndex = gif_buffer[5];
    gif_PixelAspectRatio = gif_buffer[6];

    if(debug) {
        log_i("LogicalScreenWidth=%i", gif_LogicalScreenWidth);
        log_i("LogicalScreenHeight=%i", gif_LogicalScreenHeight);
        log_i("PackedFields=%i", gif_PackedFields);
        log_i("SortFlag=%i", gif_SortFlag);
        log_i("GlobalColorTableFlag=%i", gif_GlobalColorTableFlag);
        log_i("ColorResulution=%i", gif_ColorResulution);
        log_i("SizeOfGlobalColorTable=%i", gif_SizeOfGlobalColorTable);
        log_i("BackgroundColorIndex=%i", gif_BackgroundColorIndex);
        log_i("PixelAspectRatio=%i", gif_PixelAspectRatio);
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::GIF_readImageDescriptor() {

    //    7 6 5 4 3 2 1 0        Field Name                    Type
    //    +---------------+
    // 0  |               |       Image Separator               Byte, fixed value 0x2C, always read before
    //    +---------------+
    // 1  |               |       Image Left Position           Unsigned
    //    +-             -+
    // 2  |               |
    //    +---------------+
    // 3  |               |       Image Top Position            Unsigned
    //    +-             -+
    // 4  |               |
    //    +---------------+
    // 5  |               |       Image Width                   Unsigned
    //    +-             -+
    // 6  |               |
    //    +---------------+
    // 7  |               |       Image Height                  Unsigned
    //    +-             -+
    // 8  |               |
    //    +---------------+
    // 9  | | | |   |     |       <Packed Fields>               See below
    //    +---------------+
    //
    //    <Packed Fields>  =      Local Color Table Flag        1 Bit
    //                            Interlace Flag                1 Bit
    //                            Sort Flag                     1 Bit
    //                            Reserved                      2 Bits
    //                            Size of Local Color Table     3 Bits

    gif_file.readBytes(gif_buffer, 9); // Image Descriptor
    gif_ImageLeftPosition = gif_buffer[0] + 256 * gif_buffer[1];
    gif_ImageTopPosition = gif_buffer[2] + 256 * gif_buffer[3];
    gif_ImageWidth = gif_buffer[4] + 256 * gif_buffer[5];
    gif_ImageHeight = gif_buffer[6] + 256 * gif_buffer[7];
    gif_PackedFields = gif_buffer[8];
    gif_LocalColorTableFlag = ((gif_PackedFields & 0x80) >> 7);
    gif_InterlaceFlag = ((gif_PackedFields & 0x40) >> 6);
    gif_SortFlag = ((gif_PackedFields & 0x20)) >> 5;
    gif_SizeOfLocalColorTable = (gif_PackedFields & 0x07);
    gif_SizeOfLocalColorTable = (1 << (gif_SizeOfLocalColorTable + 1));

    if(debug) {
        log_i("ImageLeftPosition=%i", gif_ImageLeftPosition);
        log_i("ImageTopPosition=%i", gif_ImageTopPosition);
        log_i("ImageWidth=%i", gif_ImageWidth);
        log_i("ImageHeight=%i", gif_ImageHeight);
        log_i("PackedFields=%i", gif_PackedFields);
        log_i("LocalColorTableFlag=%i", gif_LocalColorTableFlag);
        log_i("InterlaceFlag=%i", gif_InterlaceFlag);
        log_i("SortFlag=%i", gif_SortFlag);
        log_i("SizeOfLocalColorTable=%i", gif_SizeOfLocalColorTable);
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::GIF_readLocalColorTable() {
    // The size of the local color table can be calculated by the value given in the image descriptor.
    // Just like with the global color table, if the image descriptor specifies a size of N,
    // the color table will contain 2^(N+1) colors and will take up 3*2^(N+1) bytes.
    // The colors are specified in RGB value triplets.
    gif_LocalColorTable.clear();
    gif_LocalColorTable.shrink_to_fit();
    gif_LocalColorTable.reserve(gif_SizeOfLocalColorTable);
    if(gif_LocalColorTableFlag == 1) {
        char     rgb_buff[3];
        uint16_t i = 0;
        while(i != gif_SizeOfLocalColorTable) {
            gif_file.readBytes(rgb_buff, 3);
            // fill LocalColorTable, pass 8-bit (each) R,G,B, get back 16-bit packed color
            gif_LocalColorTable.push_back(((rgb_buff[0] & 0xF8) << 8) | ((rgb_buff[1] & 0xFC) << 3) | ((rgb_buff[2] & 0xF8) >> 3));
            i++;
        }
        // for(i=0;i<SizeOfLocalColorTable; i++) log_i("LocalColorTable %i= %i", i, LocalColorTable[i]);
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::GIF_readGlobalColorTable() {
    // Each GIF has its own color palette. That is, it has a list of all the colors that can be in the image
    // and cannot contain colors that are not in that list.
    // The global color table is where that list of colors is stored. Each color is stored in three bytes.
    // Each of the bytes represents an RGB color value. The first byte is the value for red (0-255), next green, then
    // blue. The size of the global color table is determined by the value in the packed byte of the logical screen
    // descriptor. If the value from that byte is N, then the actual number of colors stored is 2^(N+1). This means that
    // the global color table will take up 3*2^(N+1) bytes in the stream.

    //    Value In <Packed Fields>    Number Of Colors    Byte Length
    //        0                           2                   6
    //        1                           4                   12
    //        2                           8                   24
    //        3                           16                  48
    //        4                           32                  96
    //        5                           64                  192
    //        6                           128                 384
    //        7                           256                 768
    gif_GlobalColorTable.clear();
    if(gif_GlobalColorTableFlag == 1) {
        char     rgb_buff[3];
        uint16_t i = 0;
        while(i != gif_SizeOfGlobalColorTable) {
            gif_file.readBytes(rgb_buff, 3);
            // fill GlobalColorTable, pass 8-bit (each) R,G,B, get back 16-bit packed color
            gif_GlobalColorTable.push_back(((rgb_buff[0] & 0xF8) << 8) | ((rgb_buff[1] & 0xFC) << 3) | ((rgb_buff[2] & 0xF8) >> 3));
            i++;
        }
        //    for(i=0;i<gif_SizeOfGlobalColorTable;i++) log_i("GlobalColorTable %i= %i", i, gif_GlobalColorTable[i]);
        //    log_i("Read GlobalColorTable Size=%i", gif_SizeOfGlobalColorTable);
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::GIF_readGraphicControlExtension() {

    //      +---------------+
    //   0  |               |       Block Size                    Byte
    //      +---------------+
    //   1  |     |     | | |       <Packed Fields>               See below
    //      +---------------+
    //   2  |               |       Delay Time                    Unsigned
    //      +-             -+
    //   3  |               |
    //      +---------------+
    //   4  |               |       Transparent Color Index       Byte
    //      +---------------+
    //
    //      +---------------+
    //   0  |               |       Block Terminator              Byte
    //      +---------------+

    uint8_t BlockSize = 0;
    gif_file.readBytes(gif_buffer, 1);
    BlockSize = gif_buffer[0]; // Number of bytes in the block, not including the Block Terminator

    if(BlockSize == 0) return;
    gif_file.readBytes(gif_buffer, BlockSize);
    gif_PackedFields = gif_buffer[0];
    gif_DisposalMethod = (gif_PackedFields & 0x1C) >> 2;
    gif_UserInputFlag = (gif_PackedFields & 0x02);
    gif_TransparentColorFlag = gif_PackedFields & 0x01;
    gif_DelayTime = gif_buffer[1] + 256 * gif_buffer[2];
    gif_TransparentColorIndex = gif_buffer[3];

    if(debug) {
        log_i("BlockSize GraphicontrolExtension=%i", BlockSize);
        log_i("DisposalMethod=%i", gif_DisposalMethod);
        log_i("UserInputFlag=%i", gif_UserInputFlag);
        log_i("TransparentColorFlag=%i", gif_TransparentColorFlag);
        log_i("DelayTime=%i", gif_DelayTime);
        log_i("TransparentColorIndex=%i", gif_TransparentColorIndex);
    }
    gif_file.readBytes(gif_buffer, 1); // marks the end of the Graphic Control Extension
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT::GIF_readPlainTextExtension(char* buf) {

    //     7 6 5 4 3 2 1 0        Field Name                    Type
    //    +---------------+
    // 0  |               |       Block Size                    Byte
    //    +---------------+
    // 1  |               |       Text Grid Left Position       Unsigned
    //    +-             -+
    // 2  |               |
    //    +---------------+
    // 3  |               |       Text Grid Top Position        Unsigned
    //    +-             -+
    // 4  |               |
    //    +---------------+
    // 5  |               |       Text Grid Width               Unsigned
    //    +-             -+
    // 6  |               |
    //    +---------------+
    // 7  |               |       Text Grid Height              Unsigned
    //    +-             -+
    // 8  |               |
    //    +---------------+
    // 9  |               |       Character Cell Width          Byte
    //    +---------------+
    // 10  |               |       Character Cell Height         Byte
    //    +---------------+
    // 11  |               |       Text Foreground Color Index   Byte
    //    +---------------+
    // 12  |               |       Text Background Color Index   Byte
    //    +---------------+
    //
    //    +===============+
    //    |               |
    // N  |               |       Plain Text Data               Data Sub-blocks
    //    |               |
    //    +===============+
    //
    //    +---------------+
    // 0  |               |       Block Terminator              Byte
    //    +---------------+
    //
    uint8_t BlockSize = 0, numBytes = 0;
    BlockSize = gif_file.read();
    // log_i("BlockSize=%i", BlockSize);
    if(BlockSize > 0) {
        gif_file.readBytes(gif_buffer, BlockSize);
        // log_i("%s", buffer);
    }

    gif_TextGridLeftPosition = gif_buffer[0] + 256 * gif_buffer[1];
    gif_TextGridTopPosition = gif_buffer[2] + 256 * gif_buffer[3];
    gif_TextGridWidth = gif_buffer[4] + 256 * gif_buffer[5];
    gif_TextGridHeight = gif_buffer[6] + 256 * gif_buffer[7];
    gif_CharacterCellWidth = gif_buffer[8];
    gif_CharacterCellHeight = gif_buffer[9];
    gif_TextForegroundColorIndex = gif_buffer[10];
    gif_TextBackgroundColorIndex = gif_buffer[11];

    if(debug) {
        log_i("TextGridLeftPosition=%i", gif_TextGridLeftPosition);
        log_i("TextGridTopPosition=%i", gif_TextGridTopPosition);
        log_i("TextGridWidth=%i", gif_TextGridWidth);
        log_i("TextGridHeight=%i", gif_TextGridHeight);
        log_i("CharacterCellWidth=%i", gif_CharacterCellWidth);
        log_i("CharacterCellHeight=%i", gif_CharacterCellHeight);
        log_i("TextForegroundColorIndex=%i", gif_TextForegroundColorIndex);
        log_i("TextBackgroundColorIndex=%i", gif_TextBackgroundColorIndex);
    }

    numBytes = GIF_readDataSubBlock(buf);
    gif_file.readBytes(gif_buffer, 1); // BlockTerminator, marks the end of the Graphic Control Extension
    return numBytes;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT::GIF_readApplicationExtension(char* buf) {

    //     7 6 5 4 3 2 1 0        Field Name                    Type
    //    +---------------+
    // 0  |               |       Block Size                    Byte
    //    +---------------+
    // 1  |               |
    //    +-             -+
    // 2  |               |
    //    +-             -+
    // 3  |               |       Application Identifier        8 Bytes
    //    +-             -+
    // 4  |               |
    //    +-             -+
    // 5  |               |
    //    +-             -+
    // 6  |               |
    //    +-             -+
    // 7  |               |
    //    +-             -+
    // 8  |               |
    //    +---------------+
    // 9  |               |
    //    +-             -+
    // 10  |               |       Appl. Authentication Code     3 Bytes
    //    +-             -+
    // 11  |               |
    //    +---------------+
    //
    //    +===============+
    //    |               |
    //    |               |       Application Data              Data Sub-blocks
    //    |               |
    //    |               |
    //    +===============+
    //
    //    +---------------+
    // 0  |               |       Block Terminator              Byte
    //    +---------------+

    uint8_t BlockSize = 0, numBytes = 0;
    BlockSize = gif_file.read();
    if(debug) { log_i("BlockSize=%i", BlockSize); }
    if(BlockSize > 0) { gif_file.readBytes(gif_buffer, BlockSize); }
    numBytes = GIF_readDataSubBlock(buf);
    gif_file.readBytes(gif_buffer, 1); // BlockTerminator, marks the end of the Graphic Control Extension
    return numBytes;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT::GIF_readCommentExtension(char* buf) {

    //    7 6 5 4 3 2 1 0        Field Name                    Type
    //  +===============+
    //  |               |
    // N |               |       Comment Data                  Data Sub-blocks
    //  |               |
    //  +===============+
    //
    //  +---------------+
    // 0 |               |       Block Terminator              Byte
    //  +---------------+

    uint8_t numBytes = 0;
    numBytes = GIF_readDataSubBlock(buf);
    sprintf(chbuf, "GIF: Comment %s", buf);
    // if(tft_info) tft_info(chbuf);
    gif_file.readBytes(gif_buffer, 1); // BlockTerminator, marks the end of the Graphic Control Extension
    return numBytes;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT::GIF_readDataSubBlock(char* buf) {

    //     7 6 5 4 3 2 1 0        Field Name                    Type
    //    +---------------+
    // 0  |               |       Block Size                    Byte
    //    +---------------+
    // 1  |               |
    //    +-             -+
    // 2  |               |
    //    +-             -+
    // 3  |               |
    //    +-             -+
    //    |               |       Data Values                   Byte
    //    +-             -+
    // up |               |
    //    +-   . . . .   -+
    // to |               |
    //    +-             -+
    //    |               |
    //    +-             -+
    // 255|               |
    //    +---------------+
    //

    uint8_t BlockSize = 0;
    BlockSize = gif_file.read();
    if(BlockSize > 0) {
        gif_ZeroDataBlock = false;
        gif_file.readBytes(buf, BlockSize);
    }
    else gif_ZeroDataBlock = true;
    if(debug) { log_i("Blocksize = %i", BlockSize); }
    return BlockSize;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT::GIF_readExtension(char Label) {
    char buf[256];
    switch(Label) {
        case 0x01:
            if(debug) { log_i("PlainTextExtension"); }
            GIF_readPlainTextExtension(buf);
            break;
        case 0xff:
            if(debug) { log_i("ApplicationExtension"); }
            GIF_readApplicationExtension(buf);
            break;
        case 0xfe:
            if(debug) { log_i("CommentExtension"); }
            GIF_readCommentExtension(buf);
            break;
        case 0xF9:
            if(debug) { log_i("GraphicControlExtension"); }
            GIF_readGraphicControlExtension();
            break;
        default: return false;
    }
    return true;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int32_t TFT::GIF_GetCode(int32_t code_size, int32_t flag) {
    //    Assuming a character array of 8 bits per character and using 5 bit codes to be
    //    packed, an example layout would be similar to:
    //
    //         +---------------+
    //      0  |               |    bbbaaaaa
    //         +---------------+
    //      1  |               |    dcccccbb
    //         +---------------+
    //      2  |               |    eeeedddd
    //         +---------------+
    //      3  |               |    ggfffffe
    //         +---------------+
    //      4  |               |    hhhhhggg
    //         +---------------+
    //               . . .
    //         +---------------+
    //      N  |               |
    //         +---------------+

    static char    DSBbuffer[300];
    static int32_t curbit, lastbit, done, last_byte;
    int32_t        i, j, ret;
    uint8_t        count;

    if(flag) {
        curbit = 0;
        lastbit = 0;
        done = false;
        return 0;
    }

    if((curbit + code_size) >= lastbit) {
        if(done) {
            // log_i("done");
            if(curbit >= lastbit) { return 0; }
            return -1;
        }
        DSBbuffer[0] = DSBbuffer[last_byte - 2];
        DSBbuffer[1] = DSBbuffer[last_byte - 1];

        // The rest of the Image Block represent data sub-blocks. Data sub-blocks are are groups of 1 - 256 bytes.
        // The first byte in the sub-block tells you how many bytes of actual data follow. This can be a value from 0
        // (00) it 255 (FF). After you've read those bytes, the next byte you read will tell you now many more bytes of
        // data follow that one. You continue to read until you reach a sub-block that says that zero bytes follow.
        //    endWrite();
        count = GIF_readDataSubBlock(&DSBbuffer[2]);
        //    startWrite();
        // log_i("Dtatblocksize %i", count);
        if(count == 0) done = true;

        last_byte = 2 + count;

        curbit = (curbit - lastbit) + 16;

        lastbit = (2 + count) * 8;
    }
    ret = 0;
    for(i = curbit, j = 0; j < code_size; ++i, ++j) ret |= ((DSBbuffer[i / 8] & (1 << (i % 8))) != 0) << j;

    curbit += code_size;

    return ret;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int32_t TFT::GIF_LZWReadByte(bool init) {
    static int32_t fresh = false;
    int32_t        code, incode;
    static int32_t firstcode, oldcode;

    if(gif_next.capacity() < (1 << gif_MaxLzwBits)) gif_next.reserve((1 << gif_MaxLzwBits) - gif_next.capacity());
    if(gif_vals.capacity() < (1 << gif_MaxLzwBits)) gif_vals.reserve((1 << gif_MaxLzwBits) - gif_vals.capacity());
    if(gif_stack.capacity() < (1 << (gif_MaxLzwBits + 1))) gif_stack.reserve((1 << (gif_MaxLzwBits + 1)) - gif_stack.capacity());
    gif_next.clear();
    gif_vals.clear();
    gif_stack.clear();

    static uint8_t* sp;

    int32_t i;

    if(init) {
        //    LWZMinCodeSize      ColorCodes      ClearCode       EOICode
        //    2                   #0-#3           #4              #5
        //    3                   #0-#7           #8              #9
        //    4                   #0-#15          #16             #17
        //    5                   #0-#31          #32             #33
        //    6                   #0-#63          #64             #65
        //    7                   #0-#127         #128            #129
        //    8                   #0-#255         #256            #257

        gif_CodeSize = gif_LZWMinimumCodeSize + 1;
        gif_ClearCode = (1 << gif_LZWMinimumCodeSize);
        gif_EOIcode = gif_ClearCode + 1;
        gif_MaxCode = gif_ClearCode + 2;
        gif_MaxCodeSize = 2 * gif_ClearCode;

        if(debug) {
            log_i("ClearCode=%i", gif_ClearCode);
            log_i("EOIcode=%i", gif_EOIcode);
            log_i("CodeSize=%i", gif_CodeSize);
            log_i("MaxCode=%i", gif_MaxCode);
            log_i("MaxCodeSize=%i", gif_MaxCodeSize);
        }

        fresh = false;

        GIF_GetCode(0, true);

        fresh = true;

        for(i = 0; i < gif_ClearCode; i++) {
            gif_next[i] = 0;
            gif_vals[i] = i;
        }
        for(; i < (1 << gif_MaxLzwBits); i++) gif_next[i] = gif_vals[0] = 0;

        sp = &gif_stack[0];

        return 0;
    }
    else if(fresh) {
        fresh = false;
        do { firstcode = oldcode = GIF_GetCode(gif_CodeSize, false); } while(firstcode == gif_ClearCode);

        return firstcode;
    }

    if(sp > &gif_stack[0]) return *--sp;

    while((code = GIF_GetCode(gif_CodeSize, false)) >= 0) {
        if(code == gif_ClearCode) {
            for(i = 0; i < gif_ClearCode; ++i) {
                gif_next[i] = 0;
                gif_vals[i] = i;
            }
            for(; i < (1 << gif_MaxLzwBits); ++i) gif_next[i] = gif_vals[i] = 0;

            gif_CodeSize = gif_LZWMinimumCodeSize + 1;
            gif_MaxCodeSize = 2 * gif_ClearCode;
            gif_MaxCode = gif_ClearCode + 2;
            sp = &gif_stack[0];

            firstcode = oldcode = GIF_GetCode(gif_CodeSize, false);
            return firstcode;
        }
        else if(code == gif_EOIcode) {
            if(debug) { log_i("read EOI Code"); }
            int32_t count;
            char    buf[260];

            if(gif_ZeroDataBlock) return -2;
            while((count = GIF_readDataSubBlock(buf)) > 0);

            if(count != 0) return -2;
        }

        incode = code;

        if(code >= gif_MaxCode) {
            *sp++ = firstcode;
            code = oldcode;
        }

        while(code >= gif_ClearCode) {
            *sp++ = gif_vals[code];
            if(code == (int32_t)gif_next[code]) { return -1; }
            code = gif_next[code];
        }
        *sp++ = firstcode = gif_vals[code];

        if((code = gif_MaxCode) < (1 << gif_MaxLzwBits)) {
            gif_next[code] = oldcode;
            gif_vals[code] = firstcode;
            ++gif_MaxCode;
            if((gif_MaxCode >= gif_MaxCodeSize) && (gif_MaxCodeSize < (1 << gif_MaxLzwBits))) {
                gif_MaxCodeSize *= 2;
                ++gif_CodeSize;
            }
        }
        oldcode = incode;

        if(sp > &gif_stack[0]) return *--sp;
    }
    return code;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT::GIF_ReadImage(uint16_t x, uint16_t y) {
    int32_t  j, color;
    int32_t  xpos = x + gif_ImageLeftPosition;
    int32_t  ypos = y + gif_ImageTopPosition;
    int32_t  max = gif_ImageHeight * gif_ImageWidth;
    uint16_t i = 0;
    // if(gif_DisposalMethod==2) not supported yet
    // if(gif_InterlaceFlag)     not supported yet

    // The first byte of Image Block is the LZW minimum code size.
    // This value is used to decode the compressed output codes.

    gif_LZWMinimumCodeSize = gif_file.read();
    if(debug) { log_i("LZWMinimumCodeSize=%i", gif_LZWMinimumCodeSize); }

    j = GIF_LZWReadByte(true);
    if(j < 0) return false;

    uint16_t* buf = (uint16_t*)__malloc_heap_psram(max * sizeof(uint16_t));
    if(!buf) return false;

    while(i < max) {
        color = GIF_LZWReadByte(false);
        if((color == gif_TransparentColorIndex) && gif_TransparentColorFlag) { ; } // do nothing
        else {
            if(gif_LocalColorTableFlag) buf[i] = gif_LocalColorTable[color];
            else buf[i] = gif_GlobalColorTable[color];
        }
        i++;
    }

    startWrite();
    setAddrWindow(xpos, ypos, gif_ImageWidth, gif_ImageHeight);
    if(_TFTcontroller == ILI9341) {
        writeCommand(ILI9341_RAMWR); // ILI9341
        goto write16;
    }
    else if(_TFTcontroller == HX8347D) {
        writeCommand(0x22);
        goto write16;
    }
    else if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) {
        writeCommand(ILI9486_RAMWR);
        goto write16;
    }
    else if(_TFTcontroller == ILI9488) {
        writeCommand(ILI9488_RAMWR);
        goto write24;
    }
    else if(_TFTcontroller == ST7796 || _TFTcontroller == ST7796RPI) {
        writeCommand(ST7796_RAMWR);
        goto write24;
    }

write16:
    i = 0;
    while(i < max) {
        spi_TFT->write16(buf[i]);
        i++;
    }
    endWrite();
    if(buf) {
        free(buf);
        buf = NULL;
    }
    return true;

write24:
    i = 0;
    while(i < max) {
        write24BitColor(buf[i]);
        i++;
    }
    endWrite();
    if(buf) {
        free(buf);
        buf = NULL;
    }
    return true;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int32_t TFT::GIF_readGifItems() {
    GIF_readHeader();
    GIF_readLogicalScreenDescriptor();
    gif_decodeSdFile_firstread = true;
    return 0;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT::GIF_decodeGif(uint16_t x, uint16_t y) {
    char           c = 0;
    static int32_t test = 1;
    char           Label = 0;
    if(gif_decodeSdFile_firstread == true) GIF_readGlobalColorTable(); // If exists
    gif_decodeSdFile_firstread = false;

    while(c != ';') { // Trailer found
        c = gif_file.read();
        // log_i("c= %c",c);
        if(c == '!') {               // it is a Extension
            Label = gif_file.read(); // Label
            GIF_readExtension(Label);
        }
        if(c == ',') {
            GIF_readImageDescriptor(); // ImgageDescriptor
            GIF_readLocalColorTable(); // can follow the ImagrDescriptor
            GIF_ReadImage(x, y);       // read Image Data
            if(debug) { log_i("End ReadImage"); }
            test++;
            return true; // more images can follow
        }
    }
    // for(int32_t i=0; i<bigbuf.size(); i++)  log_i("bigbuf %i=%i", i, bigbuf[i]);
    // if(tft_info) tft_info("GIF: found Trailer");
    return false; // no more images to decode
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::GIF_freeMemory() {
    gif_next.clear();
    gif_next.shrink_to_fit();
    gif_vals.clear();
    gif_vals.shrink_to_fit();
    gif_stack.clear();
    gif_stack.shrink_to_fit();
    gif_GlobalColorTable.clear();
    gif_GlobalColorTable.shrink_to_fit();
    gif_LocalColorTable.clear();
    gif_LocalColorTable.shrink_to_fit();
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫ J P E G ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT::drawJpgFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight) {
#ifdef RGB_HMI  // RGB HMI display
    if(!fs.exists(path)) {log_e("file %s not exists", path); return false; }
    if(maxWidth) m_jpgWidthMax = maxWidth; else m_jpgWidthMax = m_h_res;
    if(maxHeight) m_jpgHeightMax = maxHeight; else m_jpgHeightMax = m_v_res;

    m_jpgFile = fs.open(path, FILE_READ);
    if(!m_jpgFile) {log_e("Failed to open file for reading"); JPEG_setJpgScale(1); return false;}
    JPEG_getJpgSize(&m_jpgWidth, &m_jpgHeight);
    int r = JPEG_drawJpg(x, y);
    if(r) {log_e("JPEG_drawJpg failed with error %i", r); return false;}
    m_jpgFile.close();
    esp_lcd_panel_draw_bitmap(m_panel, x, y, x + m_jpgWidth, y + m_jpgHeight, m_framebuffer);
    return true;
#endif
#ifdef LCD_SPI // SPI LCD display
    if(!fs.exists(path)) {log_e("file %s not exists", path); return false; }
    if(maxWidth) m_jpgWidthMax = maxWidth; else m_jpgWidthMax = m_h_res;
    if(maxHeight) m_jpgHeightMax = maxHeight; else m_jpgHeightMax = m_v_res;

    m_jpgFile = fs.open(path, FILE_READ);
    if(!m_jpgFile) {log_e("Failed to open file for reading"); JPEG_setJpgScale(1); return false;}
    JPEG_getJpgSize(&m_jpgWidth, &m_jpgHeight);
    int r = JPEG_drawJpg(x, y);
    if(r) {log_e("JPEG_drawJpg failed with error %i", r); return false;}
    m_jpgFile.close();
    return true;
#endif
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::JPEG_setJpgScale(uint8_t scaleFactor) {
    switch (scaleFactor) {
        case 1:  m_jpgScale = 0; break;
        case 2:  m_jpgScale = 1; break;
        case 4:  m_jpgScale = 2; break;
        case 8:  m_jpgScale = 3; break;
        default: m_jpgScale = 0;
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::JPEG_setSwapBytes(bool swapBytes){
  m_swap = swapBytes;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
unsigned int TFT::JPEG_jd_input(JDEC* jdec, uint8_t* buf, unsigned int len){
    uint32_t bytesLeft = 0;

    if (m_jpg_source == TJPG_ARRAY) {  // Handle an array input
        if (m_array_index + len > m_array_size) { len = m_array_size - m_array_index; } // Avoid running off end of array
        if (buf) memcpy_P(buf, (const uint8_t*)(m_array_data + m_array_index), len); // If buf is valid then copy len bytes to buffer
        m_array_index += len;  // Move pointer
    }
    else if (m_jpg_source == TJPG_SD_FILE) {  // Handle SD library input
        bytesLeft = m_jpgFile.available();  // Check how many bytes are available
        if (bytesLeft < len) len = bytesLeft;
        if (buf) {
            m_jpgFile.read(buf, len); // Read into buffer, pointer moved as well
        } else {
            m_jpgFile.seek(m_jpgFile.position() + len); // Buffer is null, so skip data by moving pointer
        }
    }
    return len;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
// Pass image block back to the sketch for rendering, may be a complete or partial MCU
int TFT::JPEG_jd_output(JDEC* jdec, void* bitmap, JRECT* jrect) {
    jdec = jdec; // Supress warning as ID is not used

    int16_t  x = jrect->left + m_jpeg_x;  // Retrieve rendering parameters and add any offset
    int16_t  y = jrect->top + m_jpeg_y;
    uint16_t w = jrect->right + 1 - jrect->left;
    uint16_t h = jrect->bottom + 1 - jrect->top;
    if(w > m_jpgWidthMax) return true;  // Clip width and height to the maximum allowed dimensions
    if(h > m_jpgHeightMax) return true;
    bool r = JPEG_tft_output(x, y, w, h, (uint16_t*)bitmap);
    return r;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
bool TFT::JPEG_tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
#ifdef RGB_HMI  // RGB HMI display
    if (!bitmap || w <= 0 || h <= 0) {  // Check for valid parameters
        log_e("Invalid parameters: bitmap is null or width/height is zero.");
        return false;
    }
    // Clip the rectangle to ensure it doesn't exceed framebuffer boundaries
    int16_t x_end = std::min((int16_t)(x + w), (int16_t)m_h_res); // End of rectangle in x-direction
    int16_t y_end = std::min((int16_t)(y + h), (int16_t)(m_v_res)); // End of rectangle in y-direction

    if (x >= m_h_res || y >= m_v_res || x_end <= 0 || y_end <= 0) {
        log_e("Rectangle is completely outside the framebuffer boundaries.");
        return false;
    }

    // Adjust start coordinates if they are out of bounds
    int16_t start_x = max((int16_t)0, x);        // Sichtbarer Startpunkt in x-Richtung
    int16_t start_y = max((int16_t)0, y);        // Sichtbarer Startpunkt in y-Richtung
    int16_t clip_x_offset = start_x - x;         // Offset im Bitmap in x-Richtung
    int16_t clip_y_offset = start_y - y;         // Offset im Bitmap in y-Richtung

    // Berechnung der sichtbaren Breite und Höhe
    int16_t visible_w = x_end - start_x;         // Sichtbare Breite
    int16_t visible_h = y_end - start_y;         // Sichtbare Höhe

    // Zeilenweises Kopieren mit Clipping
    for (int16_t j = 0; j < visible_h ; ++j) {
        // Quelle im Bitmap: Berechne die richtige Zeilenposition
        uint16_t* src_ptr = bitmap + (clip_y_offset + j) * w + clip_x_offset;

        // Ziel im Framebuffer: Berechne die richtige Zeilenposition
        uint16_t* dest_ptr = m_framebuffer + (start_y + j) * m_h_res + start_x;

        // Kopiere nur die sichtbare Breite
        memcpy(dest_ptr, src_ptr, visible_w * sizeof(uint16_t));
    }
 //   log_i("Bitmap erfolgreich mit Clipping gezeichnet bei x: %d, y: %d, sichtbare Breite: %d, sichtbare Höhe: %d", start_x, start_y, visible_w, visible_h);
    return true;
#endif
#ifdef LCD_SPI
    int mcu_pixels = w * h; // Number of pixels in MCU block
    startWrite();
    setAddrWindow(x, y, w, h);
    if (_TFTcontroller == ILI9341) {
        writeCommand(ILI9341_RAMWR);                      // ILI9341
        while (mcu_pixels--) spi_TFT->write16(*bitmap++); // Send to TFT 16 bits at a time
    } else if (_TFTcontroller == HX8347D) {
        writeCommand(0x22);
        while (mcu_pixels--) spi_TFT->write16(*bitmap++); // Send to TFT 16 bits at a time
    } else if (_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) {
        writeCommand(ILI9486_RAMWR);
        while (mcu_pixels--) spi_TFT->write16(*bitmap++); // Send to TFT 16 bits at a time
    } else if (_TFTcontroller == ILI9488) {
        writeCommand(ILI9488_RAMWR);
        while (mcu_pixels--) write24BitColor(*bitmap++); // Send to TFT 16 bits at a time
    } else if (_TFTcontroller == ST7796 || _TFTcontroller == ST7796RPI) {
        writeCommand(ST7796_RAMWR);
        while (mcu_pixels--) write24BitColor(*bitmap++); // Send to TFT 16 bits at a time
    }
    endWrite();

    return true;
#endif
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT::JPEG_drawJpg(int32_t x, int32_t y) {
    JDEC    jdec;
    uint8_t r = JDR_OK;

    m_jpg_source = TJPG_SD_FILE;
    m_jpeg_x = x;
    m_jpeg_y = y;
    jdec.swap = m_swap;
    r = JPEG_jd_prepare(&jdec, m_workspace, TJPGD_WORKSPACE_SIZE, 0);
    if (r == JDR_OK) { r = JPEG_jd_decomp(&jdec, m_jpgScale); } // Extract image and render
    return r;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT::JPEG_getJpgSize(uint16_t* w, uint16_t* h) {
    JDEC    jdec;
    uint8_t r = JDR_OK;
    *w = 0;
    *h = 0;

    m_jpg_source = TJPG_SD_FILE;
    r = JPEG_jd_prepare(&jdec, m_workspace, TJPGD_WORKSPACE_SIZE, 0);
    if (r == JDR_OK) {
        *w = jdec.width;
        *h = jdec.height;
    }
    m_jpgFile.seek(0);
    return r;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
#if JD_FASTDECODE == 2
	#define HUFF_BIT  10 /* Bit length to apply fast huffman decode */
	#define HUFF_LEN  (1 << HUFF_BIT)
	#define HUFF_MASK (HUFF_LEN - 1)
#endif

const uint8_t Zig[64] = {/* Zigzag-order to raster-order conversion table */
								0,  1,  8,  16, 9,  2,  3,  10, 17, 24, 32, 25, 18, 11, 4,  5,  12, 19, 26, 33, 40, 48,
								41, 34, 27, 20, 13, 6,  7,  14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23,
								30, 37, 44, 51, 58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63};

const uint16_t Ipsf[64] = {/* See also aa_idct.png */
	 (uint16_t)(1.00000 * 8192), (uint16_t)(1.38704 * 8192), (uint16_t)(1.30656 * 8192), (uint16_t)(1.17588 * 8192),
	 (uint16_t)(1.00000 * 8192), (uint16_t)(0.78570 * 8192), (uint16_t)(0.54120 * 8192), (uint16_t)(0.27590 * 8192),
	 (uint16_t)(1.38704 * 8192), (uint16_t)(1.92388 * 8192), (uint16_t)(1.81226 * 8192), (uint16_t)(1.63099 * 8192),
	 (uint16_t)(1.38704 * 8192), (uint16_t)(1.08979 * 8192), (uint16_t)(0.75066 * 8192), (uint16_t)(0.38268 * 8192),
	 (uint16_t)(1.30656 * 8192), (uint16_t)(1.81226 * 8192), (uint16_t)(1.70711 * 8192), (uint16_t)(1.53636 * 8192),
	 (uint16_t)(1.30656 * 8192), (uint16_t)(1.02656 * 8192), (uint16_t)(0.70711 * 8192), (uint16_t)(0.36048 * 8192),
	 (uint16_t)(1.17588 * 8192), (uint16_t)(1.63099 * 8192), (uint16_t)(1.53636 * 8192), (uint16_t)(1.38268 * 8192),
	 (uint16_t)(1.17588 * 8192), (uint16_t)(0.92388 * 8192), (uint16_t)(0.63638 * 8192), (uint16_t)(0.32442 * 8192),
	 (uint16_t)(1.00000 * 8192), (uint16_t)(1.38704 * 8192), (uint16_t)(1.30656 * 8192), (uint16_t)(1.17588 * 8192),
	 (uint16_t)(1.00000 * 8192), (uint16_t)(0.78570 * 8192), (uint16_t)(0.54120 * 8192), (uint16_t)(0.27590 * 8192),
	 (uint16_t)(0.78570 * 8192), (uint16_t)(1.08979 * 8192), (uint16_t)(1.02656 * 8192), (uint16_t)(0.92388 * 8192),
	 (uint16_t)(0.78570 * 8192), (uint16_t)(0.61732 * 8192), (uint16_t)(0.42522 * 8192), (uint16_t)(0.21677 * 8192),
	 (uint16_t)(0.54120 * 8192), (uint16_t)(0.75066 * 8192), (uint16_t)(0.70711 * 8192), (uint16_t)(0.63638 * 8192),
	 (uint16_t)(0.54120 * 8192), (uint16_t)(0.42522 * 8192), (uint16_t)(0.29290 * 8192), (uint16_t)(0.14932 * 8192),
	 (uint16_t)(0.27590 * 8192), (uint16_t)(0.38268 * 8192), (uint16_t)(0.36048 * 8192), (uint16_t)(0.32442 * 8192),
	 (uint16_t)(0.27590 * 8192), (uint16_t)(0.21678 * 8192), (uint16_t)(0.14932 * 8192), (uint16_t)(0.07612 * 8192)};

#if JD_TBLCLIP
	#define BYTECLIP(v) Clip8[(unsigned int)(v) & 0x3FF]
const uint8_t Clip8[1024] = {	/* 0..255 */
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
	31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
	60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88,
	89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113,
	114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136,
	137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182,
	183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205,
	206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228,
	229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251,
	252, 253, 254, 255,
	/* 256..511 */
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255,
	/* -512..-257 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* -256..-1 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

#if JD_TBLCLIP == 0 /* JD_TBLCLIP */
uint8_t TFT::JPEG_BYTECLIP(int val) {
    if(val < 0) return 0;
    else if(val > 255) return 255;
    return (uint8_t)val;
}
#endif
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void* TFT::JPEG_alloc_pool(JDEC  *jd,size_t ndata) {
	char *rp = 0;

	ndata = (ndata + 3) & ~3; /* Align block size to the word boundary */

	if(jd->sz_pool >= ndata) {
		jd->sz_pool -= ndata;
		rp = (char *)jd->pool;           /* Get start of available memory pool */
		jd->pool = (void *)(rp + ndata); /* Allocate requierd bytes */
	}

	return (void *)rp; /* Return allocated memory block (NULL:no memory to allocate) */
}//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT::JPEG_create_qt_tbl(JDEC* jd, const uint8_t* data, size_t ndata) {
    unsigned int i, zi;
    uint8_t      d;
    int32_t*     pb;

    while (ndata) {                      /* Process all tables in the segment */
        if (ndata < 65) return JDR_FMT1; /* Err: table size is unaligned */
        ndata -= 65;
        d = *data++;                                              /* Get table property */
        if (d & 0xF0) return JDR_FMT1;                            /* Err: not 8-bit resolution */
        i = d & 3;                                                /* Get table ID */
        pb = (int32_t*)JPEG_alloc_pool(jd, 64 * sizeof(int32_t)); /* Allocate a memory block for the table */
        if (!pb) return JDR_MEM1;                                 /* Err: not enough memory */
        jd->qttbl[i] = pb;                                        /* Register the table */
        for (i = 0; i < 64; i++) {                                /* Load the table */
            zi = Zig[i];                                          /* Zigzag-order to raster-order conversion */
            pb[zi] = (int32_t)((uint32_t)*data++ * Ipsf[zi]);     /* Apply scale factor of Arai algorithm to the de-quantizers */
        }
    }

    return JDR_OK;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT::JPEG_create_huffman_tbl(JDEC* jd, const uint8_t* data, size_t ndata) {
    unsigned int i, j, b, cls, num;
    size_t       np;
    uint8_t      d, *pb, *pd;
    uint16_t     hc, *ph;

    while (ndata) {                      /* Process all tables in the segment */
        if (ndata < 17) return JDR_FMT1; /* Err: wrong data size */
        ndata -= 17;
        d = *data++;                   /* Get table number and class */
        if (d & 0xEE) return JDR_FMT1; /* Err: invalid class/number */
        cls = d >> 4;
        num = d & 0x0F;                         /* class = dc(0)/ac(1), table number = 0/1 */
        pb = (uint8_t*)JPEG_alloc_pool(jd, 16); /* Allocate a memory block for the bit distribution table */
        if (!pb) return JDR_MEM1;               /* Err: not enough memory */
        jd->huffbits[num][cls] = pb;
        for (np = i = 0; i < 16; i++) { /* Load number of patterns for 1 to 16-bit code */
            np += (pb[i] = *data++);    /* Get sum of code words for each code */
        }
        ph = (uint16_t*)JPEG_alloc_pool(jd, np * sizeof(uint16_t)); /* Allocate a memory block for the code word table */
        if (!ph) return JDR_MEM1;                                   /* Err: not enough memory */
        jd->huffcode[num][cls] = ph;
        hc = 0;
        for (j = i = 0; i < 16; i++) { /* Re-build huffman code word table */
            b = pb[i];
            while (b--) ph[j++] = hc++;
            hc <<= 1;
        }

        if (ndata < np) return JDR_FMT1; /* Err: wrong data size */
        ndata -= np;
        pd = (uint8_t*)JPEG_alloc_pool(jd, np); /* Allocate a memory block for the decoded data */
        if (!pd) return JDR_MEM1;               /* Err: not enough memory */
        jd->huffdata[num][cls] = pd;
        for (i = 0; i < np; i++) { /* Load decoded data corresponds to each code word */
            d = *data++;
            if (!cls && d > 11) return JDR_FMT1;
            pd[i] = d;
        }
#if JD_FASTDECODE == 2
        { /* Create fast huffman decode table */
            unsigned int span, td, ti;
            uint16_t*    tbl_ac = 0;
            uint8_t*     tbl_dc = 0;

            if (cls) {
                tbl_ac = alloc_pool(jd, HUFF_LEN * sizeof(uint16_t)); /* LUT for AC elements */
                if (!tbl_ac) return JDR_MEM1;                         /* Err: not enough memory */
                jd->hufflut_ac[num] = tbl_ac;
                memset(tbl_ac, 0xFF, HUFF_LEN * sizeof(uint16_t)); /* Default value (0xFFFF: may be long code) */
            } else {
                tbl_dc = alloc_pool(jd, HUFF_LEN * sizeof(uint8_t)); /* LUT for AC elements */
                if (!tbl_dc) return JDR_MEM1;                        /* Err: not enough memory */
                jd->hufflut_dc[num] = tbl_dc;
                memset(tbl_dc, 0xFF, HUFF_LEN * sizeof(uint8_t)); /* Default value (0xFF: may be long code) */
            }
            for (i = b = 0; b < HUFF_BIT; b++) { /* Create LUT */
                for (j = pb[b]; j; j--) {
                    ti = ph[i] << (HUFF_BIT - 1 - b) & HUFF_MASK; /* Index of input pattern for the code */
                    if (cls) {
                        td = pd[i++] | ((b + 1) << 8); /* b15..b8: code length, b7..b0: zero run and data length */
                        for (span = 1 << (HUFF_BIT - 1 - b); span; span--, tbl_ac[ti++] = (uint16_t)td);
                    } else {
                        td = pd[i++] | ((b + 1) << 4); /* b7..b4: code length, b3..b0: data length */
                        for (span = 1 << (HUFF_BIT - 1 - b); span; span--, tbl_dc[ti++] = (uint8_t)td);
                    }
                }
            }
            jd->longofs[num][cls] = i; /* Code table offset for long code */
        }
#endif
    }

    return JDR_OK;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int TFT::JPEG_huffext(JDEC* jd, unsigned int id, unsigned int cls) {
    size_t       dc = jd->dctr;
    uint8_t*     dp = jd->dptr;
    unsigned int d, flg = 0;

#if JD_FASTDECODE == 0
    uint8_t         bm, nd, bl;
    const uint8_t*  hb = jd->huffbits[id][cls]; /* Bit distribution table */
    const uint16_t* hc = jd->huffcode[id][cls]; /* Code word table */
    const uint8_t*  hd = jd->huffdata[id][cls]; /* Data table */

    bm = jd->dbit; /* Bit mask to extract */
    d = 0;
    bl = 16; /* Max code length */
    do {
        if (!bm) {              /* Next byte? */
            if (!dc) {          /* No input data is available, re-fill input buffer */
                dp = jd->inbuf; /* Top of input buffer */
                dc = jd->infunc(jd, dp, JD_SZBUF);
                if (!dc) return 0 - (int)JDR_INP; /* Err: read error or wrong stream termination */
            } else {
                dp++; /* Next data ptr */
            }
            dc--;                                       /* Decrement number of available bytes */
            if (flg) {                                  /* In flag sequence? */
                flg = 0;                                /* Exit flag sequence */
                if (*dp != 0) return 0 - (int)JDR_FMT1; /* Err: unexpected flag is detected (may be collapted data) */
                *dp = 0xFF;                             /* The flag is a data 0xFF */
            } else {
                if (*dp == 0xFF) { /* Is start of flag sequence? */
                    flg = 1;
                    continue; /* Enter flag sequence, get trailing byte */
                }
            }
            bm = 0x80; /* Read from MSB */
        }
        d <<= 1; /* Get a bit */
        if (*dp & bm) d++;
        bm >>= 1;

        for (nd = *hb++; nd; nd--) { /* Search the code word in this bit length */
            if (d == *hc++) {        /* Matched? */
                jd->dbit = bm;
                jd->dctr = dc;
                jd->dptr = dp;
                return *hd; /* Return the decoded data */
            }
            hd++;
        }
        bl--;
    } while (bl);

#else
    const uint8_t * hb, *hd;
    const uint16_t* hc;
    unsigned int    nc, bl, wbit = jd->dbit % 32;
    uint32_t        w = jd->wreg & ((1UL << wbit) - 1);

    while (wbit < 16) { /* Prepare 16 bits into the working register */
        if (jd->marker) {
            d = 0xFF; /* Input stream has stalled for a marker. Generate stuff bits */
        } else {
            if (!dc) {          /* Buffer empty, re-fill input buffer */
                dp = jd->inbuf; /* Top of input buffer */
                dc = JPEG_jd_input(jd, dp, JD_SZBUF);
                if (!dc) return 0 - (int)JDR_INP; /* Err: read error or wrong stream termination */
            }
            d = *dp++;
            dc--;
            if (flg) {                      /* In flag sequence? */
                flg = 0;                    /* Exit flag sequence */
                if (d != 0) jd->marker = d; /* Not an escape of 0xFF but a marker */
                d = 0xFF;
            } else {
                if (d == 0xFF) { /* Is start of flag sequence? */
                    flg = 1;
                    continue; /* Enter flag sequence, get trailing byte */
                }
            }
        }
        w = w << 8 | d; /* Shift 8 bits in the working register */
        wbit += 8;
    }
    jd->dctr = dc;
    jd->dptr = dp;
    jd->wreg = w;

    #if JD_FASTDECODE == 2
    /* Table serch for the short codes */
    d = (unsigned int)(w >> (wbit - HUFF_BIT)); /* Short code as table index */
    if (cls) {                                  /* AC element */
        d = jd->hufflut_ac[id][d];              /* Table decode */
        if (d != 0xFFFF) {                      /* It is done if hit in short code */
            jd->dbit = wbit - (d >> 8);         /* Snip the code length */
            return d & 0xFF;                    /* b7..0: zero run and following data bits */
        }
    } else {                            /* DC element */
        d = jd->hufflut_dc[id][d];      /* Table decode */
        if (d != 0xFF) {                /* It is done if hit in short code */
            jd->dbit = wbit - (d >> 4); /* Snip the code length  */
            return d & 0xF;             /* b3..0: following data bits */
        }
    }

    /* Incremental serch for the codes longer than HUFF_BIT */
    hb = jd->huffbits[id][cls] + HUFF_BIT;             /* Bit distribution table */
    hc = jd->huffcode[id][cls] + jd->longofs[id][cls]; /* Code word table */
    hd = jd->huffdata[id][cls] + jd->longofs[id][cls]; /* Data table */
    bl = HUFF_BIT + 1;
    #else
    /* Incremental serch for all codes */
    hb = jd->huffbits[id][cls]; /* Bit distribution table */
    hc = jd->huffcode[id][cls]; /* Code word table */
    hd = jd->huffdata[id][cls]; /* Data table */
    bl = 1;
    #endif
    for (; bl <= 16; bl++) { /* Incremental search */
        nc = *hb++;
        if (nc) {
            d = w >> (wbit - bl);
            do {                          /* Search the code word in this bit length */
                if (d == *hc++) {         /* Matched? */
                    jd->dbit = wbit - bl; /* Snip the huffman code */
                    return *hd;           /* Return the decoded data */
                }
                hd++;
            } while (--nc);
        }
    }
#endif
    return 0 - (int)JDR_FMT1; /* Err: code not found (may be collapted data) */
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
int TFT::JPEG_bitext(JDEC* jd, unsigned int nbit) {
    size_t       dc = jd->dctr;
    uint8_t*     dp = jd->dptr;
    unsigned int d, flg = 0;

#if JD_FASTDECODE == 0
    uint8_t mbit = jd->dbit;
    d = 0;
    do {
        if (!mbit) {            /* Next byte? */
            if (!dc) {          /* No input data is available, re-fill input buffer */
                dp = jd->inbuf; /* Top of input buffer */
                dc = jd->infunc(jd, dp, JD_SZBUF);
                if (!dc) return 0 - (int)JDR_INP; /* Err: read error or wrong stream termination */
            } else {
                dp++; /* Next data ptr */
            }
            dc--;                                       /* Decrement number of available bytes */
            if (flg) {                                  /* In flag sequence? */
                flg = 0;                                /* Exit flag sequence */
                if (*dp != 0) return 0 - (int)JDR_FMT1; /* Err: unexpected flag is detected (may be collapted data) */
                *dp = 0xFF;                             /* The flag is a data 0xFF */
            } else {
                if (*dp == 0xFF) { /* Is start of flag sequence? */
                    flg = 1;
                    continue; /* Enter flag sequence */
                }
            }
            mbit = 0x80; /* Read from MSB */
        }
        d <<= 1; /* Get a bit */
        if (*dp & mbit) d |= 1;
        mbit >>= 1;
        nbit--;
    } while (nbit);

    jd->dbit = mbit;
    jd->dctr = dc;
    jd->dptr = dp;
    return (int)d;

#else
    unsigned int wbit = jd->dbit % 32;
    uint32_t     w = jd->wreg & ((1UL << wbit) - 1);

    while (wbit < nbit) { /* Prepare nbit bits into the working register */
        if (jd->marker) {
            d = 0xFF; /* Input stream stalled, generate stuff bits */
        } else {
            if (!dc) {          /* Buffer empty, re-fill input buffer */
                dp = jd->inbuf; /* Top of input buffer */
                dc = JPEG_jd_input(jd, dp, JD_SZBUF);
                if (!dc) return 0 - (int)JDR_INP; /* Err: read error or wrong stream termination */
            }
            d = *dp++;
            dc--;
            if (flg) {                      /* In flag sequence? */
                flg = 0;                    /* Exit flag sequence */
                if (d != 0) jd->marker = d; /* Not an escape of 0xFF but a marker */
                d = 0xFF;
            } else {
                if (d == 0xFF) { /* Is start of flag sequence? */
                    flg = 1;
                    continue; /* Enter flag sequence, get trailing byte */
                }
            }
        }
        w = w << 8 | d; /* Get 8 bits into the working register */
        wbit += 8;
    }
    jd->wreg = w;
    jd->dbit = wbit - nbit;
    jd->dctr = dc;
    jd->dptr = dp;

    return (int)(w >> ((wbit - nbit) % 32));
#endif
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT::JPEG_restart(JDEC* jd, uint16_t rstn) {
    unsigned int i;
    uint8_t*     dp = jd->dptr;
    size_t       dc = jd->dctr;

#if JD_FASTDECODE == 0
    uint16_t d = 0;

    /* Get two bytes from the input stream */
    for (i = 0; i < 2; i++) {
        if (!dc) { /* No input data is available, re-fill input buffer */
            dp = jd->inbuf;
            dc = jd->infunc(jd, dp, JD_SZBUF);
            if (!dc) return JDR_INP;
        } else {
            dp++;
        }
        dc--;
        d = d << 8 | *dp; /* Get a byte */
    }
    jd->dptr = dp;
    jd->dctr = dc;
    jd->dbit = 0;

    /* Check the marker */
    if ((d & 0xFFD8) != 0xFFD0 || (d & 7) != (rstn & 7)) { return JDR_FMT1; /* Err: expected RSTn marker is not detected (may be collapted data) */ }

#else
    uint16_t marker;

    if (jd->marker) { /* Generate a maker if it has been detected */
        marker = 0xFF00 | jd->marker;
        jd->marker = 0;
    } else {
        marker = 0;
        for (i = 0; i < 2; i++) { /* Get a restart marker */
            if (!dc) {            /* No input data is available, re-fill input buffer */
                dp = jd->inbuf;
                dc = JPEG_jd_input(jd, dp, JD_SZBUF);
                if (!dc) return JDR_INP;
            }
            marker = (marker << 8) | *dp++; /* Get a byte */
            dc--;
        }
        jd->dptr = dp;
        jd->dctr = dc;
    }

    /* Check the marker */
    if ((marker & 0xFFD8) != 0xFFD0 || (marker & 7) != (rstn & 7)) { return JDR_FMT1; /* Err: expected RSTn marker was not detected (may be collapted data) */ }

    jd->dbit = 0; /* Discard stuff bits */
#endif

    jd->dcv[2] = jd->dcv[1] = jd->dcv[0] = 0; /* Reset DC offset */
    return JDR_OK;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
void TFT::JPEG_block_idct(int32_t* src, jd_yuv_t* dst) {
    const int32_t M13 = (int32_t)(1.41421 * 4096), M2 = (int32_t)(1.08239 * 4096), M4 = (int32_t)(2.61313 * 4096), M5 = (int32_t)(1.84776 * 4096);
    int32_t       v0, v1, v2, v3, v4, v5, v6, v7;
    int32_t       t10, t11, t12, t13;
    int           i;

    /* Process columns */
    for (i = 0; i < 8; i++) {
        v0 = src[8 * 0]; /* Get even elements */
        v1 = src[8 * 2];
        v2 = src[8 * 4];
        v3 = src[8 * 6];

        t10 = v0 + v2; /* Process the even elements */
        t12 = v0 - v2;
        t11 = (v1 - v3) * M13 >> 12;
        v3 += v1;
        t11 -= v3;
        v0 = t10 + v3;
        v3 = t10 - v3;
        v1 = t11 + t12;
        v2 = t12 - t11;

        v4 = src[8 * 7]; /* Get odd elements */
        v5 = src[8 * 1];
        v6 = src[8 * 5];
        v7 = src[8 * 3];

        t10 = v5 - v4; /* Process the odd elements */
        t11 = v5 + v4;
        t12 = v6 - v7;
        v7 += v6;
        v5 = (t11 - v7) * M13 >> 12;
        v7 += t11;
        t13 = (t10 + t12) * M5 >> 12;
        v4 = t13 - (t10 * M2 >> 12);
        v6 = t13 - (t12 * M4 >> 12) - v7;
        v5 -= v6;
        v4 -= v5;

        src[8 * 0] = v0 + v7; /* Write-back transformed values */
        src[8 * 7] = v0 - v7;
        src[8 * 1] = v1 + v6;
        src[8 * 6] = v1 - v6;
        src[8 * 2] = v2 + v5;
        src[8 * 5] = v2 - v5;
        src[8 * 3] = v3 + v4;
        src[8 * 4] = v3 - v4;

        src++; /* Next column */
    }

    /* Process rows */
    src -= 8;
    for (i = 0; i < 8; i++) {
        v0 = src[0] + (128L << 8); /* Get even elements (remove DC offset (-128) here) */
        v1 = src[2];
        v2 = src[4];
        v3 = src[6];

        t10 = v0 + v2; /* Process the even elements */
        t12 = v0 - v2;
        t11 = (v1 - v3) * M13 >> 12;
        v3 += v1;
        t11 -= v3;
        v0 = t10 + v3;
        v3 = t10 - v3;
        v1 = t11 + t12;
        v2 = t12 - t11;

        v4 = src[7]; /* Get odd elements */
        v5 = src[1];
        v6 = src[5];
        v7 = src[3];

        t10 = v5 - v4; /* Process the odd elements */
        t11 = v5 + v4;
        t12 = v6 - v7;
        v7 += v6;
        v5 = (t11 - v7) * M13 >> 12;
        v7 += t11;
        t13 = (t10 + t12) * M5 >> 12;
        v4 = t13 - (t10 * M2 >> 12);
        v6 = t13 - (t12 * M4 >> 12) - v7;
        v5 -= v6;
        v4 -= v5;

        /* Descale the transformed values 8 bits and output a row */
#if JD_FASTDECODE >= 1
        dst[0] = (int16_t)((v0 + v7) >> 8);
        dst[7] = (int16_t)((v0 - v7) >> 8);
        dst[1] = (int16_t)((v1 + v6) >> 8);
        dst[6] = (int16_t)((v1 - v6) >> 8);
        dst[2] = (int16_t)((v2 + v5) >> 8);
        dst[5] = (int16_t)((v2 - v5) >> 8);
        dst[3] = (int16_t)((v3 + v4) >> 8);
        dst[4] = (int16_t)((v3 - v4) >> 8);
#else
        dst[0] = BYTECLIP((v0 + v7) >> 8);
        dst[7] = BYTECLIP((v0 - v7) >> 8);
        dst[1] = BYTECLIP((v1 + v6) >> 8);
        dst[6] = BYTECLIP((v1 - v6) >> 8);
        dst[2] = BYTECLIP((v2 + v5) >> 8);
        dst[5] = BYTECLIP((v2 - v5) >> 8);
        dst[3] = BYTECLIP((v3 + v4) >> 8);
        dst[4] = BYTECLIP((v3 - v4) >> 8);
#endif

        dst += 8;
        src += 8; /* Next row */
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT::JPEG_mcu_load(JDEC* jd) {
    int32_t*       tmp = (int32_t*)jd->workbuf; /* Block working buffer for de-quantize and IDCT */
    int            d, e;
    unsigned int   blk, nby, i, bc, z, id, cmp;
    jd_yuv_t*      bp;
    const int32_t* dqf;

    nby = jd->msx * jd->msy; /* Number of Y blocks (1, 2 or 4) */
    bp = jd->mcubuf;         /* Pointer to the first block of MCU */

    for (blk = 0; blk < nby + 2; blk++) {      /* Get nby Y blocks and two C blocks */
        cmp = (blk < nby) ? 0 : blk - nby + 1; /* Component number 0:Y, 1:Cb, 2:Cr */

        if (cmp && jd->ncomp != 3) { /* Clear C blocks if not exist (monochrome image) */
            for (i = 0; i < 64; bp[i++] = 128);
        } else {              /* Load Y/C blocks from input stream */
            id = cmp ? 1 : 0; /* Huffman table ID of this component */

            /* Extract a DC element from input stream */
            d = JPEG_huffext(jd, id, 0);        /* Extract a huffman coded data (bit length) */
            if (d < 0) return (uint8_t)(0 - d); /* Err: invalid code or input */
            bc = (unsigned int)d;
            d = jd->dcv[cmp];                       /* DC value of previous block */
            if (bc) {                               /* If there is any difference from previous block */
                e = JPEG_bitext(jd, bc);            /* Extract data bits */
                if (e < 0) return (uint8_t)(0 - e); /* Err: input */
                bc = 1 << (bc - 1);                 /* MSB position */
                if (!(e & bc)) e -= (bc << 1) - 1;  /* Restore negative value if needed */
                d += e;                             /* Get current value */
                jd->dcv[cmp] = (int16_t)d;          /* Save current DC value for next block */
            }
            dqf = jd->qttbl[jd->qtid[cmp]]; /* De-quantizer table ID for this component */
            tmp[0] = d * dqf[0] >> 8;       /* De-quantize, apply scale factor of Arai algorithm and descale 8 bits */

            /* Extract following 63 AC elements from input stream */
            memset(&tmp[1], 0, 63 * sizeof(int32_t)); /* Initialize all AC elements */
            z = 1;                                    /* Top of the AC elements (in zigzag-order) */
            do {
                d = JPEG_huffext(jd, id, 1);        /* Extract a huffman coded value (zero runs and bit length) */
                if (d == 0) break;                  /* EOB? */
                if (d < 0) return (uint8_t)(0 - d); /* Err: invalid code or input error */
                bc = (unsigned int)d;
                z += bc >> 4;                           /* Skip leading zero run */
                if (z >= 64) return JDR_FMT1;           /* Too long zero run */
                if (bc &= 0x0F) {                       /* Bit length? */
                    d = JPEG_bitext(jd, bc);            /* Extract data bits */
                    if (d < 0) return (uint8_t)(0 - d); /* Err: input device */
                    bc = 1 << (bc - 1);                 /* MSB position */
                    if (!(d & bc)) d -= (bc << 1) - 1;  /* Restore negative value if needed */
                    i = Zig[z];                         /* Get raster-order index */
                    tmp[i] = d * dqf[i] >> 8;           /* De-quantize, apply scale factor of Arai algorithm and descale 8 bits */
                }
            } while (++z < 64); /* Next AC element */

            if (JD_FORMAT != 2 || !cmp) {                         /* C components may not be processed if in grayscale output */
                if (z == 1 || (JD_USE_SCALE && jd->scale == 3)) { /* If no AC element or scale ratio is 1/8, IDCT can be
                                                                                                                         ommited and the block is filled with DC value */
                    d = (jd_yuv_t)((*tmp / 256) + 128);
                    if (JD_FASTDECODE >= 1) {
                        for (i = 0; i < 64; bp[i++] = d);
                    } else {
                        memset(bp, d, 64);
                    }
                } else {
                    JPEG_block_idct(tmp, bp); /* Apply IDCT and store the block to the MCU buffer */
                }
            }
        }

        bp += 64; /* Next block */
    }

    return JDR_OK; /* All blocks have been loaded successfully */
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT::JPEG_mcu_output(JDEC* jd, unsigned int x, unsigned int y) {
    const int    CVACC = (sizeof(int) > 2) ? 1024 : 128; /* Adaptive accuracy for both 16-/32-bit systems */
    unsigned int ix, iy, mx, my, rx, ry;
    int          yy, cb, cr;
    jd_yuv_t *   py, *pc;
    uint8_t*     pix;
    JRECT        rect;

    mx = jd->msx * 8;
    my = jd->msy * 8;                                /* MCU size (pixel) */
    rx = (x + mx <= jd->width) ? mx : jd->width - x; /* Output rectangular size (it may be clipped at right/bottom end of image) */
    ry = (y + my <= jd->height) ? my : jd->height - y;
    if (JD_USE_SCALE) {
        rx >>= jd->scale;
        ry >>= jd->scale;
        if (!rx || !ry) return JDR_OK; /* Skip this MCU if all pixel is to be rounded off */
        x >>= jd->scale;
        y >>= jd->scale;
    }
    rect.left = x;
    rect.right = x + rx - 1; /* Rectangular area in the frame buffer */
    rect.top = y;
    rect.bottom = y + ry - 1;

    if (!JD_USE_SCALE || jd->scale != 3) { /* Not for 1/8 scaling */
        pix = (uint8_t*)jd->workbuf;

        if (JD_FORMAT != 2) { /* RGB output (build an RGB MCU from Y/C component) */
            for (iy = 0; iy < my; iy++) {
                pc = py = jd->mcubuf;
                if (my == 16) { /* Double block height? */
                    pc += 64 * 4 + (iy >> 1) * 8;
                    if (iy >= 8) py += 64;
                } else { /* Single block height */
                    pc += mx * 8 + iy * 8;
                }
                py += iy * 8;
                for (ix = 0; ix < mx; ix++) {
                    cb = pc[0] - 128; /* Get Cb/Cr component and remove offset */
                    cr = pc[64] - 128;
                    if (mx == 16) {                /* Double block width? */
                        if (ix == 8) py += 64 - 8; /* Jump to next block if double block heigt */
                        pc += ix & 1;              /* Step forward chroma pointer every two pixels */
                    } else {                       /* Single block width */
                        pc++;                      /* Step forward chroma pointer every pixel */
                    }
                    yy = *py++; /* Get Y component */
                    *pix++ = /*R*/ JPEG_BYTECLIP(yy + ((int)(1.402 * CVACC) * cr) / CVACC);
                    *pix++ = /*G*/ JPEG_BYTECLIP(yy - ((int)(0.344 * CVACC) * cb + (int)(0.714 * CVACC) * cr) / CVACC);
                    *pix++ = /*B*/ JPEG_BYTECLIP(yy + ((int)(1.772 * CVACC) * cb) / CVACC);
                }
            }
        } else { /* Monochrome output (build a grayscale MCU from Y comopnent) */
            for (iy = 0; iy < my; iy++) {
                py = jd->mcubuf + iy * 8;
                if (my == 16) { /* Double block height? */
                    if (iy >= 8) py += 64;
                }
                for (ix = 0; ix < mx; ix++) {
                    if (mx == 16) {                /* Double block width? */
                        if (ix == 8) py += 64 - 8; /* Jump to next block if double block height */
                    }
                    if (JD_FASTDECODE >= 1) {
                        *pix++ = JPEG_BYTECLIP(*py++); /* Get and store a Y value as grayscale */
                    } else {
                        *pix++ = *py++; /* Get and store a Y value as grayscale */
                    }
                }
            }
        }

        /* Descale the MCU rectangular if needed */
        if (JD_USE_SCALE && jd->scale) {
            unsigned int x, y, r, g, b, s, w, a;
            uint8_t*     op;

            /* Get averaged RGB value of each square correcponds to a pixel */
            s = jd->scale * 2;                       /* Number of shifts for averaging */
            w = 1 << jd->scale;                      /* Width of square */
            a = (mx - w) * (JD_FORMAT != 2 ? 3 : 1); /* Bytes to skip for next line in the square */
            op = (uint8_t*)jd->workbuf;
            for (iy = 0; iy < my; iy += w) {
                for (ix = 0; ix < mx; ix += w) {
                    pix = (uint8_t*)jd->workbuf + (iy * mx + ix) * (JD_FORMAT != 2 ? 3 : 1);
                    r = g = b = 0;
                    for (y = 0; y < w; y++) { /* Accumulate RGB value in the square */
                        for (x = 0; x < w; x++) {
                            r += *pix++;          /* Accumulate R or Y (monochrome output) */
                            if (JD_FORMAT != 2) { /* RGB output? */
                                g += *pix++;      /* Accumulate G */
                                b += *pix++;      /* Accumulate B */
                            }
                        }
                        pix += a;
                    } /* Put the averaged pixel value */
                    *op++ = (uint8_t)(r >> s);     /* Put R or Y (monochrome output) */
                    if (JD_FORMAT != 2) {          /* RGB output? */
                        *op++ = (uint8_t)(g >> s); /* Put G */
                        *op++ = (uint8_t)(b >> s); /* Put B */
                    }
                }
            }
        }
    } else { /* For only 1/8 scaling (left-top pixel in each block are the DC value of the block) */

        /* Build a 1/8 descaled RGB MCU from discrete comopnents */
        pix = (uint8_t*)jd->workbuf;
        pc = jd->mcubuf + mx * my;
        cb = pc[0] - 128; /* Get Cb/Cr component and restore right level */
        cr = pc[64] - 128;
        for (iy = 0; iy < my; iy += 8) {
            py = jd->mcubuf;
            if (iy == 8) py += 64 * 2;
            for (ix = 0; ix < mx; ix += 8) {
                yy = *py; /* Get Y component */
                py += 64;
                if (JD_FORMAT != 2) {
                    *pix++ = /*R*/ JPEG_BYTECLIP(yy + ((int)(1.402 * CVACC) * cr / CVACC));
                    *pix++ = /*G*/ JPEG_BYTECLIP(yy - ((int)(0.344 * CVACC) * cb + (int)(0.714 * CVACC) * cr) / CVACC);
                    *pix++ = /*B*/ JPEG_BYTECLIP(yy + ((int)(1.772 * CVACC) * cb / CVACC));
                } else {
                    *pix++ = yy;
                }
            }
        }
    }

    /* Squeeze up pixel table if a part of MCU is to be truncated */
    mx >>= jd->scale;
    if (rx < mx) { /* Is the MCU spans rigit edge? */
        uint8_t *    s, *d;
        unsigned int x, y;

        s = d = (uint8_t*)jd->workbuf;
        for (y = 0; y < ry; y++) {
            for (x = 0; x < rx; x++) { /* Copy effective pixels */
                *d++ = *s++;
                if (JD_FORMAT != 2) {
                    *d++ = *s++;
                    *d++ = *s++;
                }
            }
            s += (mx - rx) * (JD_FORMAT != 2 ? 3 : 1); /* Skip truncated pixels */
        }
    }

    /* Convert RGB888 to RGB565 if needed */
    if (JD_FORMAT == 1) {
        uint8_t*     s = (uint8_t*)jd->workbuf;
        uint16_t     w, *d = (uint16_t*)s;
        unsigned int n = rx * ry;

        if (jd->swap) {
            do {
                w = (*s++ & 0xF8) << 8;     // RRRRR-----------
                w |= (*s++ & 0xFC) << 3;    // -----GGGGGG-----
                w |= *s++ >> 3;             // -----------BBBBB
                *d++ = (w << 8) | (w >> 8); // Swap bytes
            } while (--n);
        } else {
            do {
                w = (*s++ & 0xF8) << 8;  // RRRRR-----------
                w |= (*s++ & 0xFC) << 3; // -----GGGGGG-----
                w |= *s++ >> 3;          // -----------BBBBB
                *d++ = w;
            } while (--n);
        }
    }

    /* Output the rectangular */
    bool r = JPEG_jd_output(jd, jd->workbuf, &rect);
    return r ? JDR_OK : JDR_INTR;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT::JPEG_jd_prepare(JDEC* jd, uint8_t* pool, size_t sz_pool, void* dev) {
    uint8_t *    seg, b;
    uint16_t     marker;
    unsigned int n, i, ofs;
    size_t       len;
    uint8_t      rc;
    uint8_t      tmp = jd->swap; // Copy the swap flag

    memset(jd, 0, sizeof(JDEC)); /* Clear decompression object (this might be a problem if machine's null pointer is not all bits zero) */
    jd->pool = pool;             /* Work memory */
    jd->sz_pool = sz_pool;       /* Size of given work memory */
    // jd->infunc = infunc;         /* Stream input function */
    jd->device = dev; /* I/O device identifier */
    jd->swap = tmp;   // Restore the swap flag

    jd->inbuf = seg = (uint8_t*)JPEG_alloc_pool(jd, JD_SZBUF); /* Allocate stream input buffer */
    if (!seg) return JDR_MEM1;

    ofs = marker = 0; /* Find SOI marker */
    do {
        if (JPEG_jd_input(jd, seg, 1) != 1) return JDR_INP; /* Err: SOI was not detected */
        ofs++;
        marker = marker << 8 | seg[0];
    } while (marker != 0xFFD8);

    for (;;) { /* Parse JPEG segments */
        /* Get a JPEG marker */
        if (JPEG_jd_input(jd, seg, 4) != 4) return JDR_INP;
        marker = LDB_WORD(seg);  /* Marker */
        len = LDB_WORD(seg + 2); /* Length field */
        if (len <= 2 || (marker >> 8) != 0xFF) return JDR_FMT1;
        len -= 2;       /* Segent content size */
        ofs += 4 + len; /* Number of bytes loaded */

        switch (marker & 0xFF) {
            case 0xC0: /* SOF0 (baseline JPEG) */
                if (len > JD_SZBUF) return JDR_MEM2;
                if (JPEG_jd_input(jd, seg, len) != len) return JDR_INP; /* Load segment data */

                jd->width = LDB_WORD(&seg[3]);                         /* Image width in unit of pixel */
                jd->height = LDB_WORD(&seg[1]);                        /* Image height in unit of pixel */
                jd->ncomp = seg[5];                                    /* Number of color components */
                if (jd->ncomp != 3 && jd->ncomp != 1) return JDR_FMT3; /* Err: Supports only Grayscale and Y/Cb/Cr */

                /* Check each image component */
                for (i = 0; i < jd->ncomp; i++) {
                    b = seg[7 + 3 * i];                            /* Get sampling factor */
                    if (i == 0) {                                  /* Y component */
                        if (b != 0x11 && b != 0x22 && b != 0x21) { /* Check sampling factor */
                            return JDR_FMT3;                       /* Err: Supports only 4:4:4, 4:2:0 or 4:2:2 */
                        }
                        jd->msx = b >> 4;
                        jd->msy = b & 15;               /* Size of MCU [blocks] */
                    } else {                            /* Cb/Cr component */
                        if (b != 0x11) return JDR_FMT3; /* Err: Sampling factor of Cb/Cr must be 1 */
                    }
                    jd->qtid[i] = seg[8 + 3 * i];         /* Get dequantizer table ID for this component */
                    if (jd->qtid[i] > 3) return JDR_FMT3; /* Err: Invalid ID */
                }
                break;

            case 0xDD: /* DRI - Define Restart Interval */
                if (len > JD_SZBUF) return JDR_MEM2;
                if (JPEG_jd_input(jd, seg, len) != len) return JDR_INP; /* Load segment data */

                jd->nrst = LDB_WORD(seg); /* Get restart interval (MCUs) */
                break;

            case 0xC4: /* DHT - Define Huffman Tables */
                if (len > JD_SZBUF) return JDR_MEM2;
                if (JPEG_jd_input(jd, seg, len) != len) return JDR_INP; /* Load segment data */

                rc = JPEG_create_huffman_tbl(jd, seg, len); /* Create huffman tables */
                if (rc) return rc;
                break;

            case 0xDB: /* DQT - Define Quaitizer Tables */
                if (len > JD_SZBUF) return JDR_MEM2;
                if (JPEG_jd_input(jd, seg, len) != len) return JDR_INP; /* Load segment data */

                rc = JPEG_create_qt_tbl(jd, seg, len); /* Create de-quantizer tables */
                if (rc) return rc;
                break;

            case 0xDA: /* SOS - Start of Scan */
                if (len > JD_SZBUF) return JDR_MEM2;
                if (JPEG_jd_input(jd, seg, len) != len) return JDR_INP; /* Load segment data */

                if (!jd->width || !jd->height) return JDR_FMT1; /* Err: Invalid image size */
                if (seg[0] != jd->ncomp) return JDR_FMT3;       /* Err: Wrong color components */

                /* Check if all tables corresponding to each components have been loaded */
                for (i = 0; i < jd->ncomp; i++) {
                    b = seg[2 + 2 * i];                               /* Get huffman table ID */
                    if (b != 0x00 && b != 0x11) return JDR_FMT3;      /* Err: Different table number for DC/AC element */
                    n = i ? 1 : 0;                                    /* Component class */
                    if (!jd->huffbits[n][0] || !jd->huffbits[n][1]) { /* Check huffman table for this component */
                        return JDR_FMT1;                              /* Err: Nnot loaded */
                    }
                    if (!jd->qttbl[jd->qtid[i]]) { /* Check dequantizer table for this component */
                        return JDR_FMT1;           /* Err: Not loaded */
                    }
                }

                /* Allocate working buffer for MCU and pixel output */
                n = jd->msy * jd->msx;                                                        /* Number of Y blocks in the MCU */
                if (!n) return JDR_FMT1;                                                      /* Err: SOF0 has not been loaded */
                len = n * 64 * 2 + 64;                                                        /* Allocate buffer for IDCT and RGB output */
                if (len < 256) len = 256;                                                     /* but at least 256 byte is required for IDCT */
                jd->workbuf = JPEG_alloc_pool(jd, len);                                       /* and it may occupy a part of following MCU working buffer for RGB output */
                if (!jd->workbuf) return JDR_MEM1;                                            /* Err: not enough memory */
                jd->mcubuf = (jd_yuv_t*)JPEG_alloc_pool(jd, (n + 2) * 64 * sizeof(jd_yuv_t)); /* Allocate MCU working buffer */
                if (!jd->mcubuf) return JDR_MEM1;                                             /* Err: not enough memory */

                /* Align stream read offset to JD_SZBUF */
                if (ofs %= JD_SZBUF) { jd->dctr = JPEG_jd_input(jd, seg + ofs, (size_t)(JD_SZBUF - ofs)); }
                jd->dptr = seg + ofs - (JD_FASTDECODE ? 0 : 1);

                return JDR_OK; /* Initialization succeeded. Ready to decompress the JPEG image. */

            case 0xC1:                            /* SOF1 */
            case 0xC2:                            /* SOF2 */
            case 0xC3:                            /* SOF3 */
            case 0xC5:                            /* SOF5 */
            case 0xC6:                            /* SOF6 */
            case 0xC7:                            /* SOF7 */
            case 0xC9:                            /* SOF9 */
            case 0xCA:                            /* SOF10 */
            case 0xCB:                            /* SOF11 */
            case 0xCD:                            /* SOF13 */
            case 0xCE:                            /* SOF14 */
            case 0xCF:                            /* SOF15 */
            case 0xD9: /* EOI */ return JDR_FMT3; /* Unsuppoted JPEG standard (may be progressive JPEG) */

            default: /* Unknown segment (comment, exif or etc..) */
                /* Skip segment data (null pointer specifies to remove data from the stream) */
                if (JPEG_jd_input(jd, 0, len) != len) return JDR_INP;
        }
    }
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
uint8_t TFT::JPEG_jd_decomp(JDEC* jd, uint8_t scale) {
    unsigned int x, y, mx, my;
    uint16_t     rst, rsc;
    uint8_t      rc;

    if (scale > (JD_USE_SCALE ? 3 : 0)) return JDR_PAR;
    jd->scale = scale;
    mx = jd->msx * 8;
    my = jd->msy * 8; /* Size of the MCU (pixel) */
    jd->dcv[2] = jd->dcv[1] = jd->dcv[0] = 0; /* Initialize DC values */
    rst = rsc = 0;
    rc = JDR_OK;
    for (y = 0; y < jd->height; y += my) {       /* Vertical loop of MCUs */
        for (x = 0; x < jd->width; x += mx) {    /* Horizontal loop of MCUs */
            if (jd->nrst && rst++ == jd->nrst) { /* Process restart interval if enabled */
                rc = JPEG_restart(jd, rsc++);
                if (rc != JDR_OK) return rc;
                rst = 1;
            }
            rc = JPEG_mcu_load(jd); /* Load an MCU (decompress huffman coded stream, dequantize and apply IDCT) */
            if (rc != JDR_OK) return rc;
            rc = JPEG_mcu_output(jd, x, y); /* Output the MCU (YCbCr to RGB, scaling and output) */
            if (rc != JDR_OK) return rc;
        }
    }
    return rc;
}
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//  ⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫   T O U C H   ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫ ⏫⏫⏫⏫⏫⏫  ⏫⏫⏫⏫⏫⏫
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//        XPT2046
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
#if TP_VERSION <= 7
// Code für Touchpad mit XPT2046
TP::TP(uint8_t CS, uint8_t IRQ) {
    // log_i("TP init CS = %i, IRQ = %i", CS, IRQ);
    _TP_CS = CS;
    _TP_IRQ = IRQ;
    pinMode(_TP_CS, OUTPUT);
    digitalWrite(_TP_CS, HIGH);
    pinMode(_TP_IRQ, INPUT);
    TP_SPI = SPISettings(200000, MSBFIRST, SPI_MODE0); // slower speed
    _rotation = 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

uint16_t TP::TP_Send(uint8_t set_val) {
    uint16_t get_val;
    SPItransfer->beginTransaction(TP_SPI); // Prevent other SPI users
    digitalWrite(_TP_CS, 0);
    SPItransfer->write(set_val);
    get_val = SPItransfer->transfer16(0);
    digitalWrite(_TP_CS, 1);
    SPItransfer->endTransaction(); // Allow other SPI users
    return get_val >> 4;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TP::loop() {
    static uint16_t x1 = 0, y1 = 0;
    static uint16_t x2 = 0, y2 = 0;
    if(!digitalRead(_TP_IRQ)) {
        if(!read_TP(x, y)) { return; }

        if(x != x1 && y != y1) {
            if(tp_moved) tp_moved(x, y);
        }

        {
            x1 = x;
            y1 = y;
        }

        if(f_loop) {
            f_loop = false;
            // log_i("tp_pressed x=%d, y=%d", x, y);
            if(tp_pressed) tp_pressed(x, y);
            x2 = x;
            y2 = y;
            m_pressingTime = millis();
            m_f_isPressing = true;
        }
        else {
            if(m_f_isPressing) {
                if(m_pressingTime + 2000 < millis()) {
                    m_f_isPressing = false;
                    if(tp_long_pressed) tp_long_pressed(x2, y2);
                    m_f_longPressed = true;
                }
            }
        }
    }
    else {
        if(f_loop == false) {
            // log_i("tp_released");
            if(m_f_longPressed) {
                m_f_longPressed = false;
                if(tp_released) tp_released(x1, y1);
                else if(tp_long_released) tp_long_released(x1, y1);
            }
            else {
                if(tp_released) tp_released(x1, y1);
            }
            f_loop = true;
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TP::setRotation(uint8_t m) { _rotation = m; }
//----------------------------------------------------------------------------------------------------------------------------------------------------
void TP::setMirror(bool h, bool v) {
    m_mirror_h = h;
    m_mirror_v = v;
}

void TP::setVersion(uint8_t v) {

    switch(v) {
        case 0: TP_vers = TP_ILI9341_0; break;
        case 1: TP_vers = TP_ILI9341_1; break;
        case 2: TP_vers = TP_HX8347D_0; break;
        case 3: TP_vers = TP_ILI9486_0; break;
        case 4: TP_vers = TP_ILI9488_0; break;
        case 5: TP_vers = TP_ST7796_0; break;
        default: TP_vers = TP_ILI9341_0; break;
    }

    if(TP_vers == TP_ILI9341_0) { // ILI9341 display
        Xmax = 1913;              // Values Calibration
        Xmin = 150;
        Ymax = 1944;
        Ymin = 220;
        xFaktor = float(Xmax - Xmin) / ILI9341_WIDTH;
        yFaktor = float(Ymax - Ymin) / ILI9341_HEIGHT;
    }
    if(TP_vers == TP_ILI9341_1) { // ILI9341 display for RaspberryPI  #70
        Xmax = 1940;
        Xmin = 90;
        Ymax = 1864;
        Ymin = 105;
        xFaktor = float(Xmax - Xmin) / ILI9341_WIDTH;
        yFaktor = float(Ymax - Ymin) / ILI9341_HEIGHT;
    }
    if(TP_vers == TP_HX8347D_0) { // Waveshare HX8347D display
        Xmax = 1850;
        Xmin = 170;
        Ymax = 1880;
        Ymin = 140;
        xFaktor = float(Xmax - Xmin) / HX8347D_WIDTH;
        yFaktor = float(Ymax - Ymin) / HX8347D_HEIGHT;
    }
    if(TP_vers == TP_ILI9486_0) { // ILI9486 display for RaspberryPI
        Xmax = 1922;
        Xmin = 140;
        Ymax = 1930;
        Ymin = 125;
        xFaktor = float(Xmax - Xmin) / ILI9486_WIDTH;
        yFaktor = float(Ymax - Ymin) / ILI9486_HEIGHT;
    }
    if(TP_vers == TP_ILI9488_0) { // ILI9488 display
        Xmax = 1922;
        Xmin = 140;
        Ymax = 1930;
        Ymin = 125;
        xFaktor = float(Xmax - Xmin) / ILI9488_WIDTH;
        yFaktor = float(Ymax - Ymin) / ILI9488_HEIGHT;
    }
    if(TP_vers == TP_ST7796_0) { // ST7796 4" display
        Xmax = 1922;
        Xmin = 103;
        Ymax = 1950;
        Ymin = 110;
        xFaktor = float(Xmax - Xmin) / ST7796_WIDTH;
        yFaktor = float(Ymax - Ymin) / ST7796_HEIGHT;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

bool TP::read_TP(uint16_t& x, uint16_t& y) {
    uint32_t _y = 0;
    uint32_t _x = 0;
    uint16_t tmpxy;
    uint8_t  i = 0;

    if(digitalRead(_TP_IRQ)) return false; // TP pressed?

    for(i = 0; i < 100; i++) {
        _x += TP_Send(0xD0); // x
        _y += TP_Send(0x90); // y
    }

    if(digitalRead(_TP_IRQ)) return false; // TP must remain pressed as long as the measurement is running

    _x /= 100;
    _y /= 100;

    // log_w("_x %i, _y %i", _x, _y);

    if((_x < Xmin) || (_x > Xmax)) return false; // outside the display
    _x = Xmax - _x;
    _x /= xFaktor;

    if((_y < Ymin) || (y > Ymax)) return false; // outside the display
    _y = Ymax - _y;
    _y /= yFaktor;

    if(m_mirror_h) {
        switch(TP_vers) {
            case TP_ILI9341_0: // ILI9341
                _y = ILI9341_HEIGHT - _y;
                break;
            case TP_ILI9341_1: _y = ILI9341_HEIGHT - _y; break;
            case TP_HX8347D_0: _y = HX8347D_HEIGHT - _y; break;
            case TP_ILI9486_0: _y = ILI9486_HEIGHT - _y; break;
            case TP_ILI9488_0: _y = ILI9488_HEIGHT - _y; break;
            case TP_ST7796_0: _y = ST7796_HEIGHT - _y;
            default: break;
        }
    }

    if(m_mirror_v) {
        switch(TP_vers) {
            case TP_ILI9341_0: // ILI9341
                _x = ILI9341_WIDTH - _x;
                break;
            case TP_ILI9341_1: _x = ILI9341_WIDTH - _x; break;
            case TP_HX8347D_0: _x = HX8347D_WIDTH - _x; break;
            case TP_ILI9486_0: _x = ILI9486_WIDTH - _x; break;
            case TP_ILI9488_0: _x = ILI9488_WIDTH - _x; break;
            case TP_ST7796_0: _x = ST7796_WIDTH - _x; break;
            default: break;
        }
    }

    // log_i("_x %i, _y %i", _x, _y);
    x = _x;
    y = _y;

    //-------------------------------------------------------------
    if(TP_vers == TP_ILI9341_0) { // 320px x 240px
        if(_rotation == 0) { y = ILI9341_HEIGHT - y; }
        if(_rotation == 1) {
            tmpxy = x;
            x = y;
            y = tmpxy;
            y = ILI9341_WIDTH - y;
            x = ILI9341_HEIGHT - x;
        }
        if(_rotation == 2) { x = ILI9341_WIDTH - x; }
        if(_rotation == 3) {
            tmpxy = x;
            x = y;
            y = tmpxy;
        }
    }
    //-------------------------------------------------------------
    if(TP_vers == TP_ILI9341_1) { // 320px x 240px
        if(_rotation == 0) {
            y = ILI9341_HEIGHT - y;
            x = ILI9341_WIDTH - x;
        }
        if(_rotation == 1) {
            tmpxy = x;
            x = y;
            y = tmpxy;
            x = ILI9341_HEIGHT - x;
        }
        if(_rotation == 2) { ; }
        if(_rotation == 3) {
            tmpxy = x;
            x = y;
            y = tmpxy;
            y = ILI9341_WIDTH - y;
        }
    }
    //-------------------------------------------------------------
    if(TP_vers == TP_HX8347D_0) { // 320px x 240px
        if(_rotation == 0) {
            ; // do nothing
        }
        if(_rotation == 1) {
            tmpxy = x;
            x = y;
            y = HX8347D_WIDTH - tmpxy;
            if(x > HX8347D_HEIGHT - 1) x = 0;
            if(y > HX8347D_WIDTH - 1) y = 0;
        }
        if(_rotation == 2) {
            x = HX8347D_WIDTH - x;
            y = HX8347D_HEIGHT - y;
            if(x > HX8347D_WIDTH - 1) x = 0;
            if(y > HX8347D_HEIGHT - 1) y = 0;
        }
        if(_rotation == TP_ILI9486_0) {
            tmpxy = y;
            y = x;
            x = HX8347D_HEIGHT - tmpxy;
            if(x > HX8347D_HEIGHT - 1) x = 0;
            if(y > HX8347D_WIDTH - 1) y = 0;
        }
    }
    //-------------------------------------------------------------
    if(TP_vers == TP_ILI9486_0) { // 480px x 320px
        if(_rotation == 0) {
            ; // do nothing
        }
        if(_rotation == 1) {
            tmpxy = x;
            x = y;
            y = ILI9486_WIDTH - tmpxy;
            if(x > ILI9486_HEIGHT - 1) x = 0;
            if(y > ILI9486_WIDTH - 1) y = 0;
        }
        if(_rotation == 2) {
            x = ILI9486_WIDTH - x;
            y = ILI9486_HEIGHT - y;
            if(x > ILI9486_WIDTH - 1) x = 0;
            if(y > ILI9486_HEIGHT - 1) y = 0;
        }
        if(_rotation == 3) {
            tmpxy = y;
            y = x;
            x = ILI9486_HEIGHT - tmpxy;
            if(x > ILI9486_HEIGHT - 1) x = 0;
            if(y > ILI9486_WIDTH - 1) y = 0;
        }
    }
    //-------------------------------------------------------------
    if(TP_vers == TP_ILI9488_0) { // ILI 9488 Display V1.0, 480px x 320px
        if(_rotation == 0) { x = ILI9488_WIDTH - x; }
        if(_rotation == 1) { // landscape
            tmpxy = x;
            x = y;
            y = tmpxy;
            if(x > ILI9488_HEIGHT - 1) x = 0;
            if(y > ILI9488_WIDTH - 1) y = 0;
        }
        if(_rotation == 2) { // portrait + 180 degree
            y = ILI9488_HEIGHT - y;
            if(x > ILI9488_WIDTH - 1) x = 0;
            if(y > ILI9488_HEIGHT - 1) y = 0;
        }
        if(_rotation == 3) { // landscape + 180 degree
            tmpxy = x;
            x = ILI9488_HEIGHT - y;
            y = ILI9488_WIDTH - tmpxy;
            if(x > ILI9488_HEIGHT - 1) x = 0;
            if(y > ILI9488_WIDTH - 1) y = 0;
        }
    }
    //-------------------------------------------------------------
    if(TP_vers == TP_ST7796_0) { // ST7796 Display V1.1, 480px x 320px
        if(_rotation == 0) { x = ST7796_WIDTH - x; }
        if(_rotation == 1) { // landscape
            tmpxy = x;
            x = y;
            y = tmpxy;
            if(x > ST7796_HEIGHT - 1) x = 0;
            if(y > ST7796_WIDTH - 1) y = 0;
        }
        if(_rotation == 2) { // portrait + 180 degree
            y = ILI9488_HEIGHT - y;
            if(x > ST7796_WIDTH - 1) x = 0;
            if(y > ST7796_HEIGHT - 1) y = 0;
        }
        if(_rotation == 3) { // landscape + 180 degree
            tmpxy = x;
            x = ST7796_HEIGHT - y;
            y = ST7796_WIDTH - tmpxy;
            if(x > ST7796_HEIGHT - 1) x = 0;
            if(y > ST7796_WIDTH - 1) y = 0;
        }
    }
    //    log_i("TP_vers %d, Rotation %d, X = %i, Y = %i",TP_vers, _rotation, x, y);
    return true;
}
#endif

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//        GT911
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
#if TP_VERSION == 8
// Interrupt handling
volatile bool gt911IRQ = false;

void IRAM_ATTR gt911_irq_handler() {
    noInterrupts();
    gt911IRQ = true;
    interrupts();
}

// Code for GT911
TP::TP(TwoWire *twi){
    m_wire = twi; // I2C TwoWire Instance
}

bool TP::begin(int8_t sda, int8_t scl, uint8_t addr, uint32_t clk, int8_t intPin, int8_t rstPin) {
    m_sda = sda;
    m_scl = scl;
    m_addr = addr;
    m_clk = clk;
    m_intPin = intPin;
    m_rstPin = rstPin;

    if (m_rstPin > 0) {
        delay(300);
        reset();
        delay(200);
    }

    if (m_intPin > 0) {
        pinMode(m_intPin, INPUT);
        attachInterrupt(m_intPin, gt911_irq_handler, FALLING);
    }
    m_wire->begin(m_sda, m_scl, m_clk);
    m_wire->beginTransmission(m_addr);
    if(m_wire->endTransmission() == 0) {
        log_e("TouchPad found at 0x%02X", m_addr);
        readInfo(); // Need to get resolution to use rotation
        return true;
    }
    log_e("TouchPad not Initialized");
    return false;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void TP::setRotation(uint8_t m) {
    if(m == 2) m_rotation = Rotate::_0;
    else if(m == 3) m_rotation = Rotate::_90;
    else if(m == 0) m_rotation = Rotate::_180;
    else if(m == 1) m_rotation = Rotate::_270;

    if(m_version == TP_GT911){
        switch(m_rotation) {
            case Rotate::_0:   m_info.xResolution = 800; m_info.yResolution = 480; break;
            case Rotate::_90:  m_info.xResolution = 480; m_info.yResolution = 800; break;
            case Rotate::_180: m_info.xResolution = 800; m_info.yResolution = 480; break;
            case Rotate::_270: m_info.xResolution = 480; m_info.yResolution = 800; break;
        }
    }
 }
//----------------------------------------------------------------------------------------------------------------------------------------------------
void TP::setMirror(bool h, bool v) {
    m_mirror_h = h;
    m_mirror_v = v;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void TP::setVersion(uint8_t v) {
    switch(v) {
        case 3: m_version = TP_GT911; break;  // GT927, GT928, GT967, GT5688
        // case 4: m_version = TP_ILI2510; break; // ILI9488
        // case 5: m_version = TP_FT5406; break; // FT5446, FT6336U
    }
    log_i("Resulution: %dx%d", m_info.xResolution, m_info.yResolution);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void TP::loop() {
    static GTPoint p, p1;
    static uint32_t ts = 0;
    uint8_t t = touched(TP::GT911_MODE_POLLING); // number of touch points
    if(t == 1 && !m_f_isTouch) {
        p = getPoint(0);
        log_w("X: %d, Y: %d", p.x, p.y);
        if(tp_pressed) tp_pressed(p.x, p.y);
        ts = millis();
        m_f_isTouch = true;
        return;
    }
    if(t == 1 && m_f_isTouch) {
        p1 = getPoint(0);
        if(p1.x != p.x || p1.y != p.y) {
            p = p1;
            if(tp_moved) tp_moved(p.x, p.y);
            return;
        }
        // fall through
    }

    if(t == 1 && m_f_isTouch && (millis() > ts + 2000) && !m_f_isLongPressed) {
        m_f_isLongPressed = true;
        if(tp_long_pressed) tp_long_pressed(p.x, p.y);
        ts = millis() + 10000;
        return;
    }
    if(t == 0 && m_f_isTouch && !m_f_isLongPressed) {
        if(tp_released) tp_released(p.x, p.y);
        m_f_isTouch = false;
        return;
    }
    if(t == 0 && m_f_isLongPressed) {
        m_f_isLongPressed = false;
        if(tp_long_released) tp_long_released(p.x, p.y);
        m_f_isTouch = false;
        return;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool TP::getProductID() {
    uint8_t buf[4];
    memset(buf, 0, 4);
    readBytes(GT911_REG_ID, buf, 4);
    log_w("Product ID: %c%c%c%c", buf[0], buf[1], buf[2], buf[3]);
    return true;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void TP::reset() {
    vTaskDelay(10 / portTICK_PERIOD_MS);
    pinMode(m_intPin, OUTPUT); digitalWrite(m_intPin, LOW);
    pinMode(m_rstPin, OUTPUT); digitalWrite(m_rstPin, LOW);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    digitalWrite(m_intPin, m_addr);
    pinMode(m_rstPin, INPUT);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    digitalWrite(m_intPin, LOW);
    vTaskDelay(10 / portTICK_PERIOD_MS);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool TP::write(uint16_t reg, uint8_t data) {
    m_wire->beginTransmission(m_addr);
    m_wire->write(reg >> 8);
    m_wire->write(reg & 0xFF);
    m_wire->write(data);
  return m_wire->endTransmission(true) == 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t TP::read(uint16_t reg) {
    m_wire->beginTransmission(m_addr);
    m_wire->write(reg >> 8);
    m_wire->write(reg & 0xFF);
    m_wire->endTransmission(false);
    m_wire->requestFrom(m_addr, (uint8_t)1);
    while (m_wire->available()) {
        return m_wire->read();
    }
    m_wire->endTransmission(true);
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool TP::writeBytes(uint16_t reg, uint8_t *data, uint16_t size) {
    m_wire->beginTransmission(m_addr);
    m_wire->write(reg >> 8);
    m_wire->write(reg & 0xFF);
    for (uint16_t i = 0; i < size; i++) {
        m_wire->write(data[i]);
    }
    return m_wire->endTransmission(true) == 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool TP::readBytes(uint16_t reg, uint8_t *data, uint16_t size) {
    m_wire->beginTransmission(m_addr);
    m_wire->write(reg >> 8);
    m_wire->write(reg & 0xFF);
    m_wire->endTransmission(false);

    uint16_t index = 0;
    while (index < size) {
        uint8_t req = _min(size - index, I2C_BUFFER_LENGTH);
        m_wire->requestFrom(m_addr, req);
        while (m_wire->available()) {
            data[index++] = m_wire->read();
        }
        index++;
    }
    m_wire->endTransmission(true);
    return size == index - 1;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t TP::calcChecksum(uint8_t* buf, uint8_t len) {
    uint8_t ccsum = 0;
    for (uint8_t i = 0; i < len; i++) { ccsum += buf[i]; }

    return (~ccsum + 1) &0xFF; // complement of checksum
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t TP::readChecksum() {
    return read(GT911_REG_CHECKSUM);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
int8_t TP::readTouches() {
    uint32_t timeout = millis() + 20;
    do {
        uint8_t flag = read(GT911_REG_COORD_ADDR);
        if ((flag & 0x80) && ((flag & 0x0F) < GT911_MAX_CONTACTS)) {
            write(GT911_REG_COORD_ADDR, 0);
            return flag & 0x0F;
        }
        delay(1);
    } while (millis() < timeout);

    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
bool TP::readTouchPoints() {
    bool result = readBytes(GT911_REG_COORD_ADDR + 1, (uint8_t*)m_points, sizeof(GTPoint) * GT911_MAX_CONTACTS);
    return result;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
TP::GTInfo* TP::readInfo() {
    readBytes(GT911_REG_DATA, (uint8_t*)&m_info, sizeof(m_info));
    return &m_info;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t TP::touched(uint8_t mode) {
    bool irq = false;
    if (mode == GT911_MODE_INTERRUPT) {
        noInterrupts();
        irq = gt911IRQ;
        gt911IRQ = false;
        interrupts();
    } else if (mode == GT911_MODE_POLLING) {
        irq = true;
    }
    uint8_t contacts = 0;
    if (irq) {
        contacts = readTouches();
        if (contacts > 0) { readTouchPoints(); }
    }
    return contacts;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
TP::GTPoint TP::getPoint(uint8_t num) {
    int x_new = 0, y_new = 0;
    if(m_mirror_h) m_points[num].x = m_info.xResolution - m_points[num].x;
    if(m_mirror_v) m_points[num].y = m_info.yResolution - m_points[num].y;

    switch(m_rotation) {
        case Rotate::_0:   return m_points[num]; // No change
        case Rotate::_90:  y_new = m_info.yResolution - m_points[num].x; x_new = m_points[num].y; break;
        case Rotate::_180: x_new = m_info.xResolution - m_points[num].x; y_new = m_info.yResolution - m_points[num].y; break;
        case Rotate::_270: x_new = m_info.xResolution - m_points[num].y; y_new = m_points[num].x; break;
    }
    m_points[num].x = x_new;
    m_points[num].y = y_new;
    return m_points[num];
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
TP::GTPoint* TP::getPoints() {
    int x_new = 0, y_new = 0;
    for (uint8_t i = 0; i < GT911_MAX_CONTACTS; i++) {
        if(m_mirror_h) m_points[i].x = m_info.xResolution - m_points[i].x;
        if(m_mirror_v) m_points[i].y = m_info.yResolution - m_points[i].y;
        switch(m_rotation) {
            case Rotate::_0:   break; // No change
            case Rotate::_90:  x_new = m_info.xResolution - m_points[i].x; y_new = m_points[i].y; break;
            case Rotate::_180: x_new = m_info.xResolution - m_points[i].x; y_new = m_info.yResolution - m_points[i].y; break;
            case Rotate::_270: x_new = m_info.yResolution - m_points[i].y; y_new = m_points[i].x; break;
        }
        m_points[i].x = x_new;
        m_points[i].y = y_new;
    }
    return m_points;
}
#endif