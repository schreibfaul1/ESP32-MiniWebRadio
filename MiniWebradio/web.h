/*
 *  web.h
 *
 *  Created on: 04.10.2018
 *      Author: Wolle
 *
 *  does not work with MS Internetexplorer
 *  successfully tested with Chrome, MS Edge and Opera
 *
 */

#ifndef WEB_H_
#define WEB_H_

#include "Arduino.h"

// file in raw data format for PROGMEM
//

const char web_html[] PROGMEM = R"=====(
<!DOCTYPE HTML>

<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/jqueryui/1.12.1/jquery-ui.min.css" />
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/jsgrid/1.5.3/jsgrid.min.css" />      
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/jsgrid/1.5.3/jsgrid-theme.min.css" />

/*    <link rel="stylesheet" href="SD/css/jquery-ui.css" />     */
/*    <link rel="stylesheet" href="SD/css/jsgrid.css" />        */
/*    <link rel="stylesheet" href="SD/css//jsgrid-theme.css" /> */
    
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
            min-width: 720px;
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
            border-radius: 5px;
        }
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
            border-radius: 5px;
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
            border-radius: 5px;
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
        #label-logo {  //tab1
            left 10px;
            margin-left: 20px;
            border:2px solid black;
            display:inline-block;
            background-image: url("SD/unknown.jpg");
            width: 96px;
            height:96px;
        }
        canvas {
            left: 0px;
            margin-left: 0px;
            display: inline-block;
            width: 96px;
            height: 96px;
            border: 2px solid black;
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
        .jsgrid-header-cell {
            padding: 0.1em 0.1em !important;
        }
        .jsgrid-cell {
            overflow: hidden !important;
            white-space: nowrap !important;
            padding: 0.1em 0.2em !important;
        }
        .ui-widget-header {
            background: #11e9e9 !important;
        }
        .ui-dialog .ui-dialog-buttonpane {
            border-width: 0 0 0 0 !important;
            margin-top: 0 !important;
            padding: 0 0 0 1em !important;
        }
        .ui-dialog .ui-dialog-content {
            margin-top: 0.3em !important;
            padding: 0.1em 0.1em !important;
        }
    </style>
</head>

<script>

    //global variables and functions
    var treble_dB = ["-12,0","-10,5"," -9,0"," -7,5"," -6,0"," -4,5"," -3,0"," -1,5",
                     "  0,0"," +1,5"," +3,0"," +4,5"," +6,0"," +7,5"," +9,0","+10,5"];
    var treble_val= [8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7];

    window.onload = function() { // Fill configuration initially
        document.getElementById("dialog").style.display = "none"; // hide the div (its only a template)        
        gettone(); //Now load the tones (tab Radio)
        httpGet("to_listen", 1);
        getnetworks();
        showExcelGrid();
        loadGridFileFromSD();
        getmute();
    }
    function showTab1(){
        console.log("tab-content1 (Radio)");
        document.getElementById("tab-content1").style.display = "block";
        document.getElementById("tab-content2").style.display = "none";
        document.getElementById("tab-content3").style.display = "none";
        document.getElementById("tab-content4").style.display = "none";
        document.getElementById("tab-content5").style.display = "none";
        getmute();
    }
    function showTab2(){
        console.log("tab-content2 (Stations)");
        document.getElementById("tab-content1").style.display = "none";
        document.getElementById("tab-content2").style.display = "block";
        document.getElementById("tab-content3").style.display = "none";
        document.getElementById("tab-content4").style.display = "none";
        document.getElementById("tab-content5").style.display = "none";
        $("#jsGrid").jsGrid("refresh");
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
        console.log("tab-content4 (Search Stations)");
        document.getElementById("tab-content1").style.display = "none";
        document.getElementById("tab-content2").style.display = "none";
        document.getElementById("tab-content3").style.display = "none";
        document.getElementById("tab-content4").style.display = "block"
        document.getElementById("tab-content5").style.display = "none";    
        loadJSON("http://www.radio-browser.info/webservice/json/countries", gotCountries, 'jsonp');   
    }
    function showTab5(){
        console.log("tab-content5 (About)");
        document.getElementById("tab-content1").style.display = "none";
        document.getElementById("tab-content2").style.display = "none";
        document.getElementById("tab-content3").style.display = "none";
        document.getElementById("tab-content4").style.display = "none";
        document.getElementById("tab-content5").style.display = "block";      
    }
    function uploadTextFile(fileName, content ) {
        var fd = new FormData();
        fd.append("Text=", content);
        var theUrl = "/?uploadfile=" + fileName + "&version=" + Math.random();
        var xhr = new XMLHttpRequest();
        xhr.open('POST', theUrl, true);
        xhr.onreadystatechange = function() {   //Call a function when the state changes.
            if(xhr.readyState == 4) {
                if(xhr.responseText=="OK") alert(fileName + ' succesfully uploaded');
                else alert(fileName + ' not uploaded')
            }
        }
        xhr.send(fd);   // send
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
        var file="url(url=SD/logo/" + src + ".jpg)";
      //file=file.split(',').join('.'); //replace commas in dots, Miniradio has no commas in filenames
        document.getElementById(id).style.backgroundImage=file;
    }
 
    function httpGet(theReq, nr) {  // universal request prev, next, vol,  mute...
        var theUrl = "/?" + theReq + "&version=" + Math.random();
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() {
            if(xhr.readyState == XMLHttpRequest.DONE ) {
                if(nr==1) {
                    if(theReq.startsWith ("prev_station")||
                        theReq.startsWith("next_station")||
                        theReq.startsWith("set_station=")||
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
                        res = res.substring(n+1);       // remove stationnumber
                        n = res.indexOf(" ");
                        url = res.substring(0, n);      // streamURL
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

    function gettone() {   // tab Radio: get tones values and set them
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

    function getmute(){
        var mute;            
        var theUrl = "/?getmute" + "&version=" + Math.random();
        var xhr = new XMLHttpRequest() ;
        xhr.onreadystatechange = function() {
            if ( xhr.readyState == XMLHttpRequest.DONE ) {
                if(xhr.responseText==1) // muteOn
                    document.getElementById("Mute").src="SD/png/Button_Mute_Red.png";
                else
                    document.getElementById("Mute").src="SD/png/Button_Mute_Green.png";
            }
        }
        xhr.open ( "GET", theUrl, true) ;
        xhr.send();
    }


    function handleStation(presctrl) {  // tab Radio: preset, select a station
        httpGet("set_station=" + (presctrl.value), 1);
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
        var theUrl = "/?stationURL=" + station.value + "&version=" + Math.random();
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() {
            if(xhr.readyState == XMLHttpRequest.DONE) {
                resultstr1.value = xhr.responseText;
                showLabel('label-logo', 'unknown');
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
       
    function saveGridFileToSD(){  // save to SD
        var ws_data = $("#jsGrid").jsGrid("option", "data");
        var strJSON = JSON.stringify(ws_data);
        var objJSON = JSON.parse(strJSON);
        console.log(ws_data.length);
        var txt="";
        var l;
        var c;
        select = document.getElementById("preset");  // Radio: show stationlist
        select.options.length=1;                   
        var j=1;
        txt="X\tCy\tStationName\t\t\t\t\t\t\tStreamURL\t\t\t\t\t\t\t\t\t\t\t\t\STsubstitute\r\n"
        for (var i = 0; i<ws_data.length; i++) {
            c="";
            if(objJSON[i].X){
                c=objJSON[i].X; l=c.length;
                while(l<8){c=c+'\t'; l+=8;} txt+=c;
            }
            else txt+='\t';
            if(objJSON[i].X != '*'){
                if(j<1000){ 
                    opt = document.createElement("OPTION");
                    opt.text = (("000" + j).slice(-3)+ " - " + objJSON[i].StationName );
                    select.add(opt);
                }
                j++;
            }
            if(objJSON[i].Cy){
                c=objJSON[i].Cy; l=c.length;
                while(l<8){c=c+'\t'; l+=8;} txt+=c;
            }
            else txt+='\t';
            if(objJSON[i].StationName){
                c=objJSON[i].StationName; l=c.length;
                while(l<(8*8)){c=c+'\t'; l+=8;} txt+=c;
            }
            else txt+='\t';
            if(objJSON[i].StreamURL){
                c=objJSON[i].StreamURL; l=c.length;
                while(l<(8*13)){c=c+'\t'; l+=8;} txt+=c;
            }
            else txt+='\t';
            if(objJSON[i].STsubstitute){
                txt+=objJSON[i].STsubstitute;
            }
            txt+='\r';
            txt+='\n';
        }
        uploadTextFile("stations.txt", txt);
        updateStationlist();
    }
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    function loadGridFileFromSD(){ // load from SD
        var XL_row_object;
        var rawFile = new XMLHttpRequest();
        rawFile.open("POST", "/SD/stations.txt", true);
        rawFile.onreadystatechange = function(){
            if (rawFile.readyState === 4){
                var rawdata = rawFile.responseText;
                rawdata=rawdata.replace(/(\t\t)/g,'\t'); // shrink more tabs to one tab in 3 steps
                rawdata=rawdata.replace(/(\t\t)/g,'\t');
                rawdata=rawdata.replace(/(\t\t)/g,'\t');
                var workbook = XLSX.read(rawdata, {raw: true, type: "string", cellDates:false, cellText:true});
                workbook.SheetNames.forEach(function(sheetName){ 
                    XL_row_object = XLSX.utils.sheet_to_row_object_array(workbook.Sheets[sheetName]);
                });
                var strJSON = JSON.stringify(XL_row_object);
                var objJSON = JSON.parse(strJSON);
                $("#jsGrid").jsGrid({data: objJSON});
                updateStationlist();
            };
        }
        rawFile.send();
    }
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    function saveExcel() {  // save xlsx to PC
        var wb = XLSX.utils.book_new();
        wb.Props = {
            Title: "Stations",
            Subject: "Stationlist",
            Author: "MiniWebRadio",
            CreatedDate: new Date("2018.10.10")
        };
        wb.SheetNames.push("Stations");
        var ws_data = $("#jsGrid").jsGrid("option", "data");
        var wscols = [
            { wch: 3 },     // "characters"
            { wch: 5 },     // "characters"
            { wch: 100 },   // "characters"
            { wch: 150 },   // "characters"
            { wch: 200 }    // "characters"
        ];
        var ws = XLSX.utils.json_to_sheet(ws_data, {
            header: ["X", "Cy", "StationName", "StreamURL", "STsubstitute"]
        });
        ws["!cols"] = wscols;
        wb.Sheets["Stations"] = ws;
    
        var wbout = XLSX.write(wb, { bookType: "xlsx", type: "binary" });
    
        function s2ab(s) {
            var buf = new ArrayBuffer(s.length);
            var view = new Uint8Array(buf);
            for (var i = 0; i < s.length; i++) view[i] = s.charCodeAt(i) & 0xff;
            return buf;
        }
        saveAs(
            new Blob([s2ab(wbout)], { type: "application/octet-stream" }),
            "stations.xlsx"
        );
        updateStationlist();
    }
    


    var clients = [  // testdata
        {
            X: "*",
            Cy: "D",
            StationName: "Station",
            StreamURL: "URL",
            STsubstitute: ""
        }
    ];

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    
    function showExcelGrid() {
        $("#jsGrid").jsGrid({
            width: "100%",
            height: "432px",
            editing: true,
            sorting: true,
            paging: false,
            shrinkToFit: false,
            onItemDeleted: function(args) {updateStationlist();},
            onItemUpdated: function(args) {updateStationlist();},
            onItemInserted: function(args){updateStationlist();},
            deleteConfirm: function(item) {
                return 'The entry "' + item.StationName + '" will be removed. Are you sure?';
            },
            rowClick: function(args) {
                showDetailsDialog("Edit", args.item);
            },
            data: clients,
            fields: [
                { name: "X", type: "text", width: 20, align: "center" },
                { name: "Cy", type: "text", width: 25, align: "center" },
                { name: "StationName", type: "text", width: 170 },
                { name: "StreamURL", type: "text", width: 320 },
                { name: "STsubstitute", type: "text", width: 90 },
                {
                    type: "control",
                    modeSwitchButton: false,
                    editButton: false,
                    shrinkToFit: true,
                    headerTemplate: function() {
                        return $("<button>")
                        .attr("type", "button")
                        .text("Add")
                        .on("click", function() {
                            showDetailsDialog("Add", {});
                        });
                    }
                }
            ]
        });
    }



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    
    var showDetailsDialog = function(dialogType, client) { // popUp window
        $("#txtX").val(client.X);
        $("#txtCy").val(client.Cy);
        $("#txtStationName").val(client.StationName);
        $("#txtStreamURL").val(client.StreamURL);
        $("#txtStSub").val(client.STsubstitute);
        var divdialog = $("#dialog");
        $("#dialog").attr("title", "Edit");
        $("#dialog").dialog({
            width: 505,
            resizable: false,
            show: "fade",
            modal: false,
            buttons: {
                OK: function() {
                    client.X = $("#txtX").val();
                    client.Cy = $("#txtCy").val();
                    client.StationName = $("#txtStationName").val();
                    client.StreamURL = $("#txtStreamURL").val();
                    client.STsubstitute = $("#txtStSub").val();
                    includeStation(client, dialogType === "Add");
                    $(this).dialog("close");
                    console.log("dialog saved");
                }
            }
        });
        divdialog.dialog();
        console.log("dialog opened");
    };
    
    var includeStation = function(client, isNew) {
        $.extend(client, {
            X: $("#txX").val(),
            Cy: $("#txtCy").val(),
            StationName: $("#txtStationName").val(),
            StreamURL: $("#txtStreamURL").val(),
            StSub: $("#txtStSub").val()
        });
    
        $("#jsGrid").jsGrid(isNew ? "insertItem" : "updateItem", client);
        $("#detailsDialog").dialog("close");
    };
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    
    function loadDataExcel(event) {  // load xlsx from PC
        var file = event[0];
        var reader = new FileReader();
        var excelData = [];
        reader.onload = function(event) {
            var data = event.target.result;
            var workbook = XLSX.read(data, {
                type: "binary"
            });
            workbook.SheetNames.forEach(function(sheetName) {
                // Here is your object
                var XL_row_object = XLSX.utils.sheet_to_row_object_array(
                    workbook.Sheets[sheetName]
                );
    
                for (var i = 0; i < XL_row_object.length; i++) {
                    excelData.push(XL_row_object[i]["your column name"]);
                }
    
                var strJSON = JSON.stringify(XL_row_object);
                var objJSON = JSON.parse(strJSON);
                //alert(strJSON);
                $("#jsGrid").jsGrid({
                    data: objJSON
                });
                updateStationlist();
            });
        };
        $("#file").val(''); // allow load twice
        reader.onerror = function(ex) {
            console.log(ex);
        };
    
        reader.readAsBinaryString(file);
    }
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    
    function updateStationlist(){  // select in tab Radio
        var ws_data = $("#jsGrid").jsGrid("option", "data");
        var strJSON = JSON.stringify(ws_data);
        var objJSON = JSON.parse(strJSON);
        console.log(ws_data.length);
        select = document.getElementById("preset");  // Radio: show stationlist
        select.options.length=1; 
        j=1;
        for (var i = 0; i<ws_data.length; i++) {
            if(objJSON[i].X != '*'){
                if(j<1000){ 
                    opt = document.createElement("OPTION");
                    opt.text = (("000" + j).slice(-3)+ " - " + objJSON[i].StationName );
                    select.add(opt);
                }
                j++;
            }
        }
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
    //----------------------------------- TAB Search Stations ------------------------------------

    // global var
    var countryallstations

    function addStationsToGrid(){
        showDetailsDialog('Add', {});
        $("#txtStreamURL").val($("#streamurl").val());
        $("#txtStationName").val($("#stationname").val());
    }
    
    function loadJSON(path, success, error) {
        console.log(path);
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function () {
            if (xhr.readyState === 4) {
                if (xhr.status === 200) {
                    success(JSON.parse(xhr.responseText));
                }
                else {
                    error(xhr);
                }
            }
        };
        xhr.open('GET', path, true);
        xhr.send();
    }

    function selectcountry(presctrl) {  // tab Radio: preset, select a station
        loadJSON("http://www.radio-browser.info/webservice/json/stations/bycountry/" + presctrl.value , gotStations, 'jsonp');
    }

    function gotCountries(data){  // fill select countries
        var select=select = document.getElementById("country");
        var opt;
        select.options.length=1;
        for(var i=0; i< data.length; i++){
            if(i<2) continue;
            opt = document.createElement("OPTION");
            opt.text = data[i].name;
            select.add(opt);            
        }
        console.log(data.uuid);
    }
    
    function gotStations(data){  // fill select stations
        var select=select = document.getElementById("stations");
        var opt;
        select.options.length=1;
        for(var i=0; i< data.length; i++){
            opt = document.createElement("OPTION");
            opt.text = data[i].name;
            opt.value=i;
            select.add(opt);            
        }
        countryallstations=data;
    }

    function selectstation(sd) {  // select a station
        var e = document.getElementById("stations");
        var value = e.options[e.selectedIndex].value;
        sturl=countryallstations[value].url;
        console.log(value);
        console.log(sturl);
        var f = document.getElementById("streamurl");
        f.value=sturl;
        var g = document.getElementById("favicon");
        var favi=countryallstations[value].favicon;
        g.value=favi;
        var h = document.getElementById("homepageurl");
        h.value=countryallstations[value].homepage;
        scaleCanvasImage(favi);
        var i = document.getElementById("stations");
        var j = document.getElementById("stationname");
        j.value=countryallstations[value].name;
    }

    function teststreamurl() {  // Search: button play - enter a url to play from
        var theUrl = "/?stationURL=" + streamurl.value + "&version=" + Math.random();
        var xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() {
            if(xhr.readyState == XMLHttpRequest.DONE) {
            }
        }
        xhr.open ( "GET", theUrl, true);
        xhr.send() ;
    }

    function scaleCanvasImage(url) {
        var canvas = document.getElementById("canvas");
        var ctx = canvas.getContext("2d");
        var src;
        ctx.beginPath();
        ctx.rect(0, 0, canvas.width, canvas.height);
        ctx.fillStyle = "white";
        ctx.fill();       
        var co = "https://cors-anywhere.herokuapp.com/";
        src = co + url;
        var img_obj = new Image();
        img_obj.onload = function() {
            var imgWidth = img_obj.width;
            var imgHeight = img_obj.height;
            var scaleX = 1;
            var scaleY = 1;
            if (imgWidth > canvas.width) scaleX = canvas.width / imgWidth;
            if (imgHeight > canvas.height) scaleY = canvas.height / imgHeight;
            var scale = scaleY;
            if (scaleX < scaleY) scale = scaleX;
            if (scale < 1) {
                imgHeight = imgHeight * scale;
                imgWidth = imgWidth * scale;
            }
            var dx = (canvas.width - imgWidth) / 2;
            var dy = (canvas.height - imgHeight) / 2;
            ctx.drawImage(img_obj, 0, 0, img_obj.width, img_obj.height, dx, dy, imgWidth, imgHeight);
        }
        img_obj.crossOrigin = 'anonymous';
        img_obj.src = src;
    }

    function refreshCanvas(){
        var g = document.getElementById("favicon");
        scaleCanvasImage(g.value);
        console.log("refresh");
    }

    function uploadCanvasImage() {
        var sn = document.getElementById("stationname");
        if(sn.value != "") filename=sn.value+".jpg"; else filename="myimage.jpg";
        var canvas = document.getElementById("canvas");
        var dataURL = canvas.toDataURL("image/jpeg");
        document.getElementById('hidden_data').value = dataURL;
        var fd = new FormData(document.forms["form1"]);
        var theUrl = "/?uploadfile=" + filename + "&version=" + Math.random();
        var xhr = new XMLHttpRequest();
        xhr.open('POST', theUrl, true);
 
        xhr.upload.onprogress = function(e) {
            if (e.lengthComputable) {
                var percentComplete = (e.loaded / e.total) * 100;
                console.log(percentComplete + '% uploaded');
            }
        }
        xhr.onload = function() {
 
        }
        xhr.onreadystatechange = function() {   //Call a function when the state changes.
            if(xhr.readyState == 4) {
                if(xhr.responseText=="OK") alert(filename + ' succesfully uploaded');
                else alert(filename + ' not uploaded')
            }
        }
        xhr.send(fd);
    }

    function downloadCanvasImage() {
        var sn = document.getElementById("stationname");
        if(sn.value != "") filename=sn.value+".jpg"; else filename="myimage.jpg";
        var lnk = document.createElement('a'), e;  // create an "off-screen" anchor tag
        lnk.download = filename; // the key here is to set the download attribute of the a tag
        lnk.href = canvas.toDataURL("image/jpeg");
        if (document.createEvent) { // create a "fake" click-event to trigger the download
            e = document.createEvent("MouseEvents");
            e.initMouseEvent("click", true, true, window, 0, 0, 0, 0, 0, false, false, false, false, 0, null);
            lnk.dispatchEvent(e);
        } else if (lnk.fireEvent) {
            lnk.fireEvent("onclick");
        }
    }
    //-------------------------------------- TAB Info ---------------------------------------
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

</script>

<body id="BODY">
    <script src="https://cdnjs.cloudflare.com/ajax/libs/jquery/3.3.1/jquery.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/jqueryui/1.12.1/jquery-ui.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/jsgrid/1.5.3/jsgrid.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/xlsx/0.13.4/xlsx.full.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/FileSaver.js/1.3.8/FileSaver.min.js"></script>

<!--  <script src="SD/js/jquery.js"></script>    --->
<!--  <script src="SD/js/jquery-ui.js"></script> --->
<!--  <script src="SD/js/jsgrid.js"></script>    --->
<!--  <script src="SD/js/xlsx.js"></script>      --->
<!--  <script src="SD/js/FileSaver.js"></script> --->

    <div id="CONTENT" >
    <!--==============================================================================================-->
        <div id="tab-content1">
        <table width=100%>
            <tr>
                <td style="width:350px;">
                    <left>
                        <img src="SD/png/Radio_Yellow.png"      alt="radio"                         />
                        <img src="SD/png/Station_Green.png"     alt="station"   onclick="showTab2()"/> 
                        <img src="SD/png/MP3_Green.png"         alt="mp3"       onclick="showTab3()"/>
                        <img src="SD/png/Search_Green.png"      alt="search"    onclick="showTab4()"/>
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
                             onclick="httpGet('prev_station', 1)"/>
                        <img src="SD/png/Button_Next_Green.png" alt="next" 
                             onmousedown="this.src='SD/png/Button_Next_Yellow.png'" 
                             onmouseup="this.src='SD/png/Button_Next_Green.png'"
                             onclick="httpGet('next_station', 1)"/>
                    </left>
                </td>
                <td colspan="2">
                 <select class="select" style="width:100%;"  onChange="handleStation(this)" id="preset">
                    <option value="-1">Select a station here</option>
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
                    <input type="text" style="width:100%;" id="station" placeholder="Enter a streamURL here....">
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
                <a target="_blank" href="https://radiolise.gitlab.io">Radiolise</a>
                or 
                <a target="_blank" href="http://streamstat.net/main.cgi?mode=all"> StreamStat.NET </a>
                <br>
        </center>
    </div>
    <!--==============================================================================================-->
    <div id="tab-content2">
        <!-- ~~~~~~~~~~~~~~~~~~~~~~ hidden div ~~~~~~~~~~~~~~~~~~~~~~-->
        <div id="dialog">
            <table>
                <tr>
                    <td> x </td>
                    <td> <input type=text id="txtX" size="45"/></td>
                </tr>
                <tr>
                    <td>  Cy  </td>
                    <td> <input type=text id="txtCy" size="45"/></td>
                </tr>
                <tr>
                    <td>  StationName  </td>
                    <td> <input type=text id="txtStationName" size="45"/></td>
                </tr> 
                <tr>
                    <td>  StreamURL  </td>
                    <td> <input type=text id="txtStreamURL" size="45"/></td>
                </tr>     
                <tr>
                    <td>  STsubstitute  </td>
                    <td> <input type=text id="txtStSub" size="45"/></td>
                </tr>        
            </table>
        </div>
        <!-- ~~~~~~~~~~~~~~~~~~~~ hidden div end ~~~~~~~~~~~~~~~~~~~~~~-->  
        <table width=100%>
            <tr>
                <td style="width:350px;">
                    <left>
                        <img src="SD/png/Radio_Green.png"       alt="radio"     onclick="showTab1()"/>
                        <img src="SD/png/Station_Yellow.png"    alt="station"                       /> 
                        <img src="SD/png/MP3_Green.png"         alt="mp3"       onclick="showTab3()"/>
                        <img src="SD/png/Search_Green.png"      alt="search"    onclick="showTab4()"/>
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
        <div id="jsGrid"></div>
        <br>
        <button class="button buttongreen" 
                onclick="saveGridFileToSD()" 
                onmousedown="this.style.backgroundColor='#D62C1A'" 
                onmouseup="this.style.backgroundColor='#128F76'"
                title="Save to SD">Save
        </button>
        &nbsp;
        <button class="button buttongreen" 
                onclick="loadGridFileFromSD()"
                onmousedown="this.style.backgroundColor='#D62C1A'" 
                onmouseup="this.style.backgroundColor='#128F76'" 
                id="loadSD" title="Load from SD">Load
        </button>
        &nbsp;
        <button class="button buttonblue" onclick="saveExcel()" title="Download to PC">save xlsx</button>
        &nbsp;
        <button class="button buttonblue" 
                onclick="javascript:document.getElementById('file').click();"
                title="Load from PC">load xlsx
        </button>
        <input id="file" type="file" style='visibility: hidden; width: 0px' name="img" onchange="loadDataExcel(this.files);"/>
        <br>
        </center>
    </div>  
    <!--==============================================================================================-->
    <div id="tab-content3">
        <table width=100%>
            <tr>
                <td style="width:350px;">
                    <left>
                        <img src="SD/png/Radio_Green.png"       alt="radio"     onclick="showTab1()"/>
                        <img src="SD/png/Station_Green.png"     alt="station"   onclick="showTab2()"/>
                        <img src="SD/png/MP3_Yellow.png"        alt="mp3"                           />
                        <img src="SD/png/Search_Green.png"      alt="search"    onclick="showTab4()"/>
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
                <td style="width:350px;">
                    <left>
                        <img src="SD/png/Radio_Green.png"       alt="radio"     onclick="showTab1()"/>
                        <img src="SD/png/Station_Green.png"     alt="station"   onclick="showTab2()"/> 
                        <img src="SD/png/MP3_Green.png"         alt="mp3"       onclick="showTab3()"/>
                        <img src="SD/png/Search_Yellow.png"     alt="search"                        />
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
        This service is provided by <a target="_blank" href="http://www.radio-browser.info/gui/#/">Community Radio Browser</a>
        <br><br>
        <table width=100%>
            <tr>
                <td>
                    <select class="select" style="width:100%;" onChange="selectcountry(this)" id="country">
                        <option value="-1">Select a country here</option>
                    </select>
                </td>
                <td  width="70">
                </td>
            </tr>
            <tr>
                <td>
                    <select class="select" style="width:100%;" onChange="selectstation(this)" id="stations">
                        <option value="-1">Select a station here</option>
                    </select>
                </td>
                <td>
                </td>
            </tr>
        </table>
        <hr>
        <table width=100%>
            <tr>
                <td>
                    StreamURL<br>
                    <input type="text" style="width:100%;" id="streamurl" placeholder="StreamURL">
                    <br>
                </td>
                <td width="70">
                    <right>
                         <img src="SD/png/Button_Ready_Blue.png" alt="Vol_up" 
                         onmousedown="this.src='SD/png/Button_Ready_Yellow.png'" 
                         onmouseup="this.src='SD/png/Button_Ready_Blue.png'"
                         onclick="teststreamurl()"/>
                    </right>  
                </td>
            </tr>
            <tr>
                <td>
                    HomepageUrl<br>
                    <input type="text" style="width:100%;" id="homepageurl" placeholder="HomepageURL">
                    <br>
                </td>
                <td>
                    <right>
                         <img src="SD/png/Button_Ready_Blue.png" alt="Vol_up" 
                         onmousedown="this.src='SD/png/Button_Ready_Yellow.png'" 
                         onmouseup="this.src='SD/png/Button_Ready_Blue.png'"
                         onclick="window.open(homepageurl.value, '_blank')"/>
                    </right> 
                </td>
            </tr>
            <tr>
                <td>
                    LogoUrl<br>
                    <input type="text" style="width:100%;" onclick="refreshCanvas()" id="favicon" placeholder="Favicon">
                    <br>
                </td>
                <td>
                    <right>
                         <img src="SD/png/Button_Ready_Blue.png" alt="Vol_up" 
                         onmousedown="this.src='SD/png/Button_Ready_Yellow.png'" 
                         onmouseup="this.src='SD/png/Button_Ready_Blue.png'"
                         onclick="window.open(favicon.value, '_blank')"/>
                    </right> 
                </td>
            </tr>
        </table>
        <table width=100%>
            <tr>
                <td  width="100">
                    <canvas id="canvas" width="96" height="96" class="playable-canvas"></canvas>
                </td>
                <td>
                    <table width=100%>
                        <tr>
                            <td>
                                <input type="text" style="width:100%;" id="stationname" placeholder="Change the Stationname here">
                            </td>
                            <td width="60">
                            </td>                        
                        </tr>
                        <tr>
                            <td>
                                <left>
                                    <img src="SD/png/Button_Upload_Blue.png" alt="Upload" title="Upload to SD" 
                                    onmousedown="this.src='SD/png/Button_Upload_Yellow.png'" 
                                    onmouseup="this.src='SD/png/Button_Upload_Blue.png'"
                                    onclick="uploadCanvasImage()"/>
                                    
                                    <img src="SD/png/Button_Download_Blue.png" alt="Download" title="Download to PC" 
                                    onmousedown="this.src='SD/png/Button_Download_Yellow.png'" 
                                    onmouseup="this.src='SD/png/Button_Download_Blue.png'"
                                    onclick="downloadCanvasImage()"/>
                        
                                    <img src="SD/png/Button_Previous_Blue.png" alt="addGrid" title="add to list" 
                                    onmousedown="this.src='SD/png/Button_Previous_Yellow.png'" 
                                    onmouseup="this.src='SD/png/Button_Previous_Blue.png'"
                                    onclick="addStationsToGrid()"/>

                                    <form method="post" accept-charset="utf-8" name="form1">
                                    <input name="hidden_data" id='hidden_data' type="hidden"/>
                                    </form>

                                </left>
                            </td>
                        </tr>
                    </table> 
                </td>
                <td >
                </td>
            </tr>
        </table>

    </div>
    <!--==============================================================================================-->
    <div id="tab-content5">
        <table width=100%>
            <tr>
                <td style="width:350px;">
                    <left>
                        <img src="SD/png/Radio_Green.png"       alt="radio"     onclick="showTab1()"/>
                        <img src="SD/png/Station_Green.png"     alt="station"   onclick="showTab2()"/> 
                        <img src="SD/png/MP3_Green.png"         alt="mp3"       onclick="showTab3()"/>
                        <img src="SD/png/Search_Green.png"      alt="search"    onclick="showTab4()"/>
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
        <img src="SD/MiniWebRadio_gr.jpg" alt="MiniWebRadio_gr" border=3>
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
