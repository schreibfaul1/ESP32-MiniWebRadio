/*
 * html.h
 *
 *  Created on: 09.07.2017
 *      Author: Wolle
 */

#ifndef HTML_H_
#define HTML_H_

#include "Arduino.h"
#include "WiFi.h"
#include "SD.h"
#include "FS.h"

extern __attribute__((weak)) void HTML_info(const char*) ;
extern __attribute__((weak)) void HTML_command(const String);
extern __attribute__((weak)) void HTML_file(const String);
extern __attribute__((weak)) void HTML_request(const String);

class HTML
{
private:
	WiFiClient      cmdclient ;                               // An instance of the client for commands
	WiFiServer		cmdserver;

	bool            http_reponse_flag = false ;               // Response required
	String          http_rqfile ;                             // Requested file
	String          http_getcmd ;                             // Contents of last GET command
	char 			sbuf[256];
	String 			_Name;
	String			_Version;
	String 			contenttype;
    uint8_t 		buf[1024]; 		                          // Inputbuffer


protected:
	String   httpheader (String contentstype);
	String   getContentType(String filename);
	void     handlehttp();
	uint8_t inbyte();
	String   URLencode(const char* str);

public:
	HTML(String Name="HTML library", String Version="1.0");
	void begin();
	void stop();
	void loop();
	void show(const char* pagename, int16_t len=-1);
	void show_not_found();
	boolean streamfile(fs::FS &fs,const char* path);
	void reply(const String &response, boolean header=true);
	const char* ISO88591toUTF8(const char* str);
	const char* ASCIItoUTF8(const char* str);
	String  printhttpheader(String file);
};


#endif /* HTML_H_ */
