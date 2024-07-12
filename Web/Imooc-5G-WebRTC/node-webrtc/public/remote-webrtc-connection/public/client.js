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
    videoLocal: Element,
    videoRemote: Element,
    btnConnect: Element,
    btnLeave: Element,
    taOffer: Element,
    taAnswer: Element,
    divInformation: Element,
    cbShareDesk: Element,
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

const Context = {
    socket: undefined,
    roomId: String,
    connectionState: Number,
}

Context.updateConnectionState = function (state) {
    Context.logDebug("New Connection state: " + ConnectionState.name(state));
    Context.connectionState = state;
    if (state === ConnectionState.Initialized) {
        Elements.btnConnect.disabled = false;
        Elements.btnLeave.disabled = true;
    } else {
        Elements.btnConnect.disabled = true;
        Elements.btnLeave.disabled = false;
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
    Elements.btnConnect.onclick = initLocalMediaStream
    Elements.btnLeave.onclick = disconnectAndLeave;

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

function isDeskSharable() {


    if (isDesktopDevice()) {
        navigator.mediaDevices.getDisplayMedia({video: true})
            .then((stream) => {
                MediaHolder.localStream = stream;
                Context.logDebug("getDisplayMedia: get desktop stream success.");
            })
            .catch((error) => {
                Context.logError("getDisplayMedia: " + error);
            });
        return true;
    }
    return false;
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

        if (data.hasOwnProperty('type') && data.type === 'offer') {
            Elements.taOffer.value = data.sdp;
            // peer sets remote description
            MediaHolder.rtcPeerConnection.setRemoteDescription(new RTCSessionDescription(data))
                .catch((error) => {
                    Context.logError('Peer setting remote description: ' + error);
                });

            // peer creates answer
            MediaHolder.rtcPeerConnection.createAnswer()
                .then((answer) => {
                    MediaHolder.rtcPeerConnection.setLocalDescription(answer)
                        .then(() => {
                            Elements.taAnswer.value = answer.sdp;
                            Context.sendMessage(answer);
                        })
                        .catch((error) => {
                            Context.logError('Peer setting local description: ' + error)
                        })
                })
                .catch((error) => {
                    Context.logError('Error creating answer', error);
                });
            return;
        }

        if (data.hasOwnProperty('type') && data.type === 'answer') {
            // self sets remote description
            Elements.taAnswer.value = data.sdp;
            // answer 对象的格式默认就是：{type: "answer", sdp: "xxxxx"}
            MediaHolder.rtcPeerConnection.setRemoteDescription(new RTCSessionDescription(data)).catch((error) => {
                Context.logError('Self setting remote description: ' + error);
            });
            return;
        }

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