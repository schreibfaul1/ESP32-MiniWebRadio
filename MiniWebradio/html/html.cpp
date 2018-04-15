/*
 * html.cpp
 *
 *  Created on: 09.07.2017
 *      Author: Wolle
 */

#include "html.h"

HTML::HTML(String Name, String Version){
	_Name=Name; _Version=Version;
}

void HTML::show_not_found()
{
	cmdclient.println("HTTP/1.1 404 Not Found");
	cmdclient.println("");
	return;
}

void HTML::show(const char* pagename, int16_t len)
{
	uint TCPCHUNKSIZE = 1024;  // Max number of bytes per write
	int l=0;                  // Size of requested page
	const unsigned char* p;
	p = reinterpret_cast<const unsigned char*>(pagename);

	if (len == -1)
	{
		l = strlen(pagename);
	}
	else
		if(len>0) l = len;

	if (*p == '\n')           // If page starts with newline:
	{
		p++;                  // Skip first character
		l--;
	}
	sprintf(sbuf, "Length of page is %d\n", l);
	if (HTML_info)	HTML_info(sbuf);
	// The content of the HTTP response follows the header:
	if (l < 10)
	{
		cmdclient.println("Testline<br>");
	}
	else
	{
		while (l)                                // Loop through the output page
		{
			if (l <= TCPCHUNKSIZE)                        // Near the end?
			{
				cmdclient.write(p, l);                    // Yes, send last part
				l = 0;
			}
			else
			{
				cmdclient.write(p, TCPCHUNKSIZE);       // Send part of the page
				p += TCPCHUNKSIZE;        // Update startpoint and rest of bytes
				l -= TCPCHUNKSIZE;
			}
		}
	}
	// The HTTP response ends with another blank line:
	cmdclient.println();
	if (HTML_info)	HTML_info("Response send\n");
}

void HTML::streamfile(fs::FS &fs,const char* path){
    size_t bytesPerTransaction = 1024;
    uint8_t transBuf[bytesPerTransaction];
    size_t wIndex = 0;
	File file = fs.open(path);
	if(!file){log_e("Failed to open file for reading"); return;}
	sprintf(sbuf, "Length of file %s is %d\n", path, file.size());
	if (HTML_info)	HTML_info(sbuf);
	while(wIndex < file.size()){
		file.read(transBuf, bytesPerTransaction);
		cmdclient.write(transBuf, bytesPerTransaction);
		wIndex+=bytesPerTransaction;
	}
	file.read(transBuf,file.size()-wIndex);
	cmdclient.write(transBuf, file.size()-wIndex);
	file.close();
}


String HTML::printhttpheader(String file){
	String  ct ;                           // Content type
	ct = getContentType(file);
	if ( ( ct == "" ) || ( file == "" ) )             // Empty is illegal
	{
	    cmdclient.println ( "HTTP/1.1 404 Not Found" ) ;
	    cmdclient.println ( "" ) ;
	    return "";
	}
	cmdclient.print ( httpheader ( ct ) ) ;             // Send header
	return ct;
}


String HTML::httpheader(String contenttype) {
	String s1 = "HTTP/1.1 200 OK\nContent-type:" + contenttype + "\n";
	String s2 = "Server: " + _Name+ "\n";
	String s3 = "Cache-Control: ";
	String s4 = "max-age=3600\n";
	String s5 = "Last-Modified: " + _Version + "\n\n";
	return String(s1 + s2 + s3 + s4 + s5);
}

void HTML::begin() {
	cmdserver.begin();
}
void HTML::stop() {
	cmdserver.stop();
}

String HTML::getContentType ( String filename )
{
	if      ( filename.endsWith ( ".html" ) ) return "text/html" ;
	else if ( filename.endsWith ( ".png"  ) ) return "image/png" ;
	else if ( filename.endsWith ( ".gif"  ) ) return "image/gif" ;
	else if ( filename.endsWith ( ".jpg"  ) ) return "image/jpeg" ;
	else if ( filename.endsWith ( ".ico"  ) ) return "image/x-icon" ;
	else if ( filename.endsWith ( ".css"  ) ) return "text/css" ;
	else if ( filename.endsWith ( ".zip"  ) ) return "application/x-zip" ;
	else if ( filename.endsWith ( ".gz"   ) ) return "application/x-gzip" ;
	else if ( filename.endsWith ( ".mp3"  ) ) return "audio/mpeg" ;
	else if ( filename.endsWith ( ".pw"   ) ) return "" ;              // Passwords are secret
	return "text/plain" ;
}

void HTML::handlehttp() {
	bool wswitch=true;
	bool first = true;						// First call to rinbyt()
	char c;                                 // Next character from http input
	uint16_t inx0, inx1, inx2, inx3; 		// Pos. of search string in currenLine
	String currentLine = "";                // Build up to complete line
	String ct;								// contenttype

	if (!cmdclient.connected())				// Action if client is connected
	{
		return;								// No client active
	}
	while (wswitch==true)					// first while
	{
		c = inbyte(first);					// Get a byte
		first = false;						// No more first call
		if (c == '\n') {
			// If the current line is blank, you got two newline characters in a row.
			// that's the end of the client HTTP request, so send a response:
			if (currentLine.length() == 0) {
				wswitch=false; // use second while
				break;
			} else {
//				log_e("recieved %s", currentLine.c_str());
				// Newline seen
				inx0 = 0;
				if (currentLine.startsWith("GET /")) inx0 = 5;  // GET request?
				if (currentLine.startsWith("POST /"))inx0 = 6;  // POST request?

				if(inx0>0){
//					currentLine=currentLine.substring(inx0, currentLine.length()); //remove GET or POST
				    inx1 = currentLine.indexOf("?");	// Search for 1st parameter
					inx2 = currentLine.indexOf("&");	// Search for 2nd parameter
					inx3 = currentLine.indexOf(" HTTP");// Search for 3th parameter
					if((inx1>0) && (inx2>inx1)){  		// it is a command
						http_getcmd = currentLine.substring(inx1+1, inx2);//isolate the command
				    	http_rqfile = "";				// No file
					}
					else{								// it is a filename
						http_rqfile = currentLine.substring(inx0, inx3);
						http_getcmd = "";
					}
					if (http_getcmd.length()) {
						sprintf(sbuf, "%s\n", http_getcmd.c_str());
						if (HTML_info)HTML_info(sbuf);
						if (HTML_command) HTML_command((URLencode(http_getcmd.c_str())));
					}
					if (http_rqfile.length()) {
						sprintf(sbuf, "Filename is: %s\n", http_rqfile.c_str());
						if (HTML_info) HTML_info(sbuf);
						if (HTML_file) HTML_file(http_rqfile);
					}
					if(http_rqfile.length() == 0 && http_getcmd.length() == 0 ){   // An empty "GET"?
						sprintf(sbuf, "Filename is: %s\n", "index.html");
						if (HTML_info) HTML_info(sbuf);
						if (HTML_file) HTML_file("index.html");
					}
				}
				currentLine = "";
			}
		} else if (c != '\r'){                        // No LINFEED.  Is it a CR?
			currentLine += c;              // No, add normal char to currentLine
		}
	} //end while 1
	while(wswitch==false){					 		// second while
		c = inbyte(false);					// Get a byte
		if (c == '\n') {
			if (currentLine.length() == 0){
				wswitch=true;  // use first while
				//log_i("end request");
				break;
			}
			else {   // its the requestbody
//				log_e("requestbody %s", currentLine.c_str());
				if(currentLine[0]<32)currentLine=String();
				if (HTML_request) HTML_request(currentLine);
				currentLine += c;
				currentLine.trim();
				if (HTML_info) HTML_info(currentLine.c_str());
				currentLine = "";
			}
		}
		else if (c != '\r')currentLine += c;   // No LINFEED.  Is it a CR?
	} // end while 2
}

uint8_t HTML::inbyte(bool forcestart) {
	static uint16_t i;                                 	// Pointer in inputbuffer
	static uint16_t len;                               	// Number of bytes in buf
	uint16_t tlen;                                		// Number of available bytes
	uint16_t trycount = 0;                        		// Limit max. time to read

	if (forcestart || (i == len))                    	// Time to read new buffer
			{
		while (cmdclient.connected())      				// Loop while the client's connected
		{
			tlen = cmdclient.available(); 				// Number of bytes to read from the client
			len = tlen;                               	// Try to read whole input
			if (len == 0){                              // Any input available?
				if (++trycount > 3)                     // Not for a long time?
						{
					return '\n';                        // Error! No input
				}
				delay(10);                       		// Give communication some time
				continue;                           	// Next loop of no input yet
			}
			if (len > sizeof(buf))                      // Limit number of bytes
					{
				len = sizeof(buf);
			}
			len = cmdclient.read(buf, len); 			// Read a number of bytes from the stream
			i = 0;                                 		// Pointer to begin of buffer
			break;
		}
	}
	return buf[i++];
}

void HTML::loop() {
	cmdclient = cmdserver.available();               	// Check Input from client?
	if (cmdclient)                                      // Client connected?
	{
		if(HTML_info) HTML_info("Command client available\n");
		handlehttp();
	}
}
void HTML::reply(const String &response, bool header){
    if(header==true) cmdclient.print(httpheader("text/html"));
	cmdclient.print(response);
}

const char* HTML::ISO88591toUTF8(const char* str){
	uint16_t i=0, j=0;

	while(str[i]!=0){
		if(str[i]>0x7F){sbuf[j]=0xC3; sbuf[j+1]=str[i]-64; j+=2; i++;}
		 else{sbuf[j]=str[i]; j++; i++;}
	}
	sbuf[j]=0;
	return sbuf;
}

const char* HTML::HTMLtoUTF8(const char* str){
	uint16_t i=0, j=0;

	while(str[i]!=0){
		switch(str[i]){
		case 132:{sbuf[j]=0xC3; sbuf[j+1]=164; j+=2; i++; break;} // ä
		case 142:{sbuf[j]=0xC3; sbuf[j+1]=132; j+=2; i++; break;} // Ä
		case 148:{sbuf[j]=0xC3; sbuf[j+1]=182; j+=2; i++; break;} // ö
		case 153:{sbuf[j]=0xC3; sbuf[j+1]=150; j+=2; i++; break;} // Ö
		case 129:{sbuf[j]=0xC3; sbuf[j+1]=188; j+=2; i++; break;} // ü
		case 154:{sbuf[j]=0xC3; sbuf[j+1]=156; j+=2; i++; break;} // Ü
		case 225:{sbuf[j]=0xC3; sbuf[j+1]=159; j+=2; i++; break;} // ß
		default: {if(str[i]>127){sbuf[j]=0xC3, sbuf[j+1]=' '; j+=2; i++;} // all other
		          else {sbuf[j]=str[i]; j++; i++; break;}}}
	}
	sbuf[j]=0;
	return sbuf;
}


String HTML::URLencode(const char* str){
	String hex="0123456789ABCDEF";
	uint16_t i=0, j=0;
	while(str[i]!=0){
		if(str[i]=='%' && isHexadecimalDigit(str[i+1]) && isHexadecimalDigit(str[i+2])){
			sbuf[j]=(hex.indexOf(str[i+1])<<4) + hex.indexOf(str[i+2]);j++; i+=3;}
		else{sbuf[j]=str[i]; j++; i++;}
	}
	sbuf[j]=0;
	return String(sbuf);
}




