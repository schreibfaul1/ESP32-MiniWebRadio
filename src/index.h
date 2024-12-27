/*
 *  index.h
 *
 *  Created on: 04.10.2018
 *  Updated on: 14.12.2024
 *      Author: Wolle
 *
 *  successfully tested with Chrome and Firefox
 *
 */

#ifndef INDEX_H_
#define INDEX_H_

#include "Arduino.h"

// file in raw data format for PROGMEM
//

const char index_html[] PROGMEM = R"=====(
<!DOCTYPE HTML>
<html>
<head>
    <title>MiniWebRadio</title>
    <meta name="generator" content="Bluefish 2.2.10" >
    <meta name="author" content="Wolle" >
    <meta name="date" content="2019-10-11T20:07:54+0200" >
    <meta name="copyright" content="schreibfaul1">
    <meta name="keywords" content="">
    <meta name="description" content="index">
    <meta name="ROBOTS" content="NOINDEX, NOFOLLOW">
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <meta http-equiv="content-style-type" content="text/css">
    <meta http-equiv="expires" content="0">

    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/jqueryui/1.12.1/jquery-ui.min.css">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/jsgrid/1.5.3/jsgrid.min.css">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/jsgrid/1.5.3/jsgrid-theme.min.css">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/toastr.js/latest/css/toastr.css">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/jstree/3.2.1/themes/default/style.min.css">

<!--   <link rel="stylesheet" href="SD/css/jstree-style.css">  -->
<!--   <link rel="stylesheet" href="SD/css/jquery-ui.css">     -->
<!--   <link rel="stylesheet" href="SD/css/jsgrid.css">        -->
<!--   <link rel="stylesheet" href="SD/css//jsgrid-theme.css"> -->

    <script src="https://cdnjs.cloudflare.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/jsgrid/1.5.3/jsgrid.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/FileSaver.js/1.3.8/FileSaver.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/toastr.js/latest/toastr.min.js"></script>
    <script src="https://code.jquery.com/ui/1.12.0/jquery-ui.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/jstree/3.2.1/jstree.min.js"></script>

<!--  <script src="SD/js/jstree.js"></script>    -->
<!--  <script src="SD/js/jquery.js"></script>    -->
<!--  <script src="SD/js/jquery-ui.js"></script> -->
<!--  <script src="SD/js/jsgrid.js"></script>    -->
<!--  <script src="SD/js/FileSaver.js"></script> -->


    <style>           /* optimized with csstidy */
        html {  /* This is the groundplane */
            font-family : serif;
            height : 100%;
            font-size: 16px;
            color : DarkSlateGray;
            background-color : navy;
            margin : 0;
            padding : 0;
        }
        #dialog {
            display: none;
        }
        #content {
            min-height : 550px;
            min-width : 725px;
            overflow : hidden;
            background-color : lightskyblue;
            margin : 0;
            padding : 5px;
        }
        #tab-content1 {
            font-size: 15px;
            display : block;
            padding : 5px 0;
            max-width: 100%;
            height: 440px;
        }
        #tab-content2 {
            display : none;
            margin : 20px 20px 0px 20px;
        }
        #tab-content3 {
            display : none;
            margin : 20px;
        }
        #tab-content4 {
            display : none;
            margin : 20px;
        }
        #tab-content5 {
            display : none;
            margin : 20px;
        }
        #tab-content6 {
            display : none;
            margin : 20px;
        }
        #tab-content7 {
            display : none;
            margin : 20px;
        }
        #tab-content8 {
            display : none;
            margin : 20px;
        }
        #tab-content9 {
            display : none;
            margin : 20px;
        }
        .button_120x30 {
            width : 120px;
            height : 30px;
            background-color : #128F76;
            border : none;
            color : #FFF;
            text-align : center;
            text-decoration : none;
            font-size : 16px;
            cursor : pointer;
            border-radius : 5px;
            margin : 4px 2px 0 0;
            padding : 0;
        }
        .button_20x36 {
            width : 20px;
            height : 36px;
            background-color : #128F76;
            border : none;
            color : #FFF;
            text-align : center;
            text-decoration : none;
            display : inline-block;
            font-size : 16px;
            cursor : pointer;
            border-radius : 5px;
            margin-top: 3px;
            margin-left: 0px;
            margin-right: 10px;
        }
        .buttonblue {
            background-color : blue;
        }
        .buttongreen {
            background-color : #128F76;
        }
        .buttonred {
            background-color : red;
        }
        #label-logo {
            margin-left : 40px;
            border-color: black;
            border-style: solid;
            border-width: 2px;
            display : inline-block;
            width : 128px;
            height : 128px;
            margin-top: 5px;
            background-image : url(SD/common/unknown.jpg);
        }
        #label-bt-logo {
            margin-left : 4px;
            border-color: #99ccff;
            border-style: solid;
            border-width: 2px;
            display : inline-block;
            background-image : url(SD/png/BT.png);
            width : 128px;
            height : 128px;
            margin-top: 5px;
        }
        #label-bt-mode {
            margin-left : 4px;
            border-color: #99ccff;
            border-style: solid;
            border-width: 2px;
            display : inline-block;
            width : 128px;
            height : 38px;
            margin-top: 6px;
            font-size : 18px;
            font-weight: bold;
            text-align: center;
        }
        #label-infopic {
            margin-left : 4px;
            border-color: #99ccff;
            border-style: solid;
            border-width: 2px;
            display : inline-block;
            background-image : url(SD/png/MiniWebRadioV3.png);
            width : 480px;
            height : 320px;
            margin-top: 5px;
        }
        canvas {
            left : 0;
            margin-left : 0;
            display : inline-block;
            /* width : 96px;
               height : 96px; */
            border : #000 solid 2px;
        }
        .ui-widget-header {
            background : #11e9e9 !important ;
        }
        .ui-dialog .ui-dialog-buttonpane {
            margin-top : 0 !important ;
            border-width : 0 !important ;
            padding : 0 0 0 1em !important ;
        }
        .ui-dialog .ui-dialog-content {
            margin-top : 0.3em !important ;
            padding : 0.1em !important ;
        }
        .boxstyle {
            height : 36px;
            padding-top : 0;
            padding-left : 5px;
            padding-bottom : 0;
            background-color: white;
            font-size : 16px;
            line-height : normal;
            border-color: black;
            border-style: solid;
            border-width: thin;
            border-radius : 5px;
        }
        .boxstyle_s {
            height : 36px;
            width : 40px;
            padding-top : 0;
            padding-left : 5px;
            padding-bottom : 2px;
            background-color: white;
            font-size : 16px;
            line-height : normal;
            border-color: black;
            border-style: solid;
            border-width: thin;
            border-radius : 5px;
        }
        .boxstyle_m {
            height : 36px;
            width : 280px;
            padding-top : 0;
            padding-left : 5px;
            padding-bottom : 2px;
            background-color: white;
            font-size : 16px;
            line-height : normal;
            border-color: black;
            border-style: solid;
            border-width: thin;
            border-radius : 5px;
        }
        .boxstyle_l {
            height : 36px;
            width : 500px;
            padding-top : 0;
            padding-left : 5px;
            padding-bottom : 2px;
            background-color: white;
            font-size : 16px;
            line-height: normal;
            border-color: black;
            border-style: solid;
            border-width: thin;
            border-radius: 5px;
            margin-bottom: 4px;
        }
        .boxstyle_200x36 {
            height : 36px;
            width : 200px;
            padding-top : 0;
            padding-left : 5px;
            padding-bottom : 2px;
            background-color: white;
            font-size : 16px;
            line-height : normal;
            border-color: black;
            border-style: solid;
            border-width: thin;
            border-radius : 5px;
            margin-right: 0px;
        }
        .table_cell1 {
            padding-top : 0;
            padding-left : 5px;
            font-size : 16px;
        }
        .table_cell2 {
            padding-top : 0;
            padding-left : 5px;
            font-size : 15px;
        }
        .sdr_lbl_left {
            display: inline-block;
            float: left;
            text-align:right;
            height: 40px;
            width: 100px;
            padding-top: 0px;
            padding-right: 5px;
            padding-bottom: 0px;
        }
        .sdr_lbl_right {
            display: inline-block;
            float: left;
            text-align:right;
            height: 40px;
            width: 45px;
            padding-top: 0px;
            padding-left: 5px;
            padding-bottom: 0px;
        }
        .sdr_lbl_measure {
            display: inline-block;
            float: left;
            text-align:left;
            height: 40px;
            width: 40px;
            padding-top: 0px;
            padding-bottom: 0px;
        }
        #preloaded-images{
            display: none;
        }
        #dialog {
            display: none;
        }
        #BODY {
            display:block;
        }
        .filetree-container {
            position: relative;
            background-color: white;
            height: 420px !important;
            overflow: auto;
        }
        .progress-bar{
            display:-ms-flexbox;
            display:flex;
            -ms-flex-direction:column;
            flex-direction:column;
            -ms-flex-pack:center;
            justify-content:center;
            overflow:hidden;
            color:#fff;
            text-align:center;
            white-space:nowrap;
            background-color:#007bff;
            transition:width
        }
        .stations-container {
            width: 100%;
            max-width: 100vw;
            overflow-x: auto;
            margin-bottom: 20px;
            border: 1px solid #ccc;
        }

        .stations-table {
            width: 100%;
            border-collapse: collapse;
        }

        .stations-th{
            padding-left: 5px;
            padding-right: 0px;
            padding-top: 5px;
            padding-bottom: 5px;
            text-align: left;
            border: 1px solid black;
            white-space: nowrap;
            cursor: pointer;
            height: 25px;
        }

        .stations_tr{
            padding: 0;
        }
        .table-row.highlight {
            background-color: #f0edcc !important; /* Wichtig! Überschreibt die Hintergrundfarbe */
        }

        /* Kontextmenü-Stil */
        .context-menu {
            display: none;
            position: absolute;
            z-index: 1000;
            width: 150px;
            background-color: white;
            border: 1px solid #ccc;
            box-shadow: 2px 2px 12px rgba(0, 0, 0, 0.2);
        }

        .context-menu-item {
            padding: 10px;
            cursor: pointer;
        }

        .context-menu-item:hover {
            background-color: #f2f2f2;
        }

        .notification1, notification2 {
            position: fixed; /* Fixiert das Fenster relativ zum Ansichtsfenster */
            top: 20px; /* Abstand von oben */
            right: 20px; /* Abstand von rechts */
            background-color: #444; /* Dunkler Hintergrund */
            color: white; /* Weiße Schrift */
            padding: 15px; /* Innenabstand */
            border-radius: 5px; /* Abgerundete Ecken */
            box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.1); /* Schatten */
            display: none; /* Anfangs versteckt */
            z-index: 1000; /* Sicherstellen, dass es über anderen Elementen liegt */
        }
    </style>
</head>

<script>

// global variables and functions
/* eslint-disable no-unused-vars, no-undef */
var I2S_eq_DB = ['-40', '-37', '-34', '-31', '-28', '-25', '-22', '-19',
  '-16', '-13', '-10', ' -7', ' -4', '  0', ' +3', ' +6']

var I2S_eq_Val = [-40, -37, -34, -31, -28, -25, -22, -19, -16, -13, -10, -7, -4, 0, +3, +6]

var tft_size = 0        // (0)320x240, (1)480x320
var ir_buttons


// ---- websocket section------------------------

var socket = undefined
var host = location.hostname
var tm
var IR_addr = ""
let ir_arr = new Array(23);
var bt_RxTx = 'TX'
var state = 'RADIO'
var cur_volumeSteps = 21


function ping() {
    if (socket.readyState == 1) { // reayState 'open'
        socket.send("ping")
        console.log("send ping")
        tm = setTimeout(function () {
            toastr.warning('The connection to the MiniWebRadio is interrupted! Please reload the page!')
        }, 40000)
    }
}

function connect() {

    if (socket) {
        // Prüfe, ob der Socket noch geöffnet oder im Verbindungsaufbau ist
        // if (socket.readyState === WebSocket.OPEN || socket.readyState === WebSocket.CONNECTING) {
            socket.close(); // Schließe die bestehende Verbindung
        //}
    }

    socket = new WebSocket('ws://'+window.location.hostname+':81/');

    socket.onopen = function () {
        console.log("Websocket connected")
        socket.send('get_tftSize')
        socket.send('to_listen');
        socket.send("getmute")
        socket.send("get_timeAnnouncement")
        socket.send("gettone=")   // Now load the tones (tab Radio)
        socket.send("getnetworks=")
        socket.send("change_state=" + "RADIO")
        socket.send("getTimeFormat")
        socket.send("getSleepMode")
        setInterval(ping, 20000)
        loadStationsFromSD("/stations.json")

        loadFileFromSD("/ir_buttons.json", "application/json")
            .then(data => {ir_buttons = data;});
    };

    socket.onclose = function (e) {
        console.log(e)
        console.log('Socket is closed. Reconnect will be attempted in 1 second.', e)
        socket = null
        setTimeout(function () {
            connect()
        }, 1000)
    }

    socket.onerror = function (err) {
        console.log(err)
    }

    socket.onmessage = function(event) {
        var socketMsg = event.data

        var n   = socketMsg.indexOf('=')
        var msg = ''
        var val = ''
        if (n >= 0) {
            var msg  = socketMsg.substring(0, n)
            var val  = socketMsg.substring(n + 1)
            console.log("para ",msg, " val ",val)
        }
        else {
            msg = socketMsg
        }

        switch(msg) {
            case "pong":                clearTimeout(tm)
                                        console.log("pong")
                                        toastr.clear()
                                        break
            case "mute":                if(val == '1'){ document.getElementById('Mute').src = 'SD/png/Button_Mute_Red.png'
                                                        resultstr1.value = "mute on"
                                                        console.log("mute on")}
                                        if(val == '0'){ document.getElementById('Mute').src = 'SD/png/Button_Mute_Green.png'
                                                        resultstr1.value = "mute off"
                                                        console.log("mute off")}
                                        break
            case "tone":                resultstr1.value = val  // text shown in resultstr1 as info
                                        break
            case "settone":             lines = val.split('\n')
                                        for (i = 0; i < (lines.length - 1); i++) {
                                          parts = lines[i].split('=')
                                          setSlider(parts[0], parts[1])
                                        }
                                        break
            case "stationNr":           preselectStationList(val)
                                        break
            case "stationURL":          station.value = val
                                        break
            case "stationLogo":         showLogo1('label-logo', val)
                                        break
            case "hardcopy":            showLogo1('label-infopic', val)
            case "streamtitle":         cmd.value = val
                                        break
            case "homepage":            window.open(val, '_blank') // show the station homepage
                                        break
            case "icy_description":     resultstr1.value = val
                                        break
            case "AudioFileList":       getAudioFileList(val)
                                        break
            case "tftSize":             if(val == 's')  { tft_size = 0; // 320x240px
                                                            document.getElementById('canvas').width  = 96;
                                                            document.getElementById('canvas').height = 96;
                                                            console.log("tftSize is s");
                                        }
                                        if(val == 'm')  { tft_size = 1;
                                                            document.getElementById('canvas').width  = 128;
                                                            document.getElementById('canvas').height = 128;
                                                            console.log("tftSize is m");
                                        }
                                        break
            case  "volume":             resultstr1.value = "Volume is now " + val;
                                        break
            case  "volumeSteps":        console.log("volumeSteps: ", val);
                                        showVolumeSteps(val)
                                        cur_volumeSteps = val
                                        loadRingVolume()
                                        loadVolumeAfterAlarm()
                                        break
            case  "ringVolume":         console.log("ringvolume: ", val);
                                        showRingvolume(val)
                                        break
            case  "volAfterAlarm":      console.log("volumeAfterAlarm: ", val);
                                        showVolumeAfterAlarm(val)
                                        break
            case  "SD_playFile":        resultstr3.value = "Audiofile is " + val;
                                        break
            case  "SD_playFolder":      resultstr3.value = "play all: " + val;
                                        console.log("Audiofolder: ", val);
                                        break
            case  "stopfile":           resultstr3.value = val;
                                        break
            case  "resumefile":         resultstr3.value = val;
                                        break
            case  "timeAnnouncement":   console.log("timeAnnouncement=" + val)
                                        if(val == '0') document.getElementById('chk_timeSpeech').checked = false;
                                        if(val == '1') document.getElementById('chk_timeSpeech').checked = true;
                                        break
            case "getTimeSpeechLang":   console.log(val)
                                        select = document.getElementById('timeSpeechLang')
                                        for (let i = 0; i < select.options.length; i++) {
                                            if (select.options[i].text === val) {
                                                select.selectedIndex = i;
                                                break;
                                            }
                                        }
                                        break
            case "DLNA_Names":          addDLNAServer(val) // add to Serverlist
                                        break
            case "dlnaContent":         console.log(val)
                                        show_DLNA_Content(val)
                                        break
            case "networks":            var networks = val.split('\n')
                                        console.log(networks[i])
                                        // select = document.getElementById('ssid')
                                        // for (i = 0; i < (networks.length); i++) {
                                        //     opt = document.createElement('OPTION')
                                        //     opt.value = i
                                        //     opt.text = networks[i]
                                        //     select.add(opt)
                                        // }
                                        break
            case "test":                resultstr1.value = val
                                        break
            case "IR_address":          if(state === 'IR' && IR_addr != val){
                                            IR_addr = val
                                            ir_command_A.value=val
                                            socket.send("setIRadr=" + val)
                                        }
                                        break
            case "IR_command":          ir_command_C.value=val
                                        break
            case "timeFormat":          var radiobtn;
                                        if     (val == '12') radiobtn = document.getElementById("h12")
                                        else if(val == '24') radiobtn = document.getElementById("h24")
                                        else{console.log("wrong timeFormat ", val); break;}
                                        radiobtn.checked = true;
                                        break;
            case "sleepMode":           if  (val == "0") radiobtn = document.getElementById("sleepMode0")
                                        else if(val == '1') radiobtn = document.getElementById("sleepMode1")
                                        radiobtn.checked = true;
                                        break;
            case "changeState":         if (val == 'RADIO' && state != 'RADIO') showTab1();
                                        if (val == 'PLAYER'&& state != 'PLAYER') showTab3();
                                        if (val == 'BLUETOOTH'&& state != 'BT') showTab9();
                                        if (val == 'IR_SETTINGS' && state != 'IR') showTab8();
                                        break;
            case "KCX_BT_connected":    console.log(msg, val)
                                        if(val == '-1') {showLogo1('label-bt-logo', '/png/BT_off.png');}
                                        if(val == '0')  {showLogo1('label-bt-logo', '/png/BT.png');}
                                        if(val == '1' && bt_RxTx == 'TX') {showLogo1('label-bt-logo', '/png/BT_TX.png');}
                                        if(val == '1' && bt_RxTx == 'RX') {showLogo1('label-bt-logo', '/png/BT_RX.png');}
                                        break;
            case "KCX_BT_power":        if(val == '1'){ document.getElementById('BT_Power').src = 'SD/png/BT_Blue.png'
                                                        console.log("BT Power on")}
                                        if(val == '0'){ document.getElementById('BT_Power').src = 'SD/png/BT_Red.png'
                                                        console.log("BT Power off")}
                                        break;
            case "KCX_BT_MEM":          show_BT_memItems(val)
                                        break;
            case "KCX_BT_SCANNED":      show_BT_scannedItems(val)
                                        break;
            case "KCX_BT_MODE":         console.log(msg, val)
                                        if(val ==="TX"){
                                            document.getElementById('label-bt-mode').innerHTML= "EMITTER"
                                            bt_RxTx = 'TX'
                                        }
                                        if(val ==="RX"){
                                            document.getElementById('label-bt-mode').innerHTML= "RECEIVER"
                                            bt_RxTx = 'RX'
                                        }
                                        break;
            default:                    console.log('unknown message', msg, val)
        }
    }
}
// ---- end websocket section------------------------


document.addEventListener('readystatechange', event => {
    if (event.target.readyState === 'interactive') { // same as:  document.addEventListener('DOMContentLoaded'...
        // same as  jQuery.ready
        console.log('All HTML DOM elements are accessible')
        document.getElementById('dialog').style.display = 'none' // hide the div (its only a template)
    }
    if (event.target.readyState === 'complete') {
        console.log('Now external resources are loaded too, like css,src etc... ')
        connect();  // establish websocket connection
        audioPlayer_buildFileSystemTree("/")
        dlnaPlayer_buildFileSystemTree("/")
    }
})

toastr.options = {
    "closeButton": false,
    "debug": false,
    "newestOnTop": false,
    "progressBar": false,
    "positionClass": "toast-top-right",
    "preventDuplicates": true,
    "preventOpenDuplicates": true,
    "onclick": null,
    "showDuration": "300",
    "hideDuration": "1000",
    "timeOut": "20000",
    "extendedTimeOut": "1000",
    "showEasing": "swing",
    "hideEasing": "linear",
    "showMethod": "fadeIn",
    "hideMethod": "fadeOut"
}

// Reload images from invisible tabs when the page is fully loaded
window.onload = function () {
    const allLazyImages = document.querySelectorAll('img[data-src]');
    allLazyImages.forEach(img => {
        img.src = img.dataset.src; // load the rest of the images
        console.log("load image", img.src)
        img.removeAttribute('data-src'); // remove data-src
    });
};


function showTab1 () {
    state = 'RADIO'
    console.log('tab-content1 (Radio)')
    document.getElementById('tab-content1').style.display = 'block'
    document.getElementById('tab-content2').style.display = 'none'
    document.getElementById('tab-content3').style.display = 'none'
    document.getElementById('tab-content4').style.display = 'none'
    document.getElementById('tab-content5').style.display = 'none'
    document.getElementById('tab-content6').style.display = 'none'
    document.getElementById('tab-content7').style.display = 'none'
    document.getElementById('tab-content8').style.display = 'none'
    document.getElementById('tab-content9').style.display = 'none'
    document.getElementById('btn1').src = 'SD/png/Radio_Yellow.png'
    document.getElementById('btn2').src = 'SD/png/Station_Green.png'
    document.getElementById('btn3').src = 'SD/png/MP3_Green.png'
    document.getElementById('btn4').src = 'SD/png/Button_DLNA_Green.png'
    document.getElementById('btn5').src = 'SD/png/Search_Green.png'
    document.getElementById('btn6').src = 'SD/png/Settings_Green.png'
    document.getElementById('btn7').src = 'SD/png/About_Green.png'
    socket.send("change_state=" + "RADIO")
    socket.send("getmute")
}

function showTab2 () {
    state = 'STATIONS'
    console.log('tab-content2 (Stations)')
    document.getElementById('tab-content1').style.display = 'none'
    document.getElementById('tab-content2').style.display = 'block'
    document.getElementById('tab-content3').style.display = 'none'
    document.getElementById('tab-content4').style.display = 'none'
    document.getElementById('tab-content5').style.display = 'none'
    document.getElementById('tab-content6').style.display = 'none'
    document.getElementById('tab-content7').style.display = 'none'
    document.getElementById('tab-content8').style.display = 'none'
    document.getElementById('tab-content9').style.display = 'none'
    document.getElementById('btn1').src = 'SD/png/Radio_Green.png'
    document.getElementById('btn2').src = 'SD/png/Station_Yellow.png'
    document.getElementById('btn3').src = 'SD/png/MP3_Green.png'
    document.getElementById('btn4').src = 'SD/png/Button_DLNA_Green.png'
    document.getElementById('btn5').src = 'SD/png/Search_Green.png'
    document.getElementById('btn6').src = 'SD/png/Settings_Green.png'
    document.getElementById('btn7').src = 'SD/png/About_Green.png'
}

function showTab3 () {
    state = 'PLAYER'
    console.log('tab-content3 (Audio Player)')
    document.getElementById('tab-content1').style.display = 'none'
    document.getElementById('tab-content2').style.display = 'none'
    document.getElementById('tab-content3').style.display = 'block'
    document.getElementById('tab-content4').style.display = 'none'
    document.getElementById('tab-content5').style.display = 'none'
    document.getElementById('tab-content6').style.display = 'none'
    document.getElementById('tab-content7').style.display = 'none'
    document.getElementById('tab-content8').style.display = 'none'
    document.getElementById('tab-content8').style.display = 'none'
    document.getElementById('btn1').src = 'SD/png/Radio_Green.png'
    document.getElementById('btn2').src = 'SD/png/Station_Green.png'
    document.getElementById('btn3').src = 'SD/png/MP3_Yellow.png'
    document.getElementById('btn4').src = 'SD/png/Button_DLNA_Green.png'
    document.getElementById('btn5').src = 'SD/png/Search_Green.png'
    document.getElementById('btn6').src = 'SD/png/Settings_Green.png'
    document.getElementById('btn7').src = 'SD/png/About_Green.png'
    socket.send("change_state=" + "PLAYER")
}

function showTab4 () {
    state = 'DLNA'
    console.log('tab-content4 (DLNA)')
    document.getElementById('tab-content1').style.display = 'none'
    document.getElementById('tab-content2').style.display = 'none'
    document.getElementById('tab-content3').style.display = 'none'
    document.getElementById('tab-content4').style.display = 'block'
    document.getElementById('tab-content5').style.display = 'none'
    document.getElementById('tab-content6').style.display = 'none'
    document.getElementById('tab-content7').style.display = 'none'
    document.getElementById('tab-content8').style.display = 'none'
    document.getElementById('tab-content9').style.display = 'none'
    document.getElementById('btn1').src = 'SD/png/Radio_Green.png'
    document.getElementById('btn2').src = 'SD/png/Station_Green.png'
    document.getElementById('btn3').src = 'SD/png/MP3_Green.png'
    document.getElementById('btn4').src = 'SD/png/Button_DLNA_Yellow.png'
    document.getElementById('btn5').src = 'SD/png/Search_Green.png'
    document.getElementById('btn6').src = 'SD/png/Settings_Green.png'
    document.getElementById('btn7').src = 'SD/png/About_Green.png'
    clearDLNAServerList(0)
    socket.send('DLNA_getServer')
    socket.send("change_state=" + "DLNA")
}

function showTab5 () {
    state = 'SEARCH'
    console.log('tab-content5 (Search Stations)')
    document.getElementById('tab-content1').style.display = 'none'
    document.getElementById('tab-content2').style.display = 'none'
    document.getElementById('tab-content3').style.display = 'none'
    document.getElementById('tab-content4').style.display = 'none'
    document.getElementById('tab-content5').style.display = 'block'
    document.getElementById('tab-content6').style.display = 'none'
    document.getElementById('tab-content7').style.display = 'none'
    document.getElementById('tab-content8').style.display = 'none'
    document.getElementById('tab-content9').style.display = 'none'
    document.getElementById('btn1').src = 'SD/png/Radio_Green.png'
    document.getElementById('btn2').src = 'SD/png/Station_Green.png'
    document.getElementById('btn3').src = 'SD/png/MP3_Green.png'
    document.getElementById('btn4').src = 'SD/png/Button_DLNA_Green.png'
    document.getElementById('btn5').src = 'SD/png/Search_Yellow.png'
    document.getElementById('btn6').src = 'SD/png/Settings_Green.png'
    document.getElementById('btn7').src = 'SD/png/About_Green.png'
    document.getElementById("RB_search").value = "";
}

function showTab6 () {
    state = 'SETTINGS'
    console.log('tab-content5 (Search Stations)')
    document.getElementById('tab-content1').style.display = 'none'
    document.getElementById('tab-content2').style.display = 'none'
    document.getElementById('tab-content3').style.display = 'none'
    document.getElementById('tab-content4').style.display = 'none'
    document.getElementById('tab-content5').style.display = 'none'
    document.getElementById('tab-content6').style.display = 'block'
    document.getElementById('tab-content7').style.display = 'none'
    document.getElementById('tab-content8').style.display = 'none'
    document.getElementById('tab-content9').style.display = 'none'
    document.getElementById('btn1').src = 'SD/png/Radio_Green.png'
    document.getElementById('btn2').src = 'SD/png/Station_Green.png'
    document.getElementById('btn3').src = 'SD/png/MP3_Green.png'
    document.getElementById('btn4').src = 'SD/png/Button_DLNA_Green.png'
    document.getElementById('btn5').src = 'SD/png/Search_Green.png'
    document.getElementById('btn6').src = 'SD/png/Settings_Yellow.png'
    document.getElementById('btn7').src = 'SD/png/About_Green.png'
    // getTimeZoneName()
    loadTimeZones()
    loadRingVolume()
    loadVolumeAfterAlarm()
    loadVolumeSteps()
    socket.send('getRingVolume')
    socket.send('getVolAfterAlarm')
    socket.send("getTimeSpeechLang")
}

function showTab7 () {
    state = 'ABOUT'
    console.log('tab-content6 (About)')
    document.getElementById('tab-content1').style.display = 'none'
    document.getElementById('tab-content2').style.display = 'none'
    document.getElementById('tab-content3').style.display = 'none'
    document.getElementById('tab-content4').style.display = 'none'
    document.getElementById('tab-content5').style.display = 'none'
    document.getElementById('tab-content6').style.display = 'none'
    document.getElementById('tab-content7').style.display = 'block'
    document.getElementById('tab-content8').style.display = 'none'
    document.getElementById('tab-content9').style.display = 'none'
    document.getElementById('btn1').src = 'SD/png/Radio_Green.png'
    document.getElementById('btn2').src = 'SD/png/Station_Green.png'
    document.getElementById('btn3').src = 'SD/png/MP3_Green.png'
    document.getElementById('btn4').src = 'SD/png/Button_DLNA_Green.png'
    document.getElementById('btn5').src = 'SD/png/Search_Green.png'
    document.getElementById('btn6').src = 'SD/png/Settings_Green.png'
    document.getElementById('btn7').src = 'SD/png/About_Yellow.png'
}

function showTab8 () {  // Remote Control
    state = 'IR'
    console.log('tab-content7 (Remote Control)')
    document.getElementById('tab-content1').style.display = 'none'
    document.getElementById('tab-content2').style.display = 'none'
    document.getElementById('tab-content3').style.display = 'none'
    document.getElementById('tab-content4').style.display = 'none'
    document.getElementById('tab-content5').style.display = 'none'
    document.getElementById('tab-content6').style.display = 'none'
    document.getElementById('tab-content7').style.display = 'none'
    document.getElementById('tab-content8').style.display = 'block'
    document.getElementById('tab-content9').style.display = 'none'
    document.getElementById('btn1').src = 'SD/png/Radio_Green.png'
    document.getElementById('btn2').src = 'SD/png/Station_Green.png'
    document.getElementById('btn3').src = 'SD/png/MP3_Green.png'
    document.getElementById('btn4').src = 'SD/png/Button_DLNA_Green.png'
    document.getElementById('btn5').src = 'SD/png/Search_Green.png'
    document.getElementById('btn6').src = 'SD/png/Settings_Green.png'
    document.getElementById('btn7').src = 'SD/png/About_Green.png'
    loadFileFromSD("/ir_buttons.json", "application/json")
        .then(data => {ir_buttons = data;});
    writeJSONToTable(ir_buttons)
    socket.send("change_state=" + "IR_SETTINGS")
}

function showTab9 () {  // KCX BT Emitter
    state = 'BT'
    console.log('tab-content8 (Remote Control)')
    document.getElementById('tab-content1').style.display = 'none'
    document.getElementById('tab-content2').style.display = 'none'
    document.getElementById('tab-content3').style.display = 'none'
    document.getElementById('tab-content4').style.display = 'none'
    document.getElementById('tab-content5').style.display = 'none'
    document.getElementById('tab-content6').style.display = 'none'
    document.getElementById('tab-content7').style.display = 'none'
    document.getElementById('tab-content7').style.display = 'none'
    document.getElementById('tab-content9').style.display = 'block'
    document.getElementById('btn1').src = 'SD/png/Radio_Green.png'
    document.getElementById('btn2').src = 'SD/png/Station_Green.png'
    document.getElementById('btn3').src = 'SD/png/MP3_Green.png'
    document.getElementById('btn4').src = 'SD/png/Button_DLNA_Green.png'
    document.getElementById('btn5').src = 'SD/png/Search_Green.png'
    document.getElementById('btn6').src = 'SD/png/Settings_Green.png'
    document.getElementById('btn7').src = 'SD/png/About_Green.png'
    socket.send("change_state=" + "BLUETOOTH")
    socket.send('KCX_BT_connected')  // is connected?
    socket.send('KCX_BT_scanned')    // get scanned items
    socket.send('KCX_BT_mem')        // get saved items
    socket.send('KCX_BT_getMode')    // get mode (TX or RX)
    socket.send('KCX_BT_getPower')   // get power state
}


function saveTextFileToSD (fileName, content) {
    var fd = new FormData()
    fd.append('Text=', content)
    var theUrl = 'uploadfile?' + fileName + '&version=' + Math.random()
    var xhr = new XMLHttpRequest()
    xhr.timeout = 2000; // time in milliseconds
    xhr.open('POST', theUrl, true)
    xhr.ontimeout = (e) => {
        // XMLHttpRequest timed out.
        alert(filename + ' not uploaded, timeout')
    }
    xhr.onreadystatechange = function () { // Call a function when the state changes.
        if (xhr.readyState === 4) {
            if (xhr.status === 200) alert(fileName + ' successfully uploaded')
            else alert(fileName + 'not successfully uploaded')
        }
    }
    xhr.send(fd) // send
}

async function saveJsonFileToSD(filename, content) {
    try {
        const cacheBuster = new Date().getTime();
        const response = await fetch("SD_Upload?" + encodeURIComponent(filename) + "&cb=" + cacheBuster, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'X-Filename': filename
            },
            body: (content),
        });
        if (!response.ok) {
            throw new Error('Error saving the file: ' + response.statusText);
        }
        const result = await response.text();
        console.log('File saved successfully:', result);
    } catch (error) {
        console.error('There was a problem: ', error);
    }
}

async function loadFileFromSD(file_name, content_type) {
    var ct  = 'Content-Type: ' + content_type;
    try {
        const cacheBuster = new Date().getTime();
        const response = await fetch("SD_Download?" + encodeURIComponent(file_name) + "&cb=" + cacheBuster, {
            method: 'GET',
            headers: {
                ct
            }
        });
        if (!response.ok) {
            throw new Error(`Error loading the file: ${response.statusText}`);
        }
        const content = await response.text();
        if (content) {
            console.log('File loaded successfully:', content);
            return content; // Rückgabewert
        } else {
            return [""];
        }
    } catch (error) {
        console.error('Es gab ein Problem beim Laden der Datei:', error);
        return [""];
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------- TAB RADIO -------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------
function showLogo1(id, src) { // get the bitmap from SD, convert to URL first
    const myImg = document.getElementById(id);
    if(src == '') src = '/common/unknown.jpg'
    src = "/SD" + src
    src += "?" + new Date().getTime()
    console.log("showLogo1 id=", id, "src=", src)
    myImg.src = src;
}

function test(){
    socket.send("test=")
}

function handleStation (presctrl) { // tab Radio: preset, select a station
    cmd.value = ''
    console.log(presctrl.value)
    socket.send('set_station=' + presctrl.value)
}

function setstation () { // Radio: button play - Enter a streamURL here....
    var sel = document.getElementById('preset')
    sel.selectedIndex = 0
    cmd.value = ''
    var theUrl =  station.value;
    theUrl = theUrl.replace(/%3d/g, '=') // %3d convert to =
    theUrl = theUrl.replace(/%21/g, '!') //
    theUrl = theUrl.replace(/%22/g, '"') //
    theUrl = theUrl.replace(/%23/g, '#') //
    theUrl = theUrl.replace(/%3f/g, '?') //
    theUrl = theUrl.replace(/%40/g, '@') //
    socket.send("stationURL=" + theUrl)
}

function setSlider (elmnt, value) {
    console.log("setSlider", elmnt, value)
    if (elmnt === 'LowPass' ) { v = Math.trunc((40 + parseInt(value, 10)) /3); slider_LP_set(v); }
    if (elmnt === 'BandPass') { v = Math.trunc((40 + parseInt(value, 10)) /3); slider_BP_set(v); }
    if (elmnt === 'HighPass') { v = Math.trunc((40 + parseInt(value, 10)) /3); slider_HP_set(v); }
    if (elmnt === 'Balance')  slider_BAL_set(value)
}

function slider_LP_mouseUp () { // Slider LowPass mouseupevent
    handlectrl('LowPass', I2S_eq_Val[LowPass.value])
    console.log('LowPass=%i', Number(LowPass.value));
}

function slider_LP_change () { //  Slider LowPass changeevent
    console.log('LowPass=%i', Number(LowPass.value))
    document.getElementById('label_LP_value').innerHTML = I2S_eq_DB[LowPass.value]
}

function slider_LP_set (value) { // set Slider LowPass
    var val = Number(value)
    document.getElementById('LowPass').value = val
    document.getElementById('label_LP_value').innerHTML = I2S_eq_DB[LowPass.value]
    console.log('LowPass=%i', val)
}

function slider_BP_mouseUp () { // BandPass mouseupevent
    handlectrl('BandPass', I2S_eq_Val[BandPass.value])
    console.log('BandPass=%i', Number(BandPass.value));
}

function slider_BP_change () { //  BandPass changeevent
    console.log('BandPass=%i', Number(BandPass.value))
    document.getElementById('label_BP_value').innerHTML = I2S_eq_DB[BandPass.value]
}

function slider_BP_set (value) { // set Slider BandPass
    var val = Number(value)
    document.getElementById('BandPass').value = val
    document.getElementById('label_BP_value').innerHTML = I2S_eq_DB[BandPass.value]
    console.log('BandPass=%i', val)
}

function slider_HP_mouseUp () { // Slider HighPass mouseupevent
    handlectrl('HighPass', I2S_eq_Val[HighPass.value])
    console.log('HighPass=%i', Number(HighPass.value));
}

function slider_HP_change () { //  Slider HighPass changeevent
    console.log('HighPass=%i', Number(HighPass.value))
    document.getElementById('label_HP_value').innerHTML = I2S_eq_DB[HighPass.value]
}

function slider_HP_set (value) { // set Slider HighPass
    var val = Number(value)
    document.getElementById('HighPass').value = val
    document.getElementById('label_HP_value').innerHTML = I2S_eq_DB[HighPass.value]
    console.log('HighPass=%i', val)
}

function slider_BAL_mouseUp () { // Slider Balance mouseupevent
    handlectrl('Balance', Balance.value)
    console.log('Balance=%i', Number(Balance.value));
}

function slider_BAL_change () { //  Slider Balance changeevent
    console.log('Balance=%i', Number(Balance.value))
    document.getElementById('label_BAL_value').innerHTML = Balance.value
}

function slider_BAL_set (value) { // set Slider Balance
    var val = Number(value)
    document.getElementById('Balance').value = val
    document.getElementById('label_BAL_value').innerHTML = Balance.value
    console.log('Balance=%i', val)
}

function handlectrl (id, val) { // Radio: BP,BP,TP, BAL
    var theUrl = id + "=" + val
    console.log(theUrl)
    socket.send(theUrl)
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------- TAB CONFIG -------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------
let tableData = [];

function loadTableData() {
    const table = document.getElementById('stationsTable').querySelector('tbody');
    table.innerHTML = ''; // Tabelle leeren
    tableData.forEach((rowData, rowIndex) => {
        const row = table.insertRow();
        row.classList.add('table-row');
        rowData.forEach((cellData, cellIndex) => {
            const cell = row.insertCell();
            cell.style.paddingLeft = "5px";
            cell.style.paddingRight = "5px";
            cell.style.paddingTop = "5px";
            cell.style.paddingBottom = "5px";
            cell.style.textAlign = "left";
            cell.style.border = "1px solid black";
            cell.style.whiteSpace = "nowrap";
            cell.style.cursor = "pointer";
            cell.style.height = "25px";
            cell.style.overflow = "hidden";
            cell.style.textOverflow = "ellipsis";
            if(cellIndex === 0) {
                cell.style.paddingLeft = "0px";
                cell.style.paddingRight = "0px";
                cell.style.minWidth = "40px";
                cell.style.width = "42px";
                cell.style.maxWidth = "44px";
                cell.style.textAlign = "center";
            }
            if(cellIndex === 1) {
                cell.style.minWidth = "45px";
                cell.style.width = "47px";
                cell.style.maxWidth = "50px";
                cell.style.textAlign = "center";
            }
            if(cellIndex === 2) {
                cell.style.width = "100px";
                cell.style.maxWidth = "180px";
                cell.style.textAlign = "left";
            }
            if(cellIndex === 3) {
                cell.style.width = "200px";
                cell.style.maxWidth = "280px";
                cell.style.textAlign = "left";
            }
            cell.textContent = cellData;
            // Event zum Editieren hinzufügen
            cell.addEventListener('click', function () {
                editCell(cell, rowIndex, cellIndex);
            });
        });
        row.addEventListener('contextmenu', function (e) {
            e.preventDefault();
            selectedRowIndex = rowIndex;
            showContextMenu(e);
        });
        if (rowIndex % 2 === 1) {
            row.style.backgroundColor = '#f2f2f2';
        }
        addRowListeners();
    });
}

function editCell(cell, rowIndex, cellIndex) {
    if (cell.querySelector('input')) {
        return; // Verhindert das mehrfache Hinzufügen eines Input-Feldes
    }
    const originalContent = cell.textContent.trim();
    const cellWidth = cell.clientWidth - 5; // padding  td
    // Breite der Zelle fixieren
    cell.style.width = `${cellWidth}px`;
    // Input-Feld mit reduzierter Breite erstellen
    const inputFieldWidth = cellWidth - 15; // Reduziere die Breite um 15px für Padding und Rand
    cell.innerHTML = `<input type="text" value="${originalContent}" data-original-value="${originalContent}"
        style="width: ${inputFieldWidth}px; font-size: ${window.getComputedStyle(cell).fontSize}; font-family: ${window.getComputedStyle(cell).fontFamily};">`;
    const input = cell.querySelector('input');
    input.focus();
    input.addEventListener('blur', function () {
        let hasChanged = true;
        let newValue = input.value.trim();
        const originalValue = input.dataset.originalValue;
        if (newValue !== originalValue) {
            const validValues = ['*', '1', '2', '3'];
            if (cellIndex == 0 && !validValues.includes(newValue)) {
                newValue = ''; // Clears the field if the value is invalid
                haschanged = false;
            }
            cell.textContent = newValue;
            tableData[rowIndex][cellIndex] = newValue;
            saveJsonFileToSD("/stations.json", JSON.stringify(tableData, 0, 2));  // save modified data
            if (hasChanged) showMessage1('Cell has been successfully modified.');
            updateStationlist();
        } else {
            cell.textContent = originalContent;
        }
    });
    input.addEventListener('keydown', function (event) {
        if (event.key === 'Enter') {
            input.blur();
        }
    });
}

let deletedRowData = ['*', '', '', 'http'];

function insertRow(z) { // z: 0 = above, 1 = below
    const newRowData = deletedRowData;
    if (selectedRowIndex !== null) {
        tableData.splice(selectedRowIndex + z, 0, newRowData);
        loadTableData();
        saveJsonFileToSD("/stations.json", JSON.stringify(tableData, 0, 2));  // Speichert die geänderten Daten
        showMessage1('Row successfully inserted.');
        updateStationlist();
    }
    hideContextMenu();
}

function insertLastRow(CountryCode, Name, Url) { // z: 0 = above, 1 = below
    const newRowData = ['*', CountryCode, Name, Url];
    let rowCount = tableData.length;
    if (rowCount !== null) {
        tableData.splice(rowCount + 1, 0, newRowData);
        loadTableData();
        saveJsonFileToSD("/stations.json", JSON.stringify(tableData, 0, 2));  // Speichert die geänderten Daten
        showMessage1('Row successfully inserted.');
        updateStationlist();
        deletedRowData = ['*', '', '', 'http'];
    }
    hideContextMenu();
}


function deleteRow() {
    if (selectedRowIndex !== null && tableData.length > 1) {
        deletedRowData = tableData[selectedRowIndex];
        tableData.splice(selectedRowIndex, 1);
        loadTableData();
        saveJsonFileToSD("/stations.json", JSON.stringify(tableData, 0, 2));  // Speichert die geänderten Daten
        showMessage1('Row was successfully deleted.');
        updateStationlist();
    }
    hideContextMenu();
}

function showContextMenu(event) {
    const contextMenu = document.getElementById('contextMenu');
    contextMenu.style.left = `${event.pageX}px`;
    contextMenu.style.top = `${event.pageY}px`;
    contextMenu.style.display = 'block';
}

function hideContextMenu() {
    document.getElementById('contextMenu').style.display = 'none';
}

function showMessage1(message) {
    console.log(message);
    // show the notification1 windows
    var notification1 = document.getElementById('notification1');
    notification1.textContent = message;
    notification1.style.display = 'block';
    // hide after 3 seconds
    setTimeout(function() {
        notification1.style.display = 'none';
    }, 3000);
}

// Hide context menu when clicking outside
window.addEventListener('click', function () {
    hideContextMenu();
});


async function loadStationsFromSD(file_name) {
    try {
        const cacheBuster = new Date().getTime();
        const response = await fetch("SD_Download?" + encodeURIComponent(file_name) + "&cb=" + cacheBuster, {
            method: 'GET',
            headers: {
                'Content-Type': 'application/json'
            }
        });

        if (!response.ok) {
            throw new Error(`Error loading the file: ${response.statusText}`);
        }

        const jsonContent = await response.text();

    //    console.log('File loaded successfully : %s', jsonContent);
        console.log('File loaded successfully :', JSON.parse(jsonContent));
        if (jsonContent) {
            tableData = JSON.parse(jsonContent);
        } else {
            // Use default data when there is no stored data
            tableData = [
                ["*", "D", "0N 70s", "http://0n-70s.radionetz.de:8000/0n-70s.mp3"],
                ["*", "D", "0N 80s", "http://0n-80s.radionetz.de:8000/0n-80s.mp3"],
                ["*", "D", "0N 90s", "http://0n-90s.radionetz.de:8000/0n-90s.mp3"]
            ];
        }

        // Tabelle erst laden, wenn die Daten bereitgestellt wurden
        loadTableData();
        updateStationlist(); // und dann die Liste in RADIO aktualisieren

    } catch (error) {
        console.error('Es gab ein Problem beim Laden der Datei:', error);
            tableData = [
                ["*", "D", "0N 70s", "http://0n-70s.radionetz.de:8000/0n-70s.mp3"],
                ["*", "D", "0N 80s", "http://0n-80s.radionetz.de:8000/0n-80s.mp3"],
                ["*", "D", "0N 90s", "http://0n-90s.radionetz.de:8000/0n-90s.mp3"]
            ];
            saveJsonFileToSD("/stations.json", JSON.stringify(tableData, 0, 2));  // Speichert die geänderten Daten
    }
}

// Event-Listener für alle <tr>-Elemente in der Tabelle hinzufügen
function addRowListeners() {
    const rows = document.getElementsByClassName('table-row');
    const info = document.getElementById('stationInfo');
    for (let i = 0; i < rows.length; i++) {
        rows[i].addEventListener('mouseover', function() {
            // Entfernen der Hervorhebung von allen Zeilen
            for (let j = 0; j < rows.length; j++) {
                rows[j].classList.remove('highlight');
            }
            // Hervorheben der aktuellen Zeile
            this.classList.add('highlight');

            // Anzeige der Zeilennummer
            const rowIndex = i + 1;
            info.textContent = `Station: ${rowIndex}`;

        });
    }
}

function preselectStationList(staNr) {
    document.getElementById('preset').selectedIndex = (staNr - 1);
}

function updateStationlist () { // select in tab Radio
    var opt, select
    const selectElement = document.getElementById('preset') // Radio: show stationlists

    // Zuerst das Select-Feld leeren, falls es bereits Optionen hat
    selectElement.innerHTML = '';

    // Durchlaufen der tableData-Array
    tableData.forEach((row, index) => {
        const option = document.createElement('option');
        // Dreistellige Nummerierung, beginnend mit 001
        const prefixNumber = String(index + 1).padStart(3, '0');
        // Setze den Text der Option auf die nummerierte dritte Spalte
        option.textContent = `${prefixNumber} ${row[2]}`;//
        // Setze den Wert der Option auf den Wert der vierten Spalte
        option.value = String(index + 1);
        if (!['*', '1', '2', '3'].includes(row[0])) {
            option.style.color = "grey"
        }
        else{
            option.style.color = "black"
        }
        // Füge die Option dem Select-Element hinzu
        selectElement.appendChild(option);
    });
}

function saveStations_json(){
    // Create a blob with the content
    const blob = new Blob([JSON.stringify(tableData, 0, 2)], { type: 'application/json' });

    // Create an invisible link to download the file
    const link = document.createElement('a');
    link.href = window.URL.createObjectURL(blob);
    link.download = 'stations.json';

    // Add link element to document, simulate click and remove it again
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
}

function loadStations_json(event){
    var file = event[0]
    var reader = new FileReader()
    reader.onload = function (event) {
        var data = event.target.result
        console.log(data);
        tableData = JSON.parse(data)
        loadTableData()
        updateStationlist();
        let rowCount = tableData.length;
        if (rowCount !== null) {
            saveJsonFileToSD("/stations.json", JSON.stringify(tableData, 0, 2));  // save modified data
        }
    }
    reader.onerror = function (ex) {
        console.log(ex)
    }
    reader.readAsText(file)
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------- TAB AUDIO PLAYER ------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------

function playWebURL() { // Radio: button play - Enter a streamURL here....
    cmd.value = ''
    var theUrl =  resultstr3.value;
    theUrl = theUrl.replace(/%3d/g, '=') // %3d convert to =
    theUrl = theUrl.replace(/%21/g, '!') //
    theUrl = theUrl.replace(/%22/g, '"') //
    theUrl = theUrl.replace(/%23/g, '#') //
    theUrl = theUrl.replace(/%3f/g, '?') //
    theUrl = theUrl.replace(/%40/g, '@') //
    socket.send("webFileURL=" + theUrl)
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------- DLNA -----------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------
var dlnaLevel = 0

function clearDLNAServerList(level){
    for(i = level; i < 9; i++){
        console.log('clear DLNA server list, level=', i)
        var select

        switch(level){
            case 0:
                select = document.getElementById('server')
                select.options.length = 0;
                var option = new Option("Select a DLNA server here")
                select.appendChild(option);                                     // no break, fall through
            case 1:
                select = document.getElementById('level1')
                select.options.length = 0;
            case 2:
                select = document.getElementById('level2')
                select.options.length = 0;
            case 3:
                select = document.getElementById('level3')
                select.options.length = 0;
            case 4:
                select = document.getElementById('level4')
                select.options.length = 0;
            case 5:
                select = document.getElementById('level5')
                select.options.length = 0;
            case 6:
                select = document.getElementById('level6')
                select.options.length = 0;
            case 7:
                select = document.getElementById('level7')
                select.options.length = 0;
            case 8:
                select = document.getElementById('level8')
                select.options.length = 0;
        }
    }
}

function addDLNAServer(val){
    console.log(val)
    var obj = JSON.parse(val);
    var select = document.getElementById('server')
    for(var i = 0; i < Object.keys(obj).length; i++){
        var option = new Option(obj[i].friendlyName, obj[i].srvId); // e.g. "Wolles-FRITZBOX Mediaserver,1"  (friendlyName, ServerIdx)
        console.log(obj[i].friendlyName, obj[i].srvId);
        select.appendChild(option);
    }
}

function show_DLNA_Content(val){
    var select
    if(dlnaLevel == 1) select = document.getElementById('level1')
    if(dlnaLevel == 2) select = document.getElementById('level2')
    if(dlnaLevel == 3) select = document.getElementById('level3')
    if(dlnaLevel == 4) select = document.getElementById('level4')
    if(dlnaLevel == 5) select = document.getElementById('level5')
    if(dlnaLevel == 6) select = document.getElementById('level6')
    if(dlnaLevel == 7) select = document.getElementById('level7')
    if(dlnaLevel == 8) select = document.getElementById('level8')
    if(select.options.length == 0){
        var option = new Option("Select level " + dlnaLevel.toString())
        select.appendChild(option);
    }
    console.log(val)
    var obj = JSON.parse(val);
    for(var i = 0; i < Object.keys(obj).length; i++){
        console.log(i)
        var objectId = obj[i].objectId
        var title = obj[i].title
        var itemURL = obj[i].itemURL
        var isAudio = obj[i].isAudio
        var itemSize = obj[i].itemSize
        var childCount = obj[i].childCount
        var duration = obj[i].dur

        var isDir
        if(itemURL== "?") isDir = true
        else              isDir = false

        var n = ""
        if(isDir){
            if(childCount != "0"){
                n = title.concat('\xa0\xa0', '\(' + childCount + '\)'); // more than one space
            }
            else {
                n = title
            }
        }
        else {
            if(duration[0] == '?'){
                n = title.concat('\xa0\xa0', '\(' + itemSize + '\)'); // more than one space
            }
            else {
                n = title.concat('\xa0\xa0', '\(' + duration + '\)'); // more than one space
            }
        }
        if(isAudio == "true"){
            var option = new Option(n, itemURL);
            option.style.color = "blue"
        }
        else{
            var option = new Option(n, objectId);
            if(!isDir){
                option.style.color = "red"
            }
        }
        console.log(n, objectId);
        select.appendChild(option);
    }
}

function selectserver (presctrl) { // preset, select a server, root, level0
    socket.send('DLNA_getRoot=' + presctrl.value)
    clearDLNAServerList(1)
    dlnaLevel = 1
    console.log('DLNA_getContent=' + presctrl.value)
}
function select_l1 (presctrl) { // preset, select root
    var slectedText = presctrl.options[presctrl.selectedIndex].innerText;
    socket.send('DLNA_getContent=' + presctrl.value + "&" + slectedText)
    console.log('DLNA_getContent=' + presctrl.value + "&" + slectedText)
    clearDLNAServerList(2)
    dlnaLevel = 2
}
function select_l2 (presctrl) { // preset, select level 1
    var slectedText = presctrl.options[presctrl.selectedIndex].innerText;
    socket.send('DLNA_getContent=' + presctrl.value + "&" + slectedText)
    console.log('DLNA_getContent=' + presctrl.value + "&" + slectedText)
    clearDLNAServerList(3)
    dlnaLevel = 3
}
function select_l3 (presctrl) { // preset, select level 2
    var slectedText = presctrl.options[presctrl.selectedIndex].innerText;
    socket.send('DLNA_getContent=' + presctrl.value + "&" + slectedText)
    console.log('DLNA_getContent=' + presctrl.value + "&" + slectedText)
    clearDLNAServerList(4)
    dlnaLevel = 4
 }
 function select_l4 (presctrl) { // preset, select level 3
    var slectedText = presctrl.options[presctrl.selectedIndex].innerText;
    socket.send('DLNA_getContent=' + presctrl.value + "&" + slectedText)
    console.log('DLNA_getContent=' + presctrl.value + "&" + slectedText)
    clearDLNAServerList(5)
    dlnaLevel = 5
 }
 function select_l5 (presctrl) { // preset, select level 4
    var slectedText = presctrl.options[presctrl.selectedIndex].innerText;
    socket.send('DLNA_getContent=' + presctrl.value + "&" + slectedText)
    console.log('DLNA_getContent=' + presctrl.value + "&" + slectedText)
    clearDLNAServerList(6)
    dlnaLevel = 6
 }
function select_l6 (presctrl) { // preset, select level 5
    var slectedText = presctrl.options[presctrl.selectedIndex].innerText;
    socket.send('DLNA_getContent=' + presctrl.value + "&" + slectedText)
    console.log('DLNA_getContent=' + presctrl.value + "&" + slectedText)
    clearDLNAServerList(7)
    dlnaLevel = 7
 }
function select_l7 (presctrl) { // preset, select level 6
    var slectedText = presctrl.options[presctrl.selectedIndex].innerText;
    socket.send('DLNA_getContent=' + presctrl.value + "&" + slectedText)
    console.log('DLNA_getContent=' + presctrl.value + "&" + slectedText)
    clearDLNAServerList(8)
    dlnaLevel = 8
 }
function select_l8 (presctrl) { // preset, select level 7
    var slectedText = presctrl.options[presctrl.selectedIndex].innerText;
    socket.send('DLNA_getContent=' + presctrl.value + "&" + slectedText)
    console.log('DLNA_getContent=' + presctrl.value + "&" + slectedText)
    dlnaLevel = 9
 }
// ----------------------------------- TAB Search Stations ------------------------------------

// global var
var countryallstations
var category

function open_RB_page() {
  var res = document.getElementById("RB_search").value;
  var url = "https://www.radio-browser.info/search?page=1&order=clickcount&reverse=true&hidebroken=true&name=" + res;
  window.open(url, "_blank");
}

function addStationsToGrid () {
    insertLastRow($('#CountryCode').val(), $('#rb_stationname').val(), $('#streamurl').val());
}

function loadJSON (path, success, error) {
    console.log(path)
    var xhr = new XMLHttpRequest()
    xhr.timeout = 2000; // time in milliseconds
    xhr.onreadystatechange = function () {
        if (xhr.readyState === 4) {
            if (xhr.status === 200) {
                success(JSON.parse(xhr.responseText))
            } else {
                error(xhr)
            }
        }
    }
    xhr.open('GET', path, true)
    xhr.ontimeout = (e) => {
      // XMLHttpRequest timed out. Do something here.
    }
    xhr.send()
}

function selectcategory (presctrl) { // tab Search: preset, select a category

  if(presctrl.value == "bycountry")  {loadJSON('https://de1.api.radio-browser.info/json/countries#name', gotCountries, 'jsonp'); category="country"}
  if(presctrl.value == "bylanguage") {loadJSON('https://de1.api.radio-browser.info/json/languages/?hidebroken=true&limit=100&reverse=true&order=stationcount', gotLanguages, 'jsonp'); category="language"}
  if(presctrl.value == "bytag")      {loadJSON('https://de1.api.radio-browser.info/json/tags/?hidebroken=true&limit=100&reverse=true&order=stationcount',      gotTags, 'jsonp'); category="tag"}
}

function selectitem (presctrl) { // tab Search: preset, select a station
  if(category == "country")  loadJSON('https://de1.api.radio-browser.info/json/stations/bycountrycodeexact/'  + presctrl.value.substring(0, 2) + "#name", gotStations, 'jsonp')
  if(category == "language") loadJSON('https://de1.api.radio-browser.info/json/stations/bylanguage/' + presctrl.value.substring(0, presctrl.value.lastIndexOf(" ")), gotStations, 'jsonp')
  if(category == "tag")      loadJSON('https://de1.api.radio-browser.info/json/stations/bytag/'      + presctrl.value.substring(0, presctrl.value.lastIndexOf(" ")), gotStations, 'jsonp')

}

function getFlagEmoji(countryCode) {
    // Normalisiere die Eingabe und nimm nur die ersten beiden Zeichen
    if(!countryCode) return ""
    const codePoints = countryCode
        .toUpperCase()
        .split('')
        .map(char => 0x1F1E6 + char.charCodeAt(0) - 'A'.charCodeAt(0));
    // Konvertiere die Unicode-Codepoints in ein Emoji
    return String.fromCodePoint(...codePoints);
}

function validateStationname() {
    const inputField = document.getElementById("rb_stationname");
    const invalidChars = /[\\/:*?"<>|]/
    if (invalidChars.test(inputField.value)) { // If invalid characters are found, turn text red
        inputField.style.color = "red";
    } else { // If all characters are valid, colour text black
        inputField.style.color = "black";
    }
}

function gotCountries (data) { // fill select countries
    var select = document.getElementById('item')
    var opt
    select.options.length = 1
    for (var i = 0; i < data.length; i++) {
        if (i < 2) continue
        // if(!data[i].iso_3166_1) continue
        // if(!data[i].name) continue
        // if(!data[i].stationcount) continue
        opt = document.createElement('OPTION')
        flag = getFlagEmoji(data[i].iso_3166_1)  // 🇵🇹
        const firstChar = data[i].iso_3166_1.charAt(0)
        if(firstChar === firstChar.toLowerCase()) continue; // only uppercases are valid, "DE" but not "de"
        opt.text = data[i].iso_3166_1 + "  " + flag + "  " + data[i].name + "  (" + data[i].stationcount + ")"
        select.add(opt)
    }
    console.log(data.uuid)
    var stations = document.getElementById('stations') // set stations to default
    stations.options.length = 1
    const options = Array.from(select.options);

    select.options.length = 0; // clear select
    options.forEach(option => select.appendChild(option));
    select.selectedIndex = 0; // set default
    const selectElement = document.getElementById("stations");
    selectElement.options.length = 0;
}

function gotLanguages (data) { // fill select countries
    var select = document.getElementById('item')
    var opt
    select.options.length = 1
    for (var i = 0; i < data.length; i++) {
        if (i < 2) continue
        opt = document.createElement('OPTION')
        const firstChar = data[i].name.charAt(0)
        if(firstChar === '#') continue; // # are not valid
        opt.text = data[i].name + "  (" + data[i].stationcount + ")"
        select.add(opt)
    }
    console.log(data.uuid)
    var stations = document.getElementById('stations') // set stations to default
    stations.options.length = 1
    const options = Array.from(select.options);

    select.options.length = 0; // clear select
    options.forEach(option => select.appendChild(option));
    select.selectedIndex = 0; // set default
    const selectElement = document.getElementById("stations");
    selectElement.options.length = 0;
}

function gotTags (data) { // fill select countries
    var select = document.getElementById('item')
    var opt
    select.options.length = 1
    for (var i = 0; i < data.length; i++) {
        if (i < 2) continue
        opt = document.createElement('OPTION')
        // const firstChar = data[i].name.charAt(0)
        // if(firstChar === '#') continue; // # are not valid
        opt.text = data[i].name + "  (" + data[i].stationcount + ")"
        select.add(opt)
    }
    console.log(data.uuid)
    var stations = document.getElementById('stations') // set stations to default
    stations.options.length = 1
    const options = Array.from(select.options);

    select.options.length = 0; // clear select
    options.forEach(option => select.appendChild(option));
    select.selectedIndex = 0; // set default
    const selectElement = document.getElementById("stations");
    selectElement.options.length = 0;
}


function gotStations (data) { // fill select stations
    const cc = document.getElementById('CountryCode')
    const it = document.getElementById('item')
    if(category === "country") cc.value = it.value.substring(0, 2)
    else cc.value = ''
    var select = document.getElementById('stations')
    var opt
    select.options.length = 1
    for (var i = 0; i < data.length; i++) {
        opt = document.createElement('OPTION')
        opt.text = data[i].name
        opt.value = i
        select.add(opt)
    }
    countryallstations = data
    // sort data
    const options = Array.from(select.options);
    options.sort((a, b) => a.text.localeCompare(b.text));
    options.forEach(option => {
        if (option.text.length > 70) {
            option.text = option.text.substring(0, 67) + '...'; // max length
        }
    });
    select.options.length = 0; // clear select
    options.forEach(option => select.appendChild(option));
    select.selectedIndex = 0; // set default
}

function selectstation () { // select a station
    var e = document.getElementById('stations')
    var value = e.options[e.selectedIndex].value
    var sturl = countryallstations[value].url
    console.log(value)
    console.log(sturl)
    var f = document.getElementById('streamurl')
    f.value = sturl
    var g = document.getElementById('favicon')
    var favi = countryallstations[value].favicon
    g.value = favi
    var h = document.getElementById('homepageurl')
    h.value = countryallstations[value].homepage
    scaleCanvasImage(favi)
    var j = document.getElementById('rb_stationname')
    j.value = countryallstations[value].name.trim()
    validateStationname()
}

function teststreamurl () { // Search: button play - enter a url to play from
    var theUrl = "stationURL=" + streamurl.value
    theUrl = theUrl.replace(/%3d/g, '=') // %3d convert to =
    theUrl = theUrl.replace(/%21/g, '!') //
    theUrl = theUrl.replace(/%22/g, '"') //
    theUrl = theUrl.replace(/%23/g, '#') //
    theUrl = theUrl.replace(/%3f/g, '?') //
    theUrl = theUrl.replace(/%40/g, '@') //
    socket.send(theUrl)
}

function scaleCanvasImage (url) {
    var canvas = document.getElementById('canvas')
    var ctx = canvas.getContext('2d')
    var src
    ctx.beginPath()
    ctx.rect(0, 0, canvas.width, canvas.height)
    ctx.fillStyle = 'white'
    ctx.fill()
    var co = 'https://api.codetabs.com/v1/proxy?quest='
    //var co = 'https://corsproxy.io/?'
    src = co + url
    var imgObj = new Image()
    imgObj.crossOrigin = 'anonymous'
    imgObj.src = src

    imgObj.onload = function() {
        var imgWidth = imgObj.width
        var imgHeight = imgObj.height
        var scaleX = 1
        var scaleY = 1
        if (imgWidth > canvas.width) scaleX = canvas.width / imgWidth
        if (imgHeight > canvas.height) scaleY = canvas.height / imgHeight
        var scale = scaleY
        if (scaleX < scaleY) scale = scaleX
        if (scale < 1) {
            imgHeight = imgHeight * scale
            imgWidth = imgWidth * scale
        }
        var dx = (canvas.width - imgWidth) / 2
        var dy = (canvas.height - imgHeight) / 2
        ctx.drawImage(imgObj, 0, 0, imgObj.width, imgObj.height, dx, dy, imgWidth, imgHeight)
    }

}

function refreshCanvas () {
    var g = document.getElementById('favicon')
    scaleCanvasImage(g.value)
    console.log('refresh')
}

function uploadCanvasImage () {
    var filename
    var sn = document.getElementById('rb_stationname')
    if (sn.value !== '') filename = sn.value + '.jpg'
    else {
        alert('no stationname given')
        return
    }
    var canvas = document.getElementById('canvas')
    var dataURL = canvas.toDataURL('image/jpeg')
    document.getElementById('hidden_data').value = dataURL
    var fd = new FormData(document.forms.form1)
    var theUrl = '/uploadfile?' + filename + '&version=' + Math.random()
    var xhr = new XMLHttpRequest()
    xhr.timeout = 4000; // time in milliseconds
    xhr.open('POST', theUrl, true)

    xhr.upload.onprogress = function (e) {
        if (e.lengthComputable) {
            var percentComplete = (e.loaded / e.total) * 100
            console.log(percentComplete + '% uploaded')
        }
    }
    xhr.onload = function () {
    }
    xhr.ontimeout = (e) => {
        // XMLHttpRequest timed out.
        alert(filename + ' not uploaded, timeout')
    }
    xhr.onreadystatechange = function () { // Call a function when the state changes.
        if (xhr.readyState === 4) {
            if(xhr.status === 200) alert(filename + ' successfully uploaded')
            else alert(filename + 'not successfully uploaded')
        }
    }
    xhr.send(fd)
}

function downloadCanvasImage () {
    var filename
    var sn = document.getElementById('rb_stationname')
    if (sn.value !== '') filename = sn.value + '.jpg'
    else filename = 'myimage.jpg'
    var lnk = document.createElement('a')
    var e // create an 'off-screen' anchor tag
    lnk.download = filename // the key here is to set the download attribute of the a tag
    lnk.href = canvas.toDataURL('image/jpeg')
    if (document.createEvent) { // create a 'fake' click-event to trigger the download
        e = document.createEvent('MouseEvents')
        e.initMouseEvent('click', true, true, window, 0, 0, 0, 0, 0, false, false, false, false, 0, null)
        lnk.dispatchEvent(e)
    } else if (lnk.fireEvent) {
        lnk.fireEvent('onclick')
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------  TAB Settings  -----------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------






//----------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------    TAB Info    -----------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------
function getTimeZoneName() {
    return new Promise((resolve, reject) => {
        const xhr = new XMLHttpRequest();
        xhr.timeout = 2000; // Zeit in Millisekunden
        xhr.open('GET', 'getTimeZoneName' + '&version=' + Math.random(), true);
        xhr.onreadystatechange = function () {
            if (xhr.readyState === 4) {
                if (xhr.status === 200) {
                    const timeZoneName = xhr.responseText;
                    console.log("tzName=", timeZoneName);
                    resolve(timeZoneName); // Promise mit dem erhaltenen Wert auflösen
                } else {
                    console.log("xhr.status=", xhr.status);
                    reject(`Fehler: Status ${xhr.status}`); // Promise ablehnen, falls ein Fehler auftritt
                }
            }
        };
        xhr.ontimeout = () => {
            console.log("timeout in getTimeZoneName()");
            reject("Fehler: Anfragezeitüberschreitung"); // Promise ablehnen, falls ein Timeout auftritt
        };

        xhr.send();
    });
}

function setTimeZone(selectObject){
    var value = selectObject.value;
    var txt = selectObject.options[selectObject.selectedIndex].text;
    socket.send("setTimeZone=" + txt + "&" + value)
}

async function loadTimeZones() {
    try {
        g_timeZoneName = await getTimeZoneName(); // Warten, bis getTimeZoneName abgeschlossen ist
        const tzFile = new XMLHttpRequest();
        tzFile.timeout = 2000; // Zeit in Millisekunden
        tzFile.open('GET', 'SD_Download?/timezones.csv', true);

        tzFile.onreadystatechange = function () {
            if (tzFile.readyState === 4) {
                const tzdata = tzFile.responseText;
                const tzNames = tzdata.split("\n");
                const select = document.getElementById('TimeZoneSelect');
                select.options.length = 0;

                for (let i = 0; i < tzNames.length; i++) {
                    const [tzItem1, tzItem2] = tzNames[i].split("\t");
                    if (!tzItem1 || !tzItem2) continue;

                    const opt = document.createElement('OPTION');
                    opt.text = tzItem1;
                    opt.value = tzItem2;
                    select.add(opt);
                }

                // Auswahl basierend auf g_timeZoneName setzen
                for (let i = 0; i < select.options.length; i++) {
                    if (select.options[i].text === g_timeZoneName) {
                        select.selectedIndex = i;
                        break;
                    }
                }
            }
        };

        tzFile.ontimeout = () => {
            console.log("load SD/timezones.csv timeout");
        };

        tzFile.send();
    } catch (error) {
        console.error("Fehler beim Laden des Zeitzonennamens:", error);
    }
}

function loadRingVolume(){
    const selectRingVolume = document.getElementById('selRingVolume');

    selectRingVolume.options.length = 0;
    for (let i = 0; i <= cur_volumeSteps; i++) {
        const option = document.createElement('option');
        option.value = i;
        option.textContent = i;
        if (i === 0) {
            option.selected = true; // Setzt den Standardwert
        }
        selectRingVolume.appendChild(option);
    }
}

function loadVolumeAfterAlarm(){
    const selectVolumeAfterAlarm = document.getElementById('selVolumeAfterAlarm');

    selectVolumeAfterAlarm.options.length = 0;
    for (let i = 0; i <= cur_volumeSteps; i++) {
        const option = document.createElement('option');
        option.value = i;
        option.textContent = i;
        if (i === 0) {
            option.selected = true; // Setzt den Standardwert
        }
        selectVolumeAfterAlarm.appendChild(option);
    }
}

function loadVolumeSteps(){
    socket.send('getVolumeSteps')
    const selectVolumeSteps = document.getElementById('selVolumeSteps');

    selectVolumeSteps.options.length = 0;
    for (let i = 21; i <= 255; i++) {
        const option = document.createElement('option');
        option.value = i;
        option.textContent = i;
        if (i === 21) {
            option.selected = true; // Setzt den Standardwert
        }
        selectVolumeSteps.appendChild(option);
    }
}

function showRingvolume(val){
    const selectedValueElement = document.getElementById('txtRingVolume');
    selectedValueElement.textContent = val;
    const selectRingVolume = document.getElementById('selRingVolume');
    selectRingVolume.selectedIndex = val;
}

function showVolumeAfterAlarm(val){ // _curVolume after alarm
    const selectedValueElement = document.getElementById('txtVolumeAfterAlarm');
    selectedValueElement.textContent = val;
    const selectVolumeAfterAlarm = document.getElementById('selVolumeAfterAlarm');
    selectVolumeAfterAlarm.selectedIndex = val;
}

function showVolumeSteps(val){
    const selectedValueElement = document.getElementById('txtVolumeSteps');
    selectedValueElement.textContent = val;
    const selectVolumeSteps = document.getElementById('selVolumeSteps');
    selectVolumeSteps.selectedIndex = val- 21;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------ TAB Remote Control--------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------

function showMessage2(message) {
    console.log(message);
    // show the notification2 windows
    var notification2 = document.getElementById('notification2');
    notification2.textContent = message;
    notification2.style.display = 'block';
    // hide after 3 seconds
    setTimeout(function() {
        notification2.style.display = 'none';
    }, 3000);
}

function IRclick(btn){
  var id = "#ir_command_" + btn
  var val1 = $('#ir_command_C').val()
  $(id).val(val1)
  chIRcmd(btn)
}

function chIRcmd(btn){  // IR command, value changed
    var arrLen = 30
    var irArr = []
    var val1
    var ret = true
    for(i = 0; i < arrLen; i++){
        var id
        id = "#ir_command_" + i
        val1 = $(id).val().toString()
        irArr.push(val1)
        $(id).val(" ")
        $(id).val(val1)
        $(id).css("background-color", "white");
    }
    for (var i = 0; i < arrLen; i++) {
        for (var j = 0; j < irArr.length; j++) {
            if(irArr[i] == "-1") continue
            if(i != j && irArr[i] == irArr[j]){
                var id = "#ir_command_" + i
                if(irArr[j] != ""){
                    $(id).css("background-color", "yellow")
                    ret = false
                }
            }
        }
    }
    if(btn == -1) return
    if(ret){
        var id = "#ir_command_" + btn
        console.log("setIRcmd=" + $(id).val() + "&" + btn)
        socket.send("setIRcmd=" + $(id).val() + "&" + btn)
    }
    IRupdateJSON(btn)
    return
}

function writeJSONToTable(jsonIrString) {
    // console.log(jsonIrString)
    if (!jsonIrString) {
        console.error("Kein JSON zum Rückschreiben verfügbar.");
        return;
    }
    const data = JSON.parse(jsonIrString);
        data.forEach(item => {
        const keys = Object.keys(item); // Hole die Schlüssel des Objekts (z.B. "0", "label")
        const number = keys[0]; // Die erste Zahl (z.B. "0", "10", "20")
        const command = item[number]; // Der Wert des Befehls
        ir_arr[number] = command
        const label = item["label"]; // Das Label
        // Schreibe den Befehl zurück in das entsprechende Input-Feld
        const inputField = document.getElementById(`ir_command_${number}`);
        if (inputField) {
            inputField.value = command;
        }
        // Schreibe das Label zurück in die Tabelle
        const labelCell = inputField?.parentElement?.nextElementSibling;
        // if (labelCell) {
        //     labelCell.textContent = label;
        // }
    });
}


function getTableDataAsJSON(tableId) { // make a JSON string from IR table
    const table = document.getElementById(tableId);
    if (!table) {
        console.error(`Tabelle mit der ID ${tableId} nicht gefunden.`);
        return;
    }
    const rows = table.getElementsByTagName('tr');
    const data = [];
    for (let i = 0; i < rows.length; i++) { // with header
        var cells;
        if(i == 0) cells = rows[i].getElementsByTagName('th'); // header
        else       cells = rows[i].getElementsByTagName('td');
        if (cells.length > 0) {
            // Erste Gruppe: Erste Spalte und Kommando/Label 1
            const number1 = cells[0].textContent.trim();
            const command1 = document.getElementById(`ir_command_${number1}`)?.value || "";
            const label1 = cells[2].textContent.trim();
            if (command1 && label1) {
                const obj1 = {};
                obj1[number1] = command1;
                obj1["label"] = label1;
                data.push(obj1);
            }
            // Zweite Gruppe: Vierte Spalte und Kommando/Label 2
            const number2 = cells[3].textContent.trim();
            const command2 = document.getElementById(`ir_command_${number2}`)?.value || "";
            const label2 = cells[5].textContent.trim();
            if (command2 && label2) {
                const obj2 = {};
                obj2[number2] = command2;
                obj2["label"] = label2;
                data.push(obj2);
            }
            // Dritte Gruppe: Sechste Spalte und Kommando/Label 3
            const number3 = cells[6]?.textContent.trim() || "";
            const command3 = document.getElementById(`ir_command_${number3}`)?.value || "";
            const label3 = cells[8]?.textContent.trim() || "";
            if (command3 && label3) {
                const obj3 = {};
                obj3[number3] = command3;
                obj3["label"] = label3;
                data.push(obj3);
            }
            // Vierte Gruppe: Achte Spalte und Kommando/Label 4
            const number4 = cells[9]?.textContent.trim() || "";
            const command4 = document.getElementById(`ir_command_${number4}`)?.value || "";
            const label4 = cells[11]?.textContent.trim() || "";
            if (command4 && label4) {
                const obj4 = {};
                obj4[number4] = command4;
                obj4["label"] = label4;
                data.push(obj4);
            }
        }
    }
    return JSON.stringify(data, null, 2); // JSON formatieren (schönere Ausgabe)
}

function IRupdateJSON(btnNr){
    irButtonsJson = getTableDataAsJSON("ir_table");
    const jsonObject = JSON.parse(irButtonsJson);
    let result = null;
        jsonObject.forEach(item => {
            let str = String(btnNr);
            if (item[str]) {
            result = item[str];
        }
    });
    if(result == ir_arr[btnNr]) return;
    console.log(irButtonsJson);
    saveJsonFileToSD("/ir_buttons.json", irButtonsJson);
    showMessage2('IR button has been successfully modified.');
}

function saveIRbuttons_json(){ // to PC
    // Create a blob with the content
    irButtonsJson = getTableDataAsJSON("ir_table");
    const blob = new Blob([irButtonsJson], { type: 'application/json' });

    // Create an invisible link to download the file
    const link = document.createElement('a');
    link.href = window.URL.createObjectURL(blob);
    link.download = 'ir_buttons.json';

    // Add link element to document, simulate click and remove it again
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
}

function loadIRbuttons_json(event){ // from PC
    var file = event[0]
    var reader = new FileReader()
    reader.onload = function (event) {
        var jsonIrString = event.target.result
        console.log(jsonIrString);
        writeJSONToTable(jsonIrString)
        saveJsonFileToSD("/ir_buttons.json", jsonIrString);  // save modified data
    }
    reader.onerror = function (ex) {
        console.log(ex)
    }
    reader.readAsText(file)
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------ TAB KCX_BT_Emitter--------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------------

function show_BT_memItems(jsonstr){
    console.log('KCX MEM', jsonstr)
    var jsonData = JSON.parse(jsonstr)
    for (var i = 0; i < jsonData.length; i++) {
        document.getElementsByName('bt_addr')[i].value = jsonData[i].addr
        document.getElementsByName('bt_name')[i].value = jsonData[i].name
    }
}

function addScannedName(i){
    var scannedName
    if(i == 0) scannedName = document.getElementsByName('bt_scan_name')[i].value;
    if(i == 1) scannedName = document.getElementsByName('bt_scan_name')[i].value;
    if(i == 2) scannedName = document.getElementsByName('bt_scan_name')[i].value;
    console.log("scannedName", scannedName)
    if(scannedName === ""){
        alert("chosen BT name is empty")
        return
    }
    for (var a = 0; a < 10; a++) {
        if(scannedName === document.getElementsByName('bt_name')[a].value){
            alert("BT name \"" + scannedName + "\" already exists in the list")
            return
        }
    }
    socket.send('KCX_BT_addName=' + scannedName)
}

function addScannedAddr(i){
    var scannedAddr
    if(i == 0) scannedAddr = document.getElementsByName('bt_scan_addr')[i].value;
    if(i == 1) scannedAddr = document.getElementsByName('bt_scan_addr')[i].value;
    if(i == 2) scannedAddr = document.getElementsByName('bt_scan_addr')[i].value;
    console.log("scannedAddr", scannedAddr)
    if(scannedAddr === ""){
        alert("chosen MAC address is empty")
        return
    }
    for (var a = 0; a < 10; a++) {
        if(scannedAddr === document.getElementsByName('bt_addr')[a].value){
            alert("MAC address \"" + scannedAddr + "\" already exists in the list")
            return
        }
    }
    socket.send('KCX_BT_addAddr=' + scannedAddr)
}

function show_BT_scannedItems(jsonstr){
    console.log('KCX_BT_SCANNED', jsonstr)
    var jsonData = JSON.parse(jsonstr)
    for (var i = 0; i < jsonData.length; i++) {
        document.getElementsByName('bt_scan_name')[i].value = jsonData[i].name
        document.getElementsByName('bt_scan_addr')[i].value = jsonData[i].addr
    }
}

function clear_BT_memItems(){
    for (var i = 0; i < 10; i++) {
        document.getElementsByName('bt_addr')[i].value = ""
        document.getElementsByName('bt_name')[i].value = ""
    }
    socket.send('KCX_BT_clearItems')
}

</script>

<body id="BODY">
<div id="content" >
    <!-- ~~~~~~~~~~~~~~~~~~~~~~ hidden div ~~~~~~~~~~~~~~~~~~~~~~-->
    <div id="preloaded-images">
        <img data-src="SD/png/Radio_Green.png"                   >
        <img data-src="SD/png/Radio_Yellow.png"                  >
        <img data-src="SD/png/Station_Green.png"                 >
        <img data-src="SD/png/Station_Yellow.png"                >
        <img data-src="SD/png/MP3_Green.png"                     >
        <img data-src="SD/png/MP3_Yellow.png"                    >
        <img data-src="SD/png/Button_DLNA_Green.png"             >
        <img data-src="SD/png/Button_DLNA_Yellow.png"            >
        <img data-src="SD/png/Search_Green.png"                  >
        <img data-src="SD/png/Search_Yellow.png"                 >
        <img data-src="SD/png/Settings_Green.png"                >
        <img data-src="SD/png/Settings_Yellow.png"               >
        <img data-src="SD/png/About_Green.png"                   >
        <img data-src="SD/png/About_Yellow.png"                  >
        <img data-src="SD/png/Button_Previous_Green.png"         >
        <img data-src="SD/png/Button_Previous_Blue.png"          >
        <img data-src="SD/png/Button_Previous_Yellow.png"        >
        <img data-src="SD/png/Button_Next_Green.png"             >
        <img data-src="SD/png/Button_Next_Yellow.png"            >
        <img data-src="SD/png/Button_Volume_Down_Blue.png"       >
        <img data-src="SD/png/Button_Volume_Down_Yellow.png"     >
        <img data-src="SD/png/Button_Volume_Up_Blue.png"         >
        <img data-src="SD/png/Button_Volume_Up_Yellow.png"       >
        <img data-src="SD/png/Button_Mute_Green.png"             >
        <img data-src="SD/png/Button_Mute_Yellow.png"            >
        <img data-src="SD/png/Button_Mute_Red.png"               >
        <img data-src="SD/png/Button_Ready_Blue.png"             >
        <img data-src="SD/png/Button_Ready_Yellow.png"           >
        <img data-src="SD/png/Button_Test_Green.png"             >
        <img data-src="SD/png/Button_Test_Yellow.png"            >
        <img data-src="SD/png/Button_Upload_Blue.png"            >
        <img data-src="SD/png/Button_Upload_Yellow.png"          >
        <img data-src="SD/png/Button_Stop_Blue.png"              >
        <img data-src="SD/png/Button_Stop_Yellow.png"            >
        <img data-src="SD/png/Button_Pause_Resume_Blue.png"      >
        <img data-src="SD/png/Button_Pause_Resume_Yellow.png"    >
        <img data-src="SD/png/Remote_Control_Yellow.png"         >
        <img data-src="SD/png/Remote_Control_Blue.png"           >
        <img data-src="SD/png/Button_BT_Yellow.png"              >
        <img data-src="SD/png/Button_BT_Blue.png"                >
        <img data-src="SD/png/Button_Pause_Blue.png"             >
        <img data-src="SD/png/Button_Pause_Yellow.png"           >
        <img data-src="SD/png/Button_Download_Blue.png"          >
        <img data-src="SD/png/Button_Download_Yellow.png"        >
        <img data-src="SD/common/MiniWebRadioV3.jpg"             >
    </div>

    <div id="dialog">
        <table>
            <tr>
                <td> Fav </td>
                <td> <input type="checkbox" id="chkHide"></td>
            </tr>
            <tr>
                <td>  Cy  </td>
                <td> <input type="text" id="txtCy" size="100"></td>
            </tr>
            <tr>
                <td>  StationName  </td>
                <td> <input type="text" id="txtStationName" size="100"></td>
            </tr>
            <tr>
                <td>  StreamURL  </td>
                <td> <input type="text" id="txtStreamURL" size="100"></td>
            </tr>
        </table>
    </div>
    <!-- ~~~~~~~~~~~~~~~~~~~~ hidden div end ~~~~~~~~~~~~~~~~~~~~~~-->

<!--===============================================================================================================================================-->
    <div style="height: 66px; display: flex; padding-right: 0;">
        <div style="flex: 0 0 480px;">
            <img id="btn1" src="SD/png/Radio_Yellow.png" alt="radio" onclick="showTab1()">
            <img id="btn2" src="SD/png/Station_Green.png" alt="station" onclick="showTab2()">
            <img id="btn3" src="SD/png/MP3_Green.png" alt="mp3" onclick="showTab3()">
            <img id="btn4" src="SD/png/Button_DLNA_Green.png" alt="mp3" onclick="showTab4()">
            <img id="btn5" src="SD/png/Search_Green.png" alt="search" onclick="showTab5()">
            <img id="btn6" src="SD/png/Settings_Green.png" alt="settings" onclick="showTab6()">
            <img id="btn7" src="SD/png/About_Green.png" alt="info" onclick="showTab7()">
        </div>
        <div style="font-size: 50px; text-align: center; flex: 1; padding-left: 0;">
            MiniWebRadio
        </div>
    </div>
    <hr>
<!--===============================================================================================================================================-->
<!--=================================================================== R A D I O =================================================================-->
<!--===============================================================================================================================================-->
    <div id="tab-content1">
        <div style="height: 66px; display: flex;">
            <div style="flex: 0 0 210px;">
                <img src="SD/png/Button_Previous_Green.png" alt="previous"
                        onmousedown="this.src='SD/png/Button_Previous_Yellow.png'"
                        ontouchstart="this.src='SD/png/Button_Previous_Yellow.png'"
                        onmouseup="this.src='SD/png/Button_Previous_Green.png';"
                        ontouchend="this.src='SD/png/Button_Previous_Green.png';"
                        onclick="socket.send('prev_station')">
                <img src="SD/png/Button_Next_Green.png" alt="next"
                        onmousedown="this.src='SD/png/Button_Next_Yellow.png'"
                        ontouchstart="this.src='SD/png/Button_Next_Yellow.png'"
                        onmouseup="this.src='SD/png/Button_Next_Green.png';"
                        ontouchend="this.src='SD/png/Button_Next_Green.png';"
                        onclick="socket.send('next_station')">
            </div>
            <div style="flex:1;">
                <select class="boxstyle" style="width:100%; margin-top: 14px;" onchange="handleStation(this)" id="preset">
                    <option value="-1">Select a station here</option>
                </select>
            </div>
        </div>
        <div style="display: flex;">
            <div id="div-logo" style="flex: 0 0 210px;">
                <img id="label-logo" src="SD/common/unknown.jpg" alt="img" onclick="socket.send('homepage')"    >
            </div>
            <div id="div-tone-s" style="flex:1; justify-content: center;">
                <div style="width: 380px; height:130px;">

                    <label class="sdr_lbl_left">High:</label>
                    <div class="slidecontainer" style="float: left; width: 180px; height: 40px;">
                        <input type="range" min="0" max="15" value="13" id="HighPass"
                        onmouseup="slider_HP_mouseUp()"
                        ontouchend="slider_HP_mouseUp()"
                        oninput="slider_HP_change()">
                    </div>
                    <label id="label_HP_value" class="sdr_lbl_right">0</label>
                    <label class="sdr_lbl_measure">dB</label>

                    <label class="sdr_lbl_left">Band:</label>
                    <div class="slidecontainer" style="float: left; width: 180px; height: 40px;">
                        <input type="range" min="0" max="15" value="13" id="BandPass"
                        onmouseup="slider_BP_mouseUp()"
                        ontouchend="slider_BP_mouseUp()"
                        oninput="slider_BP_change()">
                    </div>
                    <label id="label_BP_value" class="sdr_lbl_right">0</label>
                    <label class="sdr_lbl_measure">dB</label>

                    <label class="sdr_lbl_left">Low:</label>
                    <div class="slidecontainer" style="float: left; width: 180px; height: 40px;">
                        <input type="range" min="0" max="15" value="13" id="LowPass"
                        onmouseup="slider_LP_mouseUp()"
                        ontouchend="slider_LP_mouseUp()"
                        oninput="slider_LP_change()">
                    </div>
                    <label id="label_LP_value" class="sdr_lbl_right">0</label>
                    <label class="sdr_lbl_measure">dB</label>

                    <label class="sdr_lbl_left">Balance:</label>
                    <div class="slidecontainer" style="float: left; width: 180px; height: 40px;">
                        <input type="range" min="-16" max="16" value="0" id="Balance"
                        onmouseup="slider_BAL_mouseUp()"
                        ontouchend="slider_BAL_mouseUp()"
                        oninput="slider_BAL_change()">
                    </div>
                    <label id="label_BAL_value" class="sdr_lbl_right">0</label>
                    <label class="sdr_lbl_measure"></label>


                </div>
            </div>
        </div>
        <div style="height: 66px; display: flex;">
            <div style="flex: 0 0 210px;">
                <img src="SD/png/Button_Volume_Down_Blue.png" alt="Vol_down"
                    onmousedown="this.src='SD/png/Button_Volume_Down_Yellow.png'"
                    ontouchstart="this.src='SD/png/Button_Volume_Down_Yellow.png'"
                    onmouseup="this.src='SD/png/Button_Volume_Down_Blue.png'"
                    ontouchend="this.src='SD/png/Button_Volume_Down_Blue.png'"
                    onclick="socket.send('downvolume')">
                <img src="SD/png/Button_Volume_Up_Blue.png" alt="Vol_up"
                    onmousedown="this.src='SD/png/Button_Volume_Up_Yellow.png'"
                    ontouchstart="this.src='SD/png/Button_Volume_Up_Yellow.png'"
                    onmouseup="this.src='SD/png/Button_Volume_Up_Blue.png'"
                    ontouchend="this.src='SD/png/Button_Volume_Up_Blue.png'"
                    onclick="socket.send('upvolume')">
                <img id="Mute" src="SD/png/Button_Mute_Green.png" alt="Mute"
                    onmousedown="this.src='SD/png/Button_Mute_Yellow.png'"
                    ontouchstart="this.src='SD/png/Button_Mute_Yellow.png'"
                    onclick="socket.send('setmute')">
            </div>
            <div style="flex:1;">
                <input type="text" class="boxstyle" style="width: calc(100% - 8px); margin-top: 14px; padding-left:7px 0;" id="cmd"
                                   placeholder=" Waiting....">
            </div>
        </div>
        <div style="height: 66px; display: flex;">
            <div style="flex:1;">
                <input type="text" class="boxstyle" style="width: calc(100% - 8px); margin-top: 14px; padding-left:7px 0;" id="station"
                    placeholder=" Enter a streamURL here.... , for authentication streamURL|username|password">
            </div>
            <div style="flex: 0 0 66px;">
                <img src="SD/png/Button_Ready_Blue.png" alt="Vol_up"
                    onmousedown="this.src='SD/png/Button_Ready_Yellow.png'"
                    ontouchstart="this.src='SD/png/Button_Ready_Yellow.png'"
                    onmouseup="this.src='SD/png/Button_Ready_Blue.png'"
                    ontouchend='SD/png/Button_Ready_Blue.png'"
                    onclick="setstation()">
            </div>
        </div>
        <div style="height: 66px; display: flex;">
            <div style="flex:1;">
                <input type="text" class="boxstyle" style="width: calc(100% - 8px); margin-top: 14px; padding-left:7px 0;" id="resultstr1"
                                   placeholder=" Test....">
            </div>
            <div style="flex: 0 0 66px;">
                <img src="SD/png/Button_Test_Green.png" alt="Test"
                    onmousedown="this.src='SD/png/Button_Test_Yellow.png'"
                    ontouchstart="this.src='SD/png/Button_Test_Yellow.png'"
                    onmouseup="this.src='SD/png/Button_Test_Green.png'"
                    ontouchend="this.src='SD/png/Button_Test_Green.png'"
                    onclick="test()">
            </div>
        </div>
        <hr>
        <div style="height: 46px; padding: 0; text-align:center;">
            Find new radio stations at
            <a target="_blank" href="https://radiolise.gitlab.io">Radiolise</a>
            or
            <a target="_blank" href="http://streamstat.net/main.cgi?mode=all"> StreamStat.NET </a>
        </div>
    </div>
<!--===============================================================================================================================================-->
    <div id="tab-content2">
            <div id="notification1" class="notification1"></div>
            <div class="stations-container" style="height: 500px; background-color: white; border: 2px solid black; box-sizing: border-box; ">
                <table class="stations-table" id="stationsTable">
                    <thead>
                        <tr class="stations_tr">
                            <th class="stations-th" style="text-align: center;">Fav</th>
                            <th  class="stations-th" style="text-align: center;">Cy</th>
                            <th class="stations-th">StationName</th>
                            <th class="stations-th">StreamURL</th>
                        </tr>
                    </thead>
                    <tbody class="stations-tbody">
                        <!-- Zeilen werden durch das Skript hinzugefügt -->
                    </tbody>
                </table>
            </div>

            <!-- Kontextmenü -->
            <div id="contextMenu" class="context-menu">
                <div class="context-menu-item" onclick="insertRow(0)">Insert line above</div>
                <div class="context-menu-item" onclick="insertRow(1)">Insert line below</div>
                <div class="context-menu-item" onclick="deleteRow()">Cut row</div>
            </div>

            <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 0px;">
                <p style="flex: 1; min-width: 100px; padding: 0; margin: 0 auto;" id="stationInfo"></p>
                <div style="display: flex; justify-content: center; gap: 10px; margin: 0 auto; width: 100%;">
                    &nbsp;
                    <button class="button_120x30 buttonblue" onclick="saveStations_json()" title="make a backup">save</button>
                    &nbsp;
                    <button class="button_120x30 buttonblue"
                            onclick="javascript:document.getElementById('file1').click();"
                            title="load your own stations list">load
                    </button>
                </div>
            </div>

            <input id="file1" type="file" accept="application/json" style="visibility: hidden;
                             width: 0px;" onchange="loadStations_json(this.files);">
    </div>
<!--===============================================================================================================================================-->
<!--====================================================== P L A Y E R ============================================================================-->
<!--===============================================================================================================================================-->
    <div id="tab-content3">
        <div class="container" id="filetreeContainer">
            <fieldset>
                <legend> Files </legend>
                <div class="filetree-container">
                    <div id="filebrowser">
                        <div id="audioFileTree"></div>
                    </div>
                </div>
                <hr>
                <div style="height: 66px; display: flex;">
                    <div style="flex:1;">
                        <input type="text" class="boxstyle" style="width: calc(100% - 8px);"
                               id="resultstr3" placeholder="Your WebFile-URL....">
                    </div>
                    <div style="flex: 0 0 2px;">
                    </div>
                    <div style="flex: 0 0 42px;">
                        <img src="SD/png/Button_Ready_Blue.png" alt="Upload" title="PLAY WEBFILE"
                            style="width: 42px; height: auto;"
                            onmousedown="this.src='SD/png/Button_Ready_Yellow.png'"
                            ontouchstart="this.src='SD/png/Button_Ready_Yellow.png'"
                            onmouseup="this.src='SD/png/Button_Ready_Blue.png'"
                            ontouchend="this.src='SD/png/Button_Ready_Blue.png'"
                            onclick="playWebURL()">
                    </div>
                    <div style="flex: 0 0 42px;">
                        <img src="SD/png/Button_Upload_Blue.png" alt="Upload" title="UPLOAD TO SD FOLDER"
                            style="width: 98%; height: auto;"
                            onmousedown="this.src='SD/png/Button_Upload_Yellow.png'"
                            ontouchstart="this.src='SD/png/Button_Upload_Yellow.png'"
                            onmouseup="this.src='SD/png/Button_Upload_Blue.png'"
                            ontouchend="this.src='SD/png/Button_Upload_Blue.png'"
                            onclick="javascript:document.getElementById('audioPlayer_File').click();">
                    </div>
                    <div style="flex: 0 0 42px;">
                        <img src="SD/png/Button_Stop_Blue.png" alt="Pause" title="STOP"
                            style="width: 98%; height: auto;"
                            onmousedown="this.src='SD/png/Button_Stop_Yellow.png'"
                            ontouchstart="this.src='SD/png/Button_Stop_Yellow.png'"
                            onmouseup="this.src='SD/png/Button_Stop_Blue.png'"
                            ontouchend="this.src='SD/png/Button_Stop_Blue.png'"
                            onclick="socket.send('stopfile');">
                    </div>
                    <div style="flex: 0 0 42px;">
                        <img src="SD/png/Button_Pause_Resume_Blue.png" alt="Pause Resume" title="PAUSE / RESUME"
                            style="width: 98%; height: auto;"
                            onmousedown="this.src='SD/png/Button_Pause_Resume_Yellow.png'"
                            ontouchstart="this.src='SD/png/Button_Pause_Resume_Yellow.png'"
                            onmouseup="this.src='SD/png/Button_Pause_Resume_Blue.png'"
                            ontouchend="this.src='SD/png/Button_Pause_Resume_Blue.png'"
                            onclick="socket.send('pause_resume');" />
                    </div>
                </div>
                <div id="explorerUploadProgress" class="progress-bar" role="progressbar"> </div>
                <form method="post" accept-charset="utf-8" name="form2">
                    <input id="audioPlayer_File" type="file" accept="audio/*" style="visibility: hidden; width: 0px;"
                    name="audio" onchange="uploadFile(this.files);">
                </form>
            </fieldset>
        </div>
    </div>

<!--===============================================================================================================================================-->
<!--=======================================================  D L N A  =============================================================================-->
<!--===============================================================================================================================================-->
    <div id="tab-content4">
        <center>
            <div style="flex: 0 0 calc(100% - 0px);">
                <select class="boxstyle" style="width: 100%;" onchange="selectserver(this)" id="server">
                    <option value="-1">Select a DLNA Server here</option>
                </select>
                <select class="boxstyle" style="width: 100%; margin-top: 5px;" onchange="select_l1(this)" id="level1">
                    <option value="-1"> </option>
                </select>
                <select class="boxstyle" style="width: 100%; margin-top: 5px;" onchange="select_l2(this)" id="level2">
                    <option value="-1"> </option>
                </select>
                <select class="boxstyle" style="width: 100%; margin-top: 5px;" onchange="select_l3(this)" id="level3">
                    <option value="-1"> </option>
                </select>
                <select class="boxstyle" style="width: 100%; margin-top: 5px;" onchange="select_l4(this)" id="level4">
                    <option value="-1"> </option>
                </select>
                <select class="boxstyle" style="width: 100%; margin-top: 5px;" onchange="select_l5(this)" id="level5">
                    <option value="-1"> </option>
                </select>
                <select class="boxstyle" style="width: 100%; margin-top: 5px;" onchange="select_l6(this)" id="level6">
                    <option value="-1"> </option>
                </select>
                <select class="boxstyle" style="width: 100%; margin-top: 5px;" onchange="select_l7(this)" id="level7">
                    <option value="-1"> </option>
                </select>
                <select class="boxstyle" style="width: 100%; margin-top: 5px;" onchange="select_l8(this)" id="level8">
                    <option value="-1"> </option>
                </select>
            </div>
        </center>

        <div class="container" hidden>
            <fieldset>
                <legend> DLNA </legend>
                <div class="filetree-container">
                    <div>
                        <div id="dlnaFileTree"></div>
                    </div>
                </div>
                <hr>
                <div style="height: 66px; display: flex;">
                    <div style="flex:1;">
                        <input type="text" class="boxstyle" style="width: calc(100% - 8px);"
                               id="resultstr4" placeholder="Waiting for a command....">
                    </div>
                    <div style="flex: 0 0 2px;">
                    </div>
                </div>
            </fieldset>
        </div>

    </div>
<!--===============================================================================================================================================-->
<!--=======================================================  S E A R C H ==========================================================================-->
<!--===============================================================================================================================================-->
    <div id="tab-content5">
        <div style="display: inline-block; width: 400px;">
          This service is provided by
          <a target="_blank" href="http://www.radio-browser.info/">Community Radio Browser</a>
        </div>
        <div style="display: inline-block; padding-right: 0px; width: calc(100% - 480px);">
          <input class="boxstyle" style="width: 100%;" type="text" id="RB_search" placeholder="search..." onkeypress="if(event.key === 'Enter') open_RB_page()">
        </div>
        <div style="display: flex; margin-top: 5px;">
            <div style="flex: 0 0 calc(100% - 66px);">
                <select class="boxstyle" style="width: 100%;" onchange="selectcategory(this)" id="category">
                    <option value="-1">Select a category</option>
                    <option value="bycountry">By country</option>
                    <option value="bylanguage">By language</option>
                    <option value="bytag">By tag</option>
                </select>
                <select class="boxstyle" style="width: 100%; margin-top: 5px;" onchange="selectitem(this)" id="item">
                    <option value="-1">Select a item here</option>
                </select>
                <select class="boxstyle" style="width: 100%; margin-top: 5px;" onchange="selectstation(this)" id="stations">
                    <option value="-1">Select a station here</option>
                </select>
            </div>
        </div>
        <hr>
        <div style="display: flex;">
            <div style="flex: 0 0 calc(100% - 66px); height: 66px;">
                StreamURL
                <input type="text" class="boxstyle" style="width: calc(100% - 8px);"
                id="streamurl" placeholder="StreamURL">
            </div>
             <div style="flex: 2 0 42px;">
                <img src="SD/png/Button_Ready_Blue.png" alt="Vol_up"
                    style="width: 98%; height: auto;"
                    onmousedown="this.src='SD/png/Button_Ready_Yellow.png'"
                    ontouchstart="this.src='SD/png/Button_Ready_Yellow.png'"
                    onmouseup="this.src='SD/png/Button_Ready_Blue.png'"
                    ontouchend="this.src='SD/png/Button_Ready_Blue.png'"
                    onclick="teststreamurl()">
            </div>
        </div>
        <div style="display: flex;">
            <div style="flex: 0 0 calc(100% - 66px); height: 66px;">
                HomepageUrl
                <input type="text" class="boxstyle" style=" width: calc(100% - 8px);" id="homepageurl" placeholder="HomepageURL">
            </div>
            <div style="flex: 1; padding-left: 2px; height: 66px;">
                <img src="SD/png/Button_Ready_Blue.png" alt="Vol_up"
                    onmousedown="this.src='SD/png/Button_Ready_Yellow.png'"
                    ontouchstart="this.src='SD/png/Button_Ready_Yellow.png'"
                    onmouseup="this.src='SD/png/Button_Ready_Blue.png'"
                    ontouchend="this.src='SD/png/Button_Ready_Blue.png'"
                    onclick="window.open(homepageurl.value, '_blank')"/>
            </div>
        </div>
        <div style="display: flex;">
            <div style="flex: 0 0 calc(100% - 66px); height: 66px;">
                LogoUrl
                <input type="text" class="boxstyle" style="width: calc(100% - 8px);" onclick="refreshCanvas()"
                id="favicon" placeholder="Favicon">
            </div>
            <div style="flex: 1;  padding-left: 2px; height: 66px;">
                <img src="SD/png/Button_Ready_Blue.png" alt="Vol_up"
                    onmousedown="this.src='SD/png/Button_Ready_Yellow.png'"
                    ontouchstart="this.src='SD/png/Button_Ready_Yellow.png'"
                    onmouseup="this.src='SD/png/Button_Ready_Blue.png'"
                    ontouchend="this.src='SD/png/Button_Ready_Blue.png'"
                    onclick="window.open(favicon.value, '_blank')"/>
            </div>
        </div>
        <hr>
        <div style="display: flex;">
            <div style="flex: 0 0 130px; padding 1px 5px 5px 1px; ">
                <canvas id="canvas" width="96" height="96" class="playable-canvas"></canvas>
            </div>
            <div style="flex: 1;">
                <div style="flex: 1; height: 38px; padding-left: 10px;">
                    <input type="text" class="boxstyle" style="width: calc(100% - 74px);"
                                id="rb_stationname" placeholder="Change the Stationname here"
                                oninput="validateStationname()"
                                title="Here the station name can be changed. Since the station name is the same as the file name of the logo,
                                 invalid characters colour the content red.">
                </div>
                <div style="flex: 1; justify-content: flex-end; padding-top: 4px; padding-left: 10px;">
                    <img src="SD/png/Button_Upload_Blue.png" alt="Upload" title="UPLOAD TO SD FOLDER"
                        onmousedown="this.src='SD/png/Button_Upload_Yellow.png'"
                        ontouchstart="this.src='SD/png/Button_Upload_Yellow.png'"
                        onmouseup="this.src='SD/png/Button_Upload_Blue.png'"
                        ontouchend="this.src='SD/png/Button_Upload_Blue.png'"
                        onclick="uploadCanvasImage()"/>
                    <img src="SD/png/Button_Download_Blue.png" alt="Download" title="Download to PC"
                        onmousedown="this.src='SD/png/Button_Download_Yellow.png'"
                        ontouchstart="this.src='SD/png/Button_Download_Yellow.png'"
                        onmouseup="this.src='SD/png/Button_Download_Blue.png'"
                        ontouchend="this.src='SD/png/Button_Download_Blue.png'"
                        onclick="downloadCanvasImage()"/>
                    <img src="SD/png/Button_Previous_Blue.png" alt="addGrid" title="Add station to the end of the list"
                        onmousedown="this.src='SD/png/Button_Previous_Yellow.png'"
                        ontouchstart="this.src='SD/png/Button_Previous_Yellow.png'"
                        onmouseup="this.src='SD/png/Button_Previous_Blue.png'"
                        ontouchend="this.src='SD/png/Button_Previous_Blue.png'"
                        onclick="addStationsToGrid()"/>

                    <input class="boxstyle" style="width: 50px; text-align: center; margin-top: 15px;
                           vertical-align: top; margin-left: 10px;" type="text" id="CountryCode" placeholder="CC"
                           title="Country Code">
                </div>
            </div>
            <form method="post" accept-charset="utf-8" name="form1">
                <input name="hidden_data" id="hidden_data" type="hidden">
            </form>
        </div>
    </div>
<!--===============================================================================================================================================-->
    <div id="tab-content6">   <!-- Settings -->
        <table style="width: 100%;">
            <tr>
                <td style="padding: 10px; margin-right: 0px; vertical-align: top; border-right: 3px double #999999; min-width: 365px;">

                    <div style="display: flex;">
                        <div style="width=64px; height=64px;">
                            <img src="SD/png/Remote_Control_Blue.png" alt="IR Settings" title="Remote Control Settings" onmousedown="this.src='SD/png/Remote_Control_Yellow.png'" ontouchstart="this.src='SD/png/Remote_Control_Yellow.png'" onmouseup="this.src='SD/png/Remote_Control_Blue.png'" ontouchend="this.src='SD/png/Remote_Control_Blue.png'" onclick="showTab8()">
                        </div>
                        <div style="font-size: 1.17em; font-weight: bold; padding-left: 10px;">
                            <p> IR Settings </p>
                        </div>
                    </div>
                    <br>
                    <div style="display: flex; padding-bottom: 20px; border-bottom: 3px double #999999;">
                        <div style="width=64px; height=64px;">
                            <img src="SD/png/Button_BT_Blue.png" alt="KCX_BT Settings" title="KCX_BT_Emitter Settings" onmousedown="this.src='SD/png/Button_BT_Yellow.png'" ontouchstart="this.src='SD/png/Button_BT_Yellow.png'" onmouseup="this.src='SD/png/Button_BT_Blue.png'" ontouchend="this.src='SD/png/Button_BT_Blue.png'" onclick="showTab9()">
                        </div>
                        <div style="font-size: 1.17em; font-weight: bold; padding-left: 10px;">
                            <p> KCX_BT_Emitter Settings </p>
                        </div>
                    </div>
                    <div style="margin-top: 0px;  border-bottom: 3px double #999999;">
                        <h3>
                            Timezone
                            <select class="boxstyle" onchange="setTimeZone(this)" id="TimeZoneSelect"></select>
                        </h3>
                    </div>
                    <div>
                        <h3>
                            Time announcement on the hour
                            <input style="transform: scale(1.8); margin: 10px;" type="checkbox" id="chk_timeSpeech"
                                    onclick="socket.send('set_timeAnnouncement=' + document.getElementById('chk_timeSpeech').checked);">
                            <select class="boxstyle" onchange="socket.send('setTimeSpeechLang=' + this.value)" id="timeSpeechLang" name="timeSpeechLang">
                                <option value="fr">fr</option>
                                <option value="en">en</option>
                            </select>
                        </h3>
                    </div>
                </td>
                <td style="padding: 10px; min-width: 350px; margin-left: 0px;">
                    <br>
                    <fieldset>
                        <legend> 12-hour and 24-hour time format </legend>
                        <div>
                            <input type="radio" id="h12" name="timeFormat" value="12h" onclick="socket.send('setTimeFormat=12');">
                            <label for="h12">12h</label>
                        </div>
                        <div>
                            <input type="radio" id="h24" name="timeFormat" value="24h" checked onclick="socket.send('setTimeFormat=24');">
                            <label for="h24">24h</label>
                        </div>
                    </fieldset>
                    <br>
                    <fieldset>
                        <legend> sleep mode </legend>
                        <div>
                            <input type="radio" id="sleepMode0" name="sleepMode" value="display off" onclick="socket.send('setSleepMode=0');">
                            <label for="sleepMode0">display off</label>
                        </div>
                        <div>
                            <input type="radio" id="sleepMode1" name="sleepMode" value="show the time" checked onclick="socket.send('setSleepMode=1');">
                            <label for="sleepMode1">show the time</label>
                        </div>
                    </fieldset>
                    <br>
                    <fieldset>
                        <legend> alarm </legend>
                        <div>
                            <div style="margin-bottom: 10px;">
                                <select id="selRingVolume" style="width: 50px;" onchange="socket.send('setRingVolume=' + this.value);"
                                    title="This is the volume at which the ringtone sounds in the event of an alarm. A value of 0 skips the ringtone
                                    sequence and immediately turns on the radio.">
                                </select>
                                <label for="selRingVolume">Ring Volume: </label>
                                <span class="txtRingVolume" id="txtRingVolume">
                                </span>
                            </div>
                            <div>
                                <select id="selVolumeAfterAlarm" style="width: 50px;" onchange="socket.send('setVolAfterAlarm=' + this.value);"
                                    title="This is the volume at which the radio plays. This volume is maintained as long as it is not manually changed.">
                                </select>
                                <label for="selVolumeAfterAlarm">Radio Volume After Alarm: </label>
                                <span class="txtVolumeAfterAlarm" id="txtVolumeAfterAlarm"></span>
                            </div>
                        </div>
                    </fieldset>
                    <fieldset>
                        <legend> volume steps </legend>
                        <div>
                            <select id="selVolumeSteps" style="width: 50px;" onchange="socket.send('setVolumeSteps=' + this.value);"
                                title= "Specifies the number of volume levels to choose from. 21 is the lowest value.
                                A change adjusts all other volume values to the new default.">
                            </select>
                            <label for="selVolumeSteps">Current Volume Steps: </label>
                            <span class="txtVolumeSteps" id="txtVolumeSteps"></span>
                        </div>
                    </fieldset>
                </td>
            </tr>
        </table>
    </div>
<!--===============================================================================================================================================-->
    <div id="tab-content7">  <!-- Info / About -->
        <p> MiniWebRadio -- Webradio receiver for ESP32, 2.8" or 3.5" color display and  external DAC.
         This project is documented on
            <a target="blank" href="https://github.com/schreibfaul1/ESP32-MiniWebRadio">Github</a>.
            Author: Wolle (schreibfaul1)
        </p>


        <table>
            <tr>
                <label for="label-infopic" onclick="socket.send('hardcopy')">
                    <img id="label-infopic" src="SD/png/MiniWebRadioV3.png" alt="img">
                </label>
            </tr>
        </table>
    </div>
<!--===============================================================================================================================================-->
    <div id="tab-content8">   <!-- IR Settings -->
        <div id="notification2" class="notification1"></div>
        <div>
        <table id="ir_table">
            <tr>
            <th> A </th>
            <th> <input type="text" class="boxstyle_s" id="ir_command_A"> </th>
            <th style="width:180px; text-align: left;"> IR address </th>
            <th> C </th>
            <th> <input type="text" class="boxstyle_s" id="ir_command_C"> </th>
            <th style="width:180px; text-align: left;"> IR command </th>
            <th></th>
            <th></th>
            <th style="width:180px; text-align: left;"> short pressed </th>
            <th></th>
            <th></th>
            <th style="width:180px; text-align: left;"> long pressed </th>
            <th></th>
            </tr>

            <tr>
            <td> 0 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_0" onclick="IRclick(0)"  onkeyup="chIRcmd(0)" onchange="IRupdateJSON(0)"></td>
            <td class="table_cell1"> ZERO </td>
            <td> 10 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_10" onclick="IRclick(10)" onkeyup="chIRcmd(10)" onchange="IRupdateJSON(10)"></td>
            <td class="table_cell2"> MUTE </td>
            <td> 20 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_20" onclick="IRclick(20)" onkeyup="chIRcmd(20)" onchange="IRupdateJSON(20)"></td>
            <td class="table_cell2"> ON/OFF </td>
            <td> 30 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_30" onclick="IRclick(30)" onkeyup="chIRcmd(30)" onchange="IRupdateJSON(30)"></td>
            <td class="table_cell2"> SLEEP </td>
            </tr>

            <tr>
            <td> 1 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_1" onclick="IRclick(1)" onkeyup="chIRcmd(1)" onchange="IRupdateJSON(1)"></td>
            <td class="table_cell1"> ONE</td>
            <td> 11 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_11" onclick="IRclick(11)" onkeyup="chIRcmd(11)" onchange="IRupdateJSON(11)"></td>
            <td class="table_cell2"> ARROW RIGHT </td>
            <td> 21 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_21" onclick="IRclick(21)" onkeyup="chIRcmd(21)" onchange="IRupdateJSON(21)"></td>
            <td class="table_cell2"> RADIO </td>
            <td> 31 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_31" onclick="IRclick(31)" onkeyup="chIRcmd(31)" onchange="IRupdateJSON(31)"></td>
            <td class="table_cell2"> CANCEL </td>
            </tr>

            <tr>
            <td> 2 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_2" onclick="IRclick(2)" onkeyup="chIRcmd(2)" onchange="IRupdateJSON(2)"></td>
            <td class="table_cell1">  TWO </td>
            <td> 12 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_12" onclick="IRclick(12)" onkeyup="chIRcmd(12)" onchange="IRupdateJSON(12)"></td>
            <td class="table_cell2"> ARROW LEFT </td>
            <td> 22 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_22" onclick="IRclick(22)" onkeyup="chIRcmd(22)" onchange="IRupdateJSON(22)"></td>
            <td class="table_cell2"> PLAYER </td>
            <td> 32 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_32" onclick="IRclick(32)" onkeyup="chIRcmd(32)" onchange="IRupdateJSON(32)"></td>
            <td class="table_cell2"> - </td>
            </tr>

            <tr>
            <td> 3 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_3" onclick="IRclick(3)" onkeyup="chIRcmd(3)" onchange="IRupdateJSON(3)"></td>
            <td class="table_cell1">  THREE </td>
            <td> 13 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_13" onclick="IRclick(13)" onkeyup="chIRcmd(13)" onchange="IRupdateJSON(13)"></td>
            <td class="table_cell2"> ARROW DOWN </td>
            <td> 23 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_23" onclick="IRclick(23)" onkeyup="chIRcmd(23)" onchange="IRupdateJSON(23)"></td>
            <td class="table_cell2"> DLNA </td>
            <td></td>
            <td></td>
            <td></td>
            </tr>

            <tr>
            <td> 4 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_4" onclick="IRclick(4)" onkeyup="chIRcmd(4)" onchange="IRupdateJSON(4)"></td>
            <td class="table_cell1"> FOUR </td>
            <td> 14 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_14" onclick="IRclick(14)" onkeyup="chIRcmd(14)" onchange="IRupdateJSON(14)"></td>
            <td class="table_cell2"> ARROW UP </td>
            <td> 24 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_24" onclick="IRclick(24)" onkeyup="chIRcmd(24)" onchange="IRupdateJSON(24)"></td>
            <td class="table_cell2"> CLOCK </td>
            <td></td>
            <td></td>
            <td></td>
            </tr>

            <tr>
            <td> 5 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_5" onclick="IRclick(5)" onkeyup="chIRcmd(5)" onchange="IRupdateJSON(5)"></td>
            <td class="table_cell1"> FIVE </td>
            <td> 15 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_15" onclick="IRclick(15)" onkeyup="chIRcmd(15)" onchange="IRupdateJSON(15)"></td>
            <td class="table_cell2"> MODE </td>
            <td> 25 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_25" onclick="IRclick(25)" onkeyup="chIRcmd(25)" onchange="IRupdateJSON(25)"></td>
            <td class="table_cell2"> OFF TIMER </td>
            <td></td>
            <td></td>
            <td></td>
            </tr>

            <tr>
            <td> 6 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_6" onclick="IRclick(6)" onkeyup="chIRcmd(6)" onchange="IRupdateJSON(6)"></td>
            <td class="table_cell1"> SIX </td>
            <td> 16 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_16" onclick="IRclick(16)" onkeyup="chIRcmd(16)" onchange="IRupdateJSON(16)"></td>
            <td class="table_cell2"> OK </td>
            <td> 26 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_26" onclick="IRclick(26)" onkeyup="chIRcmd(26)" onchange="IRupdateJSON(26)"></td>
            <td class="table_cell2"> VOLUME+ </td>
            <td></td>
            <td></td>
            <td></td>
            </tr>

            <tr>
            <td> 7 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_7" onclick="IRclick(7)" onkeyup="chIRcmd(7)" onchange="IRupdateJSON(7)"></td>
            <td class="table_cell1"> SEVEN </td>
            <td> 17 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_17" onclick="IRclick(17)" onkeyup="chIRcmd(17)" onchange="IRupdateJSON(17)"></td>
            <td class="table_cell2"> - </td>
            <td> 27 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_27" onclick="IRclick(27)" onkeyup="chIRcmd(27)" onchange="IRupdateJSON(27)"></td>
            <td class="table_cell2"> VOLUME- </td>
            <td></td>
            <td></td>
            <td></td>
            </tr>

            <tr>
            <td> 8 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_8" onclick="IRclick(8)" onkeyup="chIRcmd(8)" onchange="IRupdateJSON(8)"></td>
            <td class="table_cell1"> EIGHT </td>
            <td> 18 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_18" onclick="IRclick(18)" onkeyup="chIRcmd(18)" onchange="IRupdateJSON(18)"></td>
            <td class="table_cell2"> PAUSE/RESUME </td>
            <td> 28 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_28" onclick="IRclick(28)" onkeyup="chIRcmd(28)" onchange="IRupdateJSON(28)"></td>
            <td class="table_cell2"> << 30s </td>
            <td></td>
            <td></td>
            <td></td>
            </tr>

            <tr>
            <td> 9 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_9" onclick="IRclick(9)" onkeyup="chIRcmd(9)" onchange="IRupdateJSON(9)"></td>
            <td class="table_cell1"> NINE </td>
            <td> 19 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_19" onclick="IRclick(19)" onkeyup="chIRcmd(19)" onchange="IRupdateJSON(19)"></td>
            <td class="table_cell2"> STOP </td>
            <td> 29 </td>
            <td> <input type="text" class="boxstyle_s" id="ir_command_29" onclick="IRclick(29)" onkeyup="chIRcmd(29)" onchange="IRupdateJSON(29)"></td>
            <td class="table_cell2"> >> 30s </td>
            <td></td>
            <td></td>
            <td></td>
            </tr>
        </table>
        <br>

        <div style="display: flex; justify-content: center; gap: 10px; margin: 0 auto; width: 100%;">
            &nbsp;
            <button class="button_120x30 buttonblue" onclick="saveIRbuttons_json()" title="make a backup">save</button>
            &nbsp;
            <button class="button_120x30 buttonblue"
                    onclick="javascript:document.getElementById('file2').click();"
                    title="load your own ir buttons">load
            </button>
        </div>
        <input id="file2" type="file" accept="application/json" style="visibility: hidden;
                    width: 0px;" onchange="loadIRbuttons_json(this.files);">

        <p>Here you can assign a function to the buttons on your NEC remote control. Press a button on your remote
           control and then click on the text field for the desired function. You can test the function immediately.
           Once all the keys you want are assigned, save the settings. This process only needs to be done once.</p>
        </div>
    </div>
<!--===============================================================================================================================================-->
    <div id="tab-content9"> <!-- KCX BT Emitter Settings -->
        <div style="display:flex">
            <div id="div-BT-logo" style="flex: 0 0 150px;">
                <img id="label-bt-logo" onclick="socket.send('KCX_BT_connected')">
                <label for="label-logo" id="label-bt-mode"> unknown </label>
            </div>
            <div style="flex: 1 0; position: relative;  padding-top: 20px;">
            <div style="height: 223px; overflow-x: hidden; overflow-y: auto; width: 640px;" >
                <table>
                    <thead>
                    <tr>
                    <th style="height: 0;"></th>
                    <th style="height: 0;"><div style="position: absolute; top: 0; margin-left: 25px;"> saved BT MAC address </div></th>
                    <th style="height: 0;"></th>
                    <th style="height: 0;"><div style="position: absolute; top: 0; margin-left: 25px;"> saved BT Name </div></th>
                    </tr>
                    </thead>

                    <tbody>
                    <tr>
                    <td class="table_cell1"> 0 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_addr" readonly></td>
                    <td class="table_cell1"> 0 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_name" readonly></td>
                    </tr>

                    <tr>
                    <td class="table_cell1"> 1 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_addr" readonly></td>
                    <td class="table_cell1"> 1 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_name" readonly></td>
                    </tr>

                    <tr>
                    <td class="table_cell1"> 2 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_addr" readonly></td>
                    <td class="table_cell1"> 2 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_name" readonly></td>
                    </tr>

                    <tr>
                    <td class="table_cell1"> 3 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_addr" readonly></td>
                    <td class="table_cell1"> 3 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_name" readonly></td>
                    </tr>

                    <tr>
                    <td class="table_cell1"> 4 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_addr" readonly></td>
                    <td class="table_cell1"> 4 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_name" readonly></td>
                    </tr>

                    <tr>
                    <td class="table_cell1"> 5 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_addr" readonly></td>
                    <td class="table_cell1"> 5 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_name" readonly></td>
                    </tr>

                    <tr>
                    <td class="table_cell1"> 6 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_addr" readonly></td>
                    <td class="table_cell1"> 6 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_name" readonly></td>
                    </tr>

                    <tr>
                    <td class="table_cell1"> 7 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_addr" readonly></td>
                    <td class="table_cell1"> 7 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_name" readonly></td>
                    </tr>

                    <tr>
                    <td class="table_cell1"> 8 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_addr" readonly></td>
                    <td class="table_cell1"> 8 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_name" readonly></td>
                    </tr>

                    <tr>
                    <td class="table_cell1"> 9 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_addr" readonly></td>
                    <td class="table_cell1"> 9 </td>
                    <td> <input type="text" class="boxstyle_m" name="bt_name" readonly></td>
                    </tr>
                    </tbody>
                </table>
            </div>
            </div>
        </div>
        <br>
        <div style="display:flex">
            <div style="flex: 0 0 150px;">
                <button class="button_120x30 buttonblue"
                   onclick="socket.send('KCX_BT_changeMode')"
                   onmousedown="this.style.backgroundColor='#D62C1A'"
                   ontouchstart="this.style.backgroundColor='#D62C1A'"
                   onmouseup="this.style.backgroundColor='blue'"
                   ontouchend="this.style.backgroundColor='blue'"
                   id="BT Mode" title="receive <---> transmit">BT Mode
                </button>
            </div>
            <div style="flex: 1 0; padding-left: 20px;">
                &nbsp;
                <button class="button_120x30 buttonred"
                    onclick="clear_BT_memItems()"
                    onmousedown="this.style.backgroundColor='black'"
                    ontouchstart="this.style.backgroundColor='black'"
                    onmouseup="this.style.backgroundColor='red'"
                    ontouchend="this.style.backgroundColor='red'"
                    id="loadSD" title="cleat all saved items">Clear
                </button>
            </div>
        </div>
        <br>
        <div style="display:flex">
            <div style="flex: 0 0 150px;">
                <table>
                    <thead>
                    <tr>
                    <th style=""> scanned BT MAC address</th>
                    <th style=""></th>
                    <th style=""> scanned BT Name </th>
                    <th style=""></th>
                    </tr>
                    </thead>

                    <tbody>
                    <tr>
                    <td> <input type="text" class="boxstyle_200x36" name="bt_scan_addr"> </td>
                    <td class="table_cell1"> <button class="button_20x36 buttongreen"  onclick="addScannedAddr(0)"
                                                                                       onmousedown="this.style.backgroundColor='#D62C1A'"
                                                                                       ontouchstart="this.style.backgroundColor='#D62C1A'"
                                                                                       onmouseup="this.style.backgroundColor='#128F76'"
                                                                                       ontouchend="this.style.backgroundColor='#128F76'"
                                                                                       id="addName1" title="add BT Name to the table">+
                                                                                       </button></td>
                    <td> <input type="text" class="boxstyle_200x36" name="bt_scan_name"> </td>
                    <td class="table_cell1"> <button class="button_20x36 buttongreen"  onclick="addScannedName(0)"
                                                                                       onmousedown="this.style.backgroundColor='#D62C1A'"
                                                                                       ontouchstart="this.style.backgroundColor='#D62C1A'"
                                                                                       onmouseup="this.style.backgroundColor='#128F76'"
                                                                                       ontouchend="this.style.backgroundColor='#128F76'"
                                                                                       id="addName2" title="add BT Name to the table">+
                                                                                       </button></td>
                    </tr>
                    <tr>
                    <td> <input type="text" class="boxstyle_200x36" name="bt_scan_addr"> </td>
                    <td class="table_cell1"> <button class="button_20x36 buttongreen"  onclick="addScannedAddr(1)"
                                                                                       onmousedown="this.style.backgroundColor='#D62C1A'"
                                                                                       ontouchstart="this.style.backgroundColor='#D62C1A'"
                                                                                       onmouseup="this.style.backgroundColor='#128F76'"
                                                                                       ontouchend="this.style.backgroundColor='#128F76'"
                                                                                       id="addName3" title="add BT Name to the table">+
                                                                                       </button></td>
                    <td> <input type="text" class="boxstyle_200x36" name="bt_scan_name"> </td>
                    <td class="table_cell1"> <button class="button_20x36 buttongreen"  onclick="addScannedName(1)"
                                                                                       onmousedown="this.style.backgroundColor='#D62C1A'"
                                                                                       ontouchstart="this.style.backgroundColor='#D62C1A'"
                                                                                       onmouseup="this.style.backgroundColor='#128F76'"
                                                                                       ontouchend="this.style.backgroundColor='#128F76'"
                                                                                       id="addName4" title="add BT Name to the table">+
                                                                                       </button></td>
                    </tr>
                    <tr>
                    <td> <input type="text" class="boxstyle_200x36" name="bt_scan_addr"> </td>
                    <td class="table_cell1"> <button class="button_20x36 buttongreen"  onclick="addScannedAddr(2)"
                                                                                       onmousedown="this.style.backgroundColor='#D62C1A'"
                                                                                       ontouchstart="this.style.backgroundColor='#D62C1A'"
                                                                                       onmouseup="this.style.backgroundColor='#128F76'"
                                                                                       ontouchend="this.style.backgroundColor='#128F76'"
                                                                                       id="addName5" title="add BT Name to the table">+
                                                                                       </button></td>
                    <td> <input type="text" class="boxstyle_200x36" name="bt_scan_name"> </td>
                    <td class="table_cell1"> <button class="button_20x36 buttongreen"  onclick="addScannedName(2)"
                                                                                       onmousedown="this.style.backgroundColor='#D62C1A'"
                                                                                       ontouchstart="this.style.backgroundColor='#D62C1A'"
                                                                                       onmouseup="this.style.backgroundColor='#128F76'"
                                                                                       ontouchend="this.style.backgroundColor='#128F76'"
                                                                                       id="addName6" title="add BT Name to the table">+
                                                                                       </button></td>
                    </tr>
                    </tbody>
                </table>
            </div>
            <div style="flex: 1 0; padding-left: 20px;">
                <img src="SD/png/Button_Volume_Down_Blue.png" alt="BT_Vol_down"
                    onmousedown="this.src='SD/png/Button_Volume_Down_Yellow.png'"
                    ontouchstart="this.src='SD/png/Button_Volume_Down_Yellow.png'"
                    onmouseup="this.src='SD/png/Button_Volume_Down_Blue.png'"
                    ontouchend="this.src='SD/png/Button_Volume_Down_Blue.png'"
                    onclick="socket.send('KCX_BT_downvolume')">
                <img src="SD/png/Button_Volume_Up_Blue.png" alt="BT_Vol_up"
                    onmousedown="this.src='SD/png/Button_Volume_Up_Yellow.png'"
                    ontouchstart="this.src='SD/png/Button_Volume_Up_Yellow.png'"
                    onmouseup="this.src='SD/png/Button_Volume_Up_Blue.png'"
                    ontouchend="this.src='SD/png/Button_Volume_Up_Blue.png'"
                    onclick="socket.send('KCX_BT_upvolume')">
                <img src="SD/png/Button_Pause_Blue.png" alt="BT_Pause"
                    onmousedown="this.src='SD/png/Button_Pause_Yellow.png'"
                    ontouchstart="this.src='SD/png/Button_Pause_Yellow.png'"
                    onmouseup="this.src='SD/png/Button_Pause_Blue.png'"
                    ontouchend="this.src='SD/png/Button_Pause_Blue.png'"
                    onclick="socket.send('KCX_BT_pause')">
                <img id="BT_Power" src="SD/png/BT_Blue.png" alt="BT_Pause"
                    onmousedown="this.src='SD/png/BT_Yellow.png'"
                    ontouchstart="this.src='SD/png/BT_Yellow.png'"
                    onclick="socket.send('KCX_BT_power')"
                    title="switch Bluetooth on/off">
            </div>
        </div>
    </div>
<!--===============================================================================================================================================-->

</div>

<script src="index.js"></script>

</body>
</html>

)=====";

#endif /* INDEX_H_ */
