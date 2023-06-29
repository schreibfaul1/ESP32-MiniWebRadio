/*
 *  index.js.h
 *
 *  Created on: 29.06.2023
 *  Updated on: 
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


var lastSelectedNodePath = "";

$('#explorerTree').on('select_node.jstree', function (e, data) {
    console.log("node")
    $('input[name=fileOrUrl]').val(data.node.data.path);
    if (data.node.type == "folder") {
        $('.option-folder').show();
        $('.option-file').hide();
        $('#playMode option').removeAttr('selected').filter('[value=3]').attr('selected', true);
    }
    if (data.node.type == "audio") {
        $('.option-file').show();
        $('.option-folder').hide();
        $('#playMode option').removeAttr('selected').filter('[value=1]').attr('selected', true);
    }
    if(lastSelectedNodePath != data.node.data.path) {
        if (data.node.data.directory) {
            var ref = $('#explorerTree').jstree(true),
            sel = ref.get_selected();
            if(!sel.length) {
                return false;
            }
            sel = sel[0];
            var children = $("#explorerTree").jstree("get_children_dom",sel);
            /* refresh only, when there is no child -> possible not yet updated */
            if(children.length < 1){
                refreshNode(sel);
            }
        }
        lastSelectedNodePath = data.node.data.path;
    }
});

function XmlHttpReq(path, callback, obj) {
    obj.url      = path;
    obj.dataType = "json";
    obj.contentType= "application/json;charset=IBM437",
    obj.scriptCharset= "IBM437",
    obj.success  = function(data, textStatus, jqXHR) {
        if (callback) {
            callback(data);
        }
    };
    obj.error    = function(jqXHR, textStatus, errorThrown) {
        console.log("AJAX error");
        /*debugger; */
    };
    jQuery.ajax(obj);
} /* XmlHttpReq */

function getData(path, callback) {
    XmlHttpReq(path, callback, {
        method   : "GET"
    });
} /* getData */

function deleteData(path, callback, _data) {
    XmlHttpReq(path, callback, {
        method   : "DELETE",
        data: _data
    });
} /* deleteData */

function patchData(path, callback, _data) {
    XmlHttpReq(path, callback, {
        method   : "PATCH",
        data: _data
    });
} /* patchData */

function postData(path, callback, _data) {
    XmlHttpReq(path, callback, {
        method   : "POST",
        data: _data
    });
} /* postData */

function putData(path, callback, _data) {
    XmlHttpReq(path, callback, {
        method   : "PUT",
        data: _data
    });
} /* putData */

/* File Upload */
$('#explorerUploadForm').submit(function(e){
    e.preventDefault();
    console.log("Upload!");
    var data = new FormData(this);
    var ref = $('#explorerTree').jstree(true),
    sel = ref.get_selected(),
    path = "/";
    if(!sel.length) {
        alert("Please select the upload location!");return false;
    }
    if(!document.getElementById('uploaded_file').files.length > 0) {
        alert("Please select files to upload!");return false;
    }
    sel = sel[0];
    selectedNode = ref.get_node(sel);
    if(selectedNode.data.directory){
        path = selectedNode.data.path
    }
    else {
        /* remap sel to parent folder */
        sel = ref.get_node(ref.get_parent(sel));
        path = parentNode.data.path;
        console.log("Parent path: " + path);
    }
    const startTime = new Date().getTime();
    let bytesTotal = 0;
    $.ajax({
        url: '/SD_GetFolder?' + path,
        type: 'POST',
        data: data,
        contentType: false,
        processData:false,
        xhr: function() {
            var xhr = new window.XMLHttpRequest();
            xhr.upload.addEventListener("progress", function(evt) {
                if (evt.lengthComputable) {
                    const now = new Date().getTime();
                    const percent = parseInt(evt.loaded * 100 / evt.total);
                    const elapsed = (now - startTime) / 1000;
                    bytesTotal = evt.total;
                    let progressText = "Remaining time is being calculated..";
                    if(elapsed){
                        const bps = evt.loaded / elapsed;
                        const kbps = bps / 1024;
                        const remaining = Math.round((evt.total - evt.loaded) / bps);
                        const data = {
                            percent: percent,
                            remaining: {
                                unit: (remaining>60) ? ("minutes", {count: Math.round(remaining/60)}) : "seconds",
                                value: (remaining>60) ? Math.round(remaining/60) : ((remaining > 2) ? remaining : few)
                            },
                            speed: kbps.toFixed(2)
                        }
                        progressText = "{{percent}}% ({{speed}} KB/s), {{remaining.value}} {{remaining.unit}} remaining..";
                        console.log("Percent: " + percent + "%% " + kbps.toFixed(2) + " KB/s");
                    }
                    $("#explorerUploadProgress").css('width', percent+"%").text(progressText);
                }
            }, false);
            return xhr;
        },
        success: function(data, textStatus, jqXHR) {
            const now = new Date().getTime();
            const elapsed = (now - startTime) / 1000;
            let transData = {
                elapsed: "00:00",
                speed: "0,00"
            };
            if(elapsed) {
                const bps = bytesTotal / elapsed;
                const kbps = bps / 1024;
                const date = new Date(null);
                date.setSeconds(elapsed);
                const timeText = date.toISOString().substr(11, 8);
                transData = { elapsed: timeText, speed: kbps.toFixed(2) };
            }
            console.log("Upload success (" + transData.elapsed + ", " + transData.speed + " KB/s): " + textStatus);
            const progressText = "Upload successfull ({{elapsed}}, {{speed}} KB/s)"
            $("#explorerUploadProgress").text(progressText);
            document.getElementById('uploaded_file').value = '';
            document.getElementById('uploaded_file_text').innerHTML = '';
            getData("/SD_upload?path=" + path, function(data) {
                /* We now have data! */
                deleteChildrenNodes(sel);
                addFileDirectory(sel, data);
                ref.open_node(sel);
            });
        },
        error: function(request, status, error) {
            console.log("Upload ERROR!");
            $("#explorerUploadProgress").text("Upload error: " + status);
            toastr.error("Upload error: " + status);
        }
    });
});

/* File Delete */
function handleDeleteData(nodeId) {
    var ref = $('#explorerTree').jstree(true);
    var node = ref.get_node(nodeId);
    console.log("call delete request: " + node.data.path);
    deleteData("/explorer?path=" + encodeURIComponent(node.data.path));
}

function fileNameSort( a, b ) {
    if ( a.dir && !b.dir ) {
        return -1
    }
    if ( !a.dir && b.dir ) {
        return 1
    }
    if ( a.name < b.name ){
        return -1;
    }
    if ( a.name > b.name ){
        return 1;
    }
    return 0;
}

function createChild(nodeId, data) {
    var ref = $('#explorerTree').jstree(true);
    var node = ref.get_node(nodeId);
    var parentNodePath = node.data.path;
    /* In case of root node remove leading '/' to avoid '//' */
    if(parentNodePath == "/"){
        parentNodePath = "";
    }
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

function deleteChildrenNodes(nodeId) {
    var ref = $('#explorerTree').jstree(true);
    var children = $("#explorerTree").jstree("get_children_dom",nodeId);
    for(var i=0;i<children.length;i++) {
        ref.delete_node(children[i].id);
    }
}

function refreshNode(nodeId) {
    var ref = $('#explorerTree').jstree(true);
    var node = ref.get_node(nodeId);
    getData("/SD_GetFolder?" + encodeURIComponent(node.data.path), function(data) {
        /* We now have data! */
        deleteChildrenNodes(nodeId);
        addFileDirectory(nodeId, data);
        ref.open_node(nodeId);
    });
}

function getType(data) {
    var type = "";
    if(data.dir) {
        type = "folder";
    }
    else if ((/\.(mp3|MP3|ogg|wav|WAV|OGG|wma|WMA|acc|ACC|m4a|M4A|flac|FLAC)$/i).test(data.name)) {
        type = "audio";
    } else if ((/\.(png|PNG|jpg|JPG|jpeg|JPEG|bmp|BMP|gif|GIF)$/i).test(data.name)) {
        type = "image";
    }
    else {
        type = "file";
    }
    return type;
}

function addFileDirectory(parent, data) {
    data.sort( fileNameSort );
    var ref = $('#explorerTree').jstree(true);
    for (var i=0; i<data.length; i++) {
        console.log("Create Node", data[i]);
        ref.create_node(parent, createChild(parent, data[i]));
    }
} /* addFileDirectory */

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
            'default': { // file_yellow.png
              'icon': iconFileYellow
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
                                    getData("/SD_newFolder?" + encodeURIComponent(node.data.path) + "/" + encodeURIComponent(childNode.text));
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
                        getData("/SD_playFile?" + encodeURIComponent(node.data.path) + "&playmode=" + playMode);
                    }
                };
                /* Refresh */
                items.refresh = {
                    label: "Refresh",
                    action: function (x) {
                        refreshNode(nodeId);
                    }
                };
                /* Delete */
                items.delete = {
                    label: "Delete",
                    action: function (x) {
                        handleDeleteData(nodeId);
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
                            getData("/SD_rename?src=" + encodeURIComponent(srcPath) + "&dst=" + encodeURIComponent(node.data.path));
                            refreshNode(ref.get_parent(nodeId));
                        });
                    }
                };
                /* Download */
                if (!node.data.directory) {
                    items.download = {
                        label: "Download",
                        action: function (x) {
                            uri = "/SD_download?=" + encodeURIComponent(node.data.path);
                            console.log("download file: " + node.data.path);
                            var anchor = document.createElement('a');
                            anchor.href = uri;
                            anchor.target = '_blank';
                            anchor.download = node.data.path;
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
    console.log("We can now have data!")
    getData("/SD_GetFolder?/" + '&version=' + Math.random(), function(data) {
        /* We now have data! */
        console.log("We have data now!")
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