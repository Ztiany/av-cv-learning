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
    taInformation: Element,
    cbShareDesk: Element,
}

Elements.appendInformation = function (information) {
    if (!Elements.taInformation) {
        return;
    }
    Elements.taInformation.scrollTop = Elements.taInformation.scrollHeight;
    Elements.taInformation.value = Elements.taInformation.value + information + '\r';
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
    Joined_Unbound: 2,
    PeerJoined: 3,
}

const Command = {
    JOIN: "join",
    ON_JOINED: "on_joined",
    ON_FULL: "on_full",
    ON_PEER_JOINED: "on_peer_joined",

    LEAVE: "leave",
    ON_LEAVE: "on_leave",
    ON_PEER_LEAVE: "on_peer_leave",

    OFFER: "offer",
    ANSWER: "answer",
    CANDIDATE: "candidate",
}

const Context = {
    socket: undefined,
    roomId: String,
    connectionState: Number,
}

Context.updateConnectionState = function (state) {
    Context.log("New Connection state: " + state);
    Context.connectionState = state;
    if (state === ConnectionState.Initialized) {
        Elements.btnConnect.disabled = false;
        Elements.btnLeave.disabled = true;
    } else {
        Elements.btnConnect.disabled = true;
        Elements.btnLeave.disabled = false;
    }
}

Context.log = function (message) {
    console.log(message);
    Elements.appendInformation(message);
}

Context.sendMessage = function (data) {
    Context.log(`Send message to room: ${Context.roomId}, data: ${JSON.stringify(data)}`);
    if (!Context.socket) {
        console.error('socket is null');
    }
    Context.socket.emit('message', Context.roomId, data);
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
    Elements.taInformation = document.querySelector('textarea#stateInformation');
    Elements.cbShareDesk = document.querySelector('input#shareDesk');
    Elements.btnConnect.onclick = initLocalMediaStream
    Elements.btnLeave.onclick = disconnectAndLeave;

    if (!isWebRTCSupported()) {
        console.error("WebRTC is not supported in your browser.");
        alert("WebRTC is not supported in your browser.")
        return;
    }

    // init network
    Context.socket = io();
    Context.updateConnectionState(ConnectionState.Initialized);
    Context.roomId = getQueryVariable("room");
    Context.log("roomId: " + Context.roomId)
}

function initLocalMediaStream() {
    if (!Context.roomId) {
        alert("Please input the room id.")
        return
    }

    let constraints;
    if (Elements.cbShareDesk.checked && shareDesk()) {
        constraints = {
            video: false,
            audio: {
                echoCancellation: true,
                noiseSuppression: true,
                autoGainControl: true
            }
        }
    } else {
        constraints = {
            video: true,
            audio: {
                echoCancellation: true,
                noiseSuppression: true,
                autoGainControl: true
            }
        }
    }

    navigator.mediaDevices.getUserMedia(constraints)
        .then((stream) => {
            if (MediaHolder.localStream) {
                stream.getTracks().forEach((track) => {
                    // 将原来的流中的 track 移除，然后添加到新的流中。
                    MediaHolder.localStream.addTrack(track);
                    stream.removeTrack(track);
                })
            } else {
                MediaHolder.localStream = stream;
            }
            Elements.videoLocal.srcObject = MediaHolder.localStream;
            // 一定要放到 getMediaStream 之后再调用，否则就会出现绑定失败的情况。
            startConnection();
        }).catch((error) => {
        console.error("getUserMedia: " + error);
    })
}

function shareDesk() {
    if (isDesktopDevice()) {
        navigator.mediaDevices.getDisplayMedia({video: true})
            .then((stream) => {
                MediaHolder.localStream = stream;
            })
            .catch((error) => {
                console.error("getDisplayMedia: " + error);
            });
        return true;
    }
    return false;
}

// ============================================================
// Connection
// ============================================================

function startConnection() {
    Elements.appendInformation("Start connection.");
    setUpMessageHandlers();
    Context.socket.emit(Command.JOIN, Context.roomId);
}

function setUpMessageHandlers() {
    // Joined
    Context.socket.on(Command.ON_JOINED, (roomId, userId) => {
        Context.log("Joined room: " + roomId + " as user: " + userId);
        Context.updateConnectionState(ConnectionState.Joined);
        /*
        如果是多人的话，第一个人不该在这里创建 peerConnection，而是等到收到一个 other_join 时再创建，并且在这个消息里应该带当前房间的用户数。
        这里的逻辑是：如果是第一个人（这个人其实就是自己），那么就创建 peerConnection，然后等待其他人加入。
         */
        initLocalPeerConnection();
    })

    // PeerJoined
    Context.socket.on(Command.ON_PEER_JOINED, (roomId, userId) => {
        Context.log("User: " + userId + " joined the room: " + roomId);
        // 如果是多人的话，每上来一个人都要创建一个新的 peerConnection。
        if (Context.connectionState === ConnectionState.Joined_Unbound) {
            initLocalPeerConnection();
        }
        Context.updateConnectionState(ConnectionState.PeerJoined);
        sendMediaOffer();
    })

    // Room is full
    Context.socket.on(Command.ON_FULL, (roomId, userId) => {
        alert('The room is full!');
        Context.log("Room is full: " + roomId + " as user: " + userId);
        Context.updateConnectionState(ConnectionState.Initialized);
        closeLocalPeerConnection();
        closeLocalMediaStream();
    })

    // Self Leave
    Context.socket.on(Command.ON_LEAVE, (roomId, userId) => {
        Context.log("Left the room: " + roomId + " as user: " + userId);
        Context.updateConnectionState(ConnectionState.Initialized);
        closeLocalPeerConnection();
        closeLocalMediaStream();
    })

    // Peer Leave

    // Message

    // Disconnected
}

function disconnectAndLeave() {

}

// ============================================================
// WebRTC API
// ============================================================
function initLocalPeerConnection() {
    // 如果是多人的话，在这里要创建一个新的连接，新创建好的要放到一个 map 表中：key=userid, value=peerConnection。
    if (MediaHolder.rtcPeerConnection) {
        console.error("rtcPeerConnection has been created.")
        return
    }
    MediaHolder.rtcPeerConnection = new RTCPeerConnection(MediaHolder.peerConnectionConfig);
    // 当收集到了链路的时候，会触发这个事件。链路可以是 host, srflx, relay 这三种类型。
    MediaHolder.rtcPeerConnection.onicecandidate = (event) => {
        if (event.candidate) {
            Context.sendMessage({
                type: 'candidate',
                label: event.candidate.sdpMLineIndex,
                id: event.candidate.sdpMid,
                candidate: event.candidate.candidate
            })
            Context.log('Local ICE candidate: ' + event.candidate.candidate)
        } else {
            Context.log('End of candidates.')
        }
    }

    // 当收到远程流的时候，会触发这个事件。
    MediaHolder.rtcPeerConnection.ontrack = (event) => {

    }

    // 绑定本地流
    MediaHolder.localStream.getTracks().forEach((track) => {
        MediaHolder.rtcPeerConnection.addTrack(track, MediaHolder.localStream);
    })
}

function sendMediaOffer() {
    if (Context.connectionState !== ConnectionState.PeerJoined) {
        console.error("Connection state is not PeerConnected.")
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
                    Context.log("Set local description success. offer: " + offer);
                    Elements.taOffer.value = offer.sdp;
                    Context.sendMessage(offer);
                })
                .catch((error) => {
                    Context.log("Set local description error: " + error);
                });
        })
        .catch((error) => {
            Context.log("Create offer: " + error);
        })
}

function closeLocalPeerConnection() {
    if (!MediaHolder.rtcPeerConnection) {
        return
    }
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
