/*
 * about_html.h
 *
 *  Created on: 09.07.2017
 *      Author: Wolle
 */

#ifndef WEB_H_
#define WEB_H_

#include "Arduino.h"

// about.html file in raw data format for PROGMEM
//
#define about_html_version 170626
const char web_html[] PROGMEM = R"=====(
<!DOCTYPE HTML>

<html>
<head>
<meta http-equiv="cache-control" content="max-age=0" />
<meta http-equiv="cache-control" content="no-cache" />
<meta http-equiv="expires" content="0" />
<meta http-equiv="expires" content="Tue, 01 Jan 1980 1:00:00 GMT" />
<meta http-equiv="pragma" content="no-cache" />
<meta http-equiv="Content-Language" content="de">
<meta http-equiv="Content-Type" content="text/html; charset=windows-1252">
    <title>ESP32 Radio</title>
    <style type="text/css">
        html{
            margin:0px;
            height:100%;
            padding:0;
            font:16px "Trebuchet MS";
            color:#222;
            background-color:#033069
        }
        #HEADER {
            width:100%;
            min-height:15px;
            text-align: center;
            padding-top:5px;
            padding-bottom:8px;
            background-color:#033069
            color:#f0f0ff;
            font-size:22px;
            line-height:25px;
        }
        @keyframes mymove1 {
            from {color:#fff;} to {color:#fb0;}
        }           
        #HEADER span.bold1 {
            font-size:22px;
            width:98%;
            display:inline;
            animation:mymove1 3s infinite alternate
        }
        #CONTENT {
            width:96%;
            min-height:620px;
            overflow:hidden;
            margin:auto;
            background-color: lightblue;
            box-shadow: 0px 0px 10px 10px #2C3E50;
        }
        #UL1 {
            list-style-type: none;
            margin: 0;
            overflow: hidden;
            background-color: #2C3E50;
            width:100%;
            min-height:40px;
            z-index:100;
            box-shadow: 0px 0px 10px 10px #2C3E50;
        }
        .tabs li {
            float:left;
            margin-right:2px;
        }
        .tabs input[type="radio"] {
            position:absolute;
            left:-9999px
        }
        .tabs [id^="tab"]:checked+label {
            top:16px;
            padding:12px 14px 8px 14px;
            font-size:18px;
            color:#EEE;
            background:#38c;
            border-radius:5px
        }
        .tabs label {
            padding:8px 14px 8px 14px;
            font-weight:normal;
            background:#39c;
            cursor:pointer;
            position:relative;
            top:16px;
            -moz-transition:all .2s ease-in-out;
            -o-transition:all .2s ease-in-out;
            -webkit-transition:all .2s ease-in-out;
            transition:all .2s ease-in-out;
            border-radius:5px
        }
        .tabs label:hover {
            background:#3cc
        }
        #tab-content1 {
            display:block;
        }
        #tab-content2 {
            display:none;
        }
        #tab-content3 {
            display:none;
        }
        #tab-content4 {
            display:none;
            margin-left:20px;
        }
        .button {
            width: 80px;
            height: 30px;
            background-color: #128F76;
            border: none;
            color: white;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 16px;
            margin: 4px 2px;
            cursor: pointer;
            border-radius: 10px;
        }
        .buttonred  {background-color: #D62C1A; width: 120px;}
        .buttonblue {background-color: blue;    width: 120px;}
        .buttongreen{background-color: #128F76; width: 120px;}
        .select {
            width: 260px;
            height: 34px;
            padding-top: 0px;
            padding-left: 5px;
            padding-bottom: 0px;
            padding-right_ 5px;
            background: white;
            font-size: 16px;
            line-height: normal;
            border: 1;
            border-radius: 5px;
            -webkit-border-radius: 5px;
            -moz-border-radius: 5px;
            -webkit-appearance: none;
            border: 1px solid black;
            border-radius: 10px;
        }
        .selectw {width: 600px;}
        input[type="text"] {
            margin: 0;
            height: 28px;
            background: white;
            font-size: 16px;
            appearance: none;
            box-shadow: none;
            border-radius: 5px;
            -webkit-border-radius: 5px;
            -moz-border-radius: 5px;
            -webkit-appearance: none;
            border: 1px solid black;
            border-radius: 10px;
        }
        input[type="text"]:focus {
            outline: none;
        }
        input[type=submit] {
            width: 200px;
            height: 40px;
            background-color: #128F76;
            border: none;
            color: white;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 16px;
            margin: 4px 2px;
            cursor: pointer;
            border-radius: 5px;
        }
        .bild80 {
             width: 80%;
             height: auto;
        }
    </style>
</head>

<body id="BODY">
    <script>
            document.addEventListener("DOMContentLoaded", function(e) {
                e.preventDefault(); 
                document.getElementById("tab1").addEventListener("click", function() { // tab Radio
                console.log("tab-content1");
                document.getElementById("tab-content1").style.display = "block";
                document.getElementById("tab-content2").style.display = "none";
                document.getElementById("tab-content3").style.display = "none";
                document.getElementById("tab-content4").style.display = "none";
                gettone();                
                loadstationlist();
            });
            document.getElementById("tab2").addEventListener("click", function() { // tab Config
                console.log("tab-content2");
                document.getElementById("tab-content1").style.display = "none";
                document.getElementById("tab-content2").style.display = "block";
                document.getElementById("tab-content3").style.display = "none";
                document.getElementById("tab-content4").style.display = "none";
                getnetworks(); // Now load WiFi Networks
                ldef("getprefs"); // Now get the configuration parameters from preferences
            });
            document.getElementById("tab3").addEventListener("click", function() { // tab MP3 Player
                console.log("tab-content3");
                document.getElementById("tab-content1").style.display = "none";
                document.getElementById("tab-content2").style.display = "none";
                document.getElementById("tab-content3").style.display = "block";
                document.getElementById("tab-content4").style.display = "none";    
                getmp3list(); // Now get the mp3 list from SD      
            }); 
            document.getElementById("tab4").addEventListener("click", function() { // tab About
                console.log("tab-content4");
                document.getElementById("tab-content1").style.display = "none";
                document.getElementById("tab-content2").style.display = "none";
                document.getElementById("tab-content3").style.display = "none";
                document.getElementById("tab-content4").style.display = "block";                
            });
        });
        
        function httpGet(theReq, nr) {  // universal request prev, next, vol,  mute...
            var theUrl = "/?" + theReq + "&version=" + Math.random();
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function() {
                if(xhr.readyState == XMLHttpRequest.DONE ) {
                    if(nr==1) resultstr1.value = xhr.responseText;
                    if(nr==2) resultstr2.value = xhr.responseText;
                    if(nr==3) resultstr3.value = xhr.responseText;
                }
            }
            xhr.open("GET", theUrl, true);
            xhr.send();
        }

        function gettone() {   // tab Radio: get tones values and set they
            var i, sel, lines, parts ;            
            var theUrl = "/?gettone" + "&version=" + Math.random();
            var xhr = new XMLHttpRequest() ;
            xhr.onreadystatechange = function() {
                if ( xhr.readyState == XMLHttpRequest.DONE ) {
                    lines = xhr.responseText.split ( "\n" ) ;
                    for(i=0; i < (lines.length-1); i++) {
                        sel = document.getElementById("preset");
                        parts=lines[i].split("=");
                        if(parts[0].indexOf("tone") == 0) {
                            selectItemByValue ( parts[0], parts[1] ) ;
                        }
                    }
                }
            }
            xhr.open ( "GET", theUrl, true) ;
            xhr.send();
        }


        function handlepreset(presctrl) {  // tab Radio: preset, select a station
            httpGet("preset=" + presctrl.value, 1);
        }

        function handletone(tonectrl) { // Radio: treble, bass, frequ
            var theUrl = "/?" + tonectrl.id + "=" + tonectrl.value + "&version=" + Math.random();
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function() {
                if ( xhr.readyState == XMLHttpRequest.DONE ) {
                    resultstr1.value = xhr.responseText;
                }
            }
            xhr.open ( "GET", theUrl, true);
            xhr.send();
        }

        function setstation() {  // Radio: button play - enter a url to play from
            var theUrl = "/?station=" + station.value + "&version=" + Math.random();
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function() {
                if(xhr.readyState == XMLHttpRequest.DONE) {
                    resultstr1.value = xhr.responseText;
                }
            }
            xhr.open ( "GET", theUrl, true);
            xhr.send() ;
        }

        function selectItemByValue(elmnt, value) {  // tab Radio: load and set tones
            var sel = document.getElementById(elmnt);
            for(var i=0; i < sel.options.length; i++) {
                if(sel.options[i].value == value)
                sel.selectedIndex = i;
            }
        }

        function loadstationlist() { // tab Radio: load preset stations
            var i, select, opt, lines, parts ;
            var theUrl = "/?settings" + "&version=" + Math.random() ;
            var xhr = new XMLHttpRequest() ;
            xhr.onreadystatechange = function() {
                if ( xhr.readyState == XMLHttpRequest.DONE ) {
                    lines = xhr.responseText.split ( "\n" ) ;
                    select = document.getElementById("preset");
                    select.options.length=1;
                    for(i=0; i < (lines.length-1); i++) {
                        parts=lines[i].split("=");
                        if(parts[0].indexOf("preset_") == 0) {
                            opt = document.createElement("OPTION");
                            opt.value = parts[0].substring(7);
                            opt.text = parts[1] ;
                            select.add(opt);
                        }
                    }
                }
            }
            xhr.open ( "GET", theUrl, true) ;
            xhr.send();
        }
        
        function ldef(source) {  // Config: Load preferences or defaults
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function() {
                prefs.value="";
                if(xhr.readyState == XMLHttpRequest.DONE) {
                    if(source=='getprefs')prefs.value = xhr.responseText;
                    if(source=='getdefs')prefs.value=xhr.responseText;
                }
            }
            xhr.open("GET", "/?" + source  + "&version=" + Math.random(), true);
            xhr.send() ;
        }

        function fsav() {  // tab Config: save the preferences
            var str = prefs.value;
            var theUrl = "/?saveprefs&version=" + Math.random();
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function() {
                if( xhr.readyState == XMLHttpRequest.DONE) {
                    // do nothing 
                }
            }
            // Remove empty lines
            while(str.indexOf("\r\n\r\n") >= 0) {
                str=str.replace(/\r\n\r\n/g, "\r\n")
            }
            while(str.indexOf("\n\n" ) >= 0) {
                str=str.replace(/\n\n/g, "\n")
            }
            xhr.open("POST", theUrl, true);
            xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
            xhr.send(str + "\n");
        }

        function getnetworks() { // tab Config: load the connevted WiFi network
            var i, select, opt, networks;
            var theUrl = "/?getnetworks" + "&version=" + Math.random() ;
            var xhr = new XMLHttpRequest();
            xhr.onreadystatechange = function() {
                if(xhr.readyState == XMLHttpRequest.DONE) {
				    networks=xhr.responseText.split("\n");
                    for(i = 0 ; i < (networks.length - 1); i++ ) {
                        opt = document.createElement( "OPTION" );
                        opt.value = i;
                        opt.text = networks[i];
                        ssid.add(opt);
                    }
                }
            }
            xhr.open ( "GET", theUrl, true);
            xhr.send();
        }

        function trackreq ( presctrl ) {  // MP3 Player: select mp3 title from track list 
            if (presctrl.value != "-1") {
                httpGet("mp3track=" + presctrl.value, 3); 
            }
        }
        
        function getmp3list() {  // Fill track list initially
            var xhr = new XMLHttpRequest();
            var i, select, opt, tracks, strparts;
            select = document.getElementById("seltrack");
            var theUrl = "/?mp3list";           
            xhr.onreadystatechange = function(){
                if(xhr.readyState == XMLHttpRequest.DONE) {
                    tracks = xhr.responseText.split("\n");
                    //for(i=select.options.length-1; i>=0; i--) select.remove(i);
                    //select.add("chose a item here");
                    select.options.length=1;
                    for(i = 0 ; i < ( tracks.length - 1 ) ; i++) {
                        opt=document.createElement("OPTION");
                        strparts=tracks[i].substr(tracks[i].lastIndexOf("/")+1,tracks[i].lenght);
                        opt.value = tracks[i] ;
                        opt.text = strparts ;
                        select.add(opt);
                    }
                }
            }
            xhr.open ( "GET", theUrl, true ) ;
            xhr.send() ;
        }

        window.onload = function() { // Fill configuration initially
            // Now load the tones (tab Radio)
            gettone();
            // Now load the stationlist (tab Radio)
            loadstationlist(); 
        }
  
    </script>


    <div id="HEADER">   <span class="bold1"><span id="meta">** ESP32 Radio **</span></span> </div>  
    <div id="CONTENT" >
    <ul  id="UL1" class="tabs">
        <li>
            <input type="radio" name="tabs" id="tab1" checked />
            <label for="tab1">Radio</label>
        </li>               
        <li>
            <input type="radio" name="tabs" id="tab2" />
            <label for="tab2">Config</label>
        </li>               
        <li>
            <input type="radio" name="tabs" id="tab3" />
            <label for="tab3">MP3 Player</label>
        </li>
        <li>
            <input type="radio" name="tabs" id="tab4" />
            <label for="tab4">About</label>
        </li>
    </ul>
    <!-------------------------------------------------------------------------------------------------->
    <div id="tab-content1">
        <center>
        <br><br>
        <button class="button" onclick="httpGet('downpreset=1', 1)">PREV</button>
        <button class="button" onclick="httpGet('uppreset=1', 1)">NEXT</button>
        <button class="button" onclick="httpGet('downvolume=2', 1)">VOL-</button>
        <button class="button" onclick="httpGet('upvolume=2', 1)">VOL+</button>
        <button class="button" onclick="httpGet('mute', 1)">MUTE</button>
        <button class="button" onclick="httpGet('test', 1)">TEST</button>
        <table  style="width:500px">
        <tr>
        <td colspan="2"><center>
        <br>
        <label for="preset"><big>Preset:</big></label>
        <br>
        <select class="select selectw" onChange="handlepreset(this)" id="preset">
            <option value="-1">Select a preset here</option>
        </select>
        <br><br>
        </center></td>
        </tr>
        <tr>
        <td><center>
        <label for="HA"><big>Treble Gain:</big></label>

        <select class="select" onChange="handletone(this)" id="toneha">
            <option value="8">-12 dB</option>
            <option value="9">-10.5 dB</option>
            <option value="10">-9 dB</option>
            <option value="11">-7.5 dB</option>
            <option value="12">-6 dB</option>
            <option value="13">-4.5 dB</option>
            <option value="14">-3 dB</option>
            <option value="15">-1.5 dB</option>
            <option value="0" selected>Off</option>
            <option value="1">+1.5 dB</option>
            <option value="2">+3 dB</option>
            <option value="3">+4.5 dB</option>
            <option value="4">+6 dB</option>
            <option value="5">+7.5 dB</option>
            <option value="6">+9 dB</option>
            <option value="7">+10.5 dB</option>
        </select>
        </td>
        <td><center>
        <label for="HF"><big>Treble Freq:</big></label>
        <select class="select" onChange="handletone(this)" id="tonehf">
            <option value="1">1 kHz</option>
            <option value="2">2 kHz</option>
            <option value="3">3 kHz</option>
            <option value="4">4 kHz</option>
            <option value="5">5 kHz</option>
            <option value="6">6 kHz</option>
            <option value="7">7 kHz</option>
            <option value="8">8 kHz</option>
            <option value="9">9 kHz</option>
            <option value="10">10 kHz</option>
            <option value="11">11 kHz</option>
            <option value="12">12 kHz</option>
            <option value="13">13 kHz</option>
            <option value="14">14 kHz</option>
            <option value="15">15 kHz</option>
        </select>
        </center></td>
        </tr>
        <tr>
        <td><center>
        <br>
        <label for="LA"><big>Bass Gain:</big></label>
        <select class="select" onChange="handletone(this)" id="tonela">
            <option value="0" selected>Off</option>
            <option value="1">+1 dB</option>
            <option value="2">+2 dB</option>
            <option value="3">+3 dB</option>
            <option value="4">+4 dB</option>
            <option value="5">+5 dB</option>
            <option value="6">+6 dB</option>
            <option value="7">+7 dB</option>
            <option value="8">+8 dB</option>
            <option value="9">+9 dB</option>
            <option value="10">+10 dB</option>
            <option value="11">+11 dB</option>
            <option value="12">+12 dB</option>
            <option value="13">+13 dB</option>
            <option value="14">+14 dB</option>
            <option value="15">+15 dB</option>
        </select>
        </td>
        <td><center>
        <br>
        <label for="LF"><big>Bass Freq:</big></label>
        <select class="select" onChange="handletone(this)" id="tonelf">
            <option value="2">10 Hz</option>
            <option value="3">20 Hz</option>
            <option value="4">30 Hz</option>
            <option value="5">40 Hz</option>
            <option value="6">50 Hz</option>
            <option value="7">60 Hz</option>
            <option value="8">70 Hz</option>
            <option value="9">80 Hz</option>
            <option value="10">90 Hz</option>
            <option value="11">100 Hz</option>
            <option value="12">110 Hz</option>
            <option value="13">120 Hz</option>
            <option value="14">130 Hz</option>
            <option value="15">140 Hz</option>
        </select>
        </center></td>
        </tr>
        </table>
        <br><br>
        <input type="text" size="60" id="station" placeholder="Enter a station/file here....">
        <button class="button" onclick="setstation()">PLAY</button>
        <br><br>
        <br>
        <input type="text" width="600px" size="72" id="resultstr1" placeholder="Waiting for a command...."><br>
        <br><br>
        <div>Find new radio stations at <a target="_blank" href="http://vtuner.com/setupapp/guide/asp/BrowseStations/Searchform.asp"></div>
        <div>Examples: http://vtuner.com/setupapp/guide/asp/BrowseStations/Searchform.asp</div></a><br>
        </center>
    </div>
    <!-------------------------------------------------------------------------------------------------->
    <div id="tab-content2">
        <center>
        <p>You can edit the configuration here. <i>Note that this will be overwritten by "Load default".</i></p>
        <h3>Connected WiFi network
        <select class="select" onChange="handletone(this)" id="ssid"></select>
        </h3>
        <textarea wrap="off" rows="25" cols="100" id="prefs">Space for preferences</textarea>
        <br>
        <button class="button buttongreen" onclick="fsav()">Save</button>
        &nbsp;&nbsp;
        <button class="button buttonred" onclick="httpGet('reset', 2)">Restart</button>
        &nbsp;&nbsp;
        <button class="button buttongreen" onclick="ldef('getprefs')">Load</button>
        &nbsp;&nbsp;
        <button class="button buttonblue" onclick="ldef('getdefs')">Load default</button>
        <br>
        </center>
    </div>  
    <!-------------------------------------------------------------------------------------------------->
    <div id="tab-content3">
        <center>
        <br>
        <label for="seltrack"><big>MP3 files on SD card:</big></label>
        <br>
        <select class="select selectw" onChange="trackreq(this)" id="seltrack">
            <option value="-1">Select a track here</option>
        </select>
        <br><br>
        <button class="button" onclick="httpGet('mp3track=0', 3)">RANDOM</button>
        <br><br>
        <input type="text" size="80" id="resultstr3" placeholder="Waiting for a command...."><br>
        <br><br>
        </center>
    </div>
    <!-------------------------------------------------------------------------------------------------->
    <div id="tab-content4">
        <p> ESP32 Radio -- Webradio receiver for ESP32, 2.8" color display and VS1053 MP3 module.<br>
        This project is documented at <a target="blank" href="https://github.com/Edzelf/ESP32-radio">Github</a>.</p>
        <p>Author: Ed Smallenburg<br>
        Webinterface design: <a target="blank" href="http://www.sanderjochems.nl/">Sander Jochems</a><br>
        App (Android): <a target="blank" href="https://play.google.com/store/apps/details?id=com.thunkable.android.sander542jochems.ESP_Radio">Sander Jochems</a><br>
        Date: June 2017</p>
        <img src="SD/Dev_Board.gif" class="bild80" alt="ESP32_Dev_Board" border=3>
    </div>
    <!-------------------------------------------------------------------------------------------------->
</div>
</body>
</html>
<noscript>
  Sorry, ESP-radio does not work without JavaScript!
</noscript> 
    
)=====" ;



#endif /* WEB_H_ */
