// first release on 09/2019
// updated on Feb 08 2024

#include "Arduino.h"
#include "tft.h"
#include "pins_arduino.h"

JPEGDecoder JpegDec;
SPIClass*   SPItransfer;

#define __malloc_heap_psram(size) \
    heap_caps_malloc_prefer(size, 2, MALLOC_CAP_DEFAULT|MALLOC_CAP_SPIRAM, MALLOC_CAP_DEFAULT|MALLOC_CAP_INTERNAL)

#define ILI9341_MADCTL_MY   0x80
#define ILI9341_MADCTL_MX   0x40
#define ILI9341_MADCTL_MV   0x20
#define ILI9341_MADCTL_ML   0x10
#define ILI9341_MADCTL_BGR  0x08
#define ILI9341_MADCTL_MH   0x04
#define ILI9341_SLPOUT      0x11 // Sleep Out
#define ILI9341_INVOFF      0x20 // Display Invert Off
#define ILI9341_INVON       0x21 // Display Invert On
#define ILI9341_DISPON      0x29 // Display On
#define ILI9341_CASET       0x2A // Column Address Set
#define ILI9341_RASET       0x2B // Row Address Set
#define ILI9341_RAMWR       0x2C // Memory Write
#define ILI9341_RAMRD       0x2E // Memory Read
#define ILI9341_MADCTL      0x36 // Memory Data Access Control
#define ILI9341_VSCRSADD    0x37 // Vertical Scrolling Start Address
//----------------------------------------------------------------------------------------------------------------------------------------------------
#define ILI9486_INVOFF      0x20 // Display Inversion OFF
#define ILI9486_INVON       0x21 // Display Inversion ON
#define ILI9486_CASET       0x2A // Display On
#define ILI9486_PASET       0x2B // Page Address Set
#define ILI9486_RAMWR       0x2C // Memory Write
#define ILI9486_RAMRD       0x2E // Memory Read
#define ILI9486_MADCTL      0x36 // Memory Data Access Control
#define ILI9486_MADCTL_MY   0x80 // Bit 7 Parameter MADCTL
#define ILI9486_MADCTL_MX   0x40 // Bit 5 Parameter MADCTL
#define ILI9486_MADCTL_MV   0x20 // Bit 5 Parameter MADCTL
#define ILI9486_MADCTL_ML   0x10 // Bit 4 Parameter MADCTL
#define ILI9486_MADCTL_BGR  0x08 // Bit 3 Parameter MADCTL
#define ILI9486_MADCTL_MH   0x04 // Bit 2 Parameter MADCTL
#define ILI9486_WDBVAL      0x51 // Write Display Brightness Value
#define ILI9486_CDBVAL      0x53 // Write Control Display Value
//----------------------------------------------------------------------------------------------------------------------------------------------------
#define ILI9488_SLPOUT      0x11 // Sleep OUT
#define ILI9488_INVOFF      0x20 // Display Inversion OFF
#define ILI9488_INVON       0x21 // Display Inversion ON
#define ILI9488_DISPOFF     0x28 // Display OFF
#define ILI9488_DISPON      0x29 // Display ON
#define ILI9488_CASET       0x2A // Column Address Set
#define ILI9488_PASET       0x2B // Page Address Set
#define ILI9488_MADCTL      0x36 // Memory Access Control
#define ILI9488_COLMOD      0x3A // Interface Pixel Format
#define ILI9488_IFMODE      0xB0 // Interface Mode Control
#define ILI9488_FRMCTR1     0xB1 // Frame Rate Control
#define ILI9488_FRMCTR2     0xB2 // Frame Rate Control
#define ILI9488_INVTR       0xB4 // Display Inversion Control
#define ILI9488_DISCTRL     0xB6 // Display Function Control
#define ILI9488_ETMOD       0xB7 // Entry Mode Set
#define ILI9488_RAMWR       0x2C // Write_memory_start
#define ILI9488_RAMRD       0x2E // Read_memory_start
#define ILI9488_PWCTR1      0xC0 // Panel Driving Setting
#define ILI9488_PWCTR2      0xC1 // Display_Timing_Setting for Normal Mode
#define ILI9488_VMCTR1      0xC5 // Frame Rate and Inversion Control
#define ILI9488_PGAMCTRL    0xE0 // NV Memory Write
#define ILI9488_NGAMCTRL    0xE1 // NV Memory Control
#define ILI9488_MADCTL_MY   0x80
#define ILI9488_MADCTL_MX   0x40
#define ILI9488_MADCTL_MV   0x20
#define ILI9488_MADCTL_ML   0x10
#define ILI9488_MADCTL_RGB  0x00
#define ILI9488_MADCTL_BGR  0x08
#define ILI9488_MADCTL_MH   0x04
#define ILI9488_MADCTL_SS   0x02
#define ILI9488_MADCTL_GS   0x01
//----------------------------------------------------------------------------------------------------------------------------------------------------
#define ST7796_NOP          0x00 // No operation
#define ST7796_SWRESET      0x01 // Software reset
#define ST7796_SLPIN        0x10 // Sleep in
#define ST7796_SLPOUT       0x11 // Sleep Out
#define ST7796_NORON        0x13 // Normal Display Mode On
#define ST7796_INVOFF       0x20 // Display Inversion OFF
#define ST7796_INVON        0x21 // Display Inversion ON
#define ST7796_GAMSET       0x26 // Gamma set
#define ST7796_DISPOFF      0x28 // Display Off
#define ST7796_DISPON       0x29 // Display On
#define ST7796_CASET        0x2A // Column Address Set
#define ST7796_RASET        0x2B // Row Address Set
#define ST7796_RAMWR        0x2C // Memory Write
#define ST7796_RAMRD        0x2E // Memory Read
#define ST7796_MADCTL       0x36 // Memory Data Access Control
#define ST7796_COLMOD       0x3A // Interface Pixel Format
#define ST7796_IFMODE       0xB0 // RAM control
#define ST7796_FRMCTR1      0xB1 // RGB Interface Control
#define ST7796_FRMCTR2      0xB2 // Porch control
#define ST7796_FRMCTR3      0xB3 // Frame Rate Control 1 (In partial mode/ idle colors)
#define ST7796_DIC          0xB4 // Display Inversion Control
#define ST7796_BPC          0xB5 // Blanking Porch Control
#define ST7796_DFC          0xB6 // Display Function Control
#define ST7796_EM           0xB7 // Entry Mode Set
#define ST7796_VCOMS        0xBB // VCOMS setting
#define ST7796_PWR1         0xC0 // Power Control 1
#define ST7796_PWR2         0xC1 // Power Control 2
#define ST7796_PWR3         0xC2 // Power Control 3
#define ST7796_VCMPCTL      0xC5 // VCOM Control
#define ST7796_VCM          0xC6 // Vcom Offset Register
#define ST7796_NVMADW       0xD0 // NVM Address/Data Write
#define ST7796_PGC          0xE0 // Positive Gamma Control
#define ST7796_NGC          0xE1 // Negative Gamma Control
#define ST7796_DOCA         0xE8 // Display Output Ctrl Adjust
#define ST7796_CSCON        0xF0 // Command Set Control
#define ST7796_MADCTL_MY    0x80 // Bit 7 Parameter MADCTL
#define ST7796_MADCTL_MX    0x40 // Bit 6 Parameter MADCTL
#define ST7796_MADCTL_MV    0x20 // Bit 5 Parameter MADCTL
#define ST7796_MADCTL_ML    0x10 // Bit 4 Parameter MADCTL
#define ST7796_MADCTL_RGB   0x00 // Bit 3 Parameter MADCTL
#define ST7796_MADCTL_BGR   0x08 // Bit 3 Parameter MADCTL
#define ST7796_MADCTL_MH    0x04 // Bit 2 Parameter MADCTL
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::init() {
    startWrite();
    if(_TFTcontroller == ILI9341){
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ILI9341");
        writeCommand(0xCB); // POWERA
        spi_TFT->write(0x39); spi_TFT->write(0x2C); spi_TFT->write(0x00);
        spi_TFT->write(0x34); spi_TFT->write(0x02);

        writeCommand(0xCF); // POWERB
        spi_TFT->write(0x00); spi_TFT->write(0xC1); spi_TFT->write(0x30);

        writeCommand(0xE8); // DTCA
        spi_TFT->write(0x85); spi_TFT->write(0x00); spi_TFT->write(0x78);

        writeCommand(0xEA); // DTCB
        spi_TFT->write(0x00); spi_TFT->write(0x00);

        writeCommand(0xED); // POWER_SEQ
        spi_TFT->write(0x64); spi_TFT->write(0x03); spi_TFT->write(0X12);
        spi_TFT->write(0X81);

        writeCommand(0xF7);  // PRC
        spi_TFT->write(0x20);

        writeCommand(0xC0); // Power control
        spi_TFT->write(0x23); // VRH[5:0]

        writeCommand(0xC1); // Power control
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
        spi_TFT->write(0x00); spi_TFT->write(0x00);
        spi_TFT->write(0x00); spi_TFT->write(0xEF);

        writeCommand(0x2A); // PAGE_ADDR
        spi_TFT->write(0x00); spi_TFT->write(0x00);
        spi_TFT->write(0x01); spi_TFT->write(0x3F);

        writeCommand(0x26); // Gamma curve selected
        spi_TFT->write(0x01);

        writeCommand(0xE0); // Set Gamma
        spi_TFT->write(0x0F); spi_TFT->write(0x31); spi_TFT->write(0x2B);
        spi_TFT->write(0x0C); spi_TFT->write(0x0E); spi_TFT->write(0x08);
        spi_TFT->write(0x4E); spi_TFT->write(0xF1); spi_TFT->write(0x37);
        spi_TFT->write(0x07); spi_TFT->write(0x10); spi_TFT->write(0x03);
        spi_TFT->write(0x0E); spi_TFT->write(0x09); spi_TFT->write(0x00);

        writeCommand(0xE1); // Set Gamma
        spi_TFT->write(0x00); spi_TFT->write(0x0E); spi_TFT->write(0x14);
        spi_TFT->write(0x03); spi_TFT->write(0x11); spi_TFT->write(0x07);
        spi_TFT->write(0x31); spi_TFT->write(0xC1); spi_TFT->write(0x48);
        spi_TFT->write(0x08); spi_TFT->write(0x0F); spi_TFT->write(0x0C);
        spi_TFT->write(0x31); spi_TFT->write(0x36); spi_TFT->write(0x0F);

        if(_displayInversion == 0){
            writeCommand(ILI9341_INVOFF); // Display Inversion OFF, normal mode
        }
        else{
            writeCommand(ILI9341_INVON);  // Display Inversion ON
        }

        writeCommand(0x11); // Sleep out
        delay(120);
        writeCommand(0x2c);

        writeCommand(0x29); // Display on
        writeCommand(0x2c);
    }
    if(_TFTcontroller == HX8347D) {
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "HX8347D");
        //Driving ability Setting
        writeCommand(0xEA); spi_TFT->write(0x00); // PTBA[15:8]
        writeCommand(0xEB); spi_TFT->write(0x20); // PTBA[7:0]
        writeCommand(0xEC); spi_TFT->write(0x0C); // STBA[15:8]
        writeCommand(0xED); spi_TFT->write(0xC4); // STBA[7:0]
        writeCommand(0xE8); spi_TFT->write(0x40); // OPON[7:0]
        writeCommand(0xE9); spi_TFT->write(0x38); // OPON1[7:0]
        writeCommand(0xF1); spi_TFT->write(0x01); // OTPS1B
        writeCommand(0xF2); spi_TFT->write(0x10); // GEN
        writeCommand(0x27); spi_TFT->write(0xA3); // Display control 2 register

        //Gamma 2.2 Setting
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

        //Power Voltage Setting
        writeCommand(0x1B); spi_TFT->write(0x1B); // VRH=4.65V
        writeCommand(0x1A); spi_TFT->write(0x01); // BT (VGH~15V,VGL~-10V,DDVDH~5V)
        writeCommand(0x24); spi_TFT->write(0x15); // VMH(VCOM High voltage ~3.2V)
        writeCommand(0x25); spi_TFT->write(0x50); // VML(VCOM Low voltage -1.2V)

        writeCommand(0x23); spi_TFT->write(0x88); // for Flicker adjust //can reload from OTP
        //Power on Setting

        writeCommand(0x18); spi_TFT->write(0x36); // I/P_RADJ,N/P_RADJ, Normal mode 60Hz
        writeCommand(0x19); spi_TFT->write(0x01); // OSC_EN='1', start Osc

        if(_displayInversion == 0){
            writeCommand(0x01); spi_TFT->write(0x00); // DP_STB='0', out deep sleep
        }
        else {
            writeCommand(0x01);
            spi_TFT->write(0x02); // DP_STB='0', out deep sleep, invon = 1
        }

        writeCommand(0x1F); spi_TFT->write(0x88);// GAS=1, VOMG=00, PON=0, DK=1, XDK=0, DVDH_TRI=0, STB=0
        delay(5);
        writeCommand(0x1F); spi_TFT->write(0x80);// GAS=1, VOMG=00, PON=0, DK=0, XDK=0, DVDH_TRI=0, STB=0
        delay(5);
        writeCommand(0x1F); spi_TFT->write(0x90);// GAS=1, VOMG=00, PON=1, DK=0, XDK=0, DVDH_TRI=0, STB=0
        delay(5);
        writeCommand(0x1F);
        spi_TFT->write(0xD0); // GAS=1, VOMG=10, PON=1, DK=0, XDK=0, DDVDH_TRI=0, STB=0
        delay(5);
        //262k/65k color selection
        writeCommand(0x17); spi_TFT->write(0x05); //default 0x06 262k color // 0x05 65k color
        //SET PANEL
        writeCommand(0x36); spi_TFT->write(0x00); //SS_P, GS_P,REV_P,BGR_P
        //Display ON Setting
        writeCommand(0x28); spi_TFT->write(0x38); //GON=1, DTE=1, D=1000
        delay(40);
        writeCommand(0x28); spi_TFT->write(0x3C); //GON=1, DTE=1, D=1100

        writeCommand(0x16); spi_TFT->write(0x08); // MY=0, MX=0, MV=0, BGR=1
        //Set GRAM Area
        writeCommand(0x02); spi_TFT->write(0x00); // Column address start register upper byte
        writeCommand(0x03); spi_TFT->write(0x00); // Column address start register low byte
        writeCommand(0x04); spi_TFT->write(0x00);
        writeCommand(0x05); spi_TFT->write(0xEF); //Column End
        writeCommand(0x06); spi_TFT->write(0x00);
        writeCommand(0x07); spi_TFT->write(0x00); //Row Start
        writeCommand(0x08); spi_TFT->write(0x01);
        writeCommand(0x09); spi_TFT->write(0x3F); //Row End
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b){
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ILI9486");

        //Driving ability Setting
        writeCommand(0x11); // Sleep out, also SW reset
        delay(120);

        writeCommand(0x3A); // Interface Pixel Format
        spi_TFT->write16(0x55);

        writeCommand(0xC2); // Power Control 3 (For Normal Mode)
        spi_TFT->write16(0x44);

        writeCommand(0xC5); // VCOM Control
        spi_TFT->write16(0x00);
        spi_TFT->write16(0x00);
        spi_TFT->write16(0x00);
        spi_TFT->write16(0x00);

        if(_TFTcontroller ==ILI9486a){
            writeCommand(0xE0); // PGAMCTRL(Positive Gamma Control)
            spi_TFT->write16(0x00);
            spi_TFT->write16(0x04);
            spi_TFT->write16(0x0E);
            spi_TFT->write16(0x08);
            spi_TFT->write16(0x17);
            spi_TFT->write16(0x0A);
            spi_TFT->write16(0x40);
            spi_TFT->write16(0x79);
            spi_TFT->write16(0x4D);
            spi_TFT->write16(0x07);
            spi_TFT->write16(0x0E);
            spi_TFT->write16(0x0A);
            spi_TFT->write16(0x1A);
            spi_TFT->write16(0x1D);
            spi_TFT->write16(0x0F);

            writeCommand(0xE1); // NGAMCTRL (Negative Gamma Correction)
            spi_TFT->write16(0x00);
            spi_TFT->write16(0x1B);
            spi_TFT->write16(0x1F);
            spi_TFT->write16(0x02);
            spi_TFT->write16(0x10);
            spi_TFT->write16(0x05);
            spi_TFT->write16(0x32);
            spi_TFT->write16(0x34);
            spi_TFT->write16(0x43);
            spi_TFT->write16(0x02);
            spi_TFT->write16(0x0A);
            spi_TFT->write16(0x09);
            spi_TFT->write16(0x33);
            spi_TFT->write16(0x37);
            spi_TFT->write16(0x0F);
        }

        if(_TFTcontroller == ILI9486b){
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

        if(_displayInversion == 0){
            writeCommand(ILI9486_INVOFF); // Display Inversion OFF, normal mode   RPi LCD (A)
        }
        else{
            writeCommand(ILI9486_INVON);  // Display Inversion ON,                RPi LCD (B)
        }

        writeCommand(0x36); // Memory Access Control
        spi_TFT->write16(0x48);

        writeCommand(0x29); // Display ON
        delay(150);
    }
    //==========================================
    if(_TFTcontroller == ILI9488){
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

        if(_displayInversion == 0){
            writeCommand(ILI9488_INVOFF); // Display Inversion OFF, normal mode
        }
        else{
            writeCommand(ILI9488_INVON);  // Display Inversion ON
        }

        writeCommand(ILI9488_PWCTR1);     // Power Control 1
        spi_TFT->write(0x17);
        spi_TFT->write(0x15);
        writeCommand(ILI9488_PWCTR2);     // Power Control 2
        spi_TFT->write(0x41);
        writeCommand(ILI9488_VMCTR1);     // VCOM Control
        spi_TFT->write(0x00);
        spi_TFT->write(0x12);
        spi_TFT->write(0x80);
        writeCommand(ILI9488_MADCTL);     // Memory Access Control
        spi_TFT->write(0x48);
        writeCommand(ILI9488_COLMOD);     // Pixel Interface Format
        spi_TFT->write(0x66);
        writeCommand(ILI9488_IFMODE);     // Interface Mode Control
        spi_TFT->write(0x00);
        writeCommand(ILI9488_FRMCTR1);    // Frame Rate Control
        spi_TFT->write(0xA0);
        writeCommand(ILI9488_INVTR);      // Display Inversion Control
        spi_TFT->write(0x02);
        writeCommand(ILI9488_DISCTRL);    // Display Function Control
        spi_TFT->write(0x02);
        spi_TFT->write(0x02);
        spi_TFT->write(0x3B);
        writeCommand(ILI9488_ETMOD);      // Entry Mode Set
        spi_TFT->write(0xC6);
        writeCommand(0xF7);               //Adjust Control 3
        spi_TFT->write(0xA9);
        spi_TFT->write(0x51);
        spi_TFT->write(0x2C);
        spi_TFT->write(0x82);
        writeCommand(ILI9488_SLPOUT);    // Exit Sleep
        delay(120);
        writeCommand(ILI9488_DISPON);    // Display on
        delay(25);
    }
    //==========================================
    if(_TFTcontroller == ST7796){
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ST7796");

        writeCommand(ST7796_SWRESET);
        delay(120);

        writeCommand(ST7796_SLPOUT);     // Sleep Out
        delay(120);

        writeCommand(ST7796_MADCTL);     // Memory Data Access Control
        spi_TFT->write(0x40);

        writeCommand(ST7796_CSCON);      // Command Set Control
        spi_TFT->write(0xC3);            // Enable extension command 2 partI

        writeCommand(ST7796_CSCON);      // Command Set Control
        spi_TFT->write(0x96);            // Enable extension command 2 partII

        writeCommand(ST7796_DIC);        // Display Inversion Control
        spi_TFT->write(0x00);

        writeCommand(ST7796_IFMODE);     // RAM control
        spi_TFT->write(0x00);

        writeCommand(ST7796_BPC);        // Blanking Porch Control
        spi_TFT->write(0x08);
        spi_TFT->write(0x08);
        spi_TFT->write(0x00);
        spi_TFT->write(0x64);

        writeCommand(ST7796_PWR1);       // Power Control 1
        spi_TFT->write(0xF0);
        spi_TFT->write(0x17);

        writeCommand(ST7796_PWR2);       // Power Control 2
         spi_TFT->write(0x14); //

        writeCommand(ST7796_PWR3);       // Power Control 3
        spi_TFT->write(0xA7);

        writeCommand(ST7796_VCMPCTL);    // VCOM Control
        spi_TFT->write(0x20);

        writeCommand(ST7796_DOCA);       // Display Output Ctrl Adjust
        spi_TFT->write(0x40);
        spi_TFT->write(0x8A);
        spi_TFT->write(0x00);
        spi_TFT->write(0x00);
        spi_TFT->write(0x29);
        spi_TFT->write(0x01);
        spi_TFT->write(0xBF);
        spi_TFT->write(0x33);

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

        writeCommand(ST7796_NGC);        // NGAMCTRL (Negative Gamma Correction)
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

        writeCommand(ST7796_CSCON);      // Command Set Control
        spi_TFT->write(0x3C);            // Enable extension command 2 partI

        writeCommand(ST7796_CSCON);      // Command Set Control
        spi_TFT->write(0x69);            // Enable extension command 2 partII

        if(_displayInversion == 0){
            writeCommand(ST7796_INVOFF); // Display Inversion OFF, normal mode
        }
        else{
            writeCommand(ST7796_INVON);  // Display Inversion ON
        }

        writeCommand(ST7796_DISPON);     //Display on
        delay(25);
    } //===============================================================================

    if(_TFTcontroller == ST7796RPI){
        if(tft_info) tft_info("init " ANSI_ESC_CYAN "ST7796_RPI");
        writeCommand(ST7796_SWRESET);
        delay(120);

        writeCommand(ST7796_SLPOUT);     // Sleep Out
        delay(120);

        writeCommand(ST7796_CSCON);      // Command Set Control
        spi_TFT->write16(0xC3);          // Enable extension command 2 partI

        writeCommand(ST7796_CSCON);      // Command Set Control
        spi_TFT->write16(0x96);          // Enable extension command 2 partII

        writeCommand(ST7796_MADCTL);     // Memory Data Access Control
        spi_TFT->write16(0x48);

        writeCommand(ST7796_COLMOD);     //Memory Data Access Control MX, MY, RGB mode
        spi_TFT->write16(0x55);

        writeCommand(ST7796_DIC);        // Display Inversion Control
        spi_TFT->write16(0x00);

        writeCommand(ST7796_IFMODE);     // RAM control
        spi_TFT->write16(0x00);

        writeCommand(ST7796_BPC);        // Blanking Porch Control
        spi_TFT->write16(0x08);
        spi_TFT->write16(0x08);
        spi_TFT->write16(0x00);
        spi_TFT->write16(0x64);

        writeCommand(ST7796_PWR1);       // Power Control 1
        spi_TFT->write16(0xF0);
        spi_TFT->write16(0x17);

        writeCommand(ST7796_PWR2);       // Power Control 2
         spi_TFT->write16(0x14); //

        writeCommand(ST7796_PWR3);       // Power Control 3
        spi_TFT->write16(0xA7);

        writeCommand(ST7796_VCMPCTL);    // VCOM Control
        spi_TFT->write16(0x20);

        writeCommand(ST7796_DOCA);       // Display Output Ctrl Adjust
        spi_TFT->write16(0x40);
        spi_TFT->write16(0x8A);
        spi_TFT->write16(0x00);
        spi_TFT->write16(0x00);
        spi_TFT->write16(0x29);
        spi_TFT->write16(0x01);
        spi_TFT->write16(0xBF);
        spi_TFT->write16(0x33);

        //--------------------------------ST7789V gamma setting---------------------------------------//
        writeCommand(ST7796_PGC); // PGAMCTRL(Positive Gamma Control)
        spi_TFT->write16(0xF0);
        spi_TFT->write16(0x0B);
        spi_TFT->write16(0x11);
        spi_TFT->write16(0x0B);
        spi_TFT->write16(0x0A);
        spi_TFT->write16(0x27);
        spi_TFT->write16(0x3C);
        spi_TFT->write16(0x55);
        spi_TFT->write16(0x51);
        spi_TFT->write16(0x37);
        spi_TFT->write16(0x15);
        spi_TFT->write16(0x17);
        spi_TFT->write16(0x31);
        spi_TFT->write16(0x35);

        writeCommand(ST7796_NGC);        // NGAMCTRL (Negative Gamma Correction)
        spi_TFT->write16(0x4E);
        spi_TFT->write16(0x15);
        spi_TFT->write16(0x19);
        spi_TFT->write16(0x0B);
        spi_TFT->write16(0x09);
        spi_TFT->write16(0x27);
        spi_TFT->write16(0x34);
        spi_TFT->write16(0x32);
        spi_TFT->write16(0x46);
        spi_TFT->write16(0x38);
        spi_TFT->write16(0x14);
        spi_TFT->write16(0x16);
        spi_TFT->write16(0x26);
        spi_TFT->write16(0x2A);

        writeCommand(ST7796_CSCON);      // Command Set Control
        spi_TFT->write16(0x3C);            // Enable extension command 2 partI

        writeCommand(ST7796_CSCON);      // Command Set Control
        spi_TFT->write16(0x69);            // Enable extension command 2 partII

        if(_displayInversion == 0){
            writeCommand(ST7796_INVOFF); // Display Inversion OFF, normal mode
        }
        else{
            writeCommand(ST7796_INVON);  // Display Inversion ON
        }

        writeCommand(ST7796_DISPON);     //Display on
        delay(25);
    }

    endWrite();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

TFT::TFT(uint8_t TFTcontroller, uint8_t dispInv) {
    _TFTcontroller = TFTcontroller; //0=ILI9341, 1=HX8347D, 2=ILI9486(a), 3=ILI9486(b), 4= ILI9488, 5=ST7796
    _displayInversion = dispInv;    //0=off default, 1=on

    if(_TFTcontroller == ILI9341){
        _height = 320;
        _width  = 240;
        _rotation=0;
    }
    if(_TFTcontroller == HX8347D){
        _height = 320;
        _width  = 240;
        _rotation=0;
    }
    if(_TFTcontroller == ILI9486a){
        _height = 480;
        _width  = 320;
        _rotation=0;
    }
    if(_TFTcontroller == ILI9486b){ // Waveshare
        _height = 480;
        _width  = 320;
        _rotation=0;
    }
    if(_TFTcontroller == ILI9488){
        _height = 480;
        _width  = 320;
        _rotation=0;
    }
    if(_TFTcontroller == ST7796 || _TFTcontroller == ST7796RPI){
        _height = 480;
        _width  = 320;
        _rotation=0;
    }
    _freq = 20000000;
    spi_TFT = &SPI;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::setFrequency(uint32_t f){
    if(f>80000000) f=80000000;
    _freq = f;  // overwrite default
    spi_TFT->setFrequency(_freq);
    TFT_SPI = SPISettings(_freq, MSBFIRST, SPI_MODE0);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::startWrite(void){
    spi_TFT->beginTransaction(TFT_SPI);
    TFT_CS_LOW();
}

void TFT::endWrite(void){
    TFT_CS_HIGH();
    spi_TFT->endTransaction();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::writeCommand(uint16_t cmd){
    TFT_DC_LOW();
    if(_TFTcontroller == ILI9341  || _TFTcontroller == HX8347D ||
       _TFTcontroller == ILI9488  || _TFTcontroller == ST7796) spi_TFT->write(cmd);

    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b || _TFTcontroller == ST7796RPI)  spi_TFT->write16(cmd);
    TFT_DC_HIGH();
}
// Return the size of the display (per current rotation)
int16_t TFT::width(void) const {
    return _width;
}
int16_t TFT::height(void) const {
    return _height;
}
uint8_t TFT::getRotation(void) const{
    return _rotation;
}

uint16_t TFT::readCommand(){
    uint16_t ret = 0;
    TFT_DC_LOW();
    if(_TFTcontroller == ILI9341  || _TFTcontroller == HX8347D ||
       _TFTcontroller == ILI9488  || _TFTcontroller == ST7796) ret = spi_TFT->transfer(0);

    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b || _TFTcontroller == ST7796RPI)  ret = spi_TFT->transfer16(0);
    TFT_DC_HIGH();
    return ret;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::begin(uint8_t CS, uint8_t DC, uint8_t spi, uint8_t mosi, uint8_t miso, uint8_t sclk) {

    spi_TFT = new SPIClass(spi);
    spi_TFT->begin(sclk, miso, mosi, -1);
    spi_TFT->setFrequency(_freq);
    SPItransfer = spi_TFT;

    TFT_SPI = SPISettings(_freq, MSBFIRST, SPI_MODE0);


    String info="";
    _TFT_CS = CS; _TFT_DC = DC;

    pinMode(_TFT_DC, OUTPUT);
    digitalWrite(_TFT_DC, LOW);
    pinMode(_TFT_CS, OUTPUT);
    digitalWrite(_TFT_CS, HIGH);

    // log_i("DC=%d, CS=%d, MISO=%d, MOSI=%d, SCK=%d", TFT_DC, TFT_CS, TFT_MISO, TFT_MOSI, TFT_SCK);
    spi_TFT->begin(_TFT_SCK, _TFT_MISO, _TFT_MOSI, -1);

    init();  //
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

typedef struct {
        uint8_t madctl;
        uint8_t bmpctl;
        uint16_t width;
        uint16_t height;
} rotation_data_t;

const rotation_data_t ili9341_rotations[4] = {
    {   (ILI9341_MADCTL_MX|ILI9341_MADCTL_BGR),
        (ILI9341_MADCTL_MX|ILI9341_MADCTL_MY|ILI9341_MADCTL_BGR),
        ILI9341_WIDTH,
        ILI9341_HEIGHT
    },
    {   (ILI9341_MADCTL_MV|ILI9341_MADCTL_BGR),
        (ILI9341_MADCTL_MV|ILI9341_MADCTL_MX|ILI9341_MADCTL_BGR),
        ILI9341_HEIGHT,
        ILI9341_WIDTH
    },
    {   (ILI9341_MADCTL_MY|ILI9341_MADCTL_BGR),
        (ILI9341_MADCTL_BGR),
        ILI9341_WIDTH,
        ILI9341_HEIGHT
    },
    {   (ILI9341_MADCTL_MX|ILI9341_MADCTL_MY|ILI9341_MADCTL_MV|ILI9341_MADCTL_BGR),
        (ILI9341_MADCTL_MY|ILI9341_MADCTL_MV|ILI9341_MADCTL_BGR),
        ILI9341_HEIGHT,
        ILI9341_WIDTH
    }
};
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::setRotation(uint8_t m) {
    _rotation = m % 4; // can't be higher than 3

    if(_TFTcontroller == HX8347D){ //"HX8347D"
        startWrite();
        if(_rotation==0){ writeCommand(0x16);   spi_TFT->write(0x08); // 0
                    writeCommand(0x04);     spi_TFT->write(0x00);
                    writeCommand(0x05);     spi_TFT->write(0xEF);
                    writeCommand(0x08);     spi_TFT->write(0x01);
                    writeCommand(0x09);     spi_TFT->write(0x3F);
                    _width = HX8347D_WIDTH;   _height = HX8347D_HEIGHT;}
        if(_rotation==1){ writeCommand(0x16);       spi_TFT->write(0x68); // 90
                    writeCommand(0x04);     spi_TFT->write(0x01);
                    writeCommand(0x05);     spi_TFT->write(0x3F);
                    writeCommand(0x08);     spi_TFT->write(0x00);
                    writeCommand(0x09);     spi_TFT->write(0xEF);
                    _width = HX8347D_HEIGHT;  _height = HX8347D_WIDTH;}
        if(_rotation==2){ writeCommand(0x16);       spi_TFT->write(0xC8); // 180
                    writeCommand(0x04);     spi_TFT->write(0x00);
                    writeCommand(0x05);     spi_TFT->write(0xEF);
                    writeCommand(0x08);     spi_TFT->write(0x01);
                    writeCommand(0x09);     spi_TFT->write(0x3F);
                    _width = HX8347D_WIDTH;   _height = HX8347D_HEIGHT;}
        if(_rotation==3){ writeCommand(0x16);   spi_TFT->write(0xA8); // 270
                    writeCommand(0x04);     spi_TFT->write(0x01);
                    writeCommand(0x05);     spi_TFT->write(0x3F);
                    writeCommand(0x08);     spi_TFT->write(0x00);
                    writeCommand(0x09);     spi_TFT->write(0xEF);
                    _width = HX8347D_HEIGHT;  _height = HX8347D_WIDTH;}
        endWrite();
    }
    if(_TFTcontroller == ILI9341){  //ILI9341
        m = ili9341_rotations[_rotation].madctl;
        _width  = ili9341_rotations[_rotation].width;
        _height = ili9341_rotations[_rotation].height;
        startWrite();
        writeCommand(ILI9341_MADCTL);
        spi_TFT->write(m);
        endWrite();
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b){
        _rotation = m % 4; // can't be higher than 3
        startWrite();
        writeCommand(ILI9486_MADCTL);
        switch (_rotation) {
            case 0:
                spi_TFT->write16(ILI9486_MADCTL_MX | ILI9486_MADCTL_BGR);
                _width  = ILI9486_WIDTH;
                _height = ILI9486_HEIGHT;
                break;
            case 1:
                spi_TFT->write16(ILI9486_MADCTL_MV | ILI9486_MADCTL_BGR);
                _width  = ILI9486_HEIGHT;
                _height = ILI9486_WIDTH;
                break;
            case 2:
                spi_TFT->write16(ILI9486_MADCTL_MY | ILI9486_MADCTL_BGR);
                _width  = ILI9486_WIDTH;
                _height = ILI9486_HEIGHT;
                break;
            case 3:
                spi_TFT->write16(ILI9486_MADCTL_MX | ILI9486_MADCTL_MY | ILI9486_MADCTL_MV | ILI9486_MADCTL_BGR);
                _width  = ILI9486_HEIGHT;
                _height = ILI9486_WIDTH;
                break;
        }
        endWrite();
    }
    if(_TFTcontroller == ILI9488){
        _rotation = m % 4; // can't be higher than 3
        startWrite();
        writeCommand(ILI9488_MADCTL);
        switch (_rotation) {
            case 0:
                spi_TFT->write(ILI9488_MADCTL_MX | ILI9488_MADCTL_BGR);
                _width  = ILI9488_WIDTH;
                _height = ILI9488_HEIGHT;
                break;
            case 1:
                spi_TFT->write(ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR);
                _width  = ILI9488_HEIGHT;
                _height = ILI9488_WIDTH;
                break;
            case 2:
                spi_TFT->write(ILI9488_MADCTL_MY | ILI9488_MADCTL_BGR);
                _width  = ILI9488_WIDTH;
                _height = ILI9488_HEIGHT;
                break;
            case 3:
                spi_TFT->write(ILI9488_MADCTL_MX | ILI9488_MADCTL_MY | ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR);
                _width  = ILI9488_HEIGHT;
                _height = ILI9488_WIDTH;
                break;
        }
        endWrite();
    }
    if(_TFTcontroller == ST7796){
        _rotation = m % 4; // can't be higher than 3
        startWrite();
        writeCommand(ST7796_MADCTL);
        switch (_rotation) {
            case 0:
                spi_TFT->write(ST7796_MADCTL_MX | ST7796_MADCTL_BGR);
                _width  = ST7796_WIDTH;
                _height = ST7796_HEIGHT;
                break;
            case 1:
                spi_TFT->write(ST7796_MADCTL_MV | ST7796_MADCTL_BGR);
                _width  = ST7796_HEIGHT;
                _height = ST7796_WIDTH;
                break;
            case 2:
                spi_TFT->write(ST7796_MADCTL_MY | ST7796_MADCTL_BGR);
                _width  = ST7796_WIDTH;
                _height = ST7796_HEIGHT;
                break;
            case 3:
                spi_TFT->write(ST7796_MADCTL_MX | ST7796_MADCTL_MY | ST7796_MADCTL_MV | ST7796_MADCTL_BGR);
                _width  = ST7796_HEIGHT;
                _height = ST7796_WIDTH;
                break;
        }
        endWrite();
    }
    if(_TFTcontroller == ST7796RPI){
        _rotation = m % 4; // can't be higher than 3
        startWrite();
        writeCommand(ST7796_MADCTL);
        switch (_rotation) {
            case 0:
                spi_TFT->write16(ST7796_MADCTL_MX | ST7796_MADCTL_BGR);
                _width  = ST7796_WIDTH;
                _height = ST7796_HEIGHT;
                break;
            case 1:
                spi_TFT->write16(ST7796_MADCTL_MV | ST7796_MADCTL_BGR);
                _width  = ST7796_HEIGHT;
                _height = ST7796_WIDTH;
                break;
            case 2:
                spi_TFT->write16(ST7796_MADCTL_MY | ST7796_MADCTL_BGR);
                _width  = ST7796_WIDTH;
                _height = ST7796_HEIGHT;
                break;
            case 3:
                spi_TFT->write16(ST7796_MADCTL_MX | ST7796_MADCTL_MY | ST7796_MADCTL_MV | ST7796_MADCTL_BGR);
                _width  = ST7796_HEIGHT;
                _height = ST7796_WIDTH;
                break;
        }
        endWrite();
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::invertDisplay(bool i) {
    startWrite();
    if(_TFTcontroller == ILI9341)   {writeCommand(i ? ILI9341_INVON : ILI9341_INVOFF);}
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b) {writeCommand(i ? ILI9486_INVON : ILI9486_INVOFF);}
    if(_TFTcontroller == ILI9488)   {writeCommand(i ? ILI9488_INVON : ILI9488_INVOFF);}
    if(_TFTcontroller == ST7796   || _TFTcontroller == ST7796RPI){writeCommand(i ? ST7796_INVON  : ST7796_INVOFF);}
    endWrite();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::scrollTo(uint16_t y) {
    if(_TFTcontroller != ILI9341) return;
    startWrite();
    writeCommand(ILI9341_VSCRSADD);
    spi_TFT->write16(y);
    endWrite();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

// void TFT::setBrigthness(uint8_t br){ // not used
//     startWrite();
//     writeCommand(ILI9486_CDBVAL);
//     spi_TFT->write16(0);
//     writeCommand(ILI9486_WDBVAL);
//     spi_TFT->write16(128);
//     endWrite();
// }

/*
 * Transaction API
 * */
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    if(_TFTcontroller == ILI9341){  //ILI9341
        uint32_t xa = ((uint32_t)x << 16) | (x+w-1);
        uint32_t ya = ((uint32_t)y << 16) | (y+h-1);
        writeCommand(ILI9341_CASET);
        spi_TFT->write32(xa);
        writeCommand(ILI9341_RASET);
        spi_TFT->write32(ya);
        writeCommand(ILI9341_RAMWR);
    }
    if(_TFTcontroller == HX8347D){  // HX8347D
        writeCommand(0x02); spi_TFT->write(x >> 8);
        writeCommand(0x03); spi_TFT->write(x & 0xFF);        //Column Start
        writeCommand(0x04); spi_TFT->write((x+w-1) >> 8);
        writeCommand(0x05); spi_TFT->write((x+w-1) & 0xFF);  //Column End
        writeCommand(0x06); spi_TFT->write(y >> 8);
        writeCommand(0x07); spi_TFT->write(y & 0xFF);        //Row Start
        writeCommand(0x08); spi_TFT->write((y+h-1) >> 8);
        writeCommand(0x09); spi_TFT->write((y+h-1) & 0xFF);  //Row End
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b){
        writeCommand(ILI9486_CASET);        // Column addr set
        spi_TFT->write16(x >> 8);
        spi_TFT->write16(x & 0xFF);         // XSTART
        w=x+w-1;
        spi_TFT->write16(w >> 8);
        spi_TFT->write16(w & 0xFF);         // XEND
        writeCommand(ILI9486_PASET);        // Row addr set
        spi_TFT->write16(y >> 8);
        spi_TFT->write16(y & 0xFF);         // YSTART
        h=y+h-1;
        spi_TFT->write16(h >> 8);
        spi_TFT->write16(h & 0xFF);         // YEND
        writeCommand(ILI9486_RAMWR);
    }
    if(_TFTcontroller == ILI9488){
        writeCommand(ILI9488_CASET);        // Column addr set
        spi_TFT->write(x >> 8);
        spi_TFT->write(x & 0xFF);           // XSTART
        w=x+w-1;
        spi_TFT->write(w >> 8);
        spi_TFT->write(w & 0xFF);           // XEND
        writeCommand(ILI9488_PASET);        // Row addr set
        spi_TFT->write(y >> 8);
        spi_TFT->write(y & 0xFF);           // YSTART
        h=y+h-1;
        spi_TFT->write(h >> 8);
        spi_TFT->write(h & 0xFF);           // YEND
        writeCommand(ILI9488_RAMWR);
    }
    if(_TFTcontroller == ST7796){
        writeCommand(ST7796_CASET);         // Column addr set
        spi_TFT->write(x >> 8);
        spi_TFT->write(x & 0xFF);           // XSTART
        w=x+w-1;
        spi_TFT->write(w >> 8);
        spi_TFT->write(w & 0xFF);           // XEND
        writeCommand(ST7796_RASET);         // Row addr set
        spi_TFT->write(y >> 8);
        spi_TFT->write(y & 0xFF);           // YSTART
        h=y+h-1;
        spi_TFT->write(h >> 8);
        spi_TFT->write(h & 0xFF);           // YEND
        writeCommand(ST7796_RAMWR);
    }
    if(_TFTcontroller == ST7796RPI){
        writeCommand(ST7796_CASET);         // Column addr set
        spi_TFT->write16(x >> 8);
        spi_TFT->write16(x & 0xFF);         // XSTART
        w=x+w-1;
        spi_TFT->write16(w >> 8);
        spi_TFT->write16(w & 0xFF);         // XEND
        writeCommand(ST7796_RASET);         // Row addr set
        spi_TFT->write16(y >> 8);
        spi_TFT->write16(y & 0xFF);         // YSTART
        h=y+h-1;
        spi_TFT->write16(h >> 8);
        spi_TFT->write16(h & 0xFF);         // YEND
        writeCommand(ST7796_RAMWR);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::readAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    if(_TFTcontroller == ILI9341){  //ILI9341
        uint32_t xa = ((uint32_t)x << 16) | (x+w-1);
        uint32_t ya = ((uint32_t)y << 16) | (y+h-1);
        writeCommand(ILI9341_CASET);
        spi_TFT->write32(xa);
        writeCommand(ILI9341_RASET);
        spi_TFT->write32(ya);
        writeCommand(ILI9341_RAMRD);
    }
    if(_TFTcontroller == HX8347D){  // HX8347D
        writeCommand(0x02); spi_TFT->write(x >> 8);
        writeCommand(0x03); spi_TFT->write(x & 0xFF);        //Column Start
        writeCommand(0x04); spi_TFT->write((x+w-1) >> 8);
        writeCommand(0x05); spi_TFT->write((x+w-1) & 0xFF);  //Column End
        writeCommand(0x06); spi_TFT->write(y >> 8);
        writeCommand(0x07); spi_TFT->write(y & 0xFF);        //Row Start
        writeCommand(0x08); spi_TFT->write((y+h-1) >> 8);
        writeCommand(0x09); spi_TFT->write((y+h-1) & 0xFF);  //Row End
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b){
        writeCommand(ILI9486_CASET);        // Column addr set
        spi_TFT->write16(x >> 8);
        spi_TFT->write16(x & 0xFF);         // XSTART
        w=x+w-1;
        spi_TFT->write16(w >> 8);
        spi_TFT->write16(w & 0xFF);         // XEND
        writeCommand(ILI9486_PASET);        // Row addr set
        spi_TFT->write16(y >> 8);
        spi_TFT->write16(y & 0xFF);         // YSTART
        h=y+h-1;
        spi_TFT->write16(h >> 8);
        spi_TFT->write16(h & 0xFF);         // YEND
        writeCommand(ILI9486_RAMRD);
    }
    if(_TFTcontroller == ILI9488){
        writeCommand(ILI9488_CASET);        // Column addr set
        spi_TFT->write(x >> 8);
        spi_TFT->write(x & 0xFF);           // XSTART
        w=x+w-1;
        spi_TFT->write(w >> 8);
        spi_TFT->write(w & 0xFF);           // XEND
        writeCommand(ILI9488_PASET);        // Row addr set
        spi_TFT->write(y >> 8);
        spi_TFT->write(y & 0xFF);           // YSTART
        h=y+h-1;
        spi_TFT->write(h >> 8);
        spi_TFT->write(h & 0xFF);           // YEND
        writeCommand(ILI9488_RAMRD);
    }
    if(_TFTcontroller == ST7796){
        writeCommand(ST7796_CASET);         // Column addr set
        spi_TFT->write(x >> 8);
        spi_TFT->write(x & 0xFF);           // XSTART
        w=x+w-1;
        spi_TFT->write(w >> 8);
        spi_TFT->write(w & 0xFF);           // XEND
        writeCommand(ST7796_RASET);         // Row addr set
        spi_TFT->write(y >> 8);
        spi_TFT->write(y & 0xFF);           // YSTART
        h=y+h-1;
        spi_TFT->write(h >> 8);
        spi_TFT->write(h & 0xFF);           // YEND
        writeCommand(ST7796_RAMRD);
    }
    if(_TFTcontroller == ST7796RPI){
        writeCommand(ST7796_CASET);         // Column addr set
        spi_TFT->write16(x >> 8);
        spi_TFT->write16(x & 0xFF);         // XSTART
        w=x+w-1;
        spi_TFT->write16(w >> 8);
        spi_TFT->write16(w & 0xFF);         // XEND
        writeCommand(ST7796_RASET);         // Row addr set
        spi_TFT->write16(y >> 8);
        spi_TFT->write16(y & 0xFF);         // YSTART
        h=y+h-1;
        spi_TFT->write16(h >> 8);
        spi_TFT->write16(h & 0xFF);         // YEND
        writeCommand(ST7796_RAMRD);
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::startBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {

    startWrite();
    if(_TFTcontroller == ILI9341){ //ILI9341
        writeCommand(ILI9341_MADCTL);
        spi_TFT->write(ili9341_rotations[_rotation].bmpctl);
    }
    setAddrWindow(x, _height - y - h, w, h);
    if(_TFTcontroller == HX8347D){ // HX8347D
        if(_rotation==0){ writeCommand(0x16);   spi_TFT->write(0x88);} // 0
        if(_rotation==1){ writeCommand(0x16);   spi_TFT->write(0x38);} // 90
        if(_rotation==2){ writeCommand(0x16);   spi_TFT->write(0x48);} // 180
        if(_rotation==3){ writeCommand(0x16);   spi_TFT->write(0xE8);} // 270
        writeCommand(0x22);
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b){
        writeCommand(ILI9486_MADCTL);
        if(_rotation==0){spi_TFT->write16(ILI9486_MADCTL_MX | ILI9486_MADCTL_MY | ILI9486_MADCTL_ML | ILI9486_MADCTL_BGR);}
        if(_rotation==1){spi_TFT->write16(ILI9486_MADCTL_MH | ILI9486_MADCTL_MV | ILI9486_MADCTL_MX | ILI9486_MADCTL_BGR);}
        if(_rotation==2){spi_TFT->write16(ILI9486_MADCTL_MH | ILI9486_MADCTL_BGR);}
        if(_rotation==3){spi_TFT->write16(ILI9486_MADCTL_MV | ILI9486_MADCTL_MY | ILI9486_MADCTL_BGR);}
        writeCommand(ILI9486_RAMWR);
    }
    if(_TFTcontroller == ILI9488){
        writeCommand(ILI9488_MADCTL);
        if(_rotation==0){spi_TFT->write(ILI9488_MADCTL_MX | ILI9488_MADCTL_MY | ILI9488_MADCTL_ML | ILI9488_MADCTL_BGR);}
        if(_rotation==1){spi_TFT->write(ILI9488_MADCTL_MH | ILI9488_MADCTL_MV | ILI9488_MADCTL_MX | ILI9488_MADCTL_BGR);}
        if(_rotation==2){spi_TFT->write(ILI9488_MADCTL_MH | ILI9488_MADCTL_BGR);}
        if(_rotation==3){spi_TFT->write(ILI9488_MADCTL_MV | ILI9488_MADCTL_MY | ILI9488_MADCTL_BGR);}
        writeCommand(ILI9488_RAMWR);
    }
    if(_TFTcontroller == ST7796){
        writeCommand(ST7796_MADCTL);
        if(_rotation==0){spi_TFT->write(ST7796_MADCTL_MX | ST7796_MADCTL_MY | ST7796_MADCTL_ML | ST7796_MADCTL_BGR);}
        if(_rotation==1){spi_TFT->write(ST7796_MADCTL_MH | ST7796_MADCTL_MV | ST7796_MADCTL_MX | ST7796_MADCTL_BGR);}
        if(_rotation==2){spi_TFT->write(ST7796_MADCTL_MH | ST7796_MADCTL_BGR);}
        if(_rotation==3){spi_TFT->write(ST7796_MADCTL_MV | ST7796_MADCTL_MY | ST7796_MADCTL_BGR);}
        writeCommand(ST7796_RAMWR);
    }
    if(_TFTcontroller == ST7796RPI){
        writeCommand(ST7796_MADCTL);
        if(_rotation==0){spi_TFT->write16(ST7796_MADCTL_MX | ST7796_MADCTL_MY | ST7796_MADCTL_ML | ST7796_MADCTL_BGR);}
        if(_rotation==1){spi_TFT->write16(ST7796_MADCTL_MH | ST7796_MADCTL_MV | ST7796_MADCTL_MX | ST7796_MADCTL_BGR);}
        if(_rotation==2){spi_TFT->write16(ST7796_MADCTL_MH | ST7796_MADCTL_BGR);}
        if(_rotation==3){spi_TFT->write16(ST7796_MADCTL_MV | ST7796_MADCTL_MY | ST7796_MADCTL_BGR);}
        writeCommand(ST7796_RAMWR);
    }
    endWrite();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::endBitmap() {
    if(_TFTcontroller == ILI9341){  //ILI9341
        startWrite();
        writeCommand(ILI9341_MADCTL);
        spi_TFT->write(ili9341_rotations[_rotation].madctl);
        //setAddrWindow(x, _height - y - h, w, h);
        endWrite();
    }
    if(_TFTcontroller == HX8347D){ // HX8347D
        setRotation(_rotation); // return to old values
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b){
        setRotation(_rotation);
    }
    if(_TFTcontroller == ILI9488){
        setRotation(_rotation);
    }
    if(_TFTcontroller == ST7796 || _TFTcontroller == ST7796RPI){
        setRotation(_rotation);
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::startJpeg() {
    startWrite();
    if(_TFTcontroller == ILI9341){  //ILI9341
        writeCommand(ILI9341_MADCTL);
    }
    if(_TFTcontroller == HX8347D){ // HX8347D
        // nothing to do
    }
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b){
        writeCommand(ILI9486_MADCTL);
        if(_rotation==0){spi_TFT->write16(ILI9486_MADCTL_MH | ILI9486_MADCTL_MX | ILI9486_MADCTL_BGR);}
        if(_rotation==1){spi_TFT->write16(ILI9486_MADCTL_MV | ILI9486_MADCTL_BGR);}
        if(_rotation==2){spi_TFT->write16(ILI9486_MADCTL_MY | ILI9486_MADCTL_BGR);}
        if(_rotation==3){spi_TFT->write16(ILI9486_MADCTL_MV | ILI9486_MADCTL_MY | ILI9486_MADCTL_MX | ILI9486_MADCTL_BGR);}
    }
    if(_TFTcontroller == ILI9488){
        writeCommand(ILI9488_MADCTL);
        if(_rotation==0){spi_TFT->write(ILI9488_MADCTL_MH | ILI9488_MADCTL_MX | ILI9488_MADCTL_BGR);}
        if(_rotation==1){spi_TFT->write(ILI9488_MADCTL_MV | ILI9488_MADCTL_BGR);}
        if(_rotation==2){spi_TFT->write(ILI9488_MADCTL_MY | ILI9488_MADCTL_BGR);}
        if(_rotation==3){spi_TFT->write(ILI9488_MADCTL_MV | ILI9488_MADCTL_MY | ILI9488_MADCTL_MX | ILI9488_MADCTL_BGR);}
    }
    if(_TFTcontroller == ST7796){
        writeCommand(ST7796_MADCTL);
        if(_rotation==0){spi_TFT->write(ST7796_MADCTL_MH | ST7796_MADCTL_MX | ST7796_MADCTL_BGR);}
        if(_rotation==1){spi_TFT->write(ST7796_MADCTL_MV | ST7796_MADCTL_BGR);}
        if(_rotation==2){spi_TFT->write(ST7796_MADCTL_MY | ST7796_MADCTL_BGR);}
        if(_rotation==3){spi_TFT->write(ST7796_MADCTL_MV | ST7796_MADCTL_MY | ST7796_MADCTL_MX | ST7796_MADCTL_BGR);}
    }
    if(_TFTcontroller == ST7796RPI){
        writeCommand(ST7796_MADCTL);
        if(_rotation==0){spi_TFT->write16(ST7796_MADCTL_MH | ST7796_MADCTL_MX | ST7796_MADCTL_BGR);}
        if(_rotation==1){spi_TFT->write16(ST7796_MADCTL_MV | ST7796_MADCTL_BGR);}
        if(_rotation==2){spi_TFT->write16(ST7796_MADCTL_MY | ST7796_MADCTL_BGR);}
        if(_rotation==3){spi_TFT->write16(ST7796_MADCTL_MV | ST7796_MADCTL_MY | ST7796_MADCTL_MX | ST7796_MADCTL_BGR);}
    }
    endWrite();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::endJpeg() {
        setRotation(_rotation);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::writePixels(uint16_t * colors, uint32_t len){
    if((_TFTcontroller == ILI9488) || (_TFTcontroller == ST7796)){
        uint32_t i = 0;
        while(len){
            write24BitColor(*(colors + i));
            i++;
            len--;
        }
    }
    else{
        spi_TFT->writePixels((uint8_t*)colors , len * 2);
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::writeColor(uint16_t color, uint32_t len){
    if((_TFTcontroller == ILI9488) || (_TFTcontroller == ST7796)){
        uint8_t r = (color & 0xF800) >> 8;
        uint8_t g = (color & 0x07E0) >> 3;
        uint8_t b = (color & 0x001F) << 3;
        uint8_t c[3] = {r, g, b};
        spi_TFT->writePattern(c, 3, len);
    }
    else{
        uint8_t c[2];
        c[0] =(color & 0xFF00) >> 8;
        c[1] = color & 0x00FF;
        spi_TFT->writePattern(c, 2, len );
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::write24BitColor(uint16_t color) {
    spi_TFT->write((color & 0xF800) >> 8); // r
    spi_TFT->write((color & 0x07E0) >> 3); // g
    spi_TFT->write((color & 0x001F) << 3); // b
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::writePixel(int16_t x, int16_t y, uint16_t color) {
    if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;
    setAddrWindow(x,y,1,1);
    switch(_TFTcontroller){
        case ILI9341:                                spi_TFT->write16(color);   break;
        case HX8347D:   writeCommand(0x22);          spi_TFT->write16(color);   break;
        case ILI9486a:  writeCommand(ILI9486_RAMWR); spi_TFT->write16(color);   break;
        case ILI9486b:  writeCommand(ILI9486_RAMWR); spi_TFT->write16(color);   break;
        case ILI9488:   writeCommand(ILI9488_RAMWR); write24BitColor(color);    break;
        case ST7796:    writeCommand(ST7796_RAMWR);  write24BitColor(color);    break;
        case ST7796RPI: writeCommand(ST7796_RAMWR);  spi_TFT->write16(color);   break;
        default: if(tft_info) tft_info("unknown tft controller");               break;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
    if((x >= _width) || (y >= _height)) return;

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
    if(x2 >= _width)  w = _width  - x;
    if(y2 >= _height) h = _height - y;

    int32_t len = (int32_t)w * h;
    setAddrWindow(x, y, w, h);
    if(_TFTcontroller == HX8347D) writeCommand(0x22);
    if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b)  writeCommand(ILI9486_RAMWR);
    if(_TFTcontroller == ILI9488) writeCommand(ILI9488_RAMWR);
    if(_TFTcontroller == ST7796   || _TFTcontroller == ST7796RPI) writeCommand(ST7796_RAMWR);
    writeColor(color, len);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color){
    writeFillRect(x, y, 1, h, color);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color){
    writeFillRect(x, y, w, 1, color);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t TFT::readcommand8(uint8_t c, uint8_t index) {
    uint32_t freq = _freq;
    if(_freq > 24000000){
        _freq = 24000000;
    }
    startWrite();
    writeCommand(0xD9);  // woo sekret command?
    spi_TFT->write(0x10 + index);
    writeCommand(c);
    uint8_t r = spi_TFT->transfer(0);
    endWrite();
    _freq = freq;
    return r;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::drawPixel(int16_t x, int16_t y, uint16_t color){
    startWrite();
    writePixel(x, y, color);
    endWrite();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
uint16_t TFT::color565(uint8_t r, uint8_t g, uint8_t b){
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::drawFastVLine(int16_t x, int16_t y,
        int16_t h, uint16_t color) {
    startWrite();
    writeFastVLine(x, y, h, color);
    endWrite();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::drawFastHLine(int16_t x, int16_t y,
        int16_t w, uint16_t color) {
    startWrite();
    writeFastHLine(x, y, w, color);
    endWrite();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,  uint16_t color) {
//  int16_t t;
//  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
//  if (steep) {
//    t=x0; x0=y0; y0=t;  // swap (x0, y0);
//  t=x1; x1=y1; y1=t;  //  swap(x1, y1);
//  }
//  if (x0 > x1) {
//  t=x0; x0=x1; x1=t;  // swap(x0, x1);
//  t=y0; y0=y1; y1=t;  // swap(y0, y1);
//  }
//
//  int16_t dx, dy;
//  dx = x1 - x0;
//  dy = abs(y1 - y0);
//
//  int16_t err = dx / 2;
//  int16_t ystep;
//
//  if (y0 < y1) {
//    ystep = 1;
//  } else {
//    ystep = -1;
//  }
//  startWrite();
//  for (; x0<=x1; x0++) {
//    if (steep) {
//      writePixel(y0, x0, color);
//    } else {
//      writePixel(x0, y0, color);
//    }
//    err -= dy;
//    if (err < 0) {
//      y0 += ystep;
//      err += dx;
//    }
//  }
//  endWrite();


    // Bresenham's algorithm - thx wikipedia - speed enhanced by Bodmer to use
    // an eficient FastH/V Line draw routine for line segments of 2 pixels or more
    int16_t t;
    bool steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        t=x0; x0=y0; y0=t;  // swap (x0, y0);
        t=x1; x1=y1; y1=t;  // swap(x1, y1);
    }
    if (x0 > x1) {
        t=x0; x0=x1; x1=t;  // swap(x0, x1);
        t=y0; y0=y1; y1=t;  // swap(y0, y1);
    }
    int16_t dx = x1 - x0, dy = abs(y1 - y0);;
    int16_t err = dx >> 1, ystep = -1, xs = x0, dlen = 0;

    if (y0 < y1) ystep = 1;
    startWrite();
    // Split into steep and not steep for FastH/V separation
    if (steep) {
        for (; x0 <= x1; x0++) {
            dlen++;
            err -= dy;
            if (err < 0) {
                err += dx;
                if (dlen == 1) writePixel(y0, xs, color);
                else writeFastVLine(y0, xs, dlen, color);
                dlen = 0; y0 += ystep; xs = x0 + 1;
            }
        }
        if (dlen) writeFastVLine(y0, xs, dlen, color);
    }
    else {
        for (; x0 <= x1; x0++) {
            dlen++;
            err -= dy;
            if (err < 0) {
                err += dx;
                if (dlen == 1) writePixel(xs, y0, color);
                else writeFastHLine(xs, y0, dlen, color);
                dlen = 0; y0 += ystep; xs = x0 + 1;
            }
        }
        if (dlen) writeFastHLine(xs, y0, dlen, color);
    }
    endWrite();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::fillScreen(uint16_t color) {
    // Update in subclasses if desired!
    fillRect(0, 0, width(), height(), color);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
        uint16_t color) {
    startWrite();
    writeFillRect(x,y,w,h,color);
    endWrite();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
        int16_t x2, int16_t y2, uint16_t color) {
    drawLine(x0, y0, x1, y1, color);
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x0, y0, color);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
        int16_t x2, int16_t y2, uint16_t color) {

    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        _swap_int16_t(y0, y1);
        _swap_int16_t(x0, x1);
    }
    if (y1 > y2) {
        _swap_int16_t(y2, y1);
        _swap_int16_t(x2, x1);
    }
    if (y0 > y1) {
        _swap_int16_t(y0, y1);
        _swap_int16_t(x0, x1);
    }
    startWrite();
    if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if (x1 < a)
            a = x1;
        else if (x1 > b)
            b = x1;
        if (x2 < a)
            a = x2;
        else if (x2 > b)
            b = x2;
        writeFastHLine(a, y0, b - a + 1, color);
        endWrite();
        return;
    }
    int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
            dx12 = x2 - x1, dy12 = y2 - y1;
    int32_t sa = 0, sb = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if (y1 == y2)
        last = y1;   // Include y1 scanline
    else
        last = y1 - 1; // Skip it

    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
         a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
         b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
         */
        if (a > b)
            _swap_int16_t(a, b);
        writeFastHLine(a, y, b - a + 1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = (int32_t)dx12 * (y - y1);
    sb = (int32_t)dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
         a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
         b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
         */
        if (a > b)
            _swap_int16_t(a, b);
        writeFastHLine(a, y, b - a + 1, color);
    }
    endWrite();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::drawRect(int16_t Xpos, int16_t Ypos, uint16_t Width, uint16_t Height, uint16_t Color)
{
    if(Width < 1 || Height < 1) return;
    startWrite();
    writeFastHLine(Xpos, Ypos, Width, Color);
    writeFastHLine(Xpos, Ypos + Height-1, Width, Color);
    writeFastVLine(Xpos, Ypos, Height, Color);
    writeFastVLine(Xpos + Width-1, Ypos, Height, Color);
    endWrite();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    // smarter version
    startWrite();
    writeFastHLine(x + r, y, w - 2 * r, color); // Top
    writeFastHLine(x + r, y + h - 1, w - 2 * r, color); // Bottom
    writeFastVLine(x, y + r, h - 2 * r, color); // Left
    writeFastVLine(x + w - 1, y + r, h - 2 * r, color); // Right
    // draw four corners
    drawCircleHelper(x + r, y + r, r, 1, color);
    drawCircleHelper(x + w - r - 1, y + r, r, 2, color);
    drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
    drawCircleHelper(x + r, y + h - r - 1, r, 8, color);
    endWrite();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    // smarter version
    startWrite();
    writeFillRect(x + r, y, w - 2 * r, h, color);

    // draw four corners
    fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
    fillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
    endWrite();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
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

    while (x < y) {
        if (f >= 0) {
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
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::fillCircle(int16_t Xm,       //specify x position.
                     int16_t Ym,       //specify y position.
                     uint16_t  r, //specify the radius of the circle.
                     uint16_t color)  //specify the color of the circle.
{
    int32_t f = 1 - r, ddF_x = 1, ddF_y = 0 - (2 * r), x = 0, y = r;
    startWrite();
    writeFastVLine(Xm, Ym - r, 2 * r, color);

    while (x < y) {
        if (f >= 0) {
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
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        if (cornername & 0x4) {
            writePixel(x0 + x, y0 + y, color);
            writePixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            writePixel(x0 + x, y0 - y, color);
            writePixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            writePixel(x0 - y, y0 + x, color);
            writePixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            writePixel(x0 - y, y0 - x, color);
            writePixel(x0 - x, y0 - y, color);
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

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
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data){

    uint16_t color = 0;

    uint32_t dataSize = w * h;
    uint32_t counter = 0;
    startWrite();
    readAddrWindow(x, y, w, h);
    readCommand(); // Dummy read to throw away don't care value

    // Read window pixel 24-bit RGB values
    uint8_t r, g, b;

    while (dataSize--) {
        if(_TFTcontroller == ILI9488){
            // The 6 colour bits are in MS 6 bits of each byte but we do not include the extra clock pulse so we use a trick
            // and mask the middle 6 bits of the byte, then only shift 1 place left
            r = (readCommand() & 0x7E) << 1;
            g = (readCommand() & 0x7E) << 1;
            b = (readCommand() & 0x7E) << 1;
            color = color565(r, g, b);
        }
        else{
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

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::setFont(uint16_t font){

#ifdef TFT_TIMES_NEW_ROMAN
    switch(font){
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
        default:
            log_e("unknown font size for Times New Roman, size is %i", font);
            break;
    }
#endif

#ifdef TFT_GARAMOND
    switch(font){
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
        default:
            break;
    }
#endif

#ifdef TFT_FREE_SERIF_ITALIC
    switch(font){
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
        default:
            break;
    }

#endif

#ifdef TFT_ARIAL
    switch(font){
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
        default:
            break;
    }
#endif

#ifdef TFT_Z300
    switch(font){
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
        default:
            break;
    }
#endif



}
/*******************************************************************************************************************************************************************************************************
 *                                                                                                                                                                                                     *
 *        ⏫⏫⏫⏫⏫⏫                                       W R I T E    T E X T    R E L A T E D    F U N C T I O N S                                                      ⏫⏫⏫⏫⏫⏫             *
 *                                                                                                                                                                                                     *
 * *****************************************************************************************************************************************************************************************************
*/
void TFT::writeInAddrWindow(const uint8_t* bmi, uint16_t posX, uint16_t poxY, uint16_t width, uint16_t height){

    uint16_t nrOfPixels = width * height;

    auto bitreader = [&](const uint8_t* bm) {  // lambda
        static uint16_t bmi = 0;
        static uint8_t idx = 0;
        static const uint8_t* bitmap = NULL;
        if(bm) {bitmap = bm; idx = 0x80; bmi = 0; return (uint16_t)0;}
        bool bit = *(bitmap + bmi)  & idx;
        idx >>= 1;
        if(idx == 0){bmi++; idx = 0x80;}
        if(bit){return _textColor;}
        return _backGroundColor;
    };

    bitreader(bmi);

    startWrite();
        setAddrWindow(posX, poxY, width , height);
        if     (_TFTcontroller == ILI9341){
            writeCommand(ILI9341_RAMWR); //ILI9341
            while (nrOfPixels--) spi_TFT->write16(bitreader(0)); // Send to TFT 16 bits at a time
        }
        else if(_TFTcontroller == HX8347D){
            writeCommand(0x22);
            while (nrOfPixels--) spi_TFT->write16(bitreader(0)); // Send to TFT 16 bits at a time
        }
        else if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b){
            writeCommand(ILI9486_RAMWR);
            while (nrOfPixels--) spi_TFT->write16(bitreader(0)); // Send to TFT 16 bits at a time
        }
        else if(_TFTcontroller == ILI9488){
            writeCommand(ILI9488_RAMWR);
            while (nrOfPixels--) write24BitColor(bitreader(0)); // Send to TFT 16 bits at a time
        }
        else if(_TFTcontroller == ST7796   || _TFTcontroller == ST7796RPI){
            writeCommand(ST7796_RAMWR);
            while (nrOfPixels--) write24BitColor(bitreader(0)); // Send to TFT 16 bits at a time
        }
    endWrite();
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

size_t TFT::writeText(const char* str, uint16_t win_X, uint16_t win_Y, int16_t win_W, int16_t win_H, uint8_t align, bool narrow, bool noWrap){

    uint16_t idx = 0;
    uint16_t utfPosArr[512] = {0};
    uint16_t strChLength = 0; // nr. of chars
    uint16_t ANSIcolor = 0;
    uint16_t ANSIcolor1 = 0;
    uint16_t ANSIcolor2 = 0;
    uint16_t ANSIcolor3 = 0;
    uint16_t ANSIcolor4 = 0;
    int16_t  ANSIcolorPos1 = -1;
    int16_t  ANSIcolorPos2 = -1;
    int16_t  ANSIcolorPos3 = -1;
    int16_t  ANSIcolorPos4 = -1;

//-------------------------------------------------------------------------------------------------------------------
    auto fitInLine = [&](uint16_t begin, uint16_t maxPxLength, uint16_t* usedPxLength) {  // lambda
        uint16_t idx = begin;
        uint16_t pxLength = 0;
        uint16_t lastSpacePos = 0;
        uint16_t drawableChars = 0;
        uint16_t lastUsedPxLength = 0;
        uint16_t glyphPos = 0;
        while(utfPosArr[idx] != 0){
            *usedPxLength = pxLength;
            if(utfPosArr[idx] == 0x20) {lastSpacePos = drawableChars; lastUsedPxLength = pxLength;}
            glyphPos = _current_font.lookup_table[utfPosArr[idx]];
            pxLength += _current_font.glyph_dsc[glyphPos].adv_w / 16;
            if(!narrow) pxLength += _current_font.glyph_dsc[glyphPos].ofs_x;
            if(pxLength > maxPxLength || utfPosArr[idx] == '\n'){ // force wrap
                if(noWrap) {return drawableChars;}
                if(lastSpacePos) {*usedPxLength = lastUsedPxLength; return lastSpacePos;}
                else return drawableChars;
            }
            idx++;
            drawableChars++;
            *usedPxLength = pxLength;
        }
        return drawableChars;
    };
//-------------------------------------------------------------------------------------------------------------------
    auto drawChar = [&](uint16_t idx, uint16_t x, uint16_t y){ // lambda
        uint16_t glyphPos = _current_font.lookup_table[utfPosArr[idx]];
        uint16_t adv_w = _current_font.glyph_dsc[glyphPos].adv_w / 16;
        uint32_t bitmap_index = _current_font.glyph_dsc[glyphPos].bitmap_index;
        uint16_t box_w  = _current_font.glyph_dsc[glyphPos].box_w;
        uint16_t box_h  = _current_font.glyph_dsc[glyphPos].box_h;
        int16_t  ofs_x = _current_font.glyph_dsc[glyphPos].ofs_x;
        int16_t  ofs_y = _current_font.glyph_dsc[glyphPos].ofs_y;
        x += ofs_x;
        y = y + (_current_font.line_height - _current_font.base_line - 1) - box_h - ofs_y;
        writeInAddrWindow(_current_font.glyph_bitmap + bitmap_index, x, y, box_w, box_h);
        if(!narrow) adv_w += ofs_x;
        return adv_w;
    };
//-------------------------------------------------------------------------------------------------------------------
    while(str[idx] != 0){
        if((uint8_t)str[idx] == '\033'){ // ANSI sequence
            if((uint8_t)str[idx + 1] == '[' && (uint8_t)str[idx + 2] == '3' && (uint8_t)str[idx + 4] == 'm'){
                switch (str[idx + 3]) {
                    case '0':   ANSIcolor = TFT_BLACK;         break; // ANSI_ESC_BLACK
                    case '1':   ANSIcolor = TFT_RED;           break; // ANSI_ESC_RED
                    case '2':   ANSIcolor = TFT_GREEN;         break; // ANSI_ESC_GREEN
                    case '3':   ANSIcolor = TFT_YELLOW;        break; // ANSI_ESC_YELLOW
                    case '4':   ANSIcolor = TFT_BLUE;          break; // ANSI_ESC_BLUE
                    case '5':   ANSIcolor = TFT_MAGENTA;       break; // ANSI_ESC_MAGENTA
                    case '6':   ANSIcolor = TFT_CYAN;          break; // ANSI_ESC_CYAN
                    case '7':   ANSIcolor = TFT_WHITE;         break; // ANSI_ESC_WHITE
                    default: log_w("unknown ANSI ESCAPE COLOR SEQUENCE "); break;
                }
                if(ANSIcolorPos1 == -1){ANSIcolorPos1 = strChLength; ANSIcolor1 = ANSIcolor; idx += 5; continue;}
                if(ANSIcolorPos2 == -1){ANSIcolorPos2 = strChLength; ANSIcolor2 = ANSIcolor; idx += 5; continue;}
                if(ANSIcolorPos3 == -1){ANSIcolorPos3 = strChLength; ANSIcolor3 = ANSIcolor; idx += 5; continue;}
                if(ANSIcolorPos4 == -1){ANSIcolorPos4 = strChLength; ANSIcolor4 = ANSIcolor; idx += 5; continue;}
            }
        }
        uint16_t codePoint = 0;
        if((uint8_t)str[idx] >= 0xE0){
            if((uint8_t)str[idx + 1] == 0x80 && (uint8_t)str[idx + 2] == 0x99){
                idx += 3; codePoint = 0x27;   // special sign 0xe28099 (general punctuation)
            }
            else{log_w("char can't displayed 0x%x%x%x", (uint8_t)str[idx], (uint8_t)str[idx + 1], (uint8_t)str[idx + 2]); idx += 3; codePoint = 0xA4;}
        }
        else if((uint8_t)str[idx] == 0xD1 && (uint8_t)str[idx + 1] > 0x9D) {log_w("char can't displayed 0x%x%x", (uint8_t)str[idx], (uint8_t)str[idx + 1]); idx += 2; codePoint = 0xA4;}
        else if((uint8_t)str[idx] >  0xD1){log_w("char can't displayed 0x%x%x", (uint8_t)str[idx], (uint8_t)str[idx + 1]); idx += 2; codePoint = 0xA4;}
        else if((uint8_t)str[idx] >= 0xC2){ //next char is UTF-8
            codePoint =((uint8_t)str[idx + 1] + ((uint8_t)str[idx]-0xC2) * 64);
            if(codePoint > _current_font.range_length) {log_w("char can't displayed 0x%x%x (not in charset)", (uint8_t)str[idx], (uint8_t)str[idx + 1]); codePoint = 0xA4;}
            idx += 2;
        }
        else{
            codePoint = (uint8_t)str[idx];
            if(codePoint > _current_font.range_length) {log_w("char can't displayed 0x%x (not in charset)", (uint8_t)str[idx]); codePoint = 0xA4;}
            idx++;
        }
        utfPosArr[strChLength] = codePoint;
        strChLength++;
    }
    if(!strChLength) return 0;
    //----------------------------------------------------------------------
    if((win_X + win_W)  > width()) {win_W  = width()  - win_X;} // Limit, right edge of the display
    if((win_Y + win_H) > height()){win_H = height() - win_Y;} // Limit, bottom of the display

    idx = 0;
    uint16_t pX = win_X;
    uint16_t pY = win_Y;
    int16_t  pH = win_H;
    int16_t  pW = win_W;

    uint16_t charsToDraw = 0;
    uint16_t usedPxLength = 0;
    uint16_t charsDrawn = 0;

    while(true){ // outer while
        if(noWrap && idx) goto exit;
        if(pH < _current_font.line_height){goto exit;}
        charsToDraw = fitInLine(idx, pW, &usedPxLength);
        if(align == TFT_ALIGN_RIGHT){  pX += win_W - (usedPxLength + 1);}
        if(align == TFT_ALIGN_CENTER){ pX += (win_W - usedPxLength) /2;}
        uint16_t cnt = 0;
        while(true){ // inner while
            if(idx == ANSIcolorPos1) {setTextColor(ANSIcolor1); charsDrawn += 4;}
            if(idx == ANSIcolorPos2) {setTextColor(ANSIcolor2); charsDrawn += 4;}
            if(idx == ANSIcolorPos3) {setTextColor(ANSIcolor3); charsDrawn += 4;}
            if(idx == ANSIcolorPos4) {setTextColor(ANSIcolor4); charsDrawn += 4;}
            if(cnt == 0 && utfPosArr[idx] == 0x20) {idx++; charsDrawn++; continue;}
            uint16_t res = drawChar(idx, pX, pY);
            pX += res;
            pW -= res;
            idx++;
            cnt++;
            charsDrawn++;
            if(idx == strChLength) goto exit;
            if(cnt == charsToDraw) break;
        }// inner while
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

#define bmpRead32(d,o) (d[o] | (uint16_t)(d[(o)+1]) << 8 | (uint32_t)(d[(o)+2]) << 16 | (uint32_t)(d[(o)+3]) << 24)
#define bmpRead16(d,o) (d[o] | (uint16_t)(d[(o)+1]) << 8)

#define bmpColor8(c)       (((uint16_t)(((uint8_t*)(c))[0] & 0xE0) << 8) | ((uint16_t)(((uint8_t*)(c))[0] & 0x1C) << 6) | ((((uint8_t*)(c))[0] & 0x3) << 3))
#define bmpColor16(c)      ((((uint8_t*)(c))[0] | ((uint16_t)((uint8_t*)(c))[1]) << 8))
#define bmpColor24(c)      (((uint16_t)(((uint8_t*)(c))[2] & 0xF8) << 8) | ((uint16_t)(((uint8_t*)(c))[1] & 0xFC) << 3) | ((((uint8_t*)(c))[0] & 0xF8) >> 3))
#define bmpColor32(c)      (((uint16_t)(((uint8_t*)(c))[3] & 0xF8) << 8) | ((uint16_t)(((uint8_t*)(c))[2] & 0xFC) << 3) | ((((uint8_t*)(c))[1] & 0xF8) >> 3))
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::bmpSkipPixels(fs::File &file, uint8_t bitsPerPixel, size_t len){
    size_t bytesToSkip = (len * bitsPerPixel) / 8;
    file.seek(bytesToSkip, SeekCur);
}
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::bmpAddPixels(fs::File &file, uint8_t bitsPerPixel, size_t len){
    size_t bytesPerTransaction = bitsPerPixel * 4;
    uint8_t transBuf[bytesPerTransaction];
    uint16_t pixBuf[32];
    uint8_t * tBuf;
    uint8_t pixIndex = 0;
    size_t wIndex = 0, pixNow, bytesNow;
    while(wIndex < len){
        pixNow = len - wIndex;
        if(pixNow > 32){
            pixNow = 32;
        }
        bytesNow = (pixNow * bitsPerPixel) / 8;
        file.read(transBuf, bytesNow);
        tBuf = transBuf;
        for(pixIndex=0; pixIndex < pixNow; pixIndex++){
            if(bitsPerPixel == 32){
                pixBuf[pixIndex] = (bmpColor32(tBuf));
                tBuf+=4;
            } else if(bitsPerPixel == 24){
                pixBuf[pixIndex] = (bmpColor24(tBuf));
                tBuf+=3;
            } else if(bitsPerPixel == 16){
                pixBuf[pixIndex] = (bmpColor16(tBuf));
                tBuf+=2;
            } else if(bitsPerPixel == 8){
                pixBuf[pixIndex] = (bmpColor8(tBuf));
                tBuf+=1;
            } else if(bitsPerPixel == 4){
                uint16_t g = tBuf[0] & 0xF;
                if(pixIndex & 1){
                    tBuf+=1;
                } else {
                    g = tBuf[0] >> 4;
                }
                pixBuf[pixIndex] = ((g << 12) | (g << 7) | (g << 1));
            }
        }

        startWrite();
        writePixels(pixBuf, pixNow);
        endWrite();
        wIndex += pixNow;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

bool TFT::drawBmpFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight,
                         uint16_t offX, uint16_t offY) {
    if ((x + maxWidth) > width() || (y + maxHeight) > height()) {
        log_e("Bad dimensions given");
        return false;
    }

    if (!maxWidth) {
        maxWidth = width() - x;
    }
    if (!maxHeight) {
        maxHeight = height() - y;
    }
    // log_i("maxWidth=%i, maxHeight=%i", maxWidth, maxHeight);
    if (!fs.exists(path)) {
        log_e("file %s not exists", path);
        return false;
    }

    File bmp_file = fs.open(path);
    if (!bmp_file) {
        log_e("Failed to open file for reading %s", path);
        return false;
    }
    size_t headerLen = 0x22;
    size_t fileSize = bmp_file.size();
    uint8_t headerBuf[headerLen];
    if (fileSize < headerLen || bmp_file.read(headerBuf, headerLen) < headerLen) {
        log_e("Failed to read the file's header");
        bmp_file.close();
        return false;
    }

    if (headerBuf[0] != 'B' || headerBuf[1] != 'M') {
        log_e("Wrong file format");
        bmp_file.close();
        return false;
    }

    // size_t bmpSize = bmpRead32(headerBuf, 0x02);
    uint32_t dataOffset = bmpRead32(headerBuf, 0x0A);
    int32_t bmpWidthI = bmpRead32(headerBuf, 0x12);
    int32_t bmpHeightI = bmpRead32(headerBuf, 0x16);
    uint16_t bitsPerPixel = bmpRead16(headerBuf, 0x1C);

    size_t bmpWidth = abs(bmpWidthI);
    size_t bmpHeight = abs(bmpHeightI);

    if (offX >= bmpWidth || offY >= bmpHeight) {
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

    if (ovfHeight) {
        bmpSkipPixels(bmp_file, bitsPerPixel, ovfHeight * bmpWidth);
    }
    if (!offX && !ovfWidth) {
        bmpAddPixels(bmp_file, bitsPerPixel, outWidth * outHeight);
    } else {
        size_t ih;
        for (ih = 0; ih < outHeight; ih++) {
            if (offX) {
                bmpSkipPixels(bmp_file, bitsPerPixel, offX);
            }
            bmpAddPixels(bmp_file, bitsPerPixel, outWidth);
            if (ovfWidth) {
                bmpSkipPixels(bmp_file, bitsPerPixel, ovfWidth);
            }
        }
    }

    endBitmap();
    bmp_file.close();
    return true;
}

/***********************************************************************************************************************
                                                G I F
***********************************************************************************************************************/
bool TFT::drawGifFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint8_t repeat) {
    // debug=true;
    int32_t iterations = repeat;

    gif_next.clear();              gif_next.shrink_to_fit();
    gif_vals.clear();              gif_vals.shrink_to_fit();
    gif_stack.clear();             gif_stack.shrink_to_fit();
    gif_GlobalColorTable.clear();  gif_GlobalColorTable.shrink_to_fit();
    gif_LocalColorTable.clear();   gif_LocalColorTable.shrink_to_fit();

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
//----------------------------------------------------------------------------------------------------------------------------------------------------
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

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::GIF_readLogicalScreenDescriptor(){


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

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::GIF_readImageDescriptor(){

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

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::GIF_readLocalColorTable() {
    // The size of the local color table can be calculated by the value given in the image descriptor.
    // Just like with the global color table, if the image descriptor specifies a size of N,
    // the color table will contain 2^(N+1) colors and will take up 3*2^(N+1) bytes.
    // The colors are specified in RGB value triplets.
    gif_LocalColorTable.clear();
    gif_LocalColorTable.shrink_to_fit();
    gif_LocalColorTable.reserve(gif_SizeOfLocalColorTable);
    if (gif_LocalColorTableFlag == 1) {
        char rgb_buff[3];
        uint16_t i = 0;
        while (i != gif_SizeOfLocalColorTable) {
            gif_file.readBytes(rgb_buff, 3);
            // fill LocalColorTable, pass 8-bit (each) R,G,B, get back 16-bit packed color
            gif_LocalColorTable.push_back(((rgb_buff[0] & 0xF8) << 8) | ((rgb_buff[1] & 0xFC) << 3) |
                                          ((rgb_buff[2] & 0xF8) >> 3));
            i++;
        }
        // for(i=0;i<SizeOfLocalColorTable; i++) log_i("LocalColorTable %i= %i", i, LocalColorTable[i]);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

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
    if (gif_GlobalColorTableFlag == 1) {
        char rgb_buff[3];
        uint16_t i = 0;
        while (i != gif_SizeOfGlobalColorTable) {
            gif_file.readBytes(rgb_buff, 3);
            // fill GlobalColorTable, pass 8-bit (each) R,G,B, get back 16-bit packed color
            gif_GlobalColorTable.push_back(((rgb_buff[0] & 0xF8) << 8) | ((rgb_buff[1] & 0xFC) << 3) |
                                           ((rgb_buff[2] & 0xF8) >> 3));
            i++;
        }
        //    for(i=0;i<gif_SizeOfGlobalColorTable;i++) log_i("GlobalColorTable %i= %i", i, gif_GlobalColorTable[i]);
        //    log_i("Read GlobalColorTable Size=%i", gif_SizeOfGlobalColorTable);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::GIF_readGraphicControlExtension(){

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

//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t TFT::GIF_readPlainTextExtension(char* buf){

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
//10  |               |       Character Cell Height         Byte
//    +---------------+
//11  |               |       Text Foreground Color Index   Byte
//    +---------------+
//12  |               |       Text Background Color Index   Byte
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
    if(BlockSize>0){
        gif_file.readBytes(gif_buffer, BlockSize);
        // log_i("%s", buffer);
    }

    gif_TextGridLeftPosition=gif_buffer[0]+ 256*gif_buffer[1];
    gif_TextGridTopPosition=gif_buffer[2]+ 256*gif_buffer[3];
    gif_TextGridWidth=gif_buffer[4]+ 256*gif_buffer[5];
    gif_TextGridHeight=gif_buffer[6]+ 256*gif_buffer[7];
    gif_CharacterCellWidth=gif_buffer[8];
    gif_CharacterCellHeight=gif_buffer[9];
    gif_TextForegroundColorIndex=gif_buffer[10];
    gif_TextBackgroundColorIndex=gif_buffer[11];

    if(debug){
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

//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t TFT::GIF_readApplicationExtension(char* buf){

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
//10  |               |       Appl. Authentication Code     3 Bytes
//    +-             -+
//11  |               |
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
    if(debug){
        log_i("BlockSize=%i", BlockSize);
    }
    if(BlockSize>0){
        gif_file.readBytes(gif_buffer, BlockSize);
    }
    numBytes = GIF_readDataSubBlock(buf);
    gif_file.readBytes(gif_buffer, 1); // BlockTerminator, marks the end of the Graphic Control Extension
    return numBytes;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t TFT::GIF_readCommentExtension(char *buf){

//    7 6 5 4 3 2 1 0        Field Name                    Type
//  +===============+
//  |               |
//N |               |       Comment Data                  Data Sub-blocks
//  |               |
//  +===============+
//
//  +---------------+
//0 |               |       Block Terminator              Byte
//  +---------------+

    uint8_t numBytes = 0;
    numBytes = GIF_readDataSubBlock(buf);
    sprintf(chbuf, "GIF: Comment %s", buf);
    // if(tft_info) tft_info(chbuf);
    gif_file.readBytes(gif_buffer, 1); // BlockTerminator, marks the end of the Graphic Control Extension
    return numBytes;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t TFT::GIF_readDataSubBlock(char *buf){

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

    uint8_t BlockSize=0;
    BlockSize=gif_file.read();
    if(BlockSize>0){
        gif_ZeroDataBlock = false;
        gif_file.readBytes(buf, BlockSize);
    }
    else gif_ZeroDataBlock = true;
    if(debug){
        log_i("Blocksize = %i", BlockSize);
    }
    return BlockSize;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

bool TFT::GIF_readExtension(char Label){
    char buf[256];
    switch(Label){
        case 0x01 :
            if(debug) {log_i("PlainTextExtension");}
            GIF_readPlainTextExtension(buf);
            break;
        case 0xff :
            if(debug) {log_i("ApplicationExtension");}
            GIF_readApplicationExtension(buf);
            break;
        case 0xfe :
            if(debug) {log_i("CommentExtension");}
            GIF_readCommentExtension(buf);
            break;
        case 0xF9 :
            if(debug) {log_i("GraphicControlExtension");}
            GIF_readGraphicControlExtension();
            break;
        default :   return false;
    }
    return true;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

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

    static char DSBbuffer[300];
    static int32_t curbit, lastbit, done, last_byte;
    int32_t i, j, ret;
    uint8_t count;

    if (flag) {
        curbit = 0;
        lastbit = 0;
        done = false;
        return 0;
    }

    if ((curbit + code_size) >= lastbit) {
        if (done) {
            // log_i("done");
            if (curbit >= lastbit) {
                return 0;
            }
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
        if (count == 0) done = true;

        last_byte = 2 + count;

        curbit = (curbit - lastbit) + 16;

        lastbit = (2 + count) * 8;
    }
    ret = 0;
    for (i = curbit, j = 0; j < code_size; ++i, ++j) ret |= ((DSBbuffer[i / 8] & (1 << (i % 8))) != 0) << j;

    curbit += code_size;

    return ret;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

int32_t TFT::GIF_LZWReadByte(bool init) {
    static int32_t fresh = false;
    int32_t code, incode;
    static int32_t firstcode, oldcode;

    if(gif_next.capacity()  < (1 << gif_MaxLzwBits))       gif_next.reserve(( 1 << gif_MaxLzwBits) -       gif_next.capacity());
    if(gif_vals.capacity()  < (1 << gif_MaxLzwBits))       gif_vals.reserve(( 1 << gif_MaxLzwBits) -       gif_vals.capacity());
    if(gif_stack.capacity() < (1 << (gif_MaxLzwBits + 1))) gif_stack.reserve((1 << (gif_MaxLzwBits + 1)) - gif_stack.capacity());
    gif_next.clear();
    gif_vals.clear();
    gif_stack.clear();

    static uint8_t* sp;

    int32_t i;

    if (init) {
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

        if (debug) {
            log_i("ClearCode=%i", gif_ClearCode);
            log_i("EOIcode=%i", gif_EOIcode);
            log_i("CodeSize=%i", gif_CodeSize);
            log_i("MaxCode=%i", gif_MaxCode);
            log_i("MaxCodeSize=%i", gif_MaxCodeSize);
        }

        fresh = false;

        GIF_GetCode(0, true);

        fresh = true;

        for (i = 0; i < gif_ClearCode; i++) {
            gif_next[i] = 0;
            gif_vals[i] = i;
        }
        for (; i < (1 << gif_MaxLzwBits); i++) gif_next[i] = gif_vals[0] = 0;

        sp = &gif_stack[0];

        return 0;
    } else if (fresh) {
        fresh = false;
        do {
            firstcode = oldcode = GIF_GetCode(gif_CodeSize, false);
        } while (firstcode == gif_ClearCode);

        return firstcode;
    }

    if (sp > &gif_stack[0]) return *--sp;

    while ((code = GIF_GetCode(gif_CodeSize, false)) >= 0) {
        if (code == gif_ClearCode) {
            for (i = 0; i < gif_ClearCode; ++i) {
                gif_next[i] = 0;
                gif_vals[i] = i;
            }
            for (; i < (1 << gif_MaxLzwBits); ++i) gif_next[i] = gif_vals[i] = 0;

            gif_CodeSize = gif_LZWMinimumCodeSize + 1;
            gif_MaxCodeSize = 2 * gif_ClearCode;
            gif_MaxCode = gif_ClearCode + 2;
            sp = &gif_stack[0];

            firstcode = oldcode = GIF_GetCode(gif_CodeSize, false);
            return firstcode;
        } else if (code == gif_EOIcode) {
            if (debug) {
                log_i("read EOI Code");
            }
            int32_t count;
            char buf[260];

            if (gif_ZeroDataBlock) return -2;
            while ((count = GIF_readDataSubBlock(buf)) > 0)
                ;

            if (count != 0) return -2;
        }

        incode = code;

        if (code >= gif_MaxCode) {
            *sp++ = firstcode;
            code = oldcode;
        }

        while (code >= gif_ClearCode) {
            *sp++ = gif_vals[code];
            if (code == (int32_t)gif_next[code]) {
                return -1;
            }
            code = gif_next[code];
        }
        *sp++ = firstcode = gif_vals[code];

        if ((code = gif_MaxCode) < (1 << gif_MaxLzwBits)) {
            gif_next[code] = oldcode;
            gif_vals[code] = firstcode;
            ++gif_MaxCode;
            if ((gif_MaxCode >= gif_MaxCodeSize) && (gif_MaxCodeSize < (1 << gif_MaxLzwBits))) {
                gif_MaxCodeSize *= 2;
                ++gif_CodeSize;
            }
        }
        oldcode = incode;

        if (sp > &gif_stack[0]) return *--sp;
    }
    return code;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

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
            else
                buf[i] = gif_GlobalColorTable[color];
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
//----------------------------------------------------------------------------------------------------------------------------------------------------

int32_t TFT::GIF_readGifItems() {
    GIF_readHeader();
    GIF_readLogicalScreenDescriptor();
    gif_decodeSdFile_firstread=true;
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

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

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TFT::GIF_freeMemory(){
    gif_next.clear(); gif_next.shrink_to_fit();
    gif_vals.clear(); gif_vals.shrink_to_fit();
    gif_stack.clear(); gif_stack.shrink_to_fit();
    gif_GlobalColorTable.clear(); gif_GlobalColorTable.shrink_to_fit();
    gif_LocalColorTable.clear(); gif_LocalColorTable.shrink_to_fit();
}

/***********************************************************************************************************************
                                                J P E G
***********************************************************************************************************************/

bool TFT::drawJpgFile(fs::FS& fs, const char* path, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight,
                         uint16_t offX, uint16_t offY) {
    if ((x + maxWidth) > width() || (y + maxHeight) > height()) {
        log_e("Bad dimensions given");
        return false;
    }
    // log_i("maxWidth=%i, maxHeight=%i", maxWidth, maxHeight);
    if (!fs.exists(path)) {
        log_e("file %s not exists", path);
        return false;
    }

    File file = fs.open(path);
    if (!file) {
        log_e("Failed to open file for reading");
        return false;
    }
    JpegDec.decodeSdFile(file);

    //    log_i("Width: %i, Height: %i,", JpegDec.width, JpegDec.height);
    //    log_i("Components: %i, Scan type %i", JpegDec.comps, JpegDec.scanType);
    //    log_i("MCU / row: %i, MCU / col: %i", JpegDec.MCUSPerRow, JpegDec.MCUSPerCol);
    //    log_i("MCU width: %i, MCU height: %i", JpegDec.MCUWidth, JpegDec.MCUHeight);

    renderJPEG(x, y, maxWidth, maxHeight);

    return true;
}

void TFT::renderJPEG(int32_t xpos, int32_t ypos, uint16_t maxWidth, uint16_t maxHeight) {
    // retrieve infomration about the image
    uint16_t *pImg;
    uint16_t mcu_w = JpegDec.MCUWidth;
    uint16_t mcu_h = JpegDec.MCUHeight;
    uint32_t max_x = JpegDec.width;
    uint32_t max_y = JpegDec.height;

    maxWidth  = (uint)(maxWidth  / 8) * 8;  // must be a multiple of 16 (MCU blocksize)
    //log_i("maxWidth %d", maxWidth );
    maxHeight = (uint)(maxHeight / 8) * 8;   // must be a multiple of 8

    // Important jpg files hat to export as 4:4:4 (subsampling high quality)
    // MCU Blocksizes are:
    // chroma subsampling:  4:4:4 -> 8x8px
    //                      4:2:2 -> 16x8px
    //                      4:2:0 -> 16x16px

    if(maxWidth  > 0 && maxWidth  < max_x) max_x = maxWidth;  // overwrite
    if(maxHeight > 0 && maxHeight < max_y) max_y = maxHeight; // overwrite


    // Jpeg images are draw as a set of image block (tiles) called Minimum Coding Units (MCUs)
    // Typically these MCUs are 16x16 pixel blocks
    // Determine the width and height of the right and bottom edge image blocks
    uint32_t min_w = minimum(mcu_w, max_x % mcu_w);
    uint32_t min_h = minimum(mcu_h, max_y % mcu_h);

    // save the current image block size
    uint32_t win_w = mcu_w;
    uint32_t win_h = mcu_h;

    // record the current time so we can measure how long it takes to draw an image
    uint32_t drawTime = millis();

    // save the coordinate of the right and bottom edges to assist image cropping to the screen size
    max_x += xpos;
    max_y += ypos;

    // read each MCU block until there are no more
    startJpeg();

    while( JpegDec.read()){
        // save a pointer to the image block
        pImg = JpegDec.pImage;

        // calculate where the image block should be drawn on the screen
        int16_t mcu_x = JpegDec.MCUx * mcu_w + xpos;
        int16_t mcu_y = JpegDec.MCUy * mcu_h + ypos;

        // check if the image block size needs to be changed for the right and bottom edges
        if (mcu_x + mcu_w <= max_x) win_w = mcu_w;
        else win_w = min_w;
        if (mcu_y + mcu_h <= max_y) win_h = mcu_h;
        else win_h = min_h;

        // log_i("mcu_x=%i, mcu_y=%i", mcu_x, mcu_y);

        // calculate how many pixels must be drawn
        uint32_t mcu_pixels = win_w * win_h;

        // draw image block if it will fit on the screen
        if ( ( mcu_x + win_w) <= width() && ( mcu_y + win_h) <= height()) {
            // open a window onto the screen to paint the pixels into
            startWrite();
                setAddrWindow(mcu_x, mcu_y, win_w , win_h);
                if     (_TFTcontroller == ILI9341){
                    writeCommand(ILI9341_RAMWR); //ILI9341
                    while (mcu_pixels--) spi_TFT->write16(*pImg++); // Send to TFT 16 bits at a time
                }
                else if(_TFTcontroller == HX8347D){
                    writeCommand(0x22);
                    while (mcu_pixels--) spi_TFT->write16(*pImg++); // Send to TFT 16 bits at a time
                }
                else if(_TFTcontroller == ILI9486a || _TFTcontroller == ILI9486b){
                    writeCommand(ILI9486_RAMWR);
                    while (mcu_pixels--) spi_TFT->write16(*pImg++); // Send to TFT 16 bits at a time
                }
                else if(_TFTcontroller == ILI9488){
                    writeCommand(ILI9488_RAMWR);
                    while (mcu_pixels--) write24BitColor(*pImg++); // Send to TFT 16 bits at a time
                }
                else if(_TFTcontroller == ST7796   || _TFTcontroller == ST7796RPI){
                    writeCommand(ST7796_RAMWR);
                    while (mcu_pixels--) write24BitColor(*pImg++); // Send to TFT 16 bits at a time
                }
            endWrite();
        }

        // stop drawing blocks if the bottom of the screen has been reached
        // the abort function will close the file
        else if ( ( mcu_y + win_h) >= height()) JpegDec.abort();
    }

    // calculate how long it took to draw the image
    drawTime = millis() - drawTime; // Calculate the time it took

    // print the results
    // log_i("Total render time was: %ims",drawTime);;
    endJpeg();
}

/***********************************************************************************************************************
                                            J P E G D E C O D E R
***********************************************************************************************************************/

// JPEGDecoder thx to Bodmer https://github.com/Bodmer/JPEGDecoder
JPEGDecoder::JPEGDecoder(){ // @suppress("Class members should be properly initialized")
    mcu_x = 0 ;
    mcu_y = 0 ;
    is_available = 0;
    thisPtr = this;
    scanType=PJPG_GRAYSCALE;
    gScanType=PJPG_GRAYSCALE;
    g_pNeedBytesCallback=0;
    g_pCallback_data=0;
}

JPEGDecoder::~JPEGDecoder(){
    if (pImage) delete pImage;
    pImage = NULL;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t JPEGDecoder::pjpeg_callback(uint8_t* pBuf, uint8_t buf_size, uint8_t* pBytes_actually_read,
                                    void* pCallback_data) {
    JPEGDecoder* thisPtr = JpegDec.thisPtr;
    thisPtr->pjpeg_need_bytes_callback(pBuf, buf_size, pBytes_actually_read, pCallback_data);
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t JPEGDecoder::pjpeg_need_bytes_callback(uint8_t* pBuf, uint8_t buf_size, uint8_t* pBytes_actually_read,
                                               void* pCallback_data) {
    uint n;
    n = jpg_min(g_nInFileSize - g_nInFileOfs, buf_size);
    if (jpg_source == JPEG_ARRAY) {  // We are handling an array
        for (int32_t i = 0; i < n; i++) {
            pBuf[i] = pgm_read_byte(jpg_data++);
            // Serial.println(pBuf[i],HEX);
        }
    }
    if (jpg_source == JPEG_SD_FILE) g_pInFileSd.read(pBuf, n);  // else we are handling a file
    *pBytes_actually_read = (uint8_t)(n);
    g_nInFileOfs += n;
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

int32_t JPEGDecoder::decode_mcu(void) {
    status = JpegDec.pjpeg_decode_mcu();
    if (status) {
        is_available = 0 ;
        if (status != JpegDec.PJPG_NO_MORE_BLOCKS) {
            log_e("pjpeg_decode_mcu() failed with status %i", status);
            return -1;
        }
    }
    return 1;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

int32_t JPEGDecoder::read(void){
    int32_t y, x;
    uint16_t *pDst_row;
    if(is_available == 0 || mcu_y >= image_info.m_MCUSPerCol) {
        abort();
        return 0;
    }
    // Copy MCU's pixel blocks into the destination bitmap.
    pDst_row = pImage;
    for (y = 0; y < image_info.m_MCUHeight; y += 8) {
        const int32_t by_limit = jpg_min(8, image_info.m_height - (mcu_y * image_info.m_MCUHeight + y));
        for (x = 0; x < image_info.m_MCUWidth; x += 8) {
            uint16_t *pDst_block = pDst_row + x;
            // Compute source byte offset of the block in the decoder's MCU buffer.
            uint src_ofs = (x * 8U) + (y * 16U);
            const uint8_t *pSrcR = image_info.m_pMCUBufR + src_ofs;
            const uint8_t *pSrcG = image_info.m_pMCUBufG + src_ofs;
            const uint8_t *pSrcB = image_info.m_pMCUBufB + src_ofs;
            const int32_t bx_limit = jpg_min(8, image_info.m_width - (mcu_x * image_info.m_MCUWidth + x));
            if (image_info.m_scanType == PJPG_GRAYSCALE) {
                int32_t bx, by;
                for (by = 0; by < by_limit; by++) {
                    uint16_t *pDst = pDst_block;
                    for (bx = 0; bx < bx_limit; bx++) {
                        *pDst++ = (*pSrcR & 0xF8) << 8 | (*pSrcR & 0xFC) <<3 | *pSrcR >> 3;
                        pSrcR++;
                    }
                    pSrcR += (8 - bx_limit);
                    pDst_block += row_pitch;
                }
            }
            else {
                int32_t bx, by;
                for (by = 0; by < by_limit; by++) {
                    uint16_t *pDst = pDst_block;

                    for (bx = 0; bx < bx_limit; bx++) {
                        *pDst++ = (*pSrcR & 0xF8) << 8 | (*pSrcG & 0xFC) <<3 | *pSrcB >> 3;
                        pSrcR++; pSrcG++; pSrcB++;
                    }
                    pSrcR += (8 - bx_limit);
                    pSrcG += (8 - bx_limit);
                    pSrcB += (8 - bx_limit);

                    pDst_block += row_pitch;
                }
            }
        }
        pDst_row += (row_pitch * 8);
    }
    MCUx = mcu_x;
    MCUy = mcu_y;

    mcu_x++;
    if (mcu_x == image_info.m_MCUSPerRow) {
        mcu_x = 0;
        mcu_y++;
    }
    if(decode_mcu()==-1) is_available = 0 ;
    return 1;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

int32_t JPEGDecoder::readSwappedBytes(void) {
    int32_t y, x;
    uint16_t *pDst_row;

    if(is_available == 0 || mcu_y >= image_info.m_MCUSPerCol) {
        abort();
        return 0;
    }
    // Copy MCU's pixel blocks into the destination bitmap.
    pDst_row = pImage;
    for (y = 0; y < image_info.m_MCUHeight; y += 8) {
        const int32_t by_limit = jpg_min(8, image_info.m_height - (mcu_y * image_info.m_MCUHeight + y));

        for (x = 0; x < image_info.m_MCUWidth; x += 8) {
            uint16_t *pDst_block = pDst_row + x;
            // Compute source byte offset of the block in the decoder's MCU buffer.
            uint src_ofs = (x * 8U) + (y * 16U);
            const uint8_t *pSrcR = image_info.m_pMCUBufR + src_ofs;
            const uint8_t *pSrcG = image_info.m_pMCUBufG + src_ofs;
            const uint8_t *pSrcB = image_info.m_pMCUBufB + src_ofs;
            const int32_t bx_limit = jpg_min(8, image_info.m_width - (mcu_x * image_info.m_MCUWidth + x));

            if (image_info.m_scanType == PJPG_GRAYSCALE) {
                int32_t bx, by;
                for (by = 0; by < by_limit; by++) {
                    uint16_t *pDst = pDst_block;
                    for (bx = 0; bx < bx_limit; bx++) {
                        *pDst++ = (*pSrcR & 0xF8) | (*pSrcR & 0xE0) >> 5 | (*pSrcR & 0xF8) << 5 | (*pSrcR & 0x1C) << 11;
                        pSrcR++;
                    }
                }
            }
            else {
                int32_t bx, by;
                for (by = 0; by < by_limit; by++) {
                    uint16_t *pDst = pDst_block;

                    for (bx = 0; bx < bx_limit; bx++) {
                        *pDst++ = (*pSrcR & 0xF8) | (*pSrcG & 0xE0) >> 5 | (*pSrcB & 0xF8) << 5 | (*pSrcG & 0x1C) << 11;
                        pSrcR++; pSrcG++; pSrcB++;
                    }
                    pSrcR += (8 - bx_limit);
                    pSrcG += (8 - bx_limit);
                    pSrcB += (8 - bx_limit);
                    pDst_block += row_pitch;
                }
            }
        }
        pDst_row += (row_pitch * 8);
    }
    MCUx = mcu_x;
    MCUy = mcu_y;

    mcu_x++;
    if (mcu_x == image_info.m_MCUSPerRow) {
        mcu_x = 0;
        mcu_y++;
    }
    if(decode_mcu()==-1) is_available = 0 ;
    return 1;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

int32_t JPEGDecoder::decodeSdFile(File jpgFile) { // This is for the SD library
    g_pInFileSd = jpgFile;
    jpg_source = JPEG_SD_FILE; // Flag to indicate a SD file

    if (!g_pInFileSd) {
        return -1;
    }
    g_nInFileOfs = 0;
    g_nInFileSize = g_pInFileSd.size();

    return decodeCommon();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

int32_t JPEGDecoder::decodeArray(const uint8_t array[], uint32_t  array_size) {
    jpg_source = JPEG_ARRAY; // We are not processing a file, use arrays
    g_nInFileOfs = 0;
    jpg_data = (uint8_t *)array;
    g_nInFileSize = array_size;
    return decodeCommon();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

int32_t JPEGDecoder::decodeCommon(void) {
    status = JpegDec.pjpeg_decode_init(&image_info, pjpeg_callback, NULL, 0);
    if (status) {
        log_e("pjpeg_decode_init() failed with status %i", status);
        if (status == JpegDec.PJPG_UNSUPPORTED_MODE) {
            log_e("Progressive JPEG files are not supported.");
        }
        return -1;
    }
    decoded_width =  image_info.m_width;
    decoded_height =  image_info.m_height;
    row_pitch = image_info.m_MCUWidth;
    pImage = new uint16_t[image_info.m_MCUWidth * image_info.m_MCUHeight];
    if (!pImage) {
        log_e("Memory Allocation Failure");
        return -1;
    }
//  memset(pImage , 0 , sizeof(*pImage));

    row_blocks_per_mcu = image_info.m_MCUWidth >> 3;
    col_blocks_per_mcu = image_info.m_MCUHeight >> 3;
    is_available = 1 ;
    width = decoded_width;
    height = decoded_height;
    comps = 1;
    MCUSPerRow = image_info.m_MCUSPerRow;
    MCUSPerCol = image_info.m_MCUSPerCol;
    scanType = image_info.m_scanType;
    MCUWidth = image_info.m_MCUWidth;
    MCUHeight = image_info.m_MCUHeight;
    return decode_mcu();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void JPEGDecoder::abort(void) {
    mcu_x = 0 ;
    mcu_y = 0 ;
    is_available = 0;
    if(pImage) delete pImage;
    pImage = NULL;
    if (jpg_source == JPEG_SD_FILE) if (g_pInFileSd) g_pInFileSd.close();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// JPEGDecoder picojpeg
int16_t JPEGDecoder::replicateSignBit16(int8_t n){
   switch (n){
      case 0:  return 0x0000;
      case 1:  return 0x8000;
      case 2:  return 0xC000;
      case 3:  return 0xE000;
      case 4:  return 0xF000;
      case 5:  return 0xF800;
      case 6:  return 0xFC00;
      case 7:  return 0xFE00;
      case 8:  return 0xFF00;
      case 9:  return 0xFF80;
      case 10: return 0xFFC0;
      case 11: return 0xFFE0;
      case 12: return 0xFFF0;
      case 13: return 0xFFF8;
      case 14: return 0xFFFC;
      case 15: return 0xFFFE;
      default: return 0xFFFF;
   }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void JPEGDecoder::fillInBuf(void) {
    uint8_t status;
    // Reserve a few bytes at the beginning of the buffer for putting back ("stuffing") chars.
    gInBufOfs = 4;
    gInBufLeft = 0;
    status =
        (*g_pNeedBytesCallback)(gInBuf + gInBufOfs, PJPG_MAX_IN_BUF_SIZE - gInBufOfs, &gInBufLeft, g_pCallback_data);
    if (status) {
        // The user provided need bytes callback has indicated an error, so record the error and continue trying to
        // decode. The highest level pjpeg entrypoints will catch the error and return the non-zero status.
        gCallbackStatus = status;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

uint16_t JPEGDecoder::getBits(uint8_t numBits, uint8_t FFCheck){
   uint8_t origBits = numBits;
   uint16_t ret = gBitBuf;
   if (numBits > 8)   {
      numBits -= 8;
      gBitBuf <<= gBitsLeft;
      gBitBuf |= getOctet(FFCheck);
      gBitBuf <<= (8 - gBitsLeft);
      ret = (ret & 0xFF00) | (gBitBuf >> 8);
   }
   if (gBitsLeft < numBits){
      gBitBuf <<= gBitsLeft;
      gBitBuf |= getOctet(FFCheck);
      gBitBuf <<= (numBits - gBitsLeft);
      gBitsLeft = 8 - (numBits - gBitsLeft);
   }
   else{
      gBitsLeft = (uint8_t)(gBitsLeft - numBits);
      gBitBuf <<= numBits;
   }
   return ret >> (16 - origBits);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

uint16_t JPEGDecoder::getExtendTest(uint8_t i){
   switch (i){
      case 0: return 0;
      case 1: return 0x0001;
      case 2: return 0x0002;
      case 3: return 0x0004;
      case 4: return 0x0008;
      case 5: return 0x0010;
      case 6: return 0x0020;
      case 7: return 0x0040;
      case 8:  return 0x0080;
      case 9:  return 0x0100;
      case 10: return 0x0200;
      case 11: return 0x0400;
      case 12: return 0x0800;
      case 13: return 0x1000;
      case 14: return 0x2000;
      case 15: return 0x4000;
      default: return 0;
   }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
int16_t JPEGDecoder::getExtendOffset(uint8_t i){

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wshift-negative-value"

   switch (i){
      case 0: return 0;
      case 1: return ((-1)<<1) + 1;
      case 2: return ((-1)<<2) + 1;
      case 3: return ((-1)<<3) + 1;
      case 4: return ((-1)<<4) + 1;
      case 5: return ((-1)<<5) + 1;
      case 6: return ((-1)<<6) + 1;
      case 7: return ((-1)<<7) + 1;
      case 8: return ((-1)<<8) + 1;
      case 9: return ((-1)<<9) + 1;
      case 10: return ((-1)<<10) + 1;
      case 11: return ((-1)<<11) + 1;
      case 12: return ((-1)<<12) + 1;
      case 13: return ((-1)<<13) + 1;
      case 14: return ((-1)<<14) + 1;
      case 15: return ((-1)<<15) + 1;
      default: return 0;
   }

   #pragma GCC diagnostic pop
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void JPEGDecoder::huffCreate(const uint8_t* pBits, HuffTable* pHuffTable){
   uint8_t  i = 0, j=0;
   uint16_t code = 0;

   for ( ; ; ){
       uint8_t num = pBits[i];
       if (!num){
           pHuffTable->mMinCode[i] = 0x0000;
           pHuffTable->mMaxCode[i] = 0xFFFF;
           pHuffTable->mValPtr[i] = 0;
       }
       else{
           pHuffTable->mMinCode[i] = code;
           pHuffTable->mMaxCode[i] = code + num - 1;
           pHuffTable->mValPtr[i] = j;
           j = (uint8_t)(j + num);
           code = (uint16_t)(code + num);
      }
      code <<= 1;
      i++;
      if (i > 15) break;
   }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

JPEGDecoder::HuffTable* JPEGDecoder::getHuffTable(uint8_t index){
    // 0-1 = DC
    // 2-3 = AC
    switch (index){
        case 0: return &gHuffTab0;
        case 1: return &gHuffTab1;
        case 2: return &gHuffTab2;
        case 3: return &gHuffTab3;
        default: return 0;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t* JPEGDecoder::getHuffVal(uint8_t index){
    // 0-1 = DC
    // 2-3 = AC
    switch (index){
        case 0: return gHuffVal0;
        case 1: return gHuffVal1;
        case 2: return gHuffVal2;
        case 3: return gHuffVal3;
        default: return 0;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t JPEGDecoder::readDHTMarker(void){
    uint8_t bits[16];
    uint16_t left = getBits1(16);
    if (left < 2) return PJPG_BAD_DHT_MARKER;
    left -= 2;
    while(left){
        uint8_t i, tableIndex, index;
        uint8_t* pHuffVal;
        HuffTable* pHuffTable;
        uint16_t count, totalRead;
        index = (uint8_t)getBits1(8);

        if(((index & 0xF) > 1) || ((index & 0xF0) > 0x10)) return PJPG_BAD_DHT_INDEX;

        tableIndex = ((index >> 3) & 2) + (index & 1);
        pHuffTable = getHuffTable(tableIndex);
        pHuffVal = getHuffVal(tableIndex);
        gValidHuffTables |= (1 << tableIndex);
        count = 0;
        for(i = 0; i <= 15; i++){
            uint8_t n = (uint8_t)getBits1(8);
            bits[i] = n;
            count = (uint16_t)(count + n);
        }
        if(count > getMaxHuffCodes(tableIndex)) return PJPG_BAD_DHT_COUNTS;
        for(i = 0; i < count; i++) pHuffVal[i] = (uint8_t)getBits1(8);
        totalRead = 1 + 16 + count;
        if (left < totalRead) return PJPG_BAD_DHT_MARKER;
        left = (uint16_t)(left - totalRead);
        huffCreate(bits, pHuffTable);
    }
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
//void JPEGDecoder::createWinogradQuant(int16_t* pQuant);
uint8_t JPEGDecoder::readDQTMarker(void){
    uint16_t left = getBits1(16);
    if (left < 2) return PJPG_BAD_DQT_MARKER;
    left -= 2;
    while (left){
        uint8_t i;
        uint8_t n = (uint8_t)getBits1(8);
        uint8_t prec = n >> 4;
        uint16_t totalRead;

        n &= 0x0F;
        if (n > 1) return PJPG_BAD_DQT_TABLE;
        gValidQuantTables |= (n ? 2 : 1);
        // read quantization entries, in zag order
        for (i = 0; i < 64; i++){
            uint16_t temp = getBits1(8);
            if(prec) temp = (temp << 8) + getBits1(8);
            if (n) gQuant1[i] = (int16_t)temp;
        else
            gQuant0[i] = (int16_t)temp;
        }
        createWinogradQuant(n ? gQuant1 : gQuant0);
        totalRead = 64 + 1;
        if(prec) totalRead += 64;
        if(left < totalRead) return PJPG_BAD_DQT_LENGTH;
        left = (uint16_t)(left - totalRead);
    }
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t JPEGDecoder::readSOFMarker(void){
    uint8_t i;
    uint16_t left = getBits1(16);

    if (getBits1(8) != 8) return PJPG_BAD_PRECISION;
    gImageYSize = getBits1(16);
    if((!gImageYSize) || (gImageYSize > PJPG_MAX_HEIGHT)) return PJPG_BAD_HEIGHT;
    gImageXSize = getBits1(16);
    if((!gImageXSize) || (gImageXSize > PJPG_MAX_WIDTH)) return PJPG_BAD_WIDTH;
    gCompsInFrame = (uint8_t)getBits1(8);
    if(gCompsInFrame > 3) return PJPG_TOO_MANY_COMPONENTS;
    if (left != (gCompsInFrame + gCompsInFrame + gCompsInFrame + 8)) return PJPG_BAD_SOF_LENGTH;
    for (i = 0; i < gCompsInFrame; i++){
        gCompIdent[i] = (uint8_t)getBits1(8);
        gCompHSamp[i] = (uint8_t)getBits1(4);
        gCompVSamp[i] = (uint8_t)getBits1(4);
        gCompQuant[i] = (uint8_t)getBits1(8);
        if(gCompQuant[i] > 1) return PJPG_UNSUPPORTED_QUANT_TABLE;
    }
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// Used to skip unrecognized markers.
uint8_t JPEGDecoder::skipVariableMarker(void){
    uint16_t left = getBits1(16);
    if (left < 2) return PJPG_BAD_VARIABLE_MARKER;
    left -= 2;
    while(left){getBits1(8); left--;}
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// Read a define restart interval (DRI) marker.
uint8_t JPEGDecoder::readDRIMarker(void){
    if (getBits1(16) != 4) return PJPG_BAD_DRI_LENGTH;
    gRestartInterval = getBits1(16);
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// Read a start of scan (SOS) marker.
uint8_t JPEGDecoder::readSOSMarker(void){
    uint8_t i;
    uint16_t left = getBits1(16);
    uint8_t successive_high __attribute__((unused));
    uint8_t successive_low  __attribute__((unused));
    uint8_t spectral_end    __attribute__((unused));
    uint8_t spectral_start  __attribute__((unused));

    gCompsInScan = (uint8_t)getBits1(8);
    left -= 3;
    if((left!=(gCompsInScan+gCompsInScan+3))||(gCompsInScan<1)||(gCompsInScan>PJPG_MAXCOMPSINSCAN)) return PJPG_BAD_SOS_LENGTH;
    for (i = 0; i < gCompsInScan; i++){
        uint8_t cc = (uint8_t)getBits1(8);
        uint8_t c  = (uint8_t)getBits1(8);
        uint8_t ci;
        left -= 2;
        for(ci = 0; ci < gCompsInFrame; ci++) if(cc == gCompIdent[ci]) break;
        if(ci >= gCompsInFrame) return PJPG_BAD_SOS_COMP_ID;
        gCompList[i]   = ci;
        gCompDCTab[ci] = (c >> 4) & 15;
        gCompACTab[ci] = (c & 15);
    }
    spectral_start  = (uint8_t)getBits1(8);
    spectral_end    = (uint8_t)getBits1(8);
    successive_high = (uint8_t)getBits1(4);
    successive_low  = (uint8_t)getBits1(4);
    left -= 3;
    while(left){getBits1(8); left--;}
    return 0;

}
//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t JPEGDecoder::nextMarker(void){
   uint8_t c;
   uint8_t bytes = 0;

   do{
       do{
           bytes++;
           c = (uint8_t)getBits1(8);
       } while (c != 0xFF);
       do{
           c = (uint8_t)getBits1(8);
       } while (c == 0xFF);
   } while (c == 0);

   // If bytes > 0 here, there where extra bytes before the marker (not good).
   return c;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// Process markers. Returns when an SOFx, SOI, EOI, or SOS marker is
// encountered.
uint8_t JPEGDecoder::processMarkers(uint8_t* pMarker){
    for( ; ; ){
        uint8_t c = nextMarker();

        switch (c){
         case M_SOF0:
         case M_SOF1:
         case M_SOF2:
         case M_SOF3:
         case M_SOF5:
         case M_SOF6:
         case M_SOF7:
         // case M_JPG:
         case M_SOF9:
         case M_SOF10:
         case M_SOF11:
         case M_SOF13:
         case M_SOF14:
         case M_SOF15:
         case M_SOI:
         case M_EOI:
         case M_SOS:{*pMarker = c; return 0;}
         case M_DHT:{readDHTMarker(); break;}
         // Sorry, no arithmetic support at this time. Dumb patents!
         case M_DAC:{return PJPG_NO_ARITHMITIC_SUPPORT;}
         case M_DQT:{readDQTMarker(); break;}
         case M_DRI:{readDRIMarker(); break;}
         //case M_APP0:  /* no need to read the JFIF marker */
         case M_JPG:
         case M_RST0:    /* no parameters */
         case M_RST1:
         case M_RST2:
         case M_RST3:
         case M_RST4:
         case M_RST5:
         case M_RST6:
         case M_RST7:
         case M_TEM:{return PJPG_UNEXPECTED_MARKER;}
         default:    /* must be DNL, DHP, EXP, APPn, JPGn, COM, or RESn or APP0 */
         {
            skipVariableMarker();
            break;
         }
      }
   }
   return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// Finds the start of image (SOI) marker.
uint8_t JPEGDecoder::locateSOIMarker(void) {
    uint16_t bytesleft;
    uint8_t lastchar = (uint8_t)getBits1(8);
    uint8_t thischar = (uint8_t)getBits1(8);
    /* ok if it's a normal JPEG file without a special header */
    if ((lastchar == 0xFF) && (thischar == M_SOI)) return 0;
    bytesleft = 4096;  // 512;
    for (;;) {
        if (--bytesleft == 0) return PJPG_NOT_JPEG;
        lastchar = thischar;
        thischar = (uint8_t)getBits1(8);
        if (lastchar == 0xFF) {
            if (thischar == M_SOI)
                break;
            else if (thischar == M_EOI)
                return PJPG_NOT_JPEG;  // getBits1 will keep returning M_EOI if we read past the end
        }
    }
    /* Check the next character after marker: if it's not 0xFF, it can't
    be the start of the next marker, so the file is bad */
    thischar = (uint8_t)((gBitBuf >> 8) & 0xFF);
    if (thischar != 0xFF) return PJPG_NOT_JPEG;
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// Find a start of frame (SOF) marker.
uint8_t JPEGDecoder::locateSOFMarker(void){
    uint8_t c;
    uint8_t status = locateSOIMarker();
    if(status) return status;
    status = processMarkers(&c);
    if(status) return status;
    switch(c){
       case M_SOF2:{
          // Progressive JPEG - not supported by JPEGDecoder (would require too
          // much memory, or too many IDCT's for embedded systems).
          return PJPG_UNSUPPORTED_MODE;
       }
       case M_SOF0:{  /* baseline DCT */
          status = readSOFMarker();
          if (status) return status;
          break;
       }
       case M_SOF9:{return PJPG_NO_ARITHMITIC_SUPPORT;}
       case M_SOF1:  /* extended sequential DCT */
       default:{
           return PJPG_UNSUPPORTED_MARKER;
       }
    }
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// Find a start of scan (SOS) marker.
uint8_t JPEGDecoder::locateSOSMarker(uint8_t* pFoundEOI){
    uint8_t c;
    uint8_t status;

    *pFoundEOI = 0;
    status = processMarkers(&c);
    if(status) return status;
    if(c == M_EOI){
        *pFoundEOI = 1;
        return 0;
    }
    else if (c != M_SOS) return PJPG_UNEXPECTED_MARKER;
    return readSOSMarker();
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t JPEGDecoder::init(void){
    gImageXSize = 0;
    gImageYSize = 0;
    gCompsInFrame = 0;
    gRestartInterval = 0;
    gCompsInScan = 0;
    gValidHuffTables = 0;
    gValidQuantTables = 0;
    gTemFlag = 0;
    gInBufOfs = 0;
    gInBufLeft = 0;
    gBitBuf = 0;
    gBitsLeft = 8;

    getBits1(8);
    getBits1(8);

    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// This method throws back into the stream any bytes that where read
// into the bit buffer during initial marker scanning.
void JPEGDecoder::fixInBuffer(void){
    /* In case any 0xFF's where pulled into the buffer during marker scanning */
    if(gBitsLeft > 0) stuffChar((uint8_t)gBitBuf);
    stuffChar((uint8_t)(gBitBuf >> 8));
    gBitsLeft = 8;
    getBits2(8);
    getBits2(8);
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// Restart interval processing.
uint8_t JPEGDecoder::processRestart(void){
    // Let's scan a little bit to find the marker, but not _too_ far.
    // 1536 is a "fudge factor" that determines how much to scan.
    uint16_t i;
    uint8_t c = 0;
    for(i = 1536; i > 0; i--) if (getChar() == 0xFF) break;
    if(i==0) return PJPG_BAD_RESTART_MARKER;
    for( ; i > 0; i--) if ((c = getChar()) != 0xFF) break;
    if(i==0) return PJPG_BAD_RESTART_MARKER;
    // Is it the expected marker? If not, something bad happened.
    if(c != (gNextRestartNum + M_RST0)) return PJPG_BAD_RESTART_MARKER;
    // Reset each component's DC prediction values.
    gLastDC[0] = 0;
    gLastDC[1] = 0;
    gLastDC[2] = 0;
    gRestartsLeft = gRestartInterval;
    gNextRestartNum = (gNextRestartNum + 1) & 7;
    // Get the bit buffer going again
    gBitsLeft = 8;
    getBits2(8);
    getBits2(8);
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// FIXME: findEOI() is not actually called at the end of the image
// (it's optional, and probably not needed on embedded devices)
uint8_t JPEGDecoder::findEOI(void){
    uint8_t c;
    uint8_t status;
    // Prime the bit buffer
    gBitsLeft = 8;
    getBits1(8);
    getBits1(8);

    // The next marker _should_ be EOI
    status = processMarkers(&c);
    if(status) return status;
    else if(gCallbackStatus) return gCallbackStatus;
    //gTotalBytesRead -= in_buf_left;
    if(c!=M_EOI) return PJPG_UNEXPECTED_MARKER;
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t JPEGDecoder::checkHuffTables(void){
    uint8_t i;
    for(i = 0; i < gCompsInScan; i++){
        uint8_t compDCTab = gCompDCTab[gCompList[i]];
        uint8_t compACTab = gCompACTab[gCompList[i]] + 2;
        if(((gValidHuffTables & (1 << compDCTab)) == 0)||((gValidHuffTables & (1 << compACTab)) == 0))
            return PJPG_UNDEFINED_HUFF_TABLE;
    }
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t JPEGDecoder::checkQuantTables(void){
    uint8_t i;
    for(i=0; i<gCompsInScan; i++){
      uint8_t compQuantMask = gCompQuant[gCompList[i]] ? 2 : 1;
      if((gValidQuantTables & compQuantMask)==0) return PJPG_UNDEFINED_QUANT_TABLE;
    }
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t JPEGDecoder::initScan(void){
    uint8_t foundEOI;
    uint8_t status = locateSOSMarker(&foundEOI);
    if(status) return status;
    if(foundEOI) return PJPG_UNEXPECTED_MARKER;
    status=checkHuffTables();
    if(status) return status;
    status=checkQuantTables();
    if(status) return status;
    gLastDC[0] = 0;
    gLastDC[1] = 0;
    gLastDC[2] = 0;
    if(gRestartInterval){
        gRestartsLeft = gRestartInterval;
        gNextRestartNum = 0;
    }
    fixInBuffer();
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t JPEGDecoder::initFrame(void){
    if (gCompsInFrame==1){
        if((gCompHSamp[0]!=1)||(gCompVSamp[0]!=1)) return PJPG_UNSUPPORTED_SAMP_FACTORS;
        gScanType=PJPG_GRAYSCALE;
        gMaxBlocksPerMCU = 1;
        gMCUOrg[0] = 0;
        gMaxMCUXSize     = 8;
        gMaxMCUYSize     = 8;
    }
    else if(gCompsInFrame==3){
        if(((gCompHSamp[1]!=1)||(gCompVSamp[1]!=1))||((gCompHSamp[2]!=1)||(gCompVSamp[2]!=1)))
            return PJPG_UNSUPPORTED_SAMP_FACTORS;
        if((gCompHSamp[0]==1)&&(gCompVSamp[0]==1)){
            gScanType = PJPG_YH1V1;
            gMaxBlocksPerMCU = 3;
            gMCUOrg[0] = 0;
            gMCUOrg[1] = 1;
            gMCUOrg[2] = 2;
            gMaxMCUXSize = 8;
            gMaxMCUYSize = 8;
        }
        else if((gCompHSamp[0]==1)&&(gCompVSamp[0]==2)){
            gScanType = PJPG_YH1V2;
            gMaxBlocksPerMCU = 4;
            gMCUOrg[0] = 0;
            gMCUOrg[1] = 0;
            gMCUOrg[2] = 1;
            gMCUOrg[3] = 2;
            gMaxMCUXSize = 8;
            gMaxMCUYSize = 16;
        }
        else if((gCompHSamp[0]==2)&&(gCompVSamp[0]==1)){
            gScanType = PJPG_YH2V1;
            gMaxBlocksPerMCU = 4;
            gMCUOrg[0] = 0;
            gMCUOrg[1] = 0;
            gMCUOrg[2] = 1;
            gMCUOrg[3] = 2;
            gMaxMCUXSize = 16;
            gMaxMCUYSize = 8;
        }
        else if((gCompHSamp[0]==2)&&(gCompVSamp[0]==2)){
            gScanType = PJPG_YH2V2;
            gMaxBlocksPerMCU = 6;
            gMCUOrg[0] = 0;
            gMCUOrg[1] = 0;
            gMCUOrg[2] = 0;
            gMCUOrg[3] = 0;
            gMCUOrg[4] = 1;
            gMCUOrg[5] = 2;
            gMaxMCUXSize = 16;
            gMaxMCUYSize = 16;
        }
        else return PJPG_UNSUPPORTED_SAMP_FACTORS;
    }
    else return PJPG_UNSUPPORTED_COLORSPACE;
    gMaxMCUSPerRow = (gImageXSize + (gMaxMCUXSize - 1)) >> ((gMaxMCUXSize == 8) ? 3 : 4);
    gMaxMCUSPerCol = (gImageYSize + (gMaxMCUYSize - 1)) >> ((gMaxMCUYSize == 8) ? 3 : 4);
    gNumMCUSRemaining = gMaxMCUSPerRow * gMaxMCUSPerCol;
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

// Winograd IDCT: 5 multiplies per row/col, up to 80 muls for the 2D IDCT
#define PJPG_DCT_SCALE (1U << PJPG_DCT_SCALE_BITS)
//#define PJPG_DESCALE(x) PJPG_ARITH_SHIFT_RIGHT_N_16(((x) + (1U << (PJPG_DCT_SCALE_BITS - 1))), PJPG_DCT_SCALE_BITS)
#define PJPG_WFIX(x) ((x) * PJPG_DCT_SCALE + 0.5f)
//----------------------------------------------------------------------------------------------------------------------------------------------------
// Multiply quantization matrix by the Winograd IDCT scale factors
void JPEGDecoder::createWinogradQuant(int16_t* pQuant){
    uint8_t i;
    for(i=0; i<64; i++){
        long x = pQuant[i];
        x *= gWinogradQuant[i];
        pQuant[i]=(int16_t)((x+(1<<(PJPG_WINOGRAD_QUANT_SCALE_BITS-PJPG_DCT_SCALE_BITS-1)))>>(PJPG_WINOGRAD_QUANT_SCALE_BITS-PJPG_DCT_SCALE_BITS));
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void JPEGDecoder::idctRows(void){
    uint8_t i;
    int16_t* pSrc = gCoeffBuf;
    for(i=0; i<8; i++){
        if((pSrc[1] | pSrc[2] | pSrc[3] | pSrc[4] | pSrc[5] | pSrc[6] | pSrc[7]) == 0){
            // Short circuit the 1D IDCT if only the DC component is non-zero
            int16_t src0 = *pSrc;
            *(pSrc+1) = src0;
            *(pSrc+2) = src0;
            *(pSrc+3) = src0;
            *(pSrc+4) = src0;
            *(pSrc+5) = src0;
            *(pSrc+6) = src0;
            *(pSrc+7) = src0;
        }
        else{
            int16_t src4 = *(pSrc+5);
            int16_t src7 = *(pSrc+3);
            int16_t x4  = src4 - src7;
            int16_t x7  = src4 + src7;
            int16_t src5 = *(pSrc+1);
            int16_t src6 = *(pSrc+7);
            int16_t x5  = src5 + src6;
            int16_t x6  = src5 - src6;
            int16_t tmp1 = imul_b5(x4 - x6);
            int16_t stg26 = imul_b4(x6) - tmp1;
            int16_t x24 = tmp1 - imul_b2(x4);
            int16_t x15 = x5 - x7;
            int16_t x17 = x5 + x7;
            int16_t tmp2 = stg26 - x17;
            int16_t tmp3 = imul_b1_b3(x15) - tmp2;
            int16_t x44 = tmp3 + x24;
            int16_t src0 = *(pSrc+0);
            int16_t src1 = *(pSrc+4);
            int16_t x30 = src0 + src1;
            int16_t x31 = src0 - src1;
            int16_t src2 = *(pSrc+2);
            int16_t src3 = *(pSrc+6);
            int16_t x12 = src2 - src3;
            int16_t x13 = src2 + src3;
            int16_t x32 = imul_b1_b3(x12) - x13;
            int16_t x40 = x30 + x13;
            int16_t x43 = x30 - x13;
            int16_t x41 = x31 + x32;
            int16_t x42 = x31 - x32;

            *(pSrc+0) = x40 + x17;
            *(pSrc+1) = x41 + tmp2;
            *(pSrc+2) = x42 + tmp3;
            *(pSrc+3) = x43 - x44;
            *(pSrc+4) = x43 + x44;
            *(pSrc+5) = x42 - tmp3;
            *(pSrc+6) = x41 - tmp2;
            *(pSrc+7) = x40 - x17;
        }
        pSrc += 8;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void JPEGDecoder::idctCols(void){
    uint8_t i;
    int16_t* pSrc = gCoeffBuf;
    for(i=0; i<8; i++){
        if((pSrc[1*8] | pSrc[2*8] | pSrc[3*8] | pSrc[4*8] | pSrc[5*8] | pSrc[6*8] | pSrc[7*8]) == 0){
            // Short circuit the 1D IDCT if only the DC component is non-zero
            uint8_t c = clamp(PJPG_DESCALE(*pSrc) + 128);
            *(pSrc+0*8) = c;
            *(pSrc+1*8) = c;
            *(pSrc+2*8) = c;
            *(pSrc+3*8) = c;
            *(pSrc+4*8) = c;
            *(pSrc+5*8) = c;
            *(pSrc+6*8) = c;
            *(pSrc+7*8) = c;
        }
        else{
            int16_t src4 = *(pSrc+5*8);
            int16_t src7 = *(pSrc+3*8);
            int16_t x4  = src4 - src7;
            int16_t x7  = src4 + src7;
            int16_t src5 = *(pSrc+1*8);
            int16_t src6 = *(pSrc+7*8);
            int16_t x5  = src5 + src6;
            int16_t x6  = src5 - src6;
            int16_t tmp1 = imul_b5(x4 - x6);
            int16_t stg26 = imul_b4(x6) - tmp1;
            int16_t x24 = tmp1 - imul_b2(x4);
            int16_t x15 = x5 - x7;
            int16_t x17 = x5 + x7;
            int16_t tmp2 = stg26 - x17;
            int16_t tmp3 = imul_b1_b3(x15) - tmp2;
            int16_t x44 = tmp3 + x24;
            int16_t src0 = *(pSrc+0*8);
            int16_t src1 = *(pSrc+4*8);
            int16_t x30 = src0 + src1;
            int16_t x31 = src0 - src1;
            int16_t src2 = *(pSrc+2*8);
            int16_t src3 = *(pSrc+6*8);
            int16_t x12 = src2 - src3;
            int16_t x13 = src2 + src3;
            int16_t x32 = imul_b1_b3(x12) - x13;
            int16_t x40 = x30 + x13;
            int16_t x43 = x30 - x13;
            int16_t x41 = x31 + x32;
            int16_t x42 = x31 - x32;
            // descale, convert to unsigned and clamp to 8-bit
            *(pSrc+0*8) = clamp(PJPG_DESCALE(x40 + x17)  + 128);
            *(pSrc+1*8) = clamp(PJPG_DESCALE(x41 + tmp2) + 128);
            *(pSrc+2*8) = clamp(PJPG_DESCALE(x42 + tmp3) + 128);
            *(pSrc+3*8) = clamp(PJPG_DESCALE(x43 - x44)  + 128);
            *(pSrc+4*8) = clamp(PJPG_DESCALE(x43 + x44)  + 128);
            *(pSrc+5*8) = clamp(PJPG_DESCALE(x42 - tmp3) + 128);
            *(pSrc+6*8) = clamp(PJPG_DESCALE(x41 - tmp2) + 128);
            *(pSrc+7*8) = clamp(PJPG_DESCALE(x40 - x17)  + 128);
        }
        pSrc++;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void JPEGDecoder::upsampleCb(uint8_t srcOfs, uint8_t dstOfs){   // Cb upsample and accumulate, 4x4 to 8x8
    // Cb - affects G and B
    uint8_t x, y;
    int16_t* pSrc = gCoeffBuf + srcOfs;
    uint8_t* pDstG = gMCUBufG + dstOfs;
    uint8_t* pDstB = gMCUBufB + dstOfs;
    for(y=0; y<4; y++){
        for(x=0; x<4; x++){
            uint8_t cb = (uint8_t)*pSrc++;
            int16_t cbG, cbB;
            cbG = ((cb * 88U) >> 8U) - 44U;
            pDstG[0] = subAndClamp(pDstG[0], cbG);
            pDstG[1] = subAndClamp(pDstG[1], cbG);
            pDstG[8] = subAndClamp(pDstG[8], cbG);
            pDstG[9] = subAndClamp(pDstG[9], cbG);
            cbB = (cb + ((cb * 198U) >> 8U)) - 227U;
            pDstB[0] = addAndClamp(pDstB[0], cbB);
            pDstB[1] = addAndClamp(pDstB[1], cbB);
            pDstB[8] = addAndClamp(pDstB[8], cbB);
            pDstB[9] = addAndClamp(pDstB[9], cbB);
            pDstG += 2;
            pDstB += 2;
        }
        pSrc = pSrc - 4 + 8;
        pDstG = pDstG - 8 + 16;
        pDstB = pDstB - 8 + 16;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void JPEGDecoder::upsampleCbH(uint8_t srcOfs, uint8_t dstOfs){  // Cb upsample and accumulate, 4x8 to 8x8
    // Cb - affects G and B
    uint8_t x, y;
    int16_t* pSrc = gCoeffBuf + srcOfs;
    uint8_t* pDstG = gMCUBufG + dstOfs;
    uint8_t* pDstB = gMCUBufB + dstOfs;
    for(y=0; y<8; y++){
        for(x=0; x<4; x++){
            uint8_t cb = (uint8_t)*pSrc++;
            int16_t cbG, cbB;
            cbG = ((cb * 88U) >> 8U) - 44U;
            pDstG[0] = subAndClamp(pDstG[0], cbG);
            pDstG[1] = subAndClamp(pDstG[1], cbG);
            cbB = (cb + ((cb * 198U) >> 8U)) - 227U;
            pDstB[0] = addAndClamp(pDstB[0], cbB);
            pDstB[1] = addAndClamp(pDstB[1], cbB);
            pDstG += 2;
            pDstB += 2;
        }
        pSrc = pSrc - 4 + 8;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void JPEGDecoder::upsampleCbV(uint8_t srcOfs, uint8_t dstOfs){  // Cb upsample and accumulate, 8x4 to 8x8
    // Cb - affects G and B
    uint8_t x, y;
    int16_t* pSrc = gCoeffBuf + srcOfs;
    uint8_t* pDstG = gMCUBufG + dstOfs;
    uint8_t* pDstB = gMCUBufB + dstOfs;
    for(y=0; y<4; y++){
        for(x=0; x<8; x++){
            uint8_t cb = (uint8_t)*pSrc++;
            int16_t cbG, cbB;
            cbG = ((cb * 88U) >> 8U) - 44U;
            pDstG[0] = subAndClamp(pDstG[0], cbG);
            pDstG[8] = subAndClamp(pDstG[8], cbG);
            cbB = (cb + ((cb * 198U) >> 8U)) - 227U;
            pDstB[0] = addAndClamp(pDstB[0], cbB);
            pDstB[8] = addAndClamp(pDstB[8], cbB);
            ++pDstG;
            ++pDstB;
        }
        pDstG = pDstG - 8 + 16;
        pDstB = pDstB - 8 + 16;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void JPEGDecoder::upsampleCr(uint8_t srcOfs, uint8_t dstOfs){   // Cr upsample and accumulate, 4x4 to 8x8
    // Cr - affects R and G
    uint8_t x, y;
    int16_t* pSrc = gCoeffBuf + srcOfs;
    uint8_t* pDstR = gMCUBufR + dstOfs;
    uint8_t* pDstG = gMCUBufG + dstOfs;
    for(y=0; y<4; y++){
        for(x=0; x<4; x++){
            uint8_t cr = (uint8_t)*pSrc++;
            int16_t crR, crG;
            crR = (cr + ((cr * 103U) >> 8U)) - 179;
            pDstR[0] = addAndClamp(pDstR[0], crR);
            pDstR[1] = addAndClamp(pDstR[1], crR);
            pDstR[8] = addAndClamp(pDstR[8], crR);
            pDstR[9] = addAndClamp(pDstR[9], crR);
            crG = ((cr * 183U) >> 8U) - 91;
            pDstG[0] = subAndClamp(pDstG[0], crG);
            pDstG[1] = subAndClamp(pDstG[1], crG);
            pDstG[8] = subAndClamp(pDstG[8], crG);
            pDstG[9] = subAndClamp(pDstG[9], crG);
            pDstR += 2;
            pDstG += 2;
        }
        pSrc = pSrc - 4 + 8;
        pDstR = pDstR - 8 + 16;
        pDstG = pDstG - 8 + 16;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void JPEGDecoder::upsampleCrH(uint8_t srcOfs, uint8_t dstOfs){  // Cr upsample and accumulate, 4x8 to 8x8
    // Cr - affects R and G
    uint8_t x, y;
    int16_t* pSrc = gCoeffBuf + srcOfs;
    uint8_t* pDstR = gMCUBufR + dstOfs;
    uint8_t* pDstG = gMCUBufG + dstOfs;
    for(y=0; y<8; y++){
        for(x=0; x<4; x++){
            uint8_t cr = (uint8_t)*pSrc++;
            int16_t crR, crG;
            crR = (cr + ((cr * 103U) >> 8U)) - 179;
            pDstR[0] = addAndClamp(pDstR[0], crR);
            pDstR[1] = addAndClamp(pDstR[1], crR);
            crG = ((cr * 183U) >> 8U) - 91;
            pDstG[0] = subAndClamp(pDstG[0], crG);
            pDstG[1] = subAndClamp(pDstG[1], crG);
            pDstR += 2;
            pDstG += 2;
        }
        pSrc = pSrc - 4 + 8;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void JPEGDecoder::upsampleCrV(uint8_t srcOfs, uint8_t dstOfs){  // Cr upsample and accumulate, 8x4 to 8x8
    // Cr - affects R and G
    uint8_t x, y;
    int16_t* pSrc = gCoeffBuf + srcOfs;
    uint8_t* pDstR = gMCUBufR + dstOfs;
    uint8_t* pDstG = gMCUBufG + dstOfs;
    for(y=0; y<4; y++){
        for(x=0; x<8; x++){
            uint8_t cr = (uint8_t)*pSrc++;
            int16_t crR, crG;
            crR = (cr + ((cr * 103U) >> 8U)) - 179;
            pDstR[0] = addAndClamp(pDstR[0], crR);
            pDstR[8] = addAndClamp(pDstR[8], crR);
            crG = ((cr * 183U) >> 8U) - 91;
            pDstG[0] = subAndClamp(pDstG[0], crG);
            pDstG[8] = subAndClamp(pDstG[8], crG);
            ++pDstR;
            ++pDstG;
        }
        pDstR = pDstR - 8 + 16;
        pDstG = pDstG - 8 + 16;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void JPEGDecoder::copyY(uint8_t dstOfs){    // Convert Y to RGB
    uint8_t i;
    uint8_t* pRDst = gMCUBufR + dstOfs;
    uint8_t* pGDst = gMCUBufG + dstOfs;
    uint8_t* pBDst = gMCUBufB + dstOfs;
    int16_t* pSrc = gCoeffBuf;
    for(i=64; i>0; i--){
        uint8_t c = (uint8_t)*pSrc++;
        *pRDst++ = c;
        *pGDst++ = c;
        *pBDst++ = c;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void JPEGDecoder::convertCb(uint8_t dstOfs){    // Cb convert to RGB and accumulate
    uint8_t i;
    uint8_t* pDstG = gMCUBufG + dstOfs;
    uint8_t* pDstB = gMCUBufB + dstOfs;
    int16_t* pSrc = gCoeffBuf;
    for(i=64; i>0; i--){
        uint8_t cb = (uint8_t)*pSrc++;
        int16_t cbG, cbB;
        cbG = ((cb * 88U) >> 8U) - 44U;
        pDstG[0] = subAndClamp(pDstG[0], cbG);
        cbB = (cb + ((cb * 198U) >> 8U)) - 227U;
        pDstB[0] = addAndClamp(pDstB[0], cbB);
        ++pDstG;
        ++pDstB;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void JPEGDecoder::convertCr(uint8_t dstOfs){    // Cr convert to RGB and accumulate
    uint8_t i;
    uint8_t* pDstR = gMCUBufR + dstOfs;
    uint8_t* pDstG = gMCUBufG + dstOfs;
    int16_t* pSrc = gCoeffBuf;
    for(i=64; i>0; i--){
        uint8_t cr = (uint8_t)*pSrc++;
        int16_t crR, crG;
        crR = (cr + ((cr * 103U) >> 8U)) - 179;
        pDstR[0] = addAndClamp(pDstR[0], crR);
        crG = ((cr * 183U) >> 8U) - 91;
        pDstG[0] = subAndClamp(pDstG[0], crG);
        ++pDstR;
        ++pDstG;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void JPEGDecoder::transformBlock(uint8_t mcuBlock){
    idctRows();
    idctCols();
    switch (gScanType){
        case PJPG_GRAYSCALE:{copyY(0); break;}  // MCU size: 1, 1 block per MCU
        case PJPG_YH1V1:{                       // MCU size: 8x8, 3 blocks per MCU
                switch(mcuBlock){
                    case 0:{copyY(0); break;}
                    case 1:{convertCb(0); break;}
                    case 2:{convertCr(0); break;}
                } break;
        }
        case PJPG_YH1V2:{                       // MCU size: 8x16, 4 blocks per MCU
                switch(mcuBlock){
                    case 0:{copyY(0);   break;}
                    case 1:{copyY(128); break;}
                    case 2:{upsampleCbV(0, 0); upsampleCbV(4*8, 128); break;}
                    case 3:{upsampleCrV(0, 0); upsampleCrV(4*8, 128); break;}
                } break;
        }
        case PJPG_YH2V1:{                       // MCU size: 16x8, 4 blocks per MCU
                switch(mcuBlock){
                    case 0:{copyY(0);   break;}
                    case 1:{copyY(64);  break;}
                    case 2:{upsampleCbH(0, 0); upsampleCbH(4, 64); break;}
                    case 3:{upsampleCrH(0, 0); upsampleCrH(4, 64); break;}
                } break;
        }
        case PJPG_YH2V2:{                       // MCU size: 16x16, 6 blocks per MCU
                switch (mcuBlock){
                    case 0:{copyY(0);   break;}
                    case 1:{copyY(64);  break;}
                    case 2:{copyY(128); break;}
                    case 3:{copyY(192); break;}
                    case 4:{upsampleCb(0, 0); upsampleCb(4, 64); upsampleCb(4*8, 128); upsampleCb(4+4*8, 192); break;}
                    case 5:{upsampleCr(0, 0); upsampleCr(4, 64); upsampleCr(4*8, 128); upsampleCr(4+4*8, 192); break;}
                } break;
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void JPEGDecoder::transformBlockReduce(uint8_t mcuBlock){
    uint8_t c = clamp(PJPG_DESCALE(gCoeffBuf[0]) + 128);
    int16_t cbG, cbB, crR, crG;
    switch(gScanType){
        case PJPG_GRAYSCALE:{gMCUBufR[0] = c; break;}   // MCU size: 1, 1 block per MCU
        case PJPG_YH1V1:{                               // MCU size: 8x8, 3 blocks per MCU
            switch(mcuBlock){
                case 0:{gMCUBufR[0] = c; gMCUBufG[0] = c; gMCUBufB[0] = c; break;}
                case 1:{cbG = ((c * 88U) >> 8U) - 44U;
                        gMCUBufG[0] = subAndClamp(gMCUBufG[0], cbG);
                        cbB = (c + ((c * 198U) >> 8U)) - 227U;
                        gMCUBufB[0] = addAndClamp(gMCUBufB[0], cbB);
                        break;}
                case 2:{crR = (c + ((c * 103U) >> 8U)) - 179;
                        gMCUBufR[0] = addAndClamp(gMCUBufR[0], crR);
                        crG = ((c * 183U) >> 8U) - 91;
                        gMCUBufG[0] = subAndClamp(gMCUBufG[0], crG);
                        break;}
            } break;
        }
        case PJPG_YH1V2:{                               // MCU size: 8x16, 4 blocks per MCU
            switch(mcuBlock){
                case 0:{gMCUBufR[0] = c; gMCUBufG[0] = c; gMCUBufB[0] = c; break;}
                case 1:{gMCUBufR[128] = c; gMCUBufG[128] = c; gMCUBufB[128] = c; break;}
                case 2:{cbG = ((c * 88U) >> 8U) - 44U;
                        gMCUBufG[0] = subAndClamp(gMCUBufG[0], cbG);
                        gMCUBufG[128] = subAndClamp(gMCUBufG[128], cbG);
                        cbB = (c + ((c * 198U) >> 8U)) - 227U;
                        gMCUBufB[0] = addAndClamp(gMCUBufB[0], cbB);
                        gMCUBufB[128] = addAndClamp(gMCUBufB[128], cbB);
                        break;}
                case 3:{crR = (c + ((c * 103U) >> 8U)) - 179;
                        gMCUBufR[0] = addAndClamp(gMCUBufR[0], crR);
                        gMCUBufR[128] = addAndClamp(gMCUBufR[128], crR);
                        crG = ((c * 183U) >> 8U) - 91;
                        gMCUBufG[0] = subAndClamp(gMCUBufG[0], crG);
                        gMCUBufG[128] = subAndClamp(gMCUBufG[128], crG);
                        break;}
            } break;
        }
        case PJPG_YH2V1:{                               // MCU size: 16x8, 4 blocks per MCU
            switch(mcuBlock){
                case 0:{gMCUBufR[0] = c; gMCUBufG[0] = c; gMCUBufB[0] = c; break;}
                case 1:{gMCUBufR[64] = c; gMCUBufG[64] = c; gMCUBufB[64] = c; break;}
                case 2:{cbG = ((c * 88U) >> 8U) - 44U;
                        gMCUBufG[0] = subAndClamp(gMCUBufG[0], cbG);
                        gMCUBufG[64] = subAndClamp(gMCUBufG[64], cbG);
                        cbB = (c + ((c * 198U) >> 8U)) - 227U;
                        gMCUBufB[0] = addAndClamp(gMCUBufB[0], cbB);
                        gMCUBufB[64] = addAndClamp(gMCUBufB[64], cbB);
                        break;}
                case 3:{crR = (c + ((c * 103U) >> 8U)) - 179;
                        gMCUBufR[0] = addAndClamp(gMCUBufR[0], crR);
                        gMCUBufR[64] = addAndClamp(gMCUBufR[64], crR);
                        crG = ((c * 183U) >> 8U) - 91;
                        gMCUBufG[0] = subAndClamp(gMCUBufG[0], crG);
                        gMCUBufG[64] = subAndClamp(gMCUBufG[64], crG);
                        break;}
            } break;
        }
        case PJPG_YH2V2:{                               // MCU size: 16x16, 6 blocks per MCU
            switch(mcuBlock){
                case 0:{gMCUBufR[0] = c; gMCUBufG[0] = c; gMCUBufB[0] = c; break;}
                case 1:{gMCUBufR[64] = c; gMCUBufG[64] = c; gMCUBufB[64] = c; break;}
                case 2:{gMCUBufR[128] = c; gMCUBufG[128] = c; gMCUBufB[128] = c; break;}
                case 3:{gMCUBufR[192] = c; gMCUBufG[192] = c; gMCUBufB[192] = c; break;}
                case 4:{cbG = ((c * 88U) >> 8U) - 44U;
                        gMCUBufG[0] = subAndClamp(gMCUBufG[0], cbG);
                        gMCUBufG[64] = subAndClamp(gMCUBufG[64], cbG);
                        gMCUBufG[128] = subAndClamp(gMCUBufG[128], cbG);
                        gMCUBufG[192] = subAndClamp(gMCUBufG[192], cbG);
                        cbB = (c + ((c * 198U) >> 8U)) - 227U;
                        gMCUBufB[0] = addAndClamp(gMCUBufB[0], cbB);
                        gMCUBufB[64] = addAndClamp(gMCUBufB[64], cbB);
                        gMCUBufB[128] = addAndClamp(gMCUBufB[128], cbB);
                        gMCUBufB[192] = addAndClamp(gMCUBufB[192], cbB);
                        break;}
                case 5:{crR = (c + ((c * 103U) >> 8U)) - 179;
                        gMCUBufR[0] = addAndClamp(gMCUBufR[0], crR);
                        gMCUBufR[64] = addAndClamp(gMCUBufR[64], crR);
                        gMCUBufR[128] = addAndClamp(gMCUBufR[128], crR);
                        gMCUBufR[192] = addAndClamp(gMCUBufR[192], crR);
                        crG = ((c * 183U) >> 8U) - 91;
                        gMCUBufG[0] = subAndClamp(gMCUBufG[0], crG);
                        gMCUBufG[64] = subAndClamp(gMCUBufG[64], crG);
                        gMCUBufG[128] = subAndClamp(gMCUBufG[128], crG);
                        gMCUBufG[192] = subAndClamp(gMCUBufG[192], crG);
                        break;}
            } break;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t JPEGDecoder::huffDecode(const HuffTable* pHuffTable, const uint8_t* pHuffVal) {
    uint8_t i = 0;
    uint8_t j;
    uint16_t code = getBit();

    // This func only reads a bit at a time, which on modern CPU's is not terribly efficient.
    // But on microcontrollers without strong integer shifting support this seems like a
    // more reasonable approach.
    for (;;) {
        uint16_t maxCode;
        if (i == 16) return 0;

        maxCode = pHuffTable->mMaxCode[i];
        if ((code <= maxCode) && (maxCode != 0xFFFF)) break;

        i++;
        code <<= 1;
        code |= getBit();
    }

    j = pHuffTable->mValPtr[i];
    j = (uint8_t)(j + (code - pHuffTable->mMinCode[i]));
    return pHuffVal[j];
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t JPEGDecoder::decodeNextMCU(void){
    uint8_t status;
    uint8_t mcuBlock;
    if(gRestartInterval){
        if(gRestartsLeft == 0){
            status = processRestart();
            if(status) return status;
        }
        gRestartsLeft--;
    }
    for(mcuBlock = 0; mcuBlock < gMaxBlocksPerMCU; mcuBlock++){
        uint8_t componentID = gMCUOrg[mcuBlock];
        uint8_t compQuant = gCompQuant[componentID];
        uint8_t compDCTab = gCompDCTab[componentID];
        uint8_t numExtraBits, compACTab, k;
        const int16_t* pQ = compQuant ? gQuant1 : gQuant0;
        uint16_t r, dc;
        uint8_t s = huffDecode(compDCTab ? &gHuffTab1 : &gHuffTab0, compDCTab ? gHuffVal1 : gHuffVal0);
        r = 0;
        numExtraBits = s & 0xF;
        if(numExtraBits) r = getBits2(numExtraBits);
        dc = huffExtend(r, s);
        dc = dc + gLastDC[componentID];
        gLastDC[componentID] = dc;
        gCoeffBuf[0] = dc * pQ[0];
        compACTab = gCompACTab[componentID];
        if(gReduce){    // Decode, but throw out the AC coefficients in reduce mode.
            for(k=1; k<64; k++){
                s=huffDecode(compACTab ? &gHuffTab3 : &gHuffTab2, compACTab ? gHuffVal3 : gHuffVal2);
                numExtraBits = s & 0xF;
                if(numExtraBits) getBits2(numExtraBits);
                r = s >> 4;
                s &= 15;
                if(s){
                    if(r){
                        if((k+r)>63) return PJPG_DECODE_ERROR;
                        k=(uint8_t)(k + r);
                    }
                }
                else{
                    if(r==15){
                        if((k+16)>64) return PJPG_DECODE_ERROR;
                        k+=(16-1); // - 1 because the loop counter is k
                    }
                    else break;
                }
            }
            transformBlockReduce(mcuBlock);
        }
        else{   // Decode and dequantize AC coefficients
            for(k=1; k<64; k++){
                uint16_t extraBits;
                s=huffDecode(compACTab ? &gHuffTab3 : &gHuffTab2, compACTab ? gHuffVal3 : gHuffVal2);
                extraBits = 0;
                numExtraBits = s & 0xF;
                if(numExtraBits) extraBits=getBits2(numExtraBits);
                r=s>>4;
                s&=15;
                if(s){
                    int16_t ac;
                    if(r){
                        if((k+r)>63) return PJPG_DECODE_ERROR;
                        while(r){
                            gCoeffBuf[ZAG[k++]] = 0;
                            r--;
                        }
                    }
                    ac=huffExtend(extraBits, s);
                    gCoeffBuf[ZAG[k]] = ac * pQ[k];
                }
                else{
                    if(r==15){
                        if((k+16)>64) return PJPG_DECODE_ERROR;
                        for(r=16; r>0; r--) gCoeffBuf[ZAG[k++]] = 0;
                        k--; // - 1 because the loop counter is k
                    }
                    else break;
                }
            }
            while(k<64) gCoeffBuf[ZAG[k++]] = 0;
            transformBlock(mcuBlock);
        }
    }
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
uint8_t JPEGDecoder::pjpeg_decode_mcu() {
    uint8_t status;
    if (gCallbackStatus) return gCallbackStatus;
    if (!gNumMCUSRemaining) return PJPG_NO_MORE_BLOCKS;
    status = decodeNextMCU();
    if ((status) || (gCallbackStatus)) return gCallbackStatus ? gCallbackStatus : status;
    gNumMCUSRemaining--;
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

uint8_t JPEGDecoder::pjpeg_decode_init(pjpeg_image_info_t* pInfo, pjpeg_need_bytes_callback_t pNeed_bytes_callback,
                                       void* pCallback_data, uint8_t reduce) {
    uint8_t status;
    pInfo->m_width = 0;
    pInfo->m_height = 0;
    pInfo->m_comps = 0;
    pInfo->m_MCUSPerRow = 0;
    pInfo->m_MCUSPerCol = 0;
    pInfo->m_scanType = PJPG_GRAYSCALE;
    pInfo->m_MCUWidth = 0;
    pInfo->m_MCUHeight = 0;
    pInfo->m_pMCUBufR = (unsigned char*)0;
    pInfo->m_pMCUBufG = (unsigned char*)0;
    pInfo->m_pMCUBufB = (unsigned char*)0;
    g_pNeedBytesCallback = pNeed_bytes_callback;
    g_pCallback_data = pCallback_data;
    gCallbackStatus = 0;
    gReduce = reduce;
    status = init();
    if ((status) || (gCallbackStatus)) return gCallbackStatus ? gCallbackStatus : status;
    status = locateSOFMarker();
    if ((status) || (gCallbackStatus)) return gCallbackStatus ? gCallbackStatus : status;
    status = initFrame();
    if ((status) || (gCallbackStatus)) return gCallbackStatus ? gCallbackStatus : status;
    status = initScan();
    if ((status) || (gCallbackStatus)) return gCallbackStatus ? gCallbackStatus : status;
    pInfo->m_width = gImageXSize;
    pInfo->m_height = gImageYSize;
    pInfo->m_comps = gCompsInFrame;
    pInfo->m_scanType = gScanType;
    pInfo->m_MCUSPerRow = gMaxMCUSPerRow;
    pInfo->m_MCUSPerCol = gMaxMCUSPerCol;
    pInfo->m_MCUWidth = gMaxMCUXSize;
    pInfo->m_MCUHeight = gMaxMCUYSize;
    pInfo->m_pMCUBufR = gMCUBufR;
    pInfo->m_pMCUBufG = gMCUBufG;
    pInfo->m_pMCUBufB = gMCUBufB;
    return 0;
}

/***********************************************************************************************************************
                                            T O U C H P A D
***********************************************************************************************************************/

// Code für Touchpad mit XPT2046
TP::TP(uint8_t CS, uint8_t IRQ) {
    // log_i("TP init CS = %i, IRQ = %i", CS, IRQ);
    _TP_CS = CS;
    _TP_IRQ = IRQ;
    pinMode(_TP_CS, OUTPUT);
    digitalWrite(_TP_CS, HIGH);
    pinMode(_TP_IRQ, INPUT);
    TP_SPI = SPISettings(200000, MSBFIRST, SPI_MODE0);  // slower speed
    _rotation = 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

uint16_t TP::TP_Send(uint8_t set_val) {
    uint16_t get_val;
    SPItransfer->beginTransaction(TP_SPI);  // Prevent other SPI users
        digitalWrite(_TP_CS, 0);
        SPItransfer->write(set_val);
        get_val = SPItransfer->transfer16(0);
        digitalWrite(_TP_CS, 1);
    SPItransfer->endTransaction();  // Allow other SPI users
    return get_val >> 4;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void TP::loop() {
    static uint16_t x1 = 0, y1 = 0;
    static uint16_t x2 = 0, y2 = 0;
    if (!digitalRead(_TP_IRQ)) {
        if(!read_TP(x, y)){return;}

        if(x !=x1 && y != y1){
            if(tp_positionXY) tp_positionXY(x, y);
        }

        { x1 = x; y1 = y;}

        if (f_loop) {
            f_loop = false;
            // log_i("tp_pressed x=%d, y=%d", x, y);
            if (tp_pressed) tp_pressed(x, y);
            x2 = x; y2 = y;
            m_pressingTime = millis();
            m_f_isPressing = true;
        }
        else {
            if(m_f_isPressing){
                if(m_pressingTime + 2500 < millis()){
                    m_f_isPressing = false;
                    if(tp_long_pressed) tp_long_pressed(x2, y2);
                    m_f_longPressed = true;
                }
            }
        }
    } else {
        if (f_loop == false) {
            // log_i("tp_released");
            if(m_f_longPressed){
                m_f_longPressed = false;
                if (tp_long_released) tp_long_released();
            }
            else{
                if (tp_released) tp_released(x1, y1);
            }
            f_loop = true;
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

void TP::setRotation(uint8_t m){
    _rotation=m;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
void TP::setMirror(bool h, bool v){
    m_mirror_h = h;
    m_mirror_v = v;
}

void TP::setVersion(uint8_t v) {

    switch(v){
        case 0:  TP_vers = TP_ILI9341_0; break;
        case 1:  TP_vers = TP_ILI9341_1; break;
        case 2:  TP_vers = TP_HX8347D_0; break;
        case 3:  TP_vers = TP_ILI9486_0; break;
        case 4:  TP_vers = TP_ILI9488_0; break;
        case 5:  TP_vers = TP_ST7796_0;  break;
        default: TP_vers = TP_ILI9341_0; break;
    }

    if (TP_vers == TP_ILI9341_0) {  // ILI9341 display
        Xmax = 1913;     // Values Calibration
        Xmin = 150;
        Ymax = 1944;
        Ymin = 220;
        xFaktor = float(Xmax - Xmin) / ILI9341_WIDTH;
        yFaktor = float(Ymax - Ymin) / ILI9341_HEIGHT;
    }
    if (TP_vers == TP_ILI9341_1) {  // ILI9341 display for RaspberryPI  #70
        Xmax = 1940;
        Xmin = 90;
        Ymax = 1864;
        Ymin = 105;
        xFaktor = float(Xmax - Xmin) / ILI9341_WIDTH;
        yFaktor = float(Ymax - Ymin) / ILI9341_HEIGHT;
    }
    if (TP_vers == TP_HX8347D_0) {  // Waveshare HX8347D display
        Xmax = 1850;
        Xmin = 170;
        Ymax = 1880;
        Ymin = 140;
        xFaktor = float(Xmax - Xmin) / HX8347D_WIDTH;
        yFaktor = float(Ymax - Ymin) / HX8347D_HEIGHT;
    }
    if (TP_vers == TP_ILI9486_0) {  // ILI9486 display for RaspberryPI
        Xmax = 1922;
        Xmin = 140;
        Ymax = 1930;
        Ymin = 125;
        xFaktor = float(Xmax - Xmin) / ILI9486_WIDTH;
        yFaktor = float(Ymax - Ymin) / ILI9486_HEIGHT;
    }
    if (TP_vers == TP_ILI9488_0) {  // ILI9488 display
        Xmax = 1922;
        Xmin = 140;
        Ymax = 1930;
        Ymin = 125;
        xFaktor = float(Xmax - Xmin) / ILI9488_WIDTH;
        yFaktor = float(Ymax - Ymin) / ILI9488_HEIGHT;
    }
    if (TP_vers == TP_ST7796_0) {  // ST7796 4" display
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
    uint8_t i = 0;

    if(digitalRead(_TP_IRQ)) return false; // TP pressed?

    for (i = 0; i < 100; i++) {
        _x += TP_Send(0xD0);  // x
        _y += TP_Send(0x90);  // y
    }

    if(digitalRead(_TP_IRQ)) return false; // TP must remain pressed as long as the measurement is running

    _x /= 100;
    _y /= 100;

    // log_w("_x %i, _y %i", _x, _y);

    if ((_x < Xmin) || (_x > Xmax)) return false;  // outside the display
    _x = Xmax - _x;
    _x /= xFaktor;

    if ((_y < Ymin) || (y > Ymax)) return false;  // outside the display
    _y = Ymax - _y;
    _y /= yFaktor;

    if(m_mirror_h){
        switch(TP_vers){
            case TP_ILI9341_0: // ILI9341
                    _y = ILI9341_HEIGHT - _y;
                    break;
            case TP_ILI9341_1:
                    _y = ILI9341_HEIGHT - _y;
                    break;
            case TP_HX8347D_0:
                    _y = HX8347D_HEIGHT - _y;
                    break;
            case TP_ILI9486_0:
                    _y = ILI9486_HEIGHT - _y;
                    break;
            case TP_ILI9488_0:
                    _y = ILI9488_HEIGHT - _y;
                    break;
            case TP_ST7796_0:
                    _y = ST7796_HEIGHT - _y;
            default:
                break;
        }
    }

    if(m_mirror_v){
        switch(TP_vers){
            case TP_ILI9341_0: // ILI9341
                    _x = ILI9341_WIDTH - _x;
                    break;
            case TP_ILI9341_1:
                    _x = ILI9341_WIDTH - _x;
                    break;
            case TP_HX8347D_0:
                    _x = HX8347D_WIDTH - _x;
                    break;
            case TP_ILI9486_0:
                    _x = ILI9486_WIDTH - _x;
                    break;
            case TP_ILI9488_0:
                    _x = ILI9488_WIDTH - _x;
                    break;
            case TP_ST7796_0:
                    _x = ST7796_WIDTH - _x;
                    break;
            default:
                break;
        }
    }

    // log_i("_x %i, _y %i", _x, _y);
    x = _x;
    y = _y;

    //-------------------------------------------------------------
    if (TP_vers == TP_ILI9341_0) {  // 320px x 240px
        if (_rotation == 0) {
            y = ILI9341_HEIGHT - y;
        }
        if (_rotation == 1) {
            tmpxy = x;
            x = y;
            y = tmpxy;
            y = ILI9341_WIDTH - y;
            x = ILI9341_HEIGHT - x;
        }
        if (_rotation == 2) {
            x = ILI9341_WIDTH - x;
        }
        if (_rotation == 3) {
            tmpxy = x;
            x = y;
            y = tmpxy;
        }
    }
    //-------------------------------------------------------------
    if (TP_vers == TP_ILI9341_1) {  // 320px x 240px
        if (_rotation == 0) {
            y = ILI9341_HEIGHT - y;
            x = ILI9341_WIDTH - x;
        }
        if (_rotation == 1) {
            tmpxy = x;
            x = y;
            y = tmpxy;
            x = ILI9341_HEIGHT - x;
        }
        if (_rotation == 2) {
            ;
        }
        if (_rotation == 3) {
            tmpxy = x;
            x = y;
            y = tmpxy;
            y = ILI9341_WIDTH - y;
        }
    }
    //-------------------------------------------------------------
    if (TP_vers == TP_HX8347D_0) {  // 320px x 240px
        if (_rotation == 0) {
            ;  // do nothing
        }
        if (_rotation == 1) {
            tmpxy = x;
            x = y;
            y = HX8347D_WIDTH - tmpxy;
            if (x > HX8347D_HEIGHT - 1) x = 0;
            if (y > HX8347D_WIDTH - 1) y = 0;
        }
        if (_rotation == 2) {
            x = HX8347D_WIDTH - x;
            y = HX8347D_HEIGHT - y;
            if (x > HX8347D_WIDTH - 1) x = 0;
            if (y > HX8347D_HEIGHT - 1) y = 0;
        }
        if (_rotation == TP_ILI9486_0) {
            tmpxy = y;
            y = x;
            x = HX8347D_HEIGHT - tmpxy;
            if (x > HX8347D_HEIGHT - 1) x = 0;
            if (y > HX8347D_WIDTH - 1) y = 0;
        }
    }
    //-------------------------------------------------------------
    if (TP_vers == TP_ILI9486_0) {  // 480px x 320px
        if (_rotation == 0) {
            ;  // do nothing
        }
        if (_rotation == 1) {
            tmpxy = x;
            x = y;
            y = ILI9486_WIDTH - tmpxy;
            if (x > ILI9486_HEIGHT - 1) x = 0;
            if (y > ILI9486_WIDTH - 1) y = 0;
        }
        if (_rotation == 2) {
            x = ILI9486_WIDTH - x;
            y = ILI9486_HEIGHT - y;
            if (x > ILI9486_WIDTH - 1) x = 0;
            if (y > ILI9486_HEIGHT - 1) y = 0;
        }
        if (_rotation == 3) {
            tmpxy = y;
            y = x;
            x = ILI9486_HEIGHT - tmpxy;
            if (x > ILI9486_HEIGHT - 1) x = 0;
            if (y > ILI9486_WIDTH - 1) y = 0;
        }
    }
    //-------------------------------------------------------------
    if (TP_vers == TP_ILI9488_0) {  // ILI 9488 Display V1.0, 480px x 320px
        if (_rotation == 0) {
            x = ILI9488_WIDTH - x;
        }
        if (_rotation == 1) {  // landscape
            tmpxy = x;
            x = y;
            y = tmpxy;
            if (x > ILI9488_HEIGHT - 1) x = 0;
            if (y > ILI9488_WIDTH - 1) y = 0;
        }
        if (_rotation == 2) {  // portrait + 180 degree
            y = ILI9488_HEIGHT - y;
            if (x > ILI9488_WIDTH - 1) x = 0;
            if (y > ILI9488_HEIGHT - 1) y = 0;
        }
        if (_rotation == 3) {  // landscape + 180 degree
            tmpxy = x;
            x = ILI9488_HEIGHT - y;
            y = ILI9488_WIDTH - tmpxy;
            if (x > ILI9488_HEIGHT - 1) x = 0;
            if (y > ILI9488_WIDTH - 1) y = 0;
        }
    }
    //-------------------------------------------------------------
    if (TP_vers == TP_ST7796_0) {  // ST7796 Display V1.1, 480px x 320px
        if (_rotation == 0) {
            x = ST7796_WIDTH - x;
        }
        if (_rotation == 1) {  // landscape
            tmpxy = x;
            x = y;
            y = tmpxy;
            if (x > ST7796_HEIGHT - 1) x = 0;
            if (y > ST7796_WIDTH - 1) y = 0;
        }
        if (_rotation == 2) {  // portrait + 180 degree
            y = ILI9488_HEIGHT - y;
            if (x > ST7796_WIDTH - 1) x = 0;
            if (y > ST7796_HEIGHT - 1) y = 0;
        }
        if (_rotation == 3) {  // landscape + 180 degree
            tmpxy = x;
            x = ST7796_HEIGHT - y;
            y = ST7796_WIDTH - tmpxy;
            if (x > ST7796_HEIGHT - 1) x = 0;
            if (y > ST7796_WIDTH - 1) y = 0;
        }
    }
//    log_i("TP_vers %d, Rotation %d, X = %i, Y = %i",TP_vers, _rotation, x, y);
    return true;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------