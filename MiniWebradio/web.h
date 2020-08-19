/*
 *  web.h
 *
 *  Created on: 04.10.2018
 *  Updated on: 16.10.2019
 *      Author: Wolle
 *
 *  successfully tested with Chrome and Firefox
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
    <title>MiniWebRadio</title>
    <meta name="generator" content="Bluefish 2.2.10" >
    <meta name="author" content="Wolle" >
    <meta name="date" content="2019-10-11T20:07:54+0200" >
    <meta name="copyright" content="schreibfaul1">
    <meta name="keywords" content="">
    <meta name="description" content="index">
    <meta name="ROBOTS" content="NOINDEX, NOFOLLOW">
    <meta http-equiv="content-type" content="text/html; charset=UTF-8">
    <meta http-equiv="content-type" content="application/xhtml+xml; charset=UTF-8">
    <meta http-equiv="content-style-type" content="text/css">
    <meta http-equiv="expires" content="0">

    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/jqueryui/1.12.1/jquery-ui.min.css" />
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/jsgrid/1.5.3/jsgrid.min.css" />
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/jsgrid/1.5.3/jsgrid-theme.min.css" />

<!--   <link rel="stylesheet" href="SD/css/jquery-ui.css" />     -->
<!--   <link rel="stylesheet" href="SD/css/jsgrid.css" />        -->
<!--   <link rel="stylesheet" href="SD/css//jsgrid-theme.css" /> -->


    <style type="text/css">           /* optimized with csstidy */
        html {  /* This is the groundplane */
            font-family : serif;
            height : 100%;
            font-size: 16px;
            color : DarkSlateGray;
            background-color : navy;
            margin : 0;
            padding : 0;
        }

        #preloaded-images{
            display: none;
        }
        #dialog {
            display: none;
        }
        #content {
            min-height : 540px;
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
            margin : 20px;
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
        .button {
            width : 80px;
            height : 30px;
            background-color : #128F76;
            border : none;
            color : #FFF;
            text-align : center;
            text-decoration : none;
            display : inline-block;
            font-size : 16px;
            cursor : pointer;
            border-radius : 5px;
            margin : 4px 2px;
        }
        .buttonblue {
            background-color : blue;
            width : 120px;
        }
        .buttongreen {
            background-color : #128F76;
            width : 120px;
        }
        #label-logo {
            margin-left : 20px;
            border-color: black;
            border-style: solid;
            border-width: 2px;
            display : inline-block;
            background-image : url(SD/unknown.jpg);
            width : 96px;
            height : 96px;
            margin-top: 5px;
        }
        canvas {
            left : 0;
            margin-left : 0;
            display : inline-block;
            width : 96px;
            height : 96px;
            border : #000 solid 2px;
        }
        .jsgrid-header-cell {
            padding : 0.1em !important ;
        }
        .jsgrid-cell {
            overflow : hidden !important ;
            white-space : nowrap !important ;
            padding : 0.1em 0.2em !important ;
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
        .sdr_lbl_left {
            display: inline-block;
            float: left;
            text-align:right;
            height: 25px;
            width: 100px;
            padding-top: 0px;
            padding-right: 5px;
            padding-bottom: 0px;
        }
        .sdr_lbl_right {
            display: inline-block;
            float: left;
            text-align:right;
            height: 25px;
            width: 45px;
            padding-top: 0px;
            padding-left: 5px;
            padding-bottom: 0px;
        }
        .sdr_lbl_measure {
            display: inline-block;
            float: left;
            text-align:left;
            height: 25px;
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
    </style>
</head>

<script>
// global variables and functions
/* eslint-disable no-unused-vars, no-undef */
var trebleDB = ['-12,0', '-10,5', ' -9,0', ' -7,5', ' -6,0', ' -4,5', ' -3,0', ' -1,5',
  '  0,0', ' +1,5', ' +3,0', ' +4,5', ' +6,0', ' +7,5', ' +9,0', '+10,5']

var trebleVal = [8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7]

var run2s = 0

function run2sec () { // called every two seconds
  if (run2s === 0) httpGet('to_listen', 1)
  if (run2s === 1) getmute()
  if (run2s === 2) httpGet('getstreamtitle', 1)
  run2s++
  if (run2s > 2) run2s = 0
}

document.addEventListener('readystatechange', event => {
  if (event.target.readyState === 'interactive') { // same as:  document.addEventListener('DOMContentLoaded'...
    // same as  jQuery.ready
    console.log('All HTML DOM elements are accessible')
    document.getElementById('dialog').style.display = 'none' // hide the div (its only a template)
  }
  if (event.target.readyState === 'complete') {
    console.log('Now external resources are loaded too, like css,src etc... ')
    gettone() // Now load the tones (tab Radio)
    getnetworks()
    getmute()
    loadGridFileFromSD()
    showExcelGrid()
    httpGet('to_listen', 1)
    setInterval(run2sec, 2000) // this will run the function for every 2 sec.
  }
})

function showTab1 () {
  console.log('tab-content1 (Radio)')
  document.getElementById('tab-content1').style.display = 'block'
  document.getElementById('tab-content2').style.display = 'none'
  document.getElementById('tab-content3').style.display = 'none'
  document.getElementById('tab-content4').style.display = 'none'
  document.getElementById('tab-content5').style.display = 'none'
  document.getElementById('btn1').src = 'SD/png/Radio_Yellow.png'
  document.getElementById('btn2').src = 'SD/png/Station_Green.png'
  document.getElementById('btn3').src = 'SD/png/MP3_Green.png'
  document.getElementById('btn4').src = 'SD/png/Search_Green.png'
  document.getElementById('btn5').src = 'SD/png/About_Green.png'
  getmute()
}

function showTab2 () {
  console.log('tab-content2 (Stations)')
  document.getElementById('tab-content1').style.display = 'none'
  document.getElementById('tab-content2').style.display = 'block'
  document.getElementById('tab-content3').style.display = 'none'
  document.getElementById('tab-content4').style.display = 'none'
  document.getElementById('tab-content5').style.display = 'none'
  document.getElementById('btn1').src = 'SD/png/Radio_Green.png'
  document.getElementById('btn2').src = 'SD/png/Station_Yellow.png'
  document.getElementById('btn3').src = 'SD/png/MP3_Green.png'
  document.getElementById('btn4').src = 'SD/png/Search_Green.png'
  document.getElementById('btn5').src = 'SD/png/About_Green.png'
  $('#jsGrid').jsGrid('refresh')
}

function showTab3 () {
  console.log('tab-content3 (MP3 Player)')
  document.getElementById('tab-content1').style.display = 'none'
  document.getElementById('tab-content2').style.display = 'none'
  document.getElementById('tab-content3').style.display = 'block'
  document.getElementById('tab-content4').style.display = 'none'
  document.getElementById('tab-content5').style.display = 'none'
  document.getElementById('btn1').src = 'SD/png/Radio_Green.png'
  document.getElementById('btn2').src = 'SD/png/Station_Green.png'
  document.getElementById('btn3').src = 'SD/png/MP3_Yellow.png'
  document.getElementById('btn4').src = 'SD/png/Search_Green.png'
  document.getElementById('btn5').src = 'SD/png/About_Green.png'
  getmp3list() // Now get the mp3 list from SD
}

function showTab4 () {
  console.log('tab-content4 (Search Stations)')
  document.getElementById('tab-content1').style.display = 'none'
  document.getElementById('btn1').src = 'SD/png/Radio_Green.png'
  document.getElementById('tab-content2').style.display = 'none'
  document.getElementById('tab-content3').style.display = 'none'
  document.getElementById('tab-content4').style.display = 'block'
  document.getElementById('tab-content5').style.display = 'none'
  document.getElementById('btn1').src = 'SD/png/Radio_Green.png'
  document.getElementById('btn2').src = 'SD/png/Station_Green.png'
  document.getElementById('btn3').src = 'SD/png/MP3_Green.png'
  document.getElementById('btn4').src = 'SD/png/Search_Yellow.png'
  document.getElementById('btn5').src = 'SD/png/About_Green.png'
  loadJSON('https://de1.api.radio-browser.info/json/countries', gotCountries, 'jsonp')
}

function showTab5 () {
  console.log('tab-content5 (About)')
  document.getElementById('tab-content1').style.display = 'none'
  document.getElementById('tab-content2').style.display = 'none'
  document.getElementById('tab-content3').style.display = 'none'
  document.getElementById('tab-content4').style.display = 'none'
  document.getElementById('tab-content5').style.display = 'block'
  document.getElementById('btn1').src = 'SD/png/Radio_Green.png'
  document.getElementById('btn2').src = 'SD/png/Station_Green.png'
  document.getElementById('btn3').src = 'SD/png/MP3_Green.png'
  document.getElementById('btn4').src = 'SD/png/Search_Green.png'
  document.getElementById('btn5').src = 'SD/png/About_Yellow.png'
}

function uploadTextFile (fileName, content) {
  var fd = new FormData()
  fd.append('Text=', content)
  var theUrl = '/?uploadfile=' + fileName + '&version=' + Math.random()
  var xhr = new XMLHttpRequest()
  xhr.open('POST', theUrl, true)
  xhr.onreadystatechange = function () { // Call a function when the state changes.
    if (xhr.readyState === 4) {
      if (xhr.responseText === 'OK') alert(fileName + ' successfully uploaded')
      else alert(fileName + ' not uploaded')
    }
  }
  xhr.send(fd) // send
}

// ----------------------------------- TAB RADIO ------------------------------------

function showLabel (id, src) { // get the bitmap from SD, convert to URL first
  src = src.replace(/%/g, '%25') // % must be the first
  src = src.replace(/\s/g, '%20') // URLs never can have blanks
  // src=src.replace(/!/g  , '%21')  // not necessary to replace
  // src=src.replace(/\'/g , '%22')  // not allowed in Windows filenames
  // src=src.replace(/#/g  , '%23')  // can not be used, is separator in list
  // src=src.replace(/\$/g , '%24')  // not necessary to replace
  // src=src.replace(/&/g  , '%26')  // not necessary to replace
  src = src.replace(/'/g, '%27') // must be replace
  src = src.replace(/\(/g, '%28') // must be replace
  src = src.replace(/\)/g, '%29') // must be replace
  // src=src.replace(/\*/g , '%2A')  // not allowed in Windows filenames
  src = src.replace(/\+/g, '%2B') // is necessary to replace, + is the same as space
  // src=src.replace(/,/g  , '%2C')  // commas are later replaced in dots
  // src=src.replace(/\-/g , '%2D')  // not necessary to replace
  // src=src.replace(/\./g , '%2E')  // not necessary to replace
  // src=src.replace('/'   , '%2F')  // is separator, not usable
  // src=src.replace(/:/g  , '%3A')  // not allowed in Windows filenames
  // src=src.replace(/;/g  , '%3B')  // not necessary to replace
  // src=src.replace(/</g  , '%3C')  // not allowed in Windows filenames
  // src=src.replace(/\=/g , '%3D')  // can't be used in selectboxes
  // src=src.replace(/>/g  , '%3E')  // not allowed in Windows filenames
  // src=src.replace(/\?/g , '%3F')  // not allowed in Windows filenames
  // src=src.replace(/@/g  , '%40')  // not necessary to replace
  // src=src.replace(/\[/g , '%5B')  // not necessary to replace
  // src=src.replace('\'   , '%5C')  // not necessary to replace
  // src=src.replace(/\]/g , '%5D')  // not necessary to replace
  // src=src.replace(/\{/g , '%7B')  // not necessary to replace
  // src=src.replace(/\|/g , '%7C')  // not allowed in Windows filenames
  // src=src.replace(/\}/g , '%7D')  // not necessary to replace
  var file = 'url(url=SD/logo/' + src + '.jpg)'
  // file=file.split(',').join('.') //replace commas in dots, MiniWebRadio has no commas in filenames
  document.getElementById(id).style.backgroundImage = file
}

var _num = 0

function httpGet (theReq, nr) { // universal request prev, next, vol,  mute...
  var theUrl = '/?' + theReq + '&version=' + Math.random()
  var xhr = new XMLHttpRequest()
  xhr.onreadystatechange = function () {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      if (nr === 1) {
        if (theReq.startsWith('prev_station') ||
          theReq.startsWith('next_station') ||
          theReq.startsWith('set_station=') ||
          theReq.startsWith('to_listen')) {
          var res = ''
          var num = ''
          var sta = ''
          var url = ''
          var n = 0
          res = xhr.responseText
          n = res.indexOf(' ')
          num = res.substring(0, n) // stationnumber
          if (_num !== Number(num)) { // only if num has changed
            _num = Number(num)
            cmd.value = ''
            var sel = document.getElementById('preset')
            sel.selectedIndex = Number(num)
            if (n === 1) num = '00' + num
            if (n === 2) num = '0' + num
            res = res.substring(n + 1) // remove stationnumber
            n = res.indexOf(' ')
            url = res.substring(0, n) // streamURL
            sta = res.substring(n + 1)
            showLabel('label-logo', sta)
            station.value = url
          }
        } else if (xhr.responseText.startsWith('http')) {
          console.log(xhr.responseText)
          window.open(xhr.responseText, '_blank') // show the station homepage
        } else if (xhr.responseText.startsWith('Mute')) {
          console.log(xhr.responseText)
          resultstr1.value = xhr.responseText // all other
          if (xhr.responseText.endsWith('off\n')) {
            document.getElementById('Mute').src = 'SD/png/Button_Mute_Green.png'
          }
          if (xhr.responseText.endsWith('on\n')) {
            document.getElementById('Mute').src = 'SD/png/Button_Mute_Red.png'
          }
        } else if (theReq === 'mute' || theReq.startsWith('upvolume') || theReq.startsWith('downvolume')) {
          resultstr1.value = xhr.responseText
        } else if (theReq === 'getstreamtitle') {
          cmd.value = xhr.responseText
        } else resultstr1.value = xhr.responseText
      }
      if (nr === 2) resultstr2.value = xhr.responseText
      if (nr === 3) resultstr3.value = xhr.responseText
    }
  }
  xhr.open('GET', theUrl, true)
  xhr.send()
}

function gettone () { // tab Radio: get tones values and set them
  var i, lines, parts
  var theUrl = '/?gettone' + '&version=' + Math.random()
  var xhr = new XMLHttpRequest()
  xhr.onreadystatechange = function () {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      lines = xhr.responseText.split('\n')
      for (i = 0; i < (lines.length - 1); i++) {
        parts = lines[i].split('=')
        if (parts[0].indexOf('tone') === 0) {
          setSlider(parts[0], parts[1])
        }
      }
    }
  }
  xhr.open('GET', theUrl, true)
  xhr.send()
}

function getmute () {
  var theUrl = '/?getmute' + '&version=' + Math.random()
  var xhr = new XMLHttpRequest()
  xhr.onreadystatechange = function () {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      if (xhr.responseText === '1') {
        document.getElementById('Mute').src = 'SD/png/Button_Mute_Red.png' // muteOn
      } else {
        document.getElementById('Mute').src = 'SD/png/Button_Mute_Green.png'
      }
    }
  }
  xhr.open('GET', theUrl, true)
  xhr.send()
}

function handleStation (presctrl) { // tab Radio: preset, select a station
  cmd.value = ''
  httpGet('set_station=' + (presctrl.value), 1)
}

function handletone (tonectrl) { // Radio: treble, bass, freq
  var theUrl = '/?' + tonectrl.id + '=' + tonectrl.value + '&version=' + Math.random()
  var xhr = new XMLHttpRequest()
  xhr.onreadystatechange = function () {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      resultstr1.value = xhr.responseText
    }
  }
  xhr.open('GET', theUrl, true)
  xhr.send()
}

function setstation () { // Radio: button play - enter a url to play from
  var theUrl = '/?stationURL=' + station.value + '&version=' + Math.random()
  var sel = document.getElementById('preset')
  sel.selectedIndex = 0
  cmd.value = ''
  var xhr = new XMLHttpRequest()
  xhr.onreadystatechange = function () {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      resultstr1.value = xhr.responseText
      showLabel('label-logo', 'unknown')
    }
  }
  xhr.open('GET', theUrl, true)
  xhr.send()
}

function selectItemByValue (elmnt, value) { // tab Radio: load and set tones
  var sel = document.getElementById(elmnt)
  for (var i = 0; i < sel.options.length; i++) {
    if (sel.options[i].value === value) {
      sel.selectedIndex = i
    }
  }
}

function setSlider (elmnt, value) {
  if (elmnt === 'toneha') slider_TG_set(value)
  if (elmnt === 'tonehf') slider_TF_set(value)
  if (elmnt === 'tonela') slider_BG_set(value)
  if (elmnt === 'tonelf') slider_BF_set(value)
}

function slider_TG_mouseUp () { // Slider Treble Gain   mouseupevent
  handlectrl('toneha', trebleVal[TrebleGain.value])
  // console.log('Treble Gain=%i',Number(TrebleGain.value));
}

function slider_TG_change () { //  Slider Treble Gain  changeevent
  console.log('Treble Gain=%i', Number(TrebleGain.value))
  document.getElementById('label_TG_value').innerHTML = trebleDB[TrebleGain.value]
}

function slider_TG_set (value) { // set Slider Treble Gain
  var val = Number(value)
  if (val < 8) val = val + 8
  else val = val - 8
  document.getElementById('TrebleGain').value = val
  document.getElementById('label_TG_value').innerHTML = trebleDB[TrebleGain.value]
  console.log('Treble Gain=%i', val)
}

function slider_TF_mouseUp () { // Slider Treble Freq   mouseupevent
  handlectrl('tonehf', TrebleFreq.value)
  // console.log('Treble Freq=%i', Number(TrebleFreq.value));
}

function slider_TF_change () { //  Slider Treble Freq  changeevent
  console.log('Treble Freq=%i', Number(TrebleFreq.value))
  document.getElementById('label_TF_value').innerHTML = TrebleFreq.value
}

function slider_TF_set (value) { // set Slider Treble Freq
  var val = Number(value)
  document.getElementById('TrebleFreq').value = val
  document.getElementById('label_TF_value').innerHTML = TrebleFreq.value
  console.log('Treble Freq=%i', val)
}

function slider_BG_mouseUp () { // Slider Bass Gain   mouseupevent
  handlectrl('tonela', BassGain.value)
  // console.log('Bass Gain=%i', Number(BassGain.value));
}

function slider_BG_change () { //  Slider Bass Gain  changeevent
  var sign = ''
  if (BassGain.value !== '0') sign = '+'
  console.log('Bass Gain=%i', Number(BassGain.value))
  document.getElementById('label_BG_value').innerHTML = sign + BassGain.value
}

function slider_BG_set (value) { // set Slider Bass Gain
  var val = Number(value)
  var sign = ''
  if (BassGain.value !== '0') sign = '+'
  document.getElementById('BassGain').value = val
  document.getElementById('label_BG_value').innerHTML = sign + BassGain.value
  console.log('Bass Gain=%i', val)
}

function slider_BF_mouseUp () { // Slider Bass Gain   mouseupevent
  handlectrl('tonelf', BassFreq.value)
  // console.log('Bass Freq=%i', Number(BassFreq.value));
}

function slider_BF_change () { //  Slider Bass Gain  changeevent
  console.log('Bass Freq=%i', Number(BassFreq.value))
  document.getElementById('label_BF_value').innerHTML = (BassFreq.value - 1) * 10
}

function slider_BF_set (value) { // set Slider Bass Gain
  var val = Number(value)
  document.getElementById('BassFreq').value = val
  document.getElementById('label_BF_value').innerHTML = (BassFreq.value - 1) * 10
  console.log('Bass Freq=%i', val)
}

function handlectrl (id, val) { // Radio: treble, bass, freq
  var theUrl = '/?' + id + '=' + val + '&version=' + Math.random()
  var xhr = new XMLHttpRequest()
  xhr.onreadystatechange = function () {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      resultstr1.value = xhr.responseText
    }
  }
  xhr.open('GET', theUrl, true)
  xhr.send()
}

// ----------------------------------- TAB CONFIG ------------------------------------

function saveGridFileToSD () { // save to SD
  var wsData = $('#jsGrid').jsGrid('option', 'data')
  var strJSON = JSON.stringify(wsData)
  var objJSON = JSON.parse(strJSON)
  console.log(wsData.length)
  var txt = ''
  var l
  var c
  var select, opt
  select = document.getElementById('preset') // Radio: show stationlist
  select.options.length = 1
  var j = 1
  txt = 'X\tCy\tStationName\t\t\t\t\t\t\tStreamURL\t\t\t\t\t\t\t\t\t\t\t\tSTsubstitute\r\n'
  for (var i = 0; i < wsData.length; i++) {
    c = ''
    if (objJSON[i].X) {
      c = objJSON[i].X
      l = c.length
      while (l < 8) {
        c = c + '\t'
        l += 8
      }
      txt += c
    } else txt += '\t'
    if (objJSON[i].X !== '*') {
      if (j < 1000) {
        opt = document.createElement('OPTION')
        opt.text = (('000' + j).slice(-3) + ' - ' + objJSON[i].StationName)
        select.add(opt)
      }
      j++
    }
    if (objJSON[i].Cy) {
      c = objJSON[i].Cy
      l = c.length
      while (l < 8) {
        c = c + '\t'
        l += 8
      }
      txt += c
    } else txt += '\t'
    if (objJSON[i].StationName) {
      c = objJSON[i].StationName
      l = c.length
      while (l < (8 * 8)) {
        c = c + '\t'
        l += 8
      }
      txt += c
    } else txt += '\t'
    if (objJSON[i].StreamURL) {
      c = objJSON[i].StreamURL
      l = c.length
      while (l < (8 * 13)) {
        c = c + '\t'
        l += 8
      }
      txt += c
    } else txt += '\t'
    if (objJSON[i].STsubstitute) {
      txt += objJSON[i].STsubstitute
    }
    txt += '\r'
    txt += '\n'
  }
  uploadTextFile('stations.txt', txt)
  updateStationlist()
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function loadGridFileFromSD () { // load from SD
  var XLrowObject
  var rawFile = new XMLHttpRequest()
  rawFile.open('POST', '/SD/stations.txt', true)
  rawFile.onreadystatechange = function () {
    if (rawFile.readyState === 4) {
      var rawdata = rawFile.responseText
      rawdata = rawdata.replace(/(\t\t)/g, '\t') // shrink more tabs to one tab in 3 steps
      rawdata = rawdata.replace(/(\t\t)/g, '\t')
      rawdata = rawdata.replace(/(\t\t)/g, '\t')
      var workbook = XLSX.read(rawdata, {
        raw: true,
        type: 'string',
        cellDates: false,
        cellText: true
      })
      workbook.SheetNames.forEach(function (sheetName) {
        XLrowObject = XLSX.utils.sheet_to_row_object_array(workbook.Sheets[sheetName])
      })
      var strJSON = JSON.stringify(XLrowObject)
      var objJSON = JSON.parse(strJSON)
      $('#jsGrid').jsGrid({
        data: objJSON
      })
      updateStationlist()
    };
  }
  rawFile.send()
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function saveExcel () { // save xlsx to PC
  var wb = XLSX.utils.book_new()
  wb.Props = {
    Title: 'Stations',
    Subject: 'Stationlist',
    Author: 'MiniWebRadio',
    CreatedDate: new Date('2018.10.10')
  }
  wb.SheetNames.push('Stations')
  var wsData = $('#jsGrid').jsGrid('option', 'data')
  var wscols = [{
    wch: 3
  }, // 'characters'
  {
    wch: 5
  }, // 'characters'
  {
    wch: 100
  }, // 'characters'
  {
    wch: 150
  }, // 'characters'
  {
    wch: 200
  } // 'characters'
  ]
  var ws = XLSX.utils.json_to_sheet(wsData, {
    header: ['X', 'Cy', 'StationName', 'StreamURL', 'STsubstitute']
  })
  ws['!cols'] = wscols
  wb.Sheets.Stations = ws

  var wbout = XLSX.write(wb, {
    bookType: 'xlsx',
    type: 'binary'
  })

  function s2ab (s) {
    var buf = new ArrayBuffer(s.length)
    var view = new Uint8Array(buf)
    for (var i = 0; i < s.length; i++) view[i] = s.charCodeAt(i) & 0xff
    return buf
  }
  saveAs(
    new Blob([s2ab(wbout)], {
      type: 'application/octet-stream'
    }),
    'stations.xlsx'
  )
  updateStationlist()
}

var clients = [ // testdata
  {
    X: '*',
    Cy: 'D',
    StationName: 'Station',
    StreamURL: 'URL',
    STsubstitute: ''
  }
]

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function showExcelGrid () {
  $('#jsGrid').jsGrid({
    width: '100%',
    height: '432px',
    editing: true,
    sorting: true,
    paging: false,
    shrinkToFit: false,
    onItemDeleted: function (args) {
      updateStationlist()
    },
    onItemUpdated: function (args) {
      updateStationlist()
    },
    onItemInserted: function (args) {
      updateStationlist()
    },
    deleteConfirm: function (item) {
      return 'The entry ' + item.StationName + ' will be removed. Are you sure?'
    },
    rowClick: function (args) {
      showDetailsDialog('Edit', args.item)
    },
    data: clients,
    fields: [{
      name: 'X',
      type: 'text',
      width: 20,
      align: 'center'
    },
    {
      name: 'Cy',
      type: 'text',
      width: 25,
      align: 'center'
    },
    {
      name: 'StationName',
      type: 'text',
      width: 170
    },
    {
      name: 'StreamURL',
      type: 'text',
      width: 320
    },
    {
      name: 'STsubstitute',
      type: 'text',
      width: 90
    },
    {
      type: 'control',
      modeSwitchButton: false,
      editButton: false,
      shrinkToFit: true,
      headerTemplate: function () {
        return $('<button>')
          .attr('type', 'button')
          .text('Add')
          .on('click', function () {
            showDetailsDialog('Add', {})
          })
      }
    }
    ]
  })
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
var showDetailsDialog = function (dialogType, client) { // popUp window
  $('#txtX').val(client.X)
  $('#txtCy').val(client.Cy)
  $('#txtStationName').val(client.StationName)
  $('#txtStreamURL').val(client.StreamURL)
  $('#txtStSub').val(client.STsubstitute)
  var divdialog = $('#dialog')
  $('#dialog').attr('title', 'Edit')
  $('#dialog').dialog({
    width: 505,
    resizable: false,
    show: 'fade',
    modal: false,
    buttons: {
      OK: function () {
        client.X = $('#txtX').val()
        client.Cy = $('#txtCy').val()
        client.StationName = $('#txtStationName').val()
        client.StreamURL = $('#txtStreamURL').val()
        client.STsubstitute = $('#txtStSub').val()
        includeStation(client, dialogType === 'Add')
        $(this).dialog('close')
        console.log('dialog saved')
      }
    }
  })
  divdialog.dialog()
  console.log('dialog opened')
}

var includeStation = function (client, isNew) {
  $.extend(client, {
    X: $('#txX').val(),
    Cy: $('#txtCy').val(),
    StationName: $('#txtStationName').val(),
    StreamURL: $('#txtStreamURL').val(),
    StSub: $('#txtStSub').val()
  })

  $('#jsGrid').jsGrid(isNew ? 'insertItem' : 'updateItem', client)
  $('#detailsDialog').dialog('close')
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function loadDataExcel (event) { // load xlsx from PC
  var file = event[0]
  var reader = new FileReader()
  var excelData = []
  reader.onload = function (event) {
    var data = event.target.result
    var workbook = XLSX.read(data, {
      type: 'binary'
    })
    workbook.SheetNames.forEach(function (sheetName) {
      // Here is your object
      var XLrowObject = XLSX.utils.sheet_to_row_object_array(
        workbook.Sheets[sheetName]
      )

      for (var i = 0; i < XLrowObject.length; i++) {
        excelData.push(XLrowObject[i]['your column name'])
      }

      var strJSON = JSON.stringify(XLrowObject)
      var objJSON = JSON.parse(strJSON)
      // alert(strJSON);
      $('#jsGrid').jsGrid({
        data: objJSON
      })
      updateStationlist()
    })
  }
  $('#file').val('') // allow load twice
  reader.onerror = function (ex) {
    console.log(ex)
  }

  reader.readAsBinaryString(file)
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
function updateStationlist () { // select in tab Radio
  var opt, select
  var wsData = $('#jsGrid').jsGrid('option', 'data')
  var strJSON = JSON.stringify(wsData)
  var objJSON = JSON.parse(strJSON)
  console.log(wsData.length)
  select = document.getElementById('preset') // Radio: show stationlist
  select.options.length = 1
  var j = 1
  for (var i = 0; i < wsData.length; i++) {
    if (objJSON[i].X !== '*') {
      if (j < 1000) {
        opt = document.createElement('OPTION')
        opt.text = (('000' + j).slice(-3) + ' - ' + objJSON[i].StationName)
        select.add(opt)
      }
      j++
    }
  }
}

// ----------------------------------- TAB MP3 PLAYER ------------------------------------

function trackreq (presctrl) { // MP3 Player: select mp3 title from track list
  if (presctrl.value !== '-1') {
    httpGet('mp3track=' + presctrl.value, 3)
  }
}

function getmp3list () { // Fill track list initially
  var xhr = new XMLHttpRequest()
  var i, select, opt, tracks, strparts
  select = document.getElementById('seltrack')
  var theUrl = '/?mp3list' + '&version=' + Math.random()
  xhr.onreadystatechange = function () {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      tracks = xhr.responseText.split('\n')
      // for(i=select.options.length-1; i>=0; i--) select.remove(i);
      // select.add('chose a item here');
      select.options.length = 1
      for (i = 0; i < (tracks.length - 1); i++) {
        opt = document.createElement('OPTION')
        strparts = tracks[i].substr(tracks[i].lastIndexOf('/') + 1, tracks[i].length)
        opt.value = tracks[i]
        opt.text = strparts
        select.add(opt)
      }
    }
  }
  xhr.open('GET', theUrl, true)
  xhr.send()
}
// ----------------------------------- TAB Search Stations ------------------------------------

// global var
var countryallstations

function addStationsToGrid () {
  showDetailsDialog('Add', {})
  $('#txtStreamURL').val($('#streamurl').val())
  $('#txtStationName').val($('#stationname').val())
}

function loadJSON (path, success, error) {
  console.log(path)
  var xhr = new XMLHttpRequest()
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
  xhr.send()
}

function selectcountry (presctrl) { // tab Radio: preset, select a station
  loadJSON('https://de1.api.radio-browser.info/json/stations/bycountry/' + presctrl.value, gotStations, 'jsonp')
}

function gotCountries (data) { // fill select countries
  var select = document.getElementById('country')
  var opt
  select.options.length = 1
  for (var i = 0; i < data.length; i++) {
    if (i < 2) continue
    opt = document.createElement('OPTION')
    opt.text = data[i].name
    select.add(opt)
  }
  console.log(data.uuid)
}

function gotStations (data) { // fill select stations
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
  var j = document.getElementById('stationname')
  j.value = countryallstations[value].name
}

function teststreamurl () { // Search: button play - enter a url to play from
  var theUrl = '/?stationURL=' + streamurl.value + '&version=' + Math.random()
  var xhr = new XMLHttpRequest()
  xhr.onreadystatechange = function () {
    if (xhr.readyState === XMLHttpRequest.DONE) {}
  }
  xhr.open('GET', theUrl, true)
  xhr.send()
}

function scaleCanvasImage (url) {
  var canvas = document.getElementById('canvas')
  var ctx = canvas.getContext('2d')
  var src
  ctx.beginPath()
  ctx.rect(0, 0, canvas.width, canvas.height)
  ctx.fillStyle = 'white'
  ctx.fill()
  var co = 'https://cors-anywhere.herokuapp.com/'
  src = co + url
  var imgObj = new Image()
  imgObj.onload = function () {
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
  imgObj.crossOrigin = 'anonymous'
  imgObj.src = src
}

function refreshCanvas () {
  var g = document.getElementById('favicon')
  scaleCanvasImage(g.value)
  console.log('refresh')
}

function uploadCanvasImage () {
  var filename
  var sn = document.getElementById('stationname')
  if (sn.value !== '') filename = sn.value + '.jpg'
  else {
    alert('no stationname given')
    return
  }
  var canvas = document.getElementById('canvas')
  var dataURL = canvas.toDataURL('image/jpeg')
  document.getElementById('hidden_data').value = dataURL
  var fd = new FormData(document.forms.form1)
  var theUrl = '/?uploadfile=' + filename + '&version=' + Math.random()
  var xhr = new XMLHttpRequest()
  xhr.open('POST', theUrl, true)

  xhr.upload.onprogress = function (e) {
    if (e.lengthComputable) {
      var percentComplete = (e.loaded / e.total) * 100
      console.log(percentComplete + '% uploaded')
    }
  }
  xhr.onload = function () {
  }
  xhr.onreadystatechange = function () { // Call a function when the state changes.
    if (xhr.readyState === 4) {
      if (xhr.responseText === 'OK') alert(filename + ' successfully uploaded')
      else alert(filename + ' not uploaded')
    }
  }
  xhr.send(fd)
}

function downloadCanvasImage () {
  var filename
  var sn = document.getElementById('stationname')
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
// -------------------------------------- TAB Info ---------------------------------------
function getnetworks () { // tab Config: load the connected WiFi network
  var i, select, opt, networks
  var theUrl = '/?getnetworks' + '&version=' + Math.random()
  var xhr = new XMLHttpRequest()
  select = document.getElementById('ssid') // Radio: show stationlist
  xhr.onreadystatechange = function () {
    if (xhr.readyState === XMLHttpRequest.DONE) {
      networks = xhr.responseText.split('\n')
      for (i = 0; i < (networks.length - 1); i++) {
        opt = document.createElement('OPTION')
        opt.value = i
        opt.text = networks[i]
        select.add(opt)
      }
    }
  }
  xhr.open('GET', theUrl, true)
  xhr.send()
}
/* eslint-enable no-unused-vars, no-undef */

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

<div id="content" >

  <!-- ~~~~~~~~~~~~~~~~~~~~~~ hidden div ~~~~~~~~~~~~~~~~~~~~~~-->
  <div id="preloaded-images">
    <img src="SD/png/Radio_Green.png"               width="1" height="1" alt="Image 01" />
    <img src="SD/png/Radio_Yellow.png"              width="1" height="1" alt="Image 02" />
    <img src="SD/png/Station_Green.png"             width="1" height="1" alt="Image 03" />
    <img src="SD/png/Station_Yellow.png"            width="1" height="1" alt="Image 04" />
    <img src="SD/png/MP3_Green.png"                 width="1" height="1" alt="Image 05" />
    <img src="SD/png/MP3_Yellow.png"                width="1" height="1" alt="Image 06" />
    <img src="SD/png/Search_Green.png"              width="1" height="1" alt="Image 06" />
    <img src="SD/png/Search_Yellow.png"             width="1" height="1" alt="Image 07" />
    <img src="SD/png/About_Green.png"               width="1" height="1" alt="Image 08" />
    <img src="SD/png/About_Yellow.png"              width="1" height="1" alt="Image 09" />
    <img src="SD/png/Button_Previous_Green.png"     width="1" height="1" alt="Image 10" />
    <img src="SD/png/Button_Previous_Yellow.png"    width="1" height="1" alt="Image 11" />
    <img src="SD/png/Button_Previous_Blue.png"      width="1" height="1" alt="Image 12" />
    <img src="SD/png/Button_Next_Green.png"         width="1" height="1" alt="Image 13" />
    <img src="SD/png/Button_Next_Yellow.png"        width="1" height="1" alt="Image 14" />
    <img src="SD/png/Button_Volume_Down_Blue.png"   width="1" height="1" alt="Image 15" />
    <img src="SD/png/Button_Volume_Down_Yellow.png" width="1" height="1" alt="Image 16" />
    <img src="SD/png/Button_Volume_Up_Blue.png"     width="1" height="1" alt="Image 17" />
    <img src="SD/png/Button_Volume_Up_Yellow.png"   width="1" height="1" alt="Image 18" />
    <img src="SD/png/Button_Mute_Green.png"         width="1" height="1" alt="Image 19" />
    <img src="SD/png/Button_Mute_Yellow.png"        width="1" height="1" alt="Image 20" />
    <img src="SD/png/Button_Mute_Red.png"           width="1" height="1" alt="Image 21" />
    <img src="SD/png/Button_Ready_Blue.png"         width="1" height="1" alt="Image 22" />
    <img src="SD/png/Button_Ready_Yellow.png"       width="1" height="1" alt="Image 23" />
    <img src="SD/png/Button_Test_Green.png"         width="1" height="1" alt="Image 24" />
    <img src="SD/png/Button_Test_Yellow.png"        width="1" height="1" alt="Image 25" />
    <img src="SD/png/Button_Upload_Blue.png"        width="1" height="1" alt="Image 26" />
    <img src="SD/png/Button_Upload_Yellow.png"      width="1" height="1" alt="Image 27" />
    <img src="SD/png/Button_Download_Blue.png"      width="1" height="1" alt="Image 28" />
    <img src="SD/png/Button_Download_Yellow.png"    width="1" height="1" alt="Image 29" />
  </div>

  <div id="dialog">
    <table>
      <tr>
        <td> x </td>
        <td> <input type="text" id="txtX" size="100"/></td>
      </tr>
      <tr>
        <td>  Cy  </td>
        <td> <input type="text" id="txtCy" size="100"/></td>
      </tr>
      <tr>
        <td>  StationName  </td>
        <td> <input type="text" id="txtStationName" size="100"/></td>
      </tr>
      <tr>
        <td>  StreamURL  </td>
        <td> <input type="text" id="txtStreamURL" size="100"/></td>
      </tr>
      <tr>
        <td>  STsubstitute  </td>
        <td> <input type="text" id="txtStSub" size="100"/></td>
      </tr>
    </table>
  </div>
  <!-- ~~~~~~~~~~~~~~~~~~~~ hidden div end ~~~~~~~~~~~~~~~~~~~~~~-->

  <!--==============================================================================================-->
  <div style="height: 66px; display: flex;">
    <div style="flex: 0 0 345px;">
      <img id="btn1" src="SD/png/Radio_Yellow.png" alt="radio" onclick="showTab1()" />
      <img id="btn2" src="SD/png/Station_Green.png" alt="station" onclick="showTab2()" />
      <img id="btn3" src="SD/png/MP3_Green.png" alt="mp3" onclick="showTab3()" />
      <img id="btn4" src="SD/png/Search_Green.png" alt="search" onclick="showTab4()" />
      <img id="btn5" src="SD/png/About_Green.png" alt="radio" onclick="showTab5()" />
    </div>
    <div style="font-size: 50px; text-align: center; flex: 1;">
      MiniWebRadio
    </div>
  </div>
  <hr>

<!--==============================================================================================-->
  <div id="tab-content1">
    <div style="height: 66px; display: flex;">
      <div style="flex: 0 0 210px;">
        <img src="SD/png/Button_Previous_Green.png" alt="previous" onmousedown="this.src='SD/png/Button_Previous_Yellow.png'" onmouseup="this.src='  SD/png/Button_Previous_Green.png'" onclick="httpGet('prev_station', 1)" />
        <img src="SD/png/Button_Next_Green.png" alt="next" onmousedown="this.src='SD/png/Button_Next_Yellow.png'" onmouseup="this.src='  SD/png/Button_Next_Green.png'" onclick="httpGet('next_station', 1)" />
      </div>
      <div style="flex:1;">
        <select class="boxstyle" style="width:100%; margin-top: 14px;" onchange="handleStation(this)" id="preset">
          <option value="-1">Select a station here</option>
        </select>
      </div>
    </div>
    <div style="height: 112px; display: flex;">
      <div style="flex: 0 0 210px;">
        <label for="label-logo" id="label-logo" onclick="httpGet('homepage', 1)"> </label>
      </div>
      <div style="display: flex; flex:1; justify-content: center;">
        <div style="width: 380px; height:108px;">
          <label class="sdr_lbl_left">Treble Gain:</label>
          <div class="slidecontainer" style="float: left; width 180px; height: 25px;">
            <input type="range" min="0" max="15" value="8" id="TrebleGain" onmouseup="slider_TG_mouseUp()" oninput="slider_TG_change()">
          </div>
          <label id="label_TG_value" class="sdr_lbl_right">000,0</label>
          <label class="sdr_lbl_measure">dB</label>

          <label class="sdr_lbl_left">Treble Freq:</label>
          <div class="slidecontainer" style="float: left; height: 25px;">
            <input type="range" min="1" max="15" value="8" id="TrebleFreq" onmouseup="slider_TF_mouseUp()" oninput="slider_TF_change()">
          </div>
          <label id="label_TF_value" class="sdr_lbl_right">00</label>
          <label class="sdr_lbl_measure">KHz</label>

          <label class="sdr_lbl_left">Bass Gain:</label>
          <div class="slidecontainer" style="float: left; height: 25px;">
            <input type="range" min="0" max="15" value="8" id="BassGain" onmouseup="slider_BG_mouseUp()" oninput="slider_BG_change()">
          </div>
          <label id="label_BG_value" class="sdr_lbl_right">+00</label>
          <label class="sdr_lbl_measure">dB</label>

          <label class="sdr_lbl_left">Bass Freq:</label>
          <div class="slidecontainer" style="float: left; height: 25px;">
            <input type="range" min="2" max="15" value="6" id="BassFreq" onmouseup="slider_BF_mouseUp()" oninput="slider_BF_change()">
          </div>
          <label  id="label_BF_value" class="sdr_lbl_right">000</label>
          <label class="sdr_lbl_measure">Hz</label>
        </div>
      </div>
    </div>
    <div style="height: 66px; display: flex;">
      <div style="flex: 0 0 210px;">
        <img src="SD/png/Button_Volume_Down_Blue.png" alt="Vol_down" onmousedown="this.src='SD/png/Button_Volume_Down_Yellow.png'" onmouseup="this.src='SD/png/Button_Volume_Down_Blue.png'" onclick="httpGet('downvolume=2', 1)" />
        <img src="SD/png/Button_Volume_Up_Blue.png" alt="Vol_up" onmousedown="this.src='SD/png/Button_Volume_Up_Yellow.png'" onmouseup="this.src='SD/png/Button_Volume_Up_Blue.png'" onclick="httpGet('upvolume=2', 1)" />
        <img id="Mute" src="SD/png/Button_Mute_Green.png" alt="Mute" onmousedown="this.src='SD/png/Button_Mute_Yellow.png'" onclick="httpGet('mute', 1)" />
      </div>
      <div style="flex:1;">
        <input type="text" class="boxstyle" style="width: calc(100% - 8px); margin-top: 14px; padding-left:7px 0;" id="cmd" placeholder=" Waiting....">
      </div>
    </div>
    <div style="height: 66px; display: flex;">
      <div style="flex:1;">
        <input type="text" class="boxstyle" style="width: calc(100% - 8px); margin-top: 14px; padding-left:7px 0;" id="station" placeholder=" Enter a streamURL here....">
      </div>
      <div style="flex: 0 0 66px;">
        <img src="SD/png/Button_Ready_Blue.png" alt="Vol_up" onmousedown="this.src='SD/png/Button_Ready_Yellow.png'" onmouseup="this.src='SD/png/Button_Ready_Blue.png'" onclick="setstation()" />
      </div>
    </div>
    <div style="height: 66px; display: flex;">
      <div style="flex:1;">
        <input type="text" class="boxstyle" style="width: calc(100% - 8px); margin-top: 14px; padding-left:7px 0;" id="resultstr1" placeholder=" Test....">
      </div>
      <div style="flex: 0 0 66px;">
        <img src="SD/png/Button_Test_Green.png" alt="Test" onmousedown="this.src='SD/png/Button_Test_Yellow.png'" onmouseup="this.src='SD/png/Button_Test_Green.png'" onclick="httpGet('test', 1)" />
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
  <!--==============================================================================================-->
  <div id="tab-content2">
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
      <input id="file" type="file" style="visibility: hidden; width: 0px;"  name="img"
          onchange="loadDataExcel(this.files);"/>
      <br>
      </center>
  </div>
  <!--==============================================================================================-->
  <div id="tab-content3">
      <center>
      <br>
      <label for="seltrack"><big>MP3 files on SD card:</big></label>
      <br>
      <select class="boxstyle" onchange="trackreq(this)" id="seltrack">
          <option value="-1">Select a track here</option>
      </select>
      <br><br>
      <button class="button" onclick="httpGet('mp3track=0', 3)">STOP</button>
      <br><br>
      <input type="text" class="boxstyle" style="width: calc(100% - 8px);" id="resultstr3" placeholder="Waiting for a command...."> <br>
      <br><br>
      </center>
  </div>
  <!--==============================================================================================-->
  <div id="tab-content4">
      <div style="height: 30px;">
        This service is provided by
        <a target="_blank" href="http://www.radio-browser.info/gui/#/">Community Radio Browser</a>
      </div>
      <div style="display: flex;">
        <div style="flex: 0 0 calc(100% - 66px);">
          <select class="boxstyle" style="width: 100%;" onchange="selectcountry(this)" id="country">
            <option value="-1">Select a country here</option>
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
        <div style="flex: 1; padding-left: 2px; height: 66px;">
          <img src="SD/png/Button_Ready_Blue.png" alt="Vol_up"
            onmousedown="this.src='SD/png/Button_Ready_Yellow.png'"
            onmouseup="this.src='SD/png/Button_Ready_Blue.png'"
            onclick="teststreamurl()"/>
        </div>
      </div>
      <div style="display: flex;">
        <div style="flex: 0 0 calc(100% - 66px); height: 66px;">
          HomepageUrl
          <input type="text" class="boxstyle" style=" width: calc(100% - 8px);"
          id="homepageurl" placeholder="HomepageURL">
        </div>
        <div style="flex: 1; padding-left: 2px; height: 66px;">
          <img src="SD/png/Button_Ready_Blue.png" alt="Vol_up"
            onmousedown="this.src='SD/png/Button_Ready_Yellow.png'"
            onmouseup="this.src='SD/png/Button_Ready_Blue.png'"
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
            onmouseup="this.src='SD/png/Button_Ready_Blue.png'"
            onclick="window.open(favicon.value, '_blank')"/>
        </div>
      </div>
      <hr>
      <div style="display: flex;">
        <div style="flex: 0 0 105px; padding 1px 5px 5px 1px; ">
          <canvas id="canvas" width="96" height="96" class="playable-canvas"></canvas>
        </div>
        <div style="flex: 1;">
          <div style="flex: 1; height: 38px;">
            <input type="text" class="boxstyle" style="width: calc(100% - 74px);"
                id="stationname" placeholder="Change the Stationname here">
          </div>
          <div style="flex: 1;  padding-top: 4px;">
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
              <input name="hidden_data" id="hidden_data" type="hidden"/>
            </form>
          </div>
        </div>
      </div>
  </div>
  <!--==============================================================================================-->
  <div id="tab-content5">
    <p> MiniWebRadio -- Webradio receiver for ESP32, 2.8" color display and VS1053 MP3 module.<br>
    This project is documented on
    <a target="blank" href="https://github.com/schreibfaul1/ESP32-MiniWebRadio">Github</a>.</p>
    <p>Author: Wolle (schreibfaul1)</p><br>
    <img src="SD/MiniWebRadio_gr.jpg" alt="MiniWebRadio_gr" border="3">
    <h3>Connected WiFi network
      <select class="boxstyle" onchange="handletone(this)" id="ssid"></select>
    </h3>
  </div>
  <!--==============================================================================================-->
</div>
</body>
</html>

)=====" ;
#endif /* WEB_H_ */
