/*
 *  accesspoint.h
 *
 *  Created on: 12.06.2023
 *  Updated on: 12.06.2023
 *      Author: Wolle
 *
 */

#ifndef ACCESSPOINT_H_
#define ACCESSPOINT_H_

#include "Arduino.h"

const char accesspoint_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>MiniWebRadio</title>

    <style type="text/css">           /* optimized with csstidy */
        html {  /* This is the groundplane */
            font-family : serif;
            height : 100%;
            font-size: 16px;
            color : DarkSlateGray;
            background-color : lightskyblue;
            margin : 0;
            padding : 0;
        }
    </style>

</head>
<body>
<script>
    // ---- websocket section------------------------

    var socket = undefined

    function connect() {
        socket = new WebSocket('ws://'+window.location.hostname+':81/');
        socket.onopen = function () {
            console.log("Websocket connected")
            socket.send('AP_ready')
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
                case "networks":        var select = document.getElementById('sel_SSID')
                                        var lines = val.split('\n')
                                        var option
                                        for (i = 0; i < (lines.length - 1); i++) {
                                            option = new Option(lines[i])
                                            console.log(lines[i]);
                                            select.appendChild(option);
                                        }
                                        break
                default:                console.log('unknown message', msg, val)
            }
        }
    }
    // ---- end websocket section------------------------

    function submit() {
        var select = document.getElementById("sel_SSID");
        var input = document.getElementById("input_PW");

        var SSID = select.value;
        var PW = input.value;

        console.log("SSID: " + SSID);
        console.log("PW: " + PW);

        socket.send('credentials=' + SSID + '\n' + PW)
    }

    document.addEventListener('readystatechange', event => {
        if (event.target.readyState === 'interactive') { // same as:  document.addEventListener('DOMContentLoaded'...
            // same as  jQuery.ready
            console.log('All HTML DOM elements are accessible')
        }
        if (event.target.readyState === 'complete') {
            console.log('Now external resources are loaded too, like css,src etc... ')
            connect();  // establish websocket connection
        }
    })
</script>

<h1>MiniWebRadio</h1>
    <label for="WiFi Networks">WiFi Networks:</label>
    <select id="sel_SSID">
    </select>

    <br><br>

    <label for="Password">Password:</label>
    <input type="text" id="input_PW">

    <br><br>

    <button onclick="submit()">Submit</button>
</body>
</html>

)=====" ;
#endif /* ACCESSPOINT_H_ */
