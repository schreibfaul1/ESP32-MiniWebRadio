
/*
*  FTP SERVER FOR ESP8266
 * based on FTP Serveur for Arduino Due and Ethernet shield (W5100) or WIZ820io (W5200)
 * based on Jean-Michel Gallego's work
 * modified to work with esp8266 SPIFFS by David Paiva (david@nailbuster.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
//  2017: modified by @robo8080
//  2019: modified by @HenrikSte
//  2023: modified by schreibfaul1

/*******************************************************************************
 **                                                                            **
 **                       DEFINITIONS FOR FTP SERVER                           **
 **                                                                            **
 *******************************************************************************/

// Uncomment to print debugging info to console attached to ESP8266
//#define FTP_DEBUG

#ifndef FTP_SERVERESP_H
#define FTP_SERVERESP_H


#include "Arduino.h"
#include "SPI.h"
#include <WiFi.h>
#include <WiFiClient.h>

#include "SD.h"
#include "SD_MMC.h"
#include "FS.h"

#define FTP_SERVER_VERSION "FTP-2020-12-12"

#define FTP_CTRL_PORT    21             // Command port on which server is listening
#define FTP_DATA_PORT_PASV 50009        // Data port in passive mode

#define FTP_TIME_OUT     5              // Disconnect client after 5 minutes of inactivity
#define FTP_CMD_SIZE     255 + 8        // max size of a command
#define FTP_CWD_SIZE     255 + 8        // max size of a directory name
#define FTP_FIL_SIZE     255            // max size of a file name
#define FTP_BUF_SIZE     4096           // size of file buffer for read/write

extern __attribute__((weak)) void ftp_debug(const char*);


class FtpServer {

private:


public:
    FtpServer();
    ~FtpServer();
    void    begin(String uname, String pword);
    void    begin(fs::FS &fs, String uname, String pword);
    uint8_t isConnected();
    int32_t  handleFTP();

private:
    void iniVariables();
    void clientConnected();
    void disconnectClient();
    boolean userIdentity();
    boolean userPassword();
    boolean processCommand();
    boolean dataConnect();
    boolean doRetrieve();
    boolean doStore();
    void closeTransfer();
    void abortTransfer();
    boolean makePath(char *fullname);
    boolean makePath(char *fullName, char *param);
    uint8_t getDateTime(uint16_t *pyear, uint8_t *pmonth, uint8_t *pday, uint8_t *phour, uint8_t *pminute, uint8_t *second);
    char* makeDateTimeStr(char *tstr, uint16_t date, uint16_t time);
    int16_t readChar();

    IPAddress dataIp;              // IP address of client for data
    WiFiClient client;
    WiFiClient data;
    File file;
    fs::FS *_fs;

    boolean  dataPassiveConn = false;;
    uint16_t dataPort = 0;
    char     chbuf[280];                    // stores debug infos
    char     *buf = NULL;
    char     *cmdLine = NULL;               // where to store incoming char from client
    char     *cwdName = NULL;               // name of current directory
    char     command[5];                    // command sent by client
    boolean  rnfrCmd = false;               // previous command was RNFR
    char     *parameters = NULL;            // point to begin of parameters sent by client
    uint16_t iCL = 0;                       // pointer to cmdLine next incoming char
    int8_t   cmdStatus = 0;                 // status of ftp command connection
    int8_t   transferStatus = 0;            // status of ftp data transfer
    uint32_t millisTimeOut = 0;             // disconnect after 5 min of inactivity
    uint32_t millisDelay = 0;
    uint32_t millisEndConnection = 0;       //
    uint32_t millisBeginTrans = 0;          // store time of beginning of a transaction
    uint32_t bytesTransfered = 0;           //
    String   _FTP_USER = "";
    String   _FTP_PASS = "";

};

#endif // FTP_SERVERESP_H
