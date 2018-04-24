/*
 * about_html.h
 *
 *  Created on: 09.07.2017
 *      Author: Wolle
 *
 *  does not work with MS Internetexplorer
 *  successfully tested with Chrome, MS Edge and Opera
 *
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
        }
        #UL1 {
            list-style-type: none;
            margin: 0;
            overflow: hidden;
            background-color: #033069;
            width:100%;
            min-height:40px;
            z-index:100;
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
        #label-logo {
            left 10px;
            border:2px solid black;
            display:inline-block;
            background-image: url("SD/logo/unknown.bmp");
            width: 96px;
            height:96px;
        }
        #label_TG_value {
            width: 50px;
            display: inline-block;
            text-align: right;
        }
        #label_TF_value {
            width: 50px;
            display: inline-block;
            text-align: right;
        }
        #label_BG_value {
            width: 50px;
            display: inline-block;
            text-align: right;
        }
        #label_BF_value {
            width: 50px;
            display: inline-block;
            text-align: right;
        }
    </style>
</head>

<body id="BODY">

<script>
    //global variables
    var treble_dB = ["-12,0","-10,5"," -9,0"," -7,5"," -6,0"," -4,5"," -3,0"," -1,5",
                     "  0,0"," +1,5"," +3,0"," +4,5"," +6,0"," +7,5"," +9,0","+10,5"];
    var treble_val= [8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7];

    //functions
    document.addEventListener("DOMContentLoaded", function(e) {
        e.preventDefault(); 
        document.getElementById("tab1").addEventListener("click", function() {  // tab Radio
            console.log("tab-content1");
            document.getElementById("tab-content1").style.display = "block";
            document.getElementById("tab-content2").style.display = "none";
            document.getElementById("tab-content3").style.display = "none";
            document.getElementById("tab-content4").style.display = "none";
            gettone();                
            loadstationlist();
            httpGet("to_listen", 1); 
        });
        document.getElementById("tab2").addEventListener("click", function() {      // tab Config
            console.log("tab-content2");
            document.getElementById("tab-content1").style.display = "none";
            document.getElementById("tab-content2").style.display = "block";
            document.getElementById("tab-content3").style.display = "none";
            document.getElementById("tab-content4").style.display = "none";
            getnetworks(); // Now load WiFi Networks
            ldef("getprefs"); // Now get the configuration parameters from preferences
        });
        document.getElementById("tab3").addEventListener("click", function() {      // tab MP3 Player
            console.log("tab-content3");
            document.getElementById("tab-content1").style.display = "none";
            document.getElementById("tab-content2").style.display = "none";
            document.getElementById("tab-content3").style.display = "block";
            document.getElementById("tab-content4").style.display = "none";    
            getmp3list(); // Now get the mp3 list from SD      
        }); 
        document.getElementById("tab4").addEventListener("click", function() {      // tab About
            console.log("tab-content4");
            document.getElementById("tab-content1").style.display = "none";
            document.getElementById("tab-content2").style.display = "none";
            document.getElementById("tab-content3").style.display = "none";
            document.getElementById("tab-content4").style.display = "block";                
        });
    });
 
    window.onload = function() { // Fill configuration initially
        // Now load the tones (tab Radio)
        gettone();
        // Now load the stationlist (tab Radio)
        loadstationlist(); 
        // and show the current stationname and stationlogo (tab Radio)
        httpGet("to_listen", 1); 
    }
    
    //----------------------------------- TAB RADIO ------------------------------------

    function showLabel(id, src){  // get the bitmap from SD, convert to URL first
        src=src.replace(/%/g  , "%25");  // % must be the first
        src=src.replace(/\s/g , "%20");  // URLs never can have blanks
      //src=src.replace(/!/g  , "%21");  // not neccecary to replace
      //src=src.replace(/\"/g , "%22");  // not allowed in Windows filenames
      //src=src.replace(/#/g  , "%23");  // can not be used, is separator in list
      //src=src.replace(/\$/g , "%24");  // not neccecary to replace
      //src=src.replace(/&/g  , "%26");  // not neccecary to replace
        src=src.replace(/\'/g , "%27");  // must be replace
        src=src.replace(/\(/g , "%28");  // must be replace
        src=src.replace(/\)/g , "%29");  // must be replace
      //src=src.replace(/\*/g , "%2A");  // not allowed in Windows filenames
        src=src.replace(/\+/g , "%2B");  // is neccecary to replace, + is the same as space
      //src=src.replace(/,/g  , "%2C");  // commas are later replaced in dots
      //src=src.replace(/\-/g , "%2D");  // not neccecary to replace
      //src=src.replace(/\./g , "%2E");  // not neccecary to replace
      //src=src.replace("/"   , "%2F");  // is separator, not usable
      //src=src.replace(/:/g  , "%3A");  // not allowed in Windows filenames
      //src=src.replace(/;/g  , "%3B");  // not neccecary to replace
      //src=src.replace(/</g  , "%3C");  // not allowed in Windows filenames
      //src=src.replace(/\=/g , "%3D");  // can't be used in selectboxes
      //src=src.replace(/>/g  , "%3E");  // not allowed in Windows filenames
      //src=src.replace(/\?/g , "%3F");  // not allowed in Windows filenames
      //src=src.replace(/@/g  , "%40");  // not neccecary to replace
      //src=src.replace(/\[/g , "%5B");  // not neccecary to replace
      //src=src.replace("\"   , "%5C");  // not neccecary to replace
      //src=src.replace(/\]/g , "%5D");  // not neccecary to replace
      //src=src.replace(/\{/g , "%7B");  // not neccecary to replace
      //src=src.replace(/\|/g , "%7C");  // not allowed in Windows filenames
      //src=src.replace(/\}/g , "%7D");  // not neccecary to replace
        var file="url(url=SD/logo/" + src + ".bmp)";
        file=file.split(',').join('.'); //replace commas in dots, Miniradio has no commas in filenames
        document.getElementById(id).style.backgroundImage=file;
    }
 
    function httpGet(theReq, nr) {  // universal request prev, next, vol,  mute...
        var theUrl = "/?" + theReq + "&version=" + Math.random();
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() {
            if(xhr.readyState == XMLHttpRequest.DONE ) {
                if(nr==1) {resultstr1.value = xhr.responseText;
                    if(theReq.startsWith("downpreset")) showLabel('label-logo', xhr.responseText);
                    if(theReq.startsWith("uppreset"))   showLabel('label-logo', xhr.responseText);
                    if(theReq.startsWith("preset"))     showLabel('label-logo', xhr.responseText);
                    if(theReq.startsWith("to_listen"))  showLabel('label-logo', xhr.responseText);
                }
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
                resultstr1.value = xhr.responseText; showLabel('label-logo', 'unknown');
            }
        }
        xhr.open ( "GET", theUrl, true);
        xhr.send() ;
    }

    function selectItemByValue(elmnt, value) {  // tab Radio: load and set tones
        var sel = document.getElementById(elmnt);
        //for(var i=0; i < sel.options.length; i++) {
        //    if(sel.options[i].value == value)
        //    sel.selectedIndex = i;
        //}
        if(elmnt=="toneha") slider_TG_set(value);
        if(elmnt=="tonehf") slider_TF_set(value);
        if(elmnt=="tonela") slider_BG_set(value);
        if(elmnt=="tonelf") slider_BF_set(value);
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

    function slider_TG_mouseUp() {  // Slider Treble Gain   mouseupevent
        handlectrl("toneha", treble_val[TrebleGain.value]);
        console.log(TrebleGain.value);
    }
    function slider_TG_change(){    //  Slider Treble Gain  changeevent
        console.log(TrebleGain.value);
        document.getElementById("label_TG_value").innerHTML= treble_dB[TrebleGain.value];
    }
    function slider_TG_set(value){    // set Slider Treble Gain
        var val= Number(value);
        if(val<8) val=val+8;
        else val=val-8;
        document.getElementById("TrebleGain").value=val;
        document.getElementById("label_TG_value").innerHTML= treble_dB[TrebleGain.value];
        console.log(val);
    }
    
    function slider_TF_mouseUp() {  // Slider Treble Freq   mouseupevent
        handlectrl("tonehf", TrebleFreq.value);
        console.log(TrebleFreq.value);
    }
    function slider_TF_change(){    //  Slider Treble Freq  changeevent
        console.log(TrebleFreq.value);
        document.getElementById("label_TF_value").innerHTML= TrebleFreq.value;
    }
    function slider_TF_set(value){    // set Slider Treble Freq
        var val= Number(value);
        document.getElementById("TrebleFreq").value=val;
        document.getElementById("label_TF_value").innerHTML= TrebleFreq.value;
        console.log(val);
    }
    
    function slider_BG_mouseUp() {  // Slider Bass Gain   mouseupevent
        handlectrl("tonela", BassGain.value);
        console.log(BassGain.value);
    }
    function slider_BG_change(){    //  Slider Bass Gain  changeevent
        var sign="";
        if(BassGain.value!="0") sign="\+";
        console.log(BassGain.value);
        document.getElementById("label_BG_value").innerHTML= sign+BassGain.value;
    }
    function slider_BG_set(value){    // set Slider Bass Gain
        var val= Number(value);
        var sign="";
        if(BassGain.value!="0") sign="\+";
        document.getElementById("BassGain").value=val;
        document.getElementById("label_BG_value").innerHTML= sign+BassGain.value;
        console.log(val);
    }
    
    function slider_BF_mouseUp() {  // Slider Bass Gain   mouseupevent
        handlectrl("tonelf", BassFreq.value);
        console.log(BassFreq.value);
    }
    function slider_BF_change(){    //  Slider Bass Gain  changeevent
        console.log(BassFreq.value);
        document.getElementById("label_BF_value").innerHTML= (BassFreq.value-1)*10;
    }
    function slider_BF_set(value){    // set Slider Bass Gain
        var val= Number(value);
        document.getElementById("BassFreq").value=val;
        document.getElementById("label_BF_value").innerHTML= (BassFreq.value-1)*10;
        console.log(val);
    }

    function handlectrl(id, val) { // Radio: treble, bass, frequ
        var theUrl = "/?" + id + "=" + val + "&version=" + Math.random();
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() {
            if ( xhr.readyState == XMLHttpRequest.DONE ) {
                resultstr1.value = xhr.responseText;
            }
        }
        xhr.open ( "GET", theUrl, true);
        xhr.send();
    }


 
    //----------------------------------- TAB CONFIG ------------------------------------
       
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

    //----------------------------------- TAB MP3 PLAYER ------------------------------------

    function trackreq ( presctrl ) {  // MP3 Player: select mp3 title from track list 
        if (presctrl.value != "-1") {
            httpGet("mp3track=" + presctrl.value, 3); 
        }
    }
    
    function getmp3list() {  // Fill track list initially
        var xhr = new XMLHttpRequest();
        var i, select, opt, tracks, strparts;
        select = document.getElementById("seltrack");
        var theUrl = "/?mp3list" + "&version=" + Math.random() ;           
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
        </center>
        <center>
            <table width="500">
                <tr>
                    <td colspan="3">
                        <center>
                            <br>
                            <label for="preset"><big>Preset:</big></label>
                            <br>
                            <select class="select selectw" onChange="handlepreset(this)" id="preset">
                                <option value="-1">Select a preset here</option>
                            </select>
                            <br><br>    
                        </center>
                    </td>
                </tr>
                <tr>
                    <td rowspan="4" width="200" align="left">
                        <label for="label-logo" id="label-logo" onclick="showLabel('label-logo','unknown.bmp')"> </label>
                    </td>
                    <td align="right" vertical-align="middle"> 
                        <big><label>Treble Gain:</label></big>
                    </td> 
                    <td align="left" vertical-align="middle"> 
                        <class="slidecontainer">
                        <input type="range" min="0" max="15" value="8" class="slider" id="TrebleGain" onmouseup="slider_TG_mouseUp()" oninput="slider_TG_change()">
                        <big><label id="label_TG_value">000</label></big>
                        <label><big>dB</big></label>
                    </td>
                </tr>
                <tr>
                    <td align="right" vertical-align="middle"> 
                        <big><label>Treble Freq:</label></big>
                    </td>
                    <td align="left" vertical-align="middle"> 
                        <class="slidecontainer">
                        <input type="range" min="1" max="15" value="8" class="slider" id="TrebleFreq" onmouseup="slider_TF_mouseUp()" oninput="slider_TF_change()">
                        <big><label id="label_TF_value">000</label></big>
                        <label><big>kHz</big></label>
                    </td>
                </tr>
                <tr>
                    <td align="right" vertical-align="middle"> 
                        <big><label>Bass Gain:</label></big>
                    </td>
                    <td align="left" vertical-align="middle">
                        <class="slidecontainer">
                        <input type="range" min="0" max="15" value="8" class="slider" id="BassGain" onmouseup="slider_BG_mouseUp()" oninput="slider_BG_change()">
                        <big><label id="label_BG_value">000</label></big>
                        <label><big>dB</big></label>
                    </td>
                </tr>
                <tr>
                    <td align="right" vertical-align="middle"> 
                        <big><label>Bass Freq:</label></big>
                    </td>
                    <td align="left" vertical-align="middle">
                        <class="slidecontainer">
                        <input type="range" min="2" max="15" value="6" class="slider" id="BassFreq" onmouseup="slider_BF_mouseUp()" oninput="slider_BF_change()">
                        <big><label id="label_BF_value">000</label></big>
                        <label><big>Hz</big></label>
                    </td>
                </tr>

            </table>    
        </center>
        <br><br>
        <center>

            <input type="text" size="60" id="station" placeholder="Enter a stationURL here....">
            <button class="button" onclick="setstation()">PLAY</button>
            <br><br>
            <br>
            <input type="text" width="600px" size="72" id="resultstr1" placeholder="Waiting for a command...."><br>
            <br>
            <div>
                Find new radio stations at 
                <a target="_blank" href="http://vtuner.com/setupapp/guide/asp/BrowseStations/Searchform.asp">vtuner</a>
                or 
                <a target="_blank" href="http://streamstat.net/main.cgi?mode=all"> StreamStat.NET </a>
                <br>
            </div>
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
        <button class="button" onclick="httpGet('mp3track=0', 3)">STOP</button>
        <br><br>
        <input type="text" size="80" id="resultstr3" placeholder="Waiting for a command...."><br>
        <br><br>
        </center>
    </div>
    <!-------------------------------------------------------------------------------------------------->
    <div id="tab-content4">
        <p> ESP32 Radio -- Webradio receiver for ESP32, 2.8" color display and VS1053 MP3 module.<br>
        This project is documented at <a target="blank" href="https://github.com/schreibfaul1/ESP32-MiniWebRadio">Github</a>.</p>
        <p>Author: Wolle (schreibfaul1)<br>
        <img src="SD/ESP32_Radio_gr.bmp" alt="ESP32_Radio_gr" border=3>
    </div>
    <!-------------------------------------------------------------------------------------------------->
</div>





</body>
</html>
<noscript>
  Sorry, ESP-radio does not work without JavaScript!
</noscript> 
 
<noscript>
  Sorry, ESP-radio does not work without JavaScript!
</noscript> 
    
)=====" ;



#endif /* WEB_H_ */
