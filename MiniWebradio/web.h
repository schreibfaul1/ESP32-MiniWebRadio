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
    
    <title>MiniWebRadio</title>
    <style type="text/css">
        html{
            margin:0px;
            height:100%;
            padding:0;
            font:16px "Trebuchet MS";
            color:#222;
            background-color:#033069;
        }
        #CONTENT {
            min-height:620px;
            min-width: 700px;
            overflow:hidden;
            margin: 5px;
            background-color: lightblue;
        }

        #tab-content1 {
            display:block;
            margin: 20px;
        }
        #tab-content2 {
            display:none;
            margin: 20px;
        }
        #tab-content3 {
            display:none;
            margin: 20px;
        }
        #tab-content4 {
            display:none;
            margin: 20px;
        }
        #tab-content5 {
            display:none;
            margin: 20px;
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
            margin-left: 20px;
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
        #prefs{
            width: 100%;
        }
    </style>
</head>

<script>
    //global variables
    var treble_dB = ["-12,0","-10,5"," -9,0"," -7,5"," -6,0"," -4,5"," -3,0"," -1,5",
                     "  0,0"," +1,5"," +3,0"," +4,5"," +6,0"," +7,5"," +9,0","+10,5"];
    var treble_val= [8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7];

    window.onload = function() { // Fill configuration initially
        gettone(); //Now load the tones (tab Radio)
        httpGet("to_listen", 1);
        getnetworks();
        ldef("getprefs");
     }

    function showTab1(){
        console.log("tab-content1 (Radio)");
        document.getElementById("tab-content1").style.display = "block";
        document.getElementById("tab-content2").style.display = "none";
        document.getElementById("tab-content3").style.display = "none";
        document.getElementById("tab-content4").style.display = "none";
        document.getElementById("tab-content5").style.display = "none";
    }
    function showTab2(){
        console.log("tab-content2 (Stations)");
        document.getElementById("tab-content1").style.display = "none";
        document.getElementById("tab-content2").style.display = "block";
        document.getElementById("tab-content3").style.display = "none";
        document.getElementById("tab-content4").style.display = "none";
        document.getElementById("tab-content5").style.display = "none";
    }
    function showTab3(){
        console.log("tab-content3 (MP3 Player)");
        document.getElementById("tab-content1").style.display = "none";
        document.getElementById("tab-content2").style.display = "none";
        document.getElementById("tab-content3").style.display = "block";
        document.getElementById("tab-content4").style.display = "none";
        document.getElementById("tab-content5").style.display = "none";    
        getmp3list(); // Now get the mp3 list from SD      
    }
    function showTab4(){
        console.log("tab-content4 (Settings)");
        document.getElementById("tab-content1").style.display = "none";
        document.getElementById("tab-content2").style.display = "none";
        document.getElementById("tab-content3").style.display = "none";
        document.getElementById("tab-content4").style.display = "block"
        document.getElementById("tab-content5").style.display = "none";    
        getmp3list(); // Now get the mp3 list from SD      
    }
    function showTab5(){
        console.log("tab-content5 (About)");
        document.getElementById("tab-content1").style.display = "none";
        document.getElementById("tab-content2").style.display = "none";
        document.getElementById("tab-content3").style.display = "none";
        document.getElementById("tab-content4").style.display = "none";
        document.getElementById("tab-content5").style.display = "block";      
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
      //file=file.split(',').join('.'); //replace commas in dots, Miniradio has no commas in filenames
        document.getElementById(id).style.backgroundImage=file;
    }
 
    function httpGet(theReq, nr) {  // universal request prev, next, vol,  mute...
        var theUrl = "/?" + theReq + "&version=" + Math.random();
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() {
            if(xhr.readyState == XMLHttpRequest.DONE ) {
                if(nr==1) {
                    if(theReq.startsWith("downpreset")||
                        theReq.startsWith("uppreset")||
                        theReq.startsWith("preset")||
                        theReq.startsWith("to_listen")){
                        resultstr1.value = xhr.responseText;
                        var res="", num = "", sta="", url="", n=0;
                        res = xhr.responseText;
                        n = res.indexOf(" ");
                        num = res.substring(0, n);      // stationnumber
                        var sel = document.getElementById('preset');
                        sel.selectedIndex = Number(num);
                        if(n==1) num="00"+num;
                        if(n==2) num="0"+num;
               //         selectItemByValue("preset", num);
                        res = res.substring(n+1);       // remove stationnumber
                        n = res.indexOf(" ");
                        url = res.substring(0, n);      // stationURL
                        sta = res.substring(n+1);
                        showLabel('label-logo', sta);
                        resultstr1.value = "";  //sta;
                        station.value = url;
                    }
                    else if(xhr.responseText.startsWith("http")){
                        console.log(xhr.responseText);
                        window.open(xhr.responseText, '_blank');    // show the station homepage
                    }
                    else if(xhr.responseText.startsWith("Mute")){
                        console.log(xhr.responseText);
                        resultstr1.value = xhr.responseText;        // all other
                        if(xhr.responseText.endsWith("off\n")){
                            document.getElementById("Mute").src="SD/png/Button_Mute_Green.png";
                        }
                        if(xhr.responseText.endsWith("on\n")){
                            document.getElementById("Mute").src="SD/png/Button_Mute_Red.png";
                        }
                    }
                }
                if(nr==1){ 
                        if(theReq=="mute"||
                            theReq.startsWith("upvolume")||
                            theReq.startsWith("downvolume")) cmd.value=xhr.responseText; 
                else
                    resultstr1.value = xhr.responseText;
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
                        setSlider(parts[0], parts[1]) ;
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
        for(var i=0; i < sel.options.length; i++) {
            if(sel.options[i].value == value)
            sel.selectedIndex = i;
        }
    }

    function setSlider(elmnt, value){
        if(elmnt=="toneha") slider_TG_set(value);
        if(elmnt=="tonehf") slider_TF_set(value);
        if(elmnt=="tonela") slider_BG_set(value);
        if(elmnt=="tonelf") slider_BF_set(value);
    }

    function slider_TG_mouseUp() {  // Slider Treble Gain   mouseupevent
        handlectrl("toneha", treble_val[TrebleGain.value]);
        //console.log("Treble Gain=%i",Number(TrebleGain.value));
    }
    function slider_TG_change(){    //  Slider Treble Gain  changeevent
        console.log("Treble Gain=%i", Number(TrebleGain.value));
        document.getElementById("label_TG_value").innerHTML= treble_dB[TrebleGain.value];
    }
    function slider_TG_set(value){    // set Slider Treble Gain
        var val= Number(value);
        if(val<8) val=val+8;
        else val=val-8;
        document.getElementById("TrebleGain").value=val;
        document.getElementById("label_TG_value").innerHTML= treble_dB[TrebleGain.value];
        console.log("Treble Gain=%i", val);
    }
    
    function slider_TF_mouseUp() {  // Slider Treble Freq   mouseupevent
        handlectrl("tonehf", TrebleFreq.value);
        //console.log("Treble Freq=%i", Number(TrebleFreq.value));
    }
    function slider_TF_change(){    //  Slider Treble Freq  changeevent
        console.log("Treble Freq=%i", Number(TrebleFreq.value));
        document.getElementById("label_TF_value").innerHTML= TrebleFreq.value;
    }
    function slider_TF_set(value){    // set Slider Treble Freq
        var val= Number(value);
        document.getElementById("TrebleFreq").value=val;
        document.getElementById("label_TF_value").innerHTML= TrebleFreq.value;
        console.log("Treble Freq=%i", val);
    }
    
    function slider_BG_mouseUp() {  // Slider Bass Gain   mouseupevent
        handlectrl("tonela", BassGain.value);
        //console.log("Bass Gain=%i", Number(BassGain.value));
    }
    function slider_BG_change(){    //  Slider Bass Gain  changeevent
        var sign="";
        if(BassGain.value!="0") sign="\+";
        console.log("Bass Gain=%i", Number(BassGain.value));
        document.getElementById("label_BG_value").innerHTML= sign+BassGain.value;
    }
    function slider_BG_set(value){    // set Slider Bass Gain
        var val= Number(value);
        var sign="";
        if(BassGain.value!="0") sign="\+";
        document.getElementById("BassGain").value=val;
        document.getElementById("label_BG_value").innerHTML= sign+BassGain.value;
        console.log("Bass Gain=%i", val);
    }
    
    function slider_BF_mouseUp() {  // Slider Bass Gain   mouseupevent
        handlectrl("tonelf", BassFreq.value);
        //console.log("Bass Freq=%i", Number(BassFreq.value));
    }
    function slider_BF_change(){    //  Slider Bass Gain  changeevent
        console.log("Bass Freq=%i", Number(BassFreq.value));
        document.getElementById("label_BF_value").innerHTML= (BassFreq.value-1)*10;
    }
    function slider_BF_set(value){    // set Slider Bass Gain
        var val= Number(value);
        document.getElementById("BassFreq").value=val;
        document.getElementById("label_BF_value").innerHTML= (BassFreq.value-1)*10;
        console.log("Bass Freq=%i", val);
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
        var lines, opt, select, i;
        xhr.onreadystatechange = function() {
            prefs.value="";
            if(xhr.readyState == XMLHttpRequest.DONE) {
                select = document.getElementById("prefs");  // Config: show stationlist and URL
                if(source=='getprefs')select.value = xhr.responseText;
                if(source=='getdefs')select.value=xhr.responseText;
                select = document.getElementById("preset");  // Radio: show stationlist
                select.options.length=1;
                lines=xhr.responseText.split("\n");
                for(i = 0 ; i < (lines.length - 1); i++ ) {
                    lines[i]=lines[i].substring(0,lines[i].indexOf("#"));
                    lines[i]=lines[i].trim();
                    lines[i]+="\n";
                    opt = document.createElement("OPTION");
                    opt.text = lines[i];
                    select.add(opt);
                }
            }
        }
        xhr.open("GET", "/?" + source  + "&version=" + Math.random(), true);
        xhr.send() ;
    }

    function fsav() {  // tab Config: save the preferences
        var str = prefs.value;
        var theUrl = "/?saveprefs=0&version=" + Math.random();
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() {
            if( xhr.readyState == XMLHttpRequest.DONE) {
                ldef("getprefs");
            }
        }
        // Remove empty lines
        while(str.indexOf("\r\n\r\n") >= 0) {
            str=str.replace(/\r\n\r\n/g, "\r\n");
        }
        while(str.indexOf("\n\n" ) >= 0) {
            str=str.replace(/\n\n/g, "\n");
        }
        xhr.open("POST", theUrl, true);
        xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        xhr.send(str + "end -" + "\n");
    }

    function getnetworks() { // tab Config: load the connevted WiFi network
        var i, select, opt, networks;
        var theUrl = "/?getnetworks" + "&version=" + Math.random() ;
        var xhr = new XMLHttpRequest();
        select = document.getElementById("ssid");  // Radio: show stationlist
        xhr.onreadystatechange = function() {
            if(xhr.readyState == XMLHttpRequest.DONE) {
                networks=xhr.responseText.split("\n");
                for(i = 0 ; i < (networks.length - 1); i++ ) {
                    opt = document.createElement( "OPTION" );
                    opt.value = i;
                    opt.text = networks[i];
                    select.add(opt);
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

<body id="BODY">
 <div id="CONTENT" >

    <!--==============================================================================================-->
        <div id="tab-content1">
        <table width=100%>
            <tr>
                <td style="width:300px;">
                    <left>
                        <img src="SD/png/Radio_Yellow.png"      alt="radio"                         />
                        <img src="SD/png/Station_Green.png"     alt="station"   onclick="showTab2()"/> 
                        <img src="SD/png/MP3_Green.png"         alt="mp3"       onclick="showTab3()"/>
                 <!--   <img src="SD/png/Settings_Green.png"    alt="settings"  onclick="showTab4()"/> -->
                        <img src="SD/png/About_Green.png"       alt="radio"     onclick="showTab5()"/>
                    </left>
                </td>
                <td>
                    <center>
                        <label style="font-size: 50px; font-family: 'Garamond', Verdana, Halvetica, Arial;">
                            MiniWebRadio
                        </label>
                    </center>
                </td>
            </tr>
        </table> 
        <hr>
        <table width=100%>
            <tr>
                <td width="210">
                    <left>
                        <img src="SD/png/Button_Previous_Green.png" alt="previous" 
                             onmousedown="this.src='SD/png/Button_Previous_Yellow.png'" 
                             onmouseup="this.src='SD/png/Button_Previous_Green.png'"
                             onclick="httpGet('downpreset=1', 1)"/>
                        <img src="SD/png/Button_Next_Green.png" alt="next" 
                             onmousedown="this.src='SD/png/Button_Next_Yellow.png'" 
                             onmouseup="this.src='SD/png/Button_Next_Green.png'"
                             onclick="httpGet('uppreset=1', 1)"/>
                    </left>
                </td>
                <td colspan="2">
                 <select class="select" style="width:100%;"  onChange="handlepreset(this)" id="preset">
                    <option value="-1">Select a preset here</option>
                 </select>
                </td>
            </tr>
            <tr>
                <td rowspan="4"  align="left">
                    <label for="label-logo" id="label-logo" onclick="httpGet('homepage', 1)"> </label>
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
            <tr>
                <td>
                    <left>
                        <img src="SD/png/Button_Volume_Down_Blue.png" alt="Vol_down" 
                             onmousedown="this.src='SD/png/Button_Volume_Down_Yellow.png'" 
                             onmouseup="this.src='SD/png/Button_Volume_Down_Blue.png'"
                             onclick="httpGet('downvolume=2', 1)"/>
                        <img src="SD/png/Button_Volume_Up_Blue.png" alt="Vol_up" 
                             onmousedown="this.src='SD/png/Button_Volume_Up_Yellow.png'" 
                             onmouseup="this.src='SD/png/Button_Volume_Up_Blue.png'"
                             onclick="httpGet('upvolume=2', 1)"/>
                        <img id="Mute" 
                             src="SD/png/Button_Mute_Green.png" alt="Mute" 
                             onmousedown="this.src='SD/png/Button_Mute_Yellow.png'" 
                             onclick="httpGet('mute', 1)"/>
                    </left>
                </td>
                <td colspan="2">
                    <input type="text" style="width:100%;"  id="cmd" placeholder="Waiting for a command....">
                </td>
            </tr>
        </table>    
        <table width=100%>
            <tr>
            </tr>
                <td>
                    <input type="text" style="width:100%;" id="station" placeholder="Enter a stationURL here....">
                </td>
                <td width="70">
                    <right>
                             <img src="SD/png/Button_Ready_Blue.png" alt="Vol_up" 
                             onmousedown="this.src='SD/png/Button_Ready_Yellow.png'" 
                             onmouseup="this.src='SD/png/Button_Ready_Blue.png'"
                             onclick="setstation()"/>
                    </right>
                </td>
                <tr>
                <td>
                            <input type="text" style="width:100%;" id="resultstr1" placeholder="Test....">
                </td>
                <td>
                    <right>
                            <img src="SD/png/Button_Test_Green.png" alt="Test" 
                            onmousedown="this.src='SD/png/Button_Test_Yellow.png'" 
                            onmouseup="this.src='SD/png/Button_Test_Green.png'"
                            onclick="httpGet('test', 1)"/>
                    </right>
                </td>
            </tr>
        </table>
        <center>
            <br>Find new radio stations at 
                <a target="_blank" href="http://vtuner.com/setupapp/guide/asp/BrowseStations/Searchform.asp">vtuner</a>
                or 
                <a target="_blank" href="http://streamstat.net/main.cgi?mode=all"> StreamStat.NET </a>
                <br>
        </center>
    </div>
    <!--==============================================================================================-->
    <div id="tab-content2">
        <table width=100%>
            <tr>
                <td style="width:300px;">
                    <left>
                        <img src="SD/png/Radio_Green.png"       alt="radio"     onclick="showTab1()"/>
                        <img src="SD/png/Station_Yellow.png"    alt="station"                       /> 
                        <img src="SD/png/MP3_Green.png"         alt="mp3"       onclick="showTab3()"/>
                <!--    <img src="SD/png/Settings_Green.png"    alt="settings"  onclick="showTab4()"/> -->
                        <img src="SD/png/About_Green.png"       alt="radio"     onclick="showTab5()"/>
                    </left>
                </td>
                <td>
                    <center>
                        <label style="font-size: 50px; font-family: 'Garamond', Verdana, Halvetica, Arial;">
                            MiniWebRadio
                        </label>
                    </center>
                </td>
            </tr>
        </table> 
        <hr>
        <center>
        <p>You can edit the configuration here. <i>Note that this will be overwritten by "Load default".</i></p>

        <textarea wrap="off" rows="25" id="prefs">Space for preferences</textarea>
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
    <!--==============================================================================================-->
    <div id="tab-content3">
        <table width=100%>
            <tr>
                <td style="width:300px;">
                    <left>
                        <img src="SD/png/Radio_Green.png"       alt="radio"     onclick="showTab1()"/>
                        <img src="SD/png/Station_Green.png"     alt="station"   onclick="showTab2()"/>
                        <img src="SD/png/MP3_Yellow.png"        alt="mp3"                           />
                <!--    <img src="SD/png/Settings_Green.png"    alt="settings"  onclick="showTab4()"/> -->
                        <img src="SD/png/About_Green.png" alt="radio"           onclick="showTab5()"/>
                    </left>
                </td>
                <td>
                    <center>
                        <label style="font-size: 50px; font-family: 'Garamond', Verdana, Halvetica, Arial;">
                            MiniWebRadio
                        </label>
                    </center>
                </td>
            </tr>
        </table> 
        <hr>
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
    <!--==============================================================================================-->
    <div id="tab-content4">
        <table width=100%>
            <tr>
                <td style="width:300px;">
                    <left>
                        <img src="SD/png/Radio_Green.png"       alt="radio"     onclick="showTab1()"/>
                        <img src="SD/png/Station_Green.png"     alt="station"   onclick="showTab2()"/> 
                        <img src="SD/png/MP3_Green.png"         alt="mp3"       onclick="showTab3()"/>
                <!--    <img src="SD/png/Settings_Yellow.png"   alt="settings"                      /> -->
                        <img src="SD/png/About_Green.png"       alt="radio"     onclick="showTab5()"/>
                    </left>
                </td>
                <td>
                    <center>
                        <label style="font-size: 50px; font-family: 'Garamond', Verdana, Halvetica, Arial;">
                            MiniWebRadio
                        </label>
                    </center>
                </td>
            </tr>
        </table> 
        <hr>

    </div>
    <!--==============================================================================================-->
    <div id="tab-content5">
        <table width=100%>
            <tr>
                <td style="width:300px;">
                    <left>
                        <img src="SD/png/Radio_Green.png"       alt="radio"     onclick="showTab1()"/>
                        <img src="SD/png/Station_Green.png"     alt="station"   onclick="showTab2()"/> 
                        <img src="SD/png/MP3_Green.png"         alt="mp3"       onclick="showTab3()"/>
                <!--    <img src="SD/png/Settings_Green.png"    alt="settings"  onclick="showTab4()"/> -->
                        <img src="SD/png/About_Yellow.png"      alt="radio"                         />
                    </left>
                </td>
                <td>
                    <center>
                        <label style="font-size: 50px; font-family: 'Garamond', Verdana, Halvetica, Arial;">
                            MiniWebRadio
                        </label>
                    </center>
                </td>
            </tr>
        </table> 
        <hr>
        <p> MiniWebRadio -- Webradio receiver for ESP32, 2.8" color display and VS1053 MP3 module.<br>
        This project is documented at <a target="blank" href="https://github.com/schreibfaul1/ESP32-MiniWebRadio">Github</a>.</p>
        <p>Author: Wolle (schreibfaul1)<br>
        <img src="SD/MiniWebRadio_gr.bmp" alt="MiniWebRadio_gr" border=3>
        <h3>Connected WiFi network
        <select class="select" onChange="handletone(this)" id="ssid"></select>
        </h3>
    </div>
    <!--==============================================================================================-->

</div>

</body>
</html>
<noscript>
  Sorry, MiniWebRadio does not work without JavaScript!
</noscript> 
    
)=====" ;



#endif /* WEB_H_ */
