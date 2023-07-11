/*
 *  index.js.h
 *
 *  Created on: 29.06.2023
 *  Updated on: 08.07.2023
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

// --------------------------------------------------------- File Explorer ---------------------------------------------------------------------------

var iconFolderYellow = "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABgAAAAYCAYAAADgdz34AAAABHNCSVQICAgIfAhkiAAAAL9J"
                + "REFUSIntlL0KwmAMRU9qFaydHHXS53L3NfoyLr6Ui+ImCKUotr0ulg4t2l9Q6Z3CB19ObkgC3y5J42YfwaTA6bieD1DJ2i"
                + "fZM5IoJCp7AwiC3KUCCo4N4LTzVAW+CKOJbXlUL5ci8Z3OvnfJihHljloBBH4WG1RyXatFtSQOy0207m8MjRWAC2CO4U7b"
                + "T2AmJRDfUnKAwWzebCHLFN9Twheg900dAAPgBwBuFqjTa5RfBRcgTcT1eO+S8Ed6ApsnOMljAhdKAAAAAElFTkSuQmCC"

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
//----------------------------------------------------------------------------------------------------------------------
function deleteChildren(nodeId) {
    $('#explorerTree').jstree('open_node', nodeId); // need to open node for accruate selection
    var ref = $('#explorerTree').jstree(true);
    var children = $("#explorerTree").jstree("get_children_dom",nodeId);
    for(var i=0;i<children.length;i++) {
        console.log("delete child", i)
        ref.delete_node(children[i]);
    }
}
//----------------------------------------------------------------------------------------------------------------------
function addFileDirectory(nodeId, content) {
    content.sort( fileNameSort );
    var ref = $('#explorerTree').jstree(true);
    for (var i=0; i< content.length; i++) {
        console.log("Create Node", content[i]);
        ref.create_node(nodeId, createChild(nodeId, content[i]));
    }
}
//----------------------------------------------------------------------------------------------------------------------
function refreshNode(nodeId) {
    var ref = $('#explorerTree').jstree(true);
    var node = ref.get_node(nodeId);
    getData("SD_GetFolder?" + encodeURIComponent(node.data.path), function(content) {
        /* We now have data! */
        deleteChildren(nodeId);
        addFileDirectory(nodeId, content);
        ref.open_node(nodeId);
    });
}
//----------------------------------------------------------------------------------------------------------------------
function getType(data) {
    var type = "";
    if(data.dir)                                                            type = "folder";
    else if ((/\.(mp3|ogg|wav|aac|m4a|flac|opus|m3u8)$/i).test(data.name))  type = "audio";
    else if ((/\.(png|jpg|jpeg|bmp|gif)$/i).test(data.name))                type = "image";
    else if ((/\.(m3u)$/i).test(data.name))                                 type = "playlist";
    else                                                                    type = "file";
    return type;
}
//----------------------------------------------------------------------------------------------------------------------
function getData(path, callback) {
    var num = path + '&version=' + Math.random().toString()
    console.log("num: ", num );
    XmlHttpReq(num, callback, {
        method   : "GET"
    });
}
//----------------------------------------------------------------------------------------------------------------------
function deleteData(nodeId) {
    var ref = $('#explorerTree').jstree(true);
    var node = ref.get_node(nodeId);
    postData("SD_delete?" + encodeURIComponent(node.data.path));
}
//----------------------------------------------------------------------------------------------------------------------
function fileNameSort( a, b ) {
    if ( a.dir && !b.dir ) return -1
    if ( !a.dir && b.dir ) return  1
    if ( a.name < b.name ) return -1
    if ( a.name > b.name ) return  1
    return 0;
}
//----------------------------------------------------------------------------------------------------------------------
function createChild(nodeId, data) {
    var ref = $('#explorerTree').jstree(true);
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
        }
    };
    return child;
}
//----------------------------------------------------------------------------------------------------------------------
$('#explorerTree').on('select_node.jstree', function (e, data) {
    var ref = $('#explorerTree').jstree(true)
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
        refreshNode(nodeId);
    }
    ref.open_node(nodeId);
});
//----------------------------------------------------------------------------------------------------------------------
function XmlHttpReq(path, callback, obj) {
    obj.url      = path;
    obj.dataType = "json";
    obj.contentType= "application/json",
    obj.success  = function(data, textStatus, jqXHR) {
        if (callback) {
            callback(data);
        }
    };
    obj.error    = function(jqXHR, textStatus, errorThrown) {
        console.log("AJAX error ", "Path ", path);
        /*debugger; */
    };
    jQuery.ajax(obj);
}
//----------------------------------------------------------------------------------------------------------------------
function XmlHttpReq1 (method, url, postmessage) {
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
            if(resp === "refresh") refreshNode(lastNodeID);
        }
    }
    xhr.send(postmessage) // send
}
//----------------------------------------------------------------------------------------------------------------------
function postData(path, callback, _data) {
    var num = Math.random()
    XmlHttpReq(path + '&version=' + num.toString(), callback, {
        method   : "POST",
        data: _data
    });
}
//----------------------------------------------------------------------------------------------------------------------
function uploadFile(uploadFile){
    if(!uploadFile) return;
    if (!lastNode.data.directory){
        alert("selected " + lastNode.data.path + " is not a folder!")
        document.getElementById('file1').value = null;
        return
    }
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
            refreshNode(lastNode.id)
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
            if (xhr.status == 200){
                alert(filename + ' successfully uploaded')
            }
            else alert(filename + ' upload failed')
        }
        $("#explorerUploadProgress").css('width', 0 + "%").text("");
        document.getElementById('file1').value = null;
    }
    startTime = new Date().getTime();
    xhr.send(fd)
}
//----------------------------------------------------------------------------------------------------------------------
function buildFileSystemTree(path) {
    $('#explorerTree').jstree({
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
                var ref = $('#explorerTree').jstree(true);
                var node = ref.get_node(nodeId);
                var items = {};
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
                                    refreshNode(nodeId);
                                });
                            }
                        }
                    };
                }
                /* Play */
                items.play = {
                    label: "Play",
                    action: function (x) {
                        var playMode = 1;
                        if (node.data.directory) {
                            playMode = 5;
                        }
                        else {
                            if ((/\.(m3u|M3U)$/i).test(node.data.path)) {
                                playMode = 11;
                            }
                        }
                        XmlHttpReq1("GET", "SD_playFile?" +  encodeURIComponent(node.data.path), "play")
                    }
                };
                /* Refresh */
                if (node.data.directory) {
                    items.refresh = {
                        label: "Refresh",
                        action: function (x) {
                            refreshNode(nodeId);
                        }
                    };
                }
                /* Delete */
                items.delete = {
                    label: "Delete",
                    action: function (x) {
                        deleteData(nodeId);
                        refreshNode(ref.get_parent(nodeId));
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
    getData("SD_GetFolder?/", function(data) {
        /* We now have data! */
        console.log("data", data)
        $('#explorerTree').jstree(true).settings.core.data.children = [];
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
            $('#explorerTree').jstree(true).settings.core.data.children.push(newChild);
        }
        $("#explorerTree").jstree(true).refresh();
    });
} /* buildFileSystemTree */

)=====" ;
#endif /* INDEX_JS_H_ */