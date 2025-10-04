/*
 *  index.js.h
 *
 *  Created on: 29.06.2023
 *  Updated on: 25.10.2024
 *      Author: Wolle
 *
 *
 */

#ifndef INDEX_JS_H_
#define INDEX_JS_H_

#include "Arduino.h"

// file in raw data format for PROGMEM
//

const char index_js[] PROGMEM = R"=====(

/*****************************************************************************************************************************************************
 *                                              c o m m o n   J S T r e e                                                                            *
 ****************************************************************************************************************************************************/


var iconFolderYellow = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABHNCSVQICAgIfAhkiAAAAL9J"
                + "REFUSIntlL0KwmAMRU9qFaydHHXS53L3NfoyLr6Ui+ImCKUotr0ulg4t2l9Q6Z3CB19ObkgC3y5J42YfwaTA6bieD1DJ2i"
                + "fZM5IoJCp7AwiC3KUCCo4N4LTzVAW+CKOJbXlUL5ci8Z3OvnfJihHljloBBH4WG1RyXatFtSQOy0207m8MjRWAC2CO4U7b"
                + "T2AmJRDfUnKAwWzebCHLFN9Twheg900dAAPgBwBuFqjTa5RfBRcgTcT1eO+S8Ed6ApsnOMljAhdKAAAAAElFTkSuQmCC"

var iconDLNAGreen = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABHNCSVQICAgIfAhkiAAAAnNJREF"
                + "USIm1Vc9rE0EU/nY3TaPU1B9YoWSNLVI1UEO8VjGQCKHWRD1IqfTgreAlFjHUg+hfYBAVxYPIgrciaSKixCpqW8lFS7V6aG0TEj"
                + "Fg0MRUi2Z3PJSs+2M2uxH9bvNm3vvem+/NG0YihOA/wmZ2IJFNAwCGZ+O6PcEbRcQdbOjP0CqoERH3c08MAyvhcbox1nUMh1wHs"
                + "J5rNSdolHEdvvadGN0xAAC4W5zGg08ZakWEEDVBIps2zfiEy4+bvlFwDCvb4gvjuDB/WyYJuwNgwKgrsBI81jOIcz2DaGHV0hEQ"
                + "7JscwWK1oKuEqsHfQJtgnYQRiUQmso9Nsz/JBxDa3GvYNcHnZ5H58l5lq4RT4Hyn9l80Cw4Ac5Ul3Cu+hMfRgd0bu3XZX11O6Xw"
                + "8jg76O4h09iG+9zS22J0QiYRUblKucHg2DkFxNi+uYOzNLcPEmLbEYZUGezZsx4uDV3RCWmkCGlit4Xz3cV1wAIi4gxC80aYJTE"
                + "eFEv28HxWNyN9q3+Fg7RCJBI5lUfjxGa51W1H6WcG21k16guv5R9ROmS69RWgqBhvDwcG1oFpbtZQUqy17pjSPRDYtjwxg7f5DU"
                + "zEAa3OqWluFRCSIRDQlYCRCiHNigLopeKNUYSthdUs28teJrISV4HVbb3sXNYbNyKlZ7GrjMVdeUtki7uAfkY3KVOKG7wyG+AB1"
                + "b6GaV62/HkkCULyDcjhp2ucjry7j0rs7Ovt44Rlelz/Ia8EbBctoxnUdVl7s0c4+DPFBODg7HhYzuLaYkAMr/wIqAQCUf63g6ce"
                + "ZpkaD0f/c8D+QCEEyZzzKBW8U/byfOlosEfwL/AZ9yBk6MV8BXwAAAABJRU5ErkJggg=="

var iconFileRed  = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABHNCSVQICAgIfAhkiAAAAJxJR"
                + "EFUSIljYBgF5IL/DAz/ceEDMhr/3xpb/yfGHEZ8FhyU0cAq5/DkBuMBGY3/uuLCDMJnj+I0g4GBgYGJGFfgApdfvmUg5BOK"
                + "LCDGEootIGQJVSyAWYINsJBjmMP++RwOjomM////Z2RkZPzPwMDAcEBGA6sPyEpF2ID9kxsMjFjMo1oQ4QKjFoxaMGoBHSz"
                + "AW1SQYRjeymdoAgDd9jyx3CsGWQAAAABJRU5ErkJggg=="

var iconFileGreen = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABHNCSVQICAgIfAhkiAAAAKdJR"
                + "EFUSIljYBgFBAAjLgm9Q3X/ccn9ePCagVdfkOGsfhtO/TDAhE/yx4PXWPGtuOmMny++ZzC+WIXTEURZQAgQYwlFFhBjCcUW"
                + "ELKEKhbALMEGWMgxzGH/fI4DjomM////Z2RkZPzPwMDAoLYoE6sP8CbTHw9eE20ph4IowyW7JgzzqBZEuMCoBaMWEAVoWxY"
                + "x4MlL1LIAL6C5BXjLIg4FUVrbPwQAAP4cQnQllyzuAAAAAElFTkSuQmCC"

var iconFileBlue = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABHNCSVQICAgIfAhkiAAAAKZJRE"
                + "FUSIljYBgFBAAjLgnTlif/cck9vX2BwcPRjGFeghhO/TDAgk/y6e0LWMWfLfRh3MGw5X/Sglf/CVnCRMgF+MCO/acYkha8wu"
                + "lTii0gxhKKLSBkCVUsgFmCDeCNZFzAYf98jgOOPoz///9nZGRk/M/AwMAgFb8Fqw/wJlNcqQgbkFY1YDhdI4NhHtWCCBcYtW"
                + "DUglEL6GAB3rJIWtWA1vYPAQAAsLk0T5/6UqIAAAAASUVORK5CYII="

var iconFileYellow = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABHNCSVQICAgIfAhkiAAAAKRJRE"
                + "FUSIljYBgFBAAjLon/Nxj+45J7dpaLQdg+iIFDZglO/TDAgk/y2VkurOLS0d8Yny5d9//Hk5j/hCxhIuQCfODtwXUMP57E4P"
                + "QpxRYQYwnFFhCyhCoWwCzBBvBGMi7gsH8+h7RjIuP///8ZGRkZ/zMwMDA8XcqF1Qd4kymuVIQNSBl/Y2DUwDSPakGEC4xaMG"
                + "rBqAV0sICsGg2nYViKiqEPAOK0NZUptwOwAAAAAElFTkSuQmCC"


var lastNodeID = 0;
var lastNode

//----------------------------------------------------------------------------------------------------------------------------------------------------
function deleteChildren(nodeId) {
    $('#audioFileTree').jstree('open_node', nodeId); // need to open node for accurate selection
    var ref = $('#audioFileTree').jstree(true);
    var children = $("#audioFileTree").jstree("get_children_dom",nodeId);
    for(var i=0;i<children.length;i++) {
        console.log("delete child", i)
        ref.delete_node(children[i]);
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
function addFileDirectory(nodeId, content) {
    content.sort( fileNameSort );
    var ref = $('#audioFileTree').jstree(true);
    for (var i=0; i< content.length; i++) {
        console.log("Create Node", content[i]);
        ref.create_node(nodeId, createChild(nodeId, content[i]));
    }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
function refreshNode(ref, nodeId) {
    var node = ref.get_node(nodeId);
    getData(ref.text + "GetFolder?" + encodeURIComponent(node.data.path), function(content) {
        /* We now have data! */
        deleteChildren(nodeId);
        addFileDirectory(nodeId, content);
        ref.open_node(nodeId);
    });
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

function getType(data) {
    var type = "";
    if(data.dir)                                                                type = "folder";
    else if ((/\.(mp3|ogg|oga|wav|aac|m4a|flac|opus|m3u8)$/i).test(data.name))  type = "audio";
    else if ((/\.(png|jpg|jpeg|bmp|gif)$/i).test(data.name))                    type = "image";
    else if ((/\.(m3u)$/i).test(data.name))                                     type = "playlist";
    else                                                                        type = "file";
    return type;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

function getData(path, callback) {
    var theUrl = path + '&version=' + Math.random().toString()
    var xhr = new XMLHttpRequest()
    xhr.open('GET', theUrl, true)
    xhr.setRequestHeader('dataType', 'json')
    xhr.setRequestHeader('Content-type', 'application/json; charset=utf-8')
    xhr.onreadystatechange = function () { // Call a function when the state changes.
        if (xhr.readyState === 4) {
            if (xhr.status != 200) console.log("getData error:", path, "status:", xhr.status)
            else{
                var json = JSON.parse(xhr.responseText)
                if(callback) callback(json)
            }
        }
    }
    xhr.send() // send
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

function deleteNode(nodeId) {
    var ref = $('#audioFileTree').jstree(true);
    var node = ref.get_node(nodeId);
    var theUrl = "SD/?" + encodeURIComponent(node.data.path) + '&version=' + Math.random().toString()
    var xhr = new XMLHttpRequest()
    xhr.timeout = 2000; // time in milliseconds
    xhr.open('DELETE', theUrl, true)
    xhr.ontimeout = (e) => {
        // XMLHttpRequest timed out.
        alert('delete ' + node.data.path, 'timeout')
    }
    xhr.onreadystatechange = function () { // Call a function when the state changes.
        if (xhr.readyState === 4) {
            if (xhr.status == 200) console.log(node.data.path + ' successfully deleted')
            if (xhr.status == 400) alert(node.data.path + ' could not be deleted')
        }
    }
    xhr.send() // send
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

function fileNameSort( a, b ) {
    if ( a.dir && !b.dir ) return -1
    if ( !a.dir && b.dir ) return  1
    if ( a.name < b.name ) return -1
    if ( a.name > b.name ) return  1
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

function createChild(nodeId, data) {
    var ref = $('#audioFileTree').jstree(true);
    var node = ref.get_node(nodeId);
    var parentNodePath = node.data.path;
    /* In case of root node remove leading '/' to avoid '//' */
    if(parentNodePath == "/") parentNodePath = "";
    var child = {
        text: data.name,
        type: getType(data),
        data: {
            path: parentNodePath + "/" + data.name,
            directory: data.dir
        },
        a_attr: {title:data.size}
    };
    return child;
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

function XmlHttpReq1 (method, url, postmessage) {
    var ref = $('#audioFileTree').jstree(true)
    ref.text = "SD_"
    var theUrl = url+ '&version=' + Math.random()
    var xhr = new XMLHttpRequest()
    xhr.timeout = 2000; // time in milliseconds
    xhr.open(method, theUrl, true)
    xhr.ontimeout = (e) => {
        //XMLHttpRequest timed out.
        //alert(url + 'timeout')
    }
    xhr.onreadystatechange = function () { // Call a function when the state changes.
        if (xhr.readyState === 4) {
            var resp = xhr.responseText
            console.log(resp);
            if(resp.startsWith('SD_playFile=')){
                resultstr3.value = "Audiofile is " + resp.substring(12)
            }
            if(resp === "refresh") refreshNode(ref, lastNodeID);
        }
    }
    xhr.send(postmessage) // send
}
//----------------------------------------------------------------------------------------------------------------------------------------------------

function uploadFile(uploadFile){
    if(!uploadFile) return;
    if (!lastNode.data.directory){
        alert("selected " + lastNode.data.path + " is not a folder! Select target folder and try again.")
        document.getElementById('audioPlayer_File').value = null;
        return
    }
    var ref = $('#audioFileTree').jstree(true)
    ref.text = "SD_"
    var startTime, total
    var file = uploadFile[0]
    console.log(file.name, file.size)
    if(lastNode.data.path === '/') filename = file.name
    else filename = "/" + file.name
    var theUrl = 'SD_Upload?' + lastNode.data.path + filename + '&version=' + Math.random()
    var fd = new FormData(document.forms.form2)
    var xhr = new XMLHttpRequest()
    // xhr.timeout = 2000; // time in milliseconds
    xhr.open('POST', theUrl, true)
    xhr.upload.onprogress = function (e) {
        if (e.lengthComputable) {
            var percentComplete = (e.loaded / e.total) * 100
            console.log(percentComplete + '% uploaded')
            deltaT = (new Date().getTime() - startTime) / 1000 ;
            Bps = e.loaded /deltaT
            KBps = Bps / 1024
            console.log("KB/s =", KBps)
            $("#explorerUploadProgress").css('width', percentComplete + "%").text(KBps.toFixed(2) + " KB/s");
        }
    }
    xhr.onload = function () {
    }
    xhr.ontimeout = (e) => {
        // XMLHttpRequest timed out.
        alert(filename + ' not uploaded, timeout')
    }
    xhr.onreadystatechange = function () {        // Call a function when the state changes.
        if (xhr.readyState === 4) {
            refreshNode(ref, lastNode.id)
            if (xhr.status == 200){
                alert(filename + ' successfully uploaded')
            }
            else alert(filename + ' upload failed')
        }
        $("#explorerUploadProgress").css('width', 0 + "%").text("");
        document.getElementById('audioPlayer_File').value = null;
    }
    startTime = new Date().getTime();
    xhr.send(fd)
}

/*****************************************************************************************************************************************************
 *                                              a u d i o F i l e T r e e                                                                            *
 ****************************************************************************************************************************************************/

$('#audioFileTree').on('select_node.jstree', function (e, data) {
    var ref = $('#audioFileTree').jstree(true)
    ref.text = "SD_"
    var node = data.node;
    var nodeId = node.id;
    var path = node.data.path;
    console.log("select: nodeId=", nodeId, "path=", path)
    lastNode = node
    lastNodeID = nodeId
    resultstr3.value = path;
    if (data.node.type == "folder") {
        $('.option-folder').show();
        $('.option-file').hide();
    }
    if (data.node.data.directory) {
        refreshNode(ref, nodeId);
    }
    ref.open_node(nodeId);
});
//----------------------------------------------------------------------------------------------------------------------------------------------------

function audioPlayer_buildFileSystemTree(path) {
    $('#audioFileTree').jstree({
        "core": {
            "check_callback": true,
            'force_text': true,
            'strings': {
              "Loading ...": "Please wait..."
            },
            "themes": {
                "stripes": true
            },
            'data': {
                text: '/',
                state: {
                    opened: true
                },
                type: 'folder',
                children: [],
                data: {
                    path: '/',
                    directory: true
                }
            }
        },
        'types': {
            'folder': {  // folder_yellow.png (24x24px)
              'icon': iconFolderYellow
            },
            'file': { // file_red.png
              'icon': iconFileRed
            },
            'audio': { // file_green.png
              'icon': iconFileGreen
            },
            'image': { // file_blue.png
              'icon': iconFileBlue
            },
            'playlist': { // file_blue.png
              'icon': iconFileYellow
            },
            'default': { // file_yellow.png
              'icon': iconFileRed
            }
        },
        plugins: ["contextmenu", "themes", "types"],
        contextmenu: {
            items: function (nodeId) {
                var ref = $('#audioFileTree').jstree(true)
                ref.text = "SD_"
                var node = ref.get_node(nodeId);
                var items = {};
                /* Play All */
                if (node.data.directory){
                    items.play = {
                        label: "Play All",
                        action: function (x) {
                            XmlHttpReq1("GET", "SD_playAllFiles?" +  encodeURIComponent(node.data.path), "playAll")
                        }
                    }
                };
                /* New Folder */
                if (node.data.directory) {
                    items.createDir = {
                        label: "New Folder",
                        action: function (x) {
                            var childNode = ref.create_node(nodeId, {
                                text: "New Folder", type: "folder"
                            });
                            if (childNode) {
                                ref.edit(childNode, null, function (childNode, status) {
                                    var pathAndName = encodeURIComponent(node.data.path) + "/" + encodeURIComponent(childNode.text)
                                    getData("SD_newFolder?" + pathAndName);
                                    refreshNode(ref, nodeId);
                                });
                            }
                        }
                    };
                }
                /* Play */
                if ((/\.(mp3|ogg|oga|wav|aac|m4a|flac|opus|m3u)$/i).test(node.data.path)) {
                    items.play = {
                        label: "Play",
                        action: function (x) {
                            XmlHttpReq1("GET", "SD_playFile?" +  encodeURIComponent(node.data.path), "play")
                        }
                    };
                }
                /* Refresh */
                if (node.data.directory) {
                    items.refresh = {
                        label: "Refresh",
                        action: function (x) {
                            refreshNode(ref, nodeId);
                        }
                    };
                }
                /* Delete */
                items.delete = {
                    label: "Delete",
                    action: function (x) {
                        deleteNode(nodeId);
                        refreshNode(ref, ref.get_parent(nodeId));
                    }
                };
                /* Rename */
                items.rename = {
                    label: "Rename",
                    action: function (x) {
                        var srcPath = node.data.path;
                        ref.edit(nodeId, null, function (node, status) {
                            node.data.path = node.data.path.substring(0, node.data.path.lastIndexOf("/") + 1) + node.text;
                            lastNodeID = ref.get_parent(nodeId)
                            XmlHttpReq1("GET", "SD_rename?" + encodeURIComponent(srcPath) + "&" + encodeURIComponent(node.data.path), "rename")
                        });
                    }
                };
                /* Download */
                if (!node.data.directory) {
                    items.download = {
                        label: "Download",
                        action: function (x) {
                            uri = "SD_Download?" + encodeURIComponent(node.data.path);
                            var anchor = document.createElement('a');
                            anchor.href = uri;
                            anchor.target = '_blank';
                            console.log("download file: ", node.text);
                            anchor.download = node.text;
                            anchor.click();
                            document.body.removeChild(document.body.lastElementChild);
                        }
                    }
                };
                return items;
            }
        }
    })
    if (path.length == 0) {
        return;
    }
    var ref = $('#audioFileTree').jstree(true)
    ref.text = "SD_"
    getData("SD_GetFolder?/", function(data) {
        /* We now have data! */
        console.log("data", data)
        ref.settings.core.data.children = [];
        data.sort( fileNameSort );
        for (var i=0; i<data.length; i++) {
            var newChild = {
                text: data[i].name,
                type: getType(data[i]),
                data: {
                    path: "/" + data[i].name,
                    directory: data[i].dir
                },
                a_attr: {title:data[i].size},
                children: []
            };
            ref.settings.core.data.children.push(newChild);
        }
        ref.refresh();
    });
} /* audioPlayer_buildFileSystemTree */

/*****************************************************************************************************************************************************
 *                                               d l n a F i l e T r e e                                                                             *
 ****************************************************************************************************************************************************/

$('#dlnaFileTree').on('select_node.jstree', function (e, data) {
    var ref = $('#dlnaFileTree').jstree(true)
    ref.text = "DLNA_"
    var node = data.node;
    var nodeId = node.id;
    var path = node.data.path;
    console.log("select: nodeId=", nodeId, "path=", path)
    lastNode = node
    lastNodeID = nodeId
    resultstr3.value = path;
    if (data.node.type == "folder") {
        $('.option-folder').show();
        $('.option-file').hide();
    }
    if (data.node.data.directory) {
        refreshNode(ref, nodeId);
    }
    ref.open_node(nodeId);
});
//----------------------------------------------------------------------------------------------------------------------------------------------------

function dlnaPlayer_buildFileSystemTree(path) {
    $('#dlnaFileTree').jstree({
        "core": {
            "check_callback": true,
            'force_text': true,
            'strings': {
              "Loading ...": "Please wait..."
            },
            "themes": {
                "stripes": true
            },
            'data': {
                text: '/',
                state: {
                    opened: true
                },
                type: 'server',
                children: [],
                data: {
                    path: '/',
                    directory: true
                }
            }
        },
        'types': {
            'server': {  // dlna server green
                'icon': iconDLNAGreen
            },
            'folder': {  // folder_yellow.png (24x24px)
                'icon': iconFolderYellow
            },
            'file': { // file_red.png
                'icon': iconFileRed
            },
            'audio': { // file_green.png
                'icon': iconFileGreen
            },
            'image': { // file_blue.png
                'icon': iconFileBlue
            },
            'playlist': { // file_blue.png
                'icon': iconFileYellow
            },
            'default': { // file_yellow.png
                'icon': iconFileRed
            }
        },
        plugins: ["contextmenu", "themes", "types"],
        contextmenu: {
            items: function (nodeId) {
                var ref = $('#dlnaFileTree').jstree(true)
                ref.text = "DLNA_"
                var node = ref.get_node(nodeId);
                var items = {};
                /* Play */
                if ((/\.(mp3|ogg|oga|wav|aac|m4a|flac|opus|m3u)$/i).test(node.data.path)) {
                    items.play = {
                        label: "Play",
                        action: function (x) {
                            XmlHttpReq1("GET", "DLNA_playFile?" +  encodeURIComponent(node.data.path), "play")
                        }
                    };
                }
                /* Refresh */
                if (node.data.directory) {
                    items.refresh = {
                        label: "Refresh",
                        action: function (x) {
                            refreshNode(ref, nodeId);
                        }
                    };
                }
                /* Download */
                if (!node.data.directory) {
                    items.download = {
                        label: "Download",
                        action: function (x) {
                            uri = "DLNA_Download?" + encodeURIComponent(node.data.path);
                            var anchor = document.createElement('a');
                            anchor.href = uri;
                            anchor.target = '_blank';
                            console.log("download file: ", node.text);
                            anchor.download = node.text;
                            anchor.click();
                            document.body.removeChild(document.body.lastElementChild);
                        }
                    }
                };
                return items;
            }
        }
    })
    if (path.length == 0) {
        return;
    }
    var ref = $('#audioFileTree').jstree(true)
    ref.text = "DLNA_"
    getData("DLNA_GetFolder?/", function(data) {
        /* We now have data! */
        console.log("data", data)
        ref.settings.core.data.children = [];
        data.sort( fileNameSort );
        for (var i=0; i<data.length; i++) {
            var newChild = {
                text: data[i].name,
                type: getType(data[i]),
                data: {
                    path: "/" + data[i].name,
                    directory: data[i].dir
                },
                children: []
            };
            ref.settings.core.data.children.push(newChild);
        }
        ref.refresh();
    });
} /* dlnaPlayer_buildFileSystemTree */

)=====" ;
#endif /* INDEX_JS_H_ */