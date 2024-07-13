'use strict'

window.onload = init;

// ============================================================
// Tools
// ============================================================

/**
 * @returns {boolean} 如果返回 true 则说明是桌面设备，否则是移动设备。
 */
function isDesktopDevice() {
    const userAgentInfo = navigator.userAgent;
    const Agents = ["Android", "iPhone", "SymbianOS", "Windows Phone", "iPad", "iPod"];
    let flag = true;

    for (let v = 0; v < Agents.length; v++) {
        if (userAgentInfo.indexOf(Agents[v]) > 0) {
            flag = false;
            break;
        }
    }

    return flag;
}

/**
 * @returns {boolean} 如果返回 true 则说明是 Android  设备，否则是 iOS 设备。
 */
function isAndroid() {
    const u = navigator.userAgent, app = navigator.appVersion;
    const isAndroid = u.indexOf('Android') > -1 || u.indexOf('Linux') > -1; //g
    const isIOS = !!u.match(/\(i[^;]+;( U;)? CPU.+Mac OS X/); //ios终端
    if (isAndroid) {
        //这个是安卓操作系统
        return true;
    }
    if (isIOS) {
        //这个是ios操作系统
        return false;
    }
}

/**
 * @param variable {string} 查询参数的 key。
 * @returns {boolean|string} 返回查询参数的值，如果没有则返回 false。
 */
function getQueryVariable(variable) {
    const query = window.location.search.substring(1);
    const queries = query.split("&");
    for (let i = 0; i < queries.length; i++) {
        const pair = queries[i].split("=");
        if (pair[0] === variable) {
            return pair[1];
        }
    }
    return false;
}

/**
 * @returns 如果返回 true 则说明当前浏览器支持 WebRTC，否则不支持。
 */
function isWebRTCSupported() {
    return navigator.mediaDevices && navigator.mediaDevices.enumerateDevices;
}

// ============================================================
// Variables
// ============================================================

const Elements = {
    videoLocal: undefined,
    videoRemote: undefined,

    taOffer: undefined,
    taAnswer: undefined,

    btnConnect: undefined,
    btnLeave: undefined,
    divInformation: undefined,
    cbShareDesk: undefined,
    selectBandwidth: undefined,

    taChatPanel: undefined,
    taChatInput: undefined,
    btnSendText: undefined,

    divFileBitrate: undefined,
    inputFile: undefined,
    spanFileStatus: undefined,
    aDownloadFile: undefined,
    progressSending: undefined,
    progressReceiving: undefined,
    btnSendFile: undefined,
    btnAbortFileSending: undefined,
}

Elements.appendInformation = function (color, information) {
    if (!Elements.divInformation) {
        return;
    }
    Elements.divInformation.scrollTop = Elements.divInformation.scrollHeight;
    const div = document.createElement('p');
    div.style.color = color;
    div.style.fontSize = '12px';
    div.innerHTML = information;
    Elements.divInformation.appendChild(div);
    Elements.divInformation.appendChild(document.createElement('br'));
}

const MediaHolder = {
    peerConnectionConfig: {},
    localStream: undefined,
    remoteStream: undefined,
    rtcPeerConnection: undefined,
    chatDataChannel: undefined,
    fileDataChannel: undefined,
}

const FileHolder = {
    // sending
    fileReader: undefined,

    // receiving
    receiveBuffer: undefined,
    receivedSize: 0,
    fileName: "",
    fileSize: 0,
    lastModifyTime: 0,
    fileType: ""
}

const Statistics = {
    bitrateGraph: undefined,
    bitrateSeries: undefined,
    packetGraph: undefined,
    packetSeries: undefined,
    lastResult: undefined,
}

Statistics.init = function () {
    Statistics.bitrateSeries = new TimelineDataSeries();
    Statistics.bitrateGraph = new TimelineGraphView('bitrateGraph', 'bitrateCanvas');
    Statistics.bitrateGraph.updateEndDate();

    Statistics.packetSeries = new TimelineDataSeries();
    Statistics.packetGraph = new TimelineGraphView('packetGraph', 'packetCanvas');
    Statistics.packetGraph.updateEndDate();

    setInterval(Statistics.updateStats, 1000);
}

Statistics.updateStats = function () {
    if (Context.connectionState === ConnectionState.Initialized) {
        Statistics.lastResult = undefined;
        return;
    }

    if (!MediaHolder.rtcPeerConnection) {
        Statistics.lastResult = undefined;
        return;
    }

    const sender = MediaHolder.rtcPeerConnection.getSenders()[0];
    if (!sender) {
        Statistics.lastResult = undefined;
        return;
    }

    sender.getStats().then(rtcStatsReport => {

        rtcStatsReport.forEach(report => {
            let bytes;
            let packets;
            if (report.type === 'outbound-rtp') {
                const now = report.timestamp;
                bytes = report.bytesSent;
                packets = report.packetsSent;

                if (Statistics.lastResult && Statistics.lastResult.has(report.id)) {
                    // calculate bitrate, unit: bps
                    const bitrate = 8 * (bytes - Statistics.lastResult.get(report.id).bytesSent) / (now - Statistics.lastResult.get(report.id).timestamp);
                    // append to chart
                    Statistics.bitrateSeries.addPoint(now, bitrate);
                    Statistics.bitrateGraph.setDataSeries([Statistics.bitrateSeries]);
                    Statistics.bitrateGraph.updateEndDate();
                    // calculate number of packets and append to chart
                    Statistics.packetSeries.addPoint(now, packets - Statistics.lastResult.get(report.id).packetsSent);
                    Statistics.packetGraph.setDataSeries([Statistics.packetSeries]);
                    Statistics.packetGraph.updateEndDate();
                }
            }
        });

        Statistics.lastResult = rtcStatsReport;
    });
}

const Context = {
    socket: undefined,
    roomId: undefined,
    connectionState: 0,
}

Context.updateConnectionState = function (state) {
    Context.logDebug("New Connection state: " + ConnectionState.name(state));
    Context.connectionState = state;
    if (state === ConnectionState.Initialized) {
        Elements.btnConnect.disabled = false;
        Elements.btnLeave.disabled = true;
        Elements.selectBandwidth.disabled = true;

        Elements.taChatInput.disabled = true;
        Elements.btnSendText.disabled = true;
        Elements.inputFile.disabled = true;
        Elements.btnSendFile.disabled = true;
        Elements.btnAbortFileSending.disabled = true;
    } else {
        Elements.btnConnect.disabled = true;
        Elements.btnLeave.disabled = false;
        Elements.selectBandwidth.disabled = false;
    }
}

Context.logDebug = function (message) {
    console.log(message);
    Elements.appendInformation("blue", message);
}

Context.logError = function (message) {
    console.error(message);
    Elements.appendInformation("red", message);
}

Context.sendMessage = function (data) {
    Context.logDebug(`Send message to room: ${Context.roomId}, data: ${JSON.stringify(data)}`);
    if (!Context.socket) {
        Context.logError('socket is null');
    }
    Context.socket.emit('message', Context.roomId, data);
}

Context.sendCommand = function (command) {
    Context.logDebug(`Send command to room: ${Context.roomId}, command: ${command}`);
    if (!Context.socket) {
        Context.logError('socket is null');
    }
    Context.socket.emit(command, Context.roomId);
}

Context.closeSocket = function () {
    if (!Context.socket) {
        return;
    }
    Context.socket.disconnect();
    Context.socket = null;
}

Context.initSocket = function () {
    if (Context.socket) {
        return;
    }
    Context.socket = io();
}

const ConnectionState = {
    Initialized: 0,
    Joined: 1,
    PeerLeft: 2,
    PeerJoined: 3,
}

ConnectionState.name = function (state) {
    switch (state) {
        case ConnectionState.Initialized:
            return "Initialized";
        case ConnectionState.Joined:
            return "Joined";
        case ConnectionState.PeerLeft:
            return "PeerLeft";
        case ConnectionState.PeerJoined:
            return "PeerJoined";
        default:
            return "Unknown";
    }
}

const Command = {
    JOIN: "join",
    ON_JOINED: "on_joined",
    ON_FULL: "on_full",
    ON_PEER_JOINED: "on_peer_joined",

    LEAVE: "leave",
    ON_LEAVE: "on_leave",
    ON_PEER_LEAVE: "on_peer_leave",
}

// ============================================================
// Initialization
// ============================================================
function init() {
    // init elements
    Elements.videoLocal = document.querySelector('video#localVideo');
    Elements.videoRemote = document.querySelector('video#remoteVideo');
    Elements.btnConnect = document.querySelector('button#connect');
    Elements.btnLeave = document.querySelector('button#leave');
    Elements.taOffer = document.querySelector('textarea#offer');
    Elements.taAnswer = document.querySelector('textarea#answer');
    Elements.divInformation = document.querySelector('div#stateInformation');
    Elements.cbShareDesk = document.querySelector('input#shareDesk');
    Elements.selectBandwidth = document.querySelector('select#bandwidth');
    Elements.taChatPanel = document.querySelector('textarea#chatPanel');
    Elements.taChatInput = document.querySelector('textarea#chatInput');
    Elements.btnSendText = document.querySelector('button#sendText');

    Elements.divFileBitrate = document.querySelector("div#fileBitrate")
    Elements.inputFile = document.querySelector("input#fileInput")
    Elements.spanFileStatus = document.querySelector("span#fileStatus")
    Elements.aDownloadFile = document.querySelector("a#downloadFile")
    Elements.progressSending = document.querySelector("progress#sendProgress");
    Elements.progressReceiving = document.querySelector("progress#receiveProgress")
    Elements.btnSendFile = document.querySelector("button#sendFile")
    Elements.btnAbortFileSending = document.querySelector("button#abortButton")

    Elements.btnConnect.onclick = initLocalMediaStream
    Elements.btnLeave.onclick = disconnectAndLeave;
    Elements.selectBandwidth.onchange = switchBandwidth;
    Elements.btnSendText.onclick = sendChatMessageByWebrtcChannel;
    Elements.btnSendFile.onclick = sendFileByWebrtcChannel;
    Elements.btnAbortFileSending.onclick = abortFileSending;
    Elements.inputFile.onchange = handleFileInputChange;

    if (!isWebRTCSupported()) {
        Context.logError("WebRTC is not supported in your browser.");
        alert("WebRTC is not supported in your browser.")
        return;
    }

    // init state
    Context.updateConnectionState(ConnectionState.Initialized);
    Context.roomId = getQueryVariable("room");
    Context.logDebug("roomId: " + Context.roomId)
}

function initLocalMediaStream() {
    if (!isWebRTCSupported()) {
        alert("WebRTC is not supported in your browser.")
        return
    }
    if (!Context.roomId) {
        alert("Please input the room id.");
        return;
    }

    Context.initSocket();

    assembleLocalMediaStream()
        .then((stream) => {
            if (stream) {
                Context.logDebug("assembleLocalMediaStream success. stream: " + JSON.stringify(stream));
                MediaHolder.localStream = stream;
                Elements.videoLocal.srcObject = MediaHolder.localStream;
                // 一定要放到 getMediaStream 之后再调用，否则就会出现绑定失败的情况。
                startConnection();
            } else {
                Context.logError("assembleLocalMediaStream: stream is null.")
            }
        })
        .catch((error) => {
            Context.logError("assembleLocalMediaStream: " + error)
        })
}

async function assembleLocalMediaStream() {
    const constraints = {
        video: true,
        audio: {
            echoCancellation: true,
            noiseSuppression: true,
            autoGainControl: true
        }
    }

    // If the user ask to share the desktop and the device is desktop, then share the desktop. When sharing the desktop, the video should be disabled.
    if (Elements.cbShareDesk.checked && isDesktopDevice()) {
        try {
            constraints.video = false;
            const desktopStream = await navigator.mediaDevices.getDisplayMedia({video: true})
            const mediaStream = await navigator.mediaDevices.getUserMedia(constraints)
            // 将用户媒体流中的 track 移动到桌面流中。
            mediaStream.getTracks().forEach(track => {
                desktopStream.addTrack(track);
                mediaStream.removeTrack(track);
            });
            return desktopStream;
        } catch (error) {
            Context.logError("getDisplayMedia + getMediaStream: " + error);
            throw error;
        }
    }

    try {
        return await navigator.mediaDevices.getUserMedia(constraints);
    } catch (error) {
        Context.logError("getUserMedia: " + error);
        throw error;
    }
}

// ============================================================
// Connection
// ============================================================

function startConnection() {
    Context.logDebug("Start connection.");
    setUpMessageHandlers();
    Context.sendCommand(Command.JOIN);

    Statistics.init();
}

function setUpMessageHandlers() {
    // Self joined
    Context.socket.on(Command.ON_JOINED, (roomId, userId) => {
        Context.logDebug("Joined room: " + roomId + " as user: " + userId);
        Context.updateConnectionState(ConnectionState.Joined);
        /*
        如果是多人的话，第一个人不该在这里创建 peerConnection，而是等到收到一个 other_join 时再创建，并且在这个消息里应该带当前房间的用户数。
        这里的逻辑是：如果是第一个人（这个人其实就是自己），那么就创建 peerConnection，然后等待其他人加入。
         */
        initLocalPeerConnection();
    })

    // Peer joined
    Context.socket.on(Command.ON_PEER_JOINED, (roomId, userId) => {
        Context.logDebug("User: " + userId + " joined the room: " + roomId);
        // 如果是多人的话，每上来一个人都要创建一个新的 peerConnection。
        if (Context.connectionState === ConnectionState.PeerLeft) {
            initLocalPeerConnection();
        }
        Context.updateConnectionState(ConnectionState.PeerJoined);
        // Self init data channel and peer will receive this data channel.
        initWebrtcChatChannel();
        initWebrtcFileChannel();
        // Send media offer
        sendMediaOffer();
    })

    // Room is full
    Context.socket.on(Command.ON_FULL, (roomId, userId) => {
        alert('The room is full!');
        Context.logDebug("Room is full: " + roomId);
        Context.updateConnectionState(ConnectionState.Initialized);
        closeLocalPeerConnection();
        closeLocalMediaStream();
    })

    // Self left
    Context.socket.on(Command.ON_LEAVE, (roomId, userId) => {
        Context.logDebug("Left the room: " + roomId + " as user: " + userId);
        Context.updateConnectionState(ConnectionState.Initialized);
        closeLocalPeerConnection();
        closeLocalMediaStream();
        Context.closeSocket();
    })

    // Disconnect
    Context.socket.on('disconnect', () => {
        Context.logDebug("Disconnected.");
        // The logic is the same as the user left the room. It is necessary to close the connection and release the resources.
        // For example, if the user closes the browser or the connection closes accidentally.
        Context.updateConnectionState(ConnectionState.Initialized);
        closeLocalPeerConnection();
        closeLocalMediaStream();
    })

    // Peer left
    Context.socket.on(Command.ON_PEER_LEAVE, (roomId, userId) => {
        Context.logDebug("User: " + userId + " left the room: " + roomId);
        Context.updateConnectionState(ConnectionState.PeerLeft);
        closeLocalPeerConnection();
        Elements.taAnswer.value = "";
        Elements.taOffer.value = "";
    })

    // Message
    Context.socket.on('message', (roomId, data) => {
        Context.logDebug('Receive message: ' + (data ? JSON.stringify(data) : 'null'));
        if (data === null || data === undefined) {
            Context.logDebug('the message is invalid!');
            return;
        }

        // handle webrtc offer
        if (data.hasOwnProperty('type') && data.type === 'offer') {
            Elements.taOffer.value = data.sdp;
            // peer sets remote description
            MediaHolder.rtcPeerConnection.setRemoteDescription(new RTCSessionDescription(data))
                .then(() => {
                    Context.logDebug('Peer setting remote description success.');
                    return MediaHolder.rtcPeerConnection.createAnswer()
                })
                .then((answer) => {
                    Context.logDebug('Peer creating answer success.');
                    return new Promise((resolve, reject) => {
                        MediaHolder.rtcPeerConnection.setLocalDescription(answer)
                            .then(() => {
                                resolve(answer);
                            })
                            .catch((error) => {
                                reject(error);
                            })
                    })
                })
                .then((answer) => {
                    Elements.taAnswer.value = answer.sdp;
                    Context.sendMessage(answer);
                    Elements.selectBandwidth.disabled = false;
                })
                .catch((error) => {
                    Context.logError('Peer handle offer failed: ' + error);
                });
            return;
        }

        // handle webrtc answer
        if (data.hasOwnProperty('type') && data.type === 'answer') {
            // self sets remote description
            Elements.taAnswer.value = data.sdp;
            // answer 对象的格式默认就是：{type: "answer", sdp: "xxxxx"}
            MediaHolder.rtcPeerConnection.setRemoteDescription(new RTCSessionDescription(data))
                .then(() => {
                    Elements.selectBandwidth.disabled = false;
                    Context.logDebug('Self setting remote description success.');
                })
                .catch((error) => {
                    Context.logError('Self setting remote description: ' + error);
                });
            return;
        }

        // handle webrtc ice candidate
        if (data.hasOwnProperty('type') && data.type === 'candidate') {
            // self and peer add ice candidate
            const candidate = new RTCIceCandidate({
                sdpMLineIndex: data.label,
                candidate: data.candidate
            });
            MediaHolder.rtcPeerConnection.addIceCandidate(candidate).catch((error) => {
                Context.logError('Error adding received ice candidate', error);
            });
            return;
        }

        // handle webrtc file info
        if (data.hasOwnProperty('type') && data.type === 'fileInfo') {
            FileHolder.fileName = data.name;
            FileHolder.fileType = data.filetype;
            FileHolder.fileSize = data.size;
            FileHolder.lastModifyTime = data.lastModify;
            Elements.progressReceiving.max = FileHolder.fileSize;
        }

        Context.logError('The message is invalid!');
    });
}

function disconnectAndLeave() {
    Context.sendCommand(Command.LEAVE);
    Context.updateConnectionState(ConnectionState.Initialized);
    closeLocalPeerConnection();
    closeLocalMediaStream();
    Elements.taOffer.value = "";
    Elements.taAnswer.value = "";
}

/**
 * Open `chrome://webrtc-internals/` in Chrome to see the WebRTC debug information.
 * When you changed the bandwidth, you can see the change in the `Bandwidth` column.
 */
function switchBandwidth() {
    Elements.selectBandwidth.disabled = true;
    const targetBandwidth = Elements.selectBandwidth.options[Elements.selectBandwidth.selectedIndex].value
    const vSender = MediaHolder.rtcPeerConnection.getSenders().find(sender => sender.track.kind === 'video');
    const parameters = vSender.getParameters();
    if (!parameters.encodings) {
        parameters.encodings = [{}];
    }
    if (targetBandwidth === 'unlimited') {
        delete parameters.encodings[0].maxBitrate;
    } else {
        parameters.encodings[0].maxBitrate = targetBandwidth * 1000;
    }
    vSender.setParameters(parameters)
        .then(() => {
            Elements.selectBandwidth.disabled = false;
            Context.logDebug('Bandwidth is set to: ' + targetBandwidth + ' kbps');
        })
        .catch((error) => {
            Elements.selectBandwidth.disabled = false;
            Context.logError('Error setting bandwidth: ' + error);
        });
}

// ============================================================
// WebRTC API
// ============================================================
function initLocalPeerConnection() {
    // 如果是多人的话，在这里要创建一个新的连接，新创建好的要放到一个 map 表中：key=userid, value=peerConnection。
    if (MediaHolder.rtcPeerConnection) {
        Context.logError("rtcPeerConnection has already been created.")
        return
    }
    MediaHolder.rtcPeerConnection = new RTCPeerConnection(MediaHolder.peerConnectionConfig);

    // 当收集到了链路的时候，会触发这个事件。链路可以是 host, srflx, relay 这三种类型。
    MediaHolder.rtcPeerConnection.onicecandidate = (event) => {
        if (event.candidate) {
            Context.sendMessage({
                type: "candidate",
                label: event.candidate.sdpMLineIndex,
                id: event.candidate.sdpMid,
                candidate: event.candidate.candidate
            })
            Context.logDebug('Local ICE candidate: ' + event.candidate.candidate)
        } else {
            Context.logDebug('End of candidates.')
        }
    }

    // Peer will receive the data channel by this event.
    MediaHolder.rtcPeerConnection.ondatachannel = (event) => {
        onReceiveDataChannel(event);
    }

    // 当收到远程流的时候，会触发这个事件。
    MediaHolder.rtcPeerConnection.ontrack = (event) => {
        Elements.videoRemote.srcObject = event.streams[0];
    }

    // 绑定本地流
    MediaHolder.localStream.getTracks().forEach((track) => {
        MediaHolder.rtcPeerConnection.addTrack(track, MediaHolder.localStream);
    })
}

function sendMediaOffer() {
    if (Context.connectionState !== ConnectionState.PeerJoined) {
        Context.logError("Connection state is not PeerConnected.")
        return
    }

    const offerOptions = {
        offerToReceiveVideo: true,
        offerToReceiveAudio: true,
    };

    MediaHolder.rtcPeerConnection.createOffer(offerOptions)
        .then((offer) => {
            MediaHolder.rtcPeerConnection.setLocalDescription(offer)
                .then(() => {
                    Context.logDebug("Set local description success.");
                    Elements.taOffer.value = offer.sdp;
                    // offer 对象的格式默认就是：{type: "offer", sdp: "xxxxx"}
                    Context.sendMessage(offer);
                })
                .catch((error) => {
                    Context.logDebug("Set local description error: " + error);
                });
        })
        .catch((error) => {
            Context.logDebug("Create offer: " + error);
        })
}

function closeLocalPeerConnection() {
    if (!MediaHolder.rtcPeerConnection) {
        Context.logDebug("closeLocalPeerConnection: rtcPeerConnection is null.")
        return
    }
    Context.logDebug("closeLocalPeerConnection: rtcPeerConnection is closed.")
    MediaHolder.rtcPeerConnection.close();
    MediaHolder.rtcPeerConnection = null;
    MediaHolder.chatDataChannel = null;
}

function closeLocalMediaStream() {
    if (!MediaHolder.localStream) {
        return
    }
    MediaHolder.localStream.getTracks().forEach((track) => {
        track.stop();
    })
    MediaHolder.localStream = null;
}

function onReceiveDataChannel(event) {
    if (!event || !event.channel) {
        Context.logError('onReceiveDataChannel: event or event.channel is null.');
        return;
    }
    if (event.channel.label === 'chat-channel') {
        initWebrtcChatChannel(event);
    } else if (event.channel.label === 'file-channel') {
        initWebrtcFileChannel(event);
    }
}

function initWebrtcChatChannel(event) {
    Context.logDebug('initWebRtcChatChannel. event: ' + JSON.stringify(event))

    if (!MediaHolder.rtcPeerConnection) {
        Context.logError('rtcPeerConnection is null.');
        return;
    }
    if (MediaHolder.chatDataChannel) {
        Context.logError('chatDataChannel has already been created.');
        return;
    }

    if (event) {
        MediaHolder.chatDataChannel = event.channel;
    } else {
        MediaHolder.chatDataChannel = MediaHolder.rtcPeerConnection.createDataChannel('chat-channel');
    }

    MediaHolder.chatDataChannel.onmessage = (event) => {
        const msg = event.data;
        if (msg) {
            Context.logDebug('received webrtc chat msg: ' + msg);
            Elements.taChatPanel.value += "He/She: " + msg + "\r\n";
        } else {
            Context.logError('received webrtc chat msg is null.');
        }
    }

    MediaHolder.chatDataChannel.onopen = () => {
        Context.logDebug('Chat Data channel is open.');
        Elements.taChatInput.disabled = false;
        Elements.btnSendText.disabled = false;
    }

    MediaHolder.chatDataChannel.onclose = () => {
        Context.logDebug('Chat Data channel is close.');
        Elements.taChatInput.disabled = true;
        Elements.btnSendText.disabled = true;
    }
}

function sendChatMessageByWebrtcChannel() {
    if (!MediaHolder.chatDataChannel) {
        Context.logError('dataChannel is null.');
        return;
    }
    const msg = Elements.taChatInput.value;
    if (!msg) {
        Context.logError('msg is null.');
        return;
    }
    Context.logDebug('send webrtc msg: ' + msg);
    MediaHolder.chatDataChannel.send(msg);
    Elements.taChatPanel.value += "You: " + msg + "\r\n";
    Elements.taChatInput.value = "";
}

function initWebrtcFileChannel(event) {
    Context.logDebug('initWebRtcFileChannel. event: ' + JSON.stringify(event))

    if (!MediaHolder.rtcPeerConnection) {
        Context.logError('rtcPeerConnection is null.');
        return;
    }
    if (MediaHolder.fileDataChannel) {
        Context.logError('fileDataChannel has already been created.');
        return;
    }

    if (event) {
        MediaHolder.fileDataChannel = event.channel;
    } else {
        MediaHolder.fileDataChannel = MediaHolder.rtcPeerConnection.createDataChannel('file-channel');
    }

    MediaHolder.fileDataChannel.onmessage = (event) => {
        const msg = event.data;
        if (msg) {
            receiveFileSlice(msg);
            Context.logDebug('received webrtc file msg: ' + msg);
        } else {
            Context.logError('received webrtc file msg is null.');
        }
    }

    MediaHolder.fileDataChannel.onopen = () => {
        Context.logDebug('File Data channel is open.');
        Elements.inputFile.disabled = false;
        Elements.btnAbortFileSending.disabled = false;
    }

    MediaHolder.fileDataChannel.onclose = () => {
        Context.logDebug('File Data channel is closed.');
        Elements.inputFile.disabled = true;
        Elements.btnSendFile.disabled = true;
        Elements.btnAbortFileSending.disabled = true;
    }
}

function sendFileByWebrtcChannel() {
    Elements.btnSendFile.disabled = true;

    let offset = 0;
    const chunkSize = 16384;
    const file = Elements.inputFile.files[0];
    Context.logDebug(`File is ${[file.name, file.size, file.type, file.lastModified].join(' ')}`);

    // Handle 0 size files.
    Elements.spanFileStatus.textContent = '';
    Elements.aDownloadFile.textContent = '';
    if (file.size === 0) {
        Elements.divFileBitrate.innerHTML = '';
        Elements.spanFileStatus.textContent = 'File is empty, please select a non-empty file';
        return;
    }

    Elements.progressSending.max = file.size;

    FileHolder.fileReader = new FileReader();
    FileHolder.fileReader.onerror = error => Context.logError('Error reading file:' + error);
    FileHolder.fileReader.onabort = event => Context.logError('File reading aborted:', event);
    FileHolder.fileReader.onload = e => {
        Context.logDebug('FileRead.onload: ' + e);
        MediaHolder.fileDataChannel.send(e.target.result);
        offset += e.target.result.byteLength;
        Elements.progressSending.value = offset;
        if (offset < file.size) {
            readSlice(offset);
        }
    }

    const readSlice = o => {
        Context.logDebug('readSlice: ' + o);
        const slice = file.slice(offset, o + chunkSize);
        FileHolder.fileReader.readAsArrayBuffer(slice);
    };

    readSlice(0);
}

function receiveFileSlice(data) {
    if (!FileHolder.receiveBuffer) {
        FileHolder.receiveBuffer = [];
        FileHolder.receivedSize = 0;
    }

    Context.logDebug(`Received Message ${data.byteLength}`);

    FileHolder.receiveBuffer.push(data);
    FileHolder.receivedSize += data.byteLength;
    Elements.progressReceiving.value = FileHolder.receivedSize;
    if (FileHolder.receivedSize === FileHolder.fileSize) {
        const received = new Blob(FileHolder.receiveBuffer);
        MediaHolder.receiveBuffer = [];
        MediaHolder.receivedSize = 0;
        Elements.aDownloadFile.href = URL.createObjectURL(received);
        Elements.aDownloadFile.download = FileHolder.fileName;
        Elements.aDownloadFile.textContent = `Click to download '${FileHolder.fileName}' (${FileHolder.fileSize} bytes)`;
        Elements.aDownloadFile.style.display = 'block';
    }
}

function abortFileSending() {
    if (FileHolder.fileReader && FileHolder.fileReader.readyState === 1) {
        Context.logError('abort read');
        FileHolder.fileReader.abort();
    }
}

function handleFileInputChange() {
    const file = Elements.inputFile.files[0];
    if (!file) {
        Context.logError('No file chosen');
    } else {
        // before sending the file, send the file info to the peer.
        Context.sendMessage({
            type: 'fileInfo',
            name: file.name,
            size: file.size,
            filetype: file.type,
            lastModify: file.lastModified
        });

        Elements.btnSendFile.disabled = false;
        Elements.progressSending.value = 0;
    }
}
