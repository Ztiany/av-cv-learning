'use strict'

const localVideo = document.querySelector('video#localVideo');
const remoteVideo = document.querySelector('video#remoteVideo');
const btnStart = document.querySelector('button#start');
const btnCall = document.querySelector('button#call');
const btnHangUp = document.querySelector('button#hangup');
const offerSdpTextarea = document.querySelector('textarea#offer');
const answerSdpTextarea = document.querySelector('textarea#answer');

let localStream;
let localRtcPeerConnection;
let remoteRtcPeerConnection;

window.onload = inti;

function isWebRTCSupported() {
    return navigator.mediaDevices && navigator.mediaDevices.enumerateDevices;
}

function inti() {
    setUpListeners()
}

function setUpListeners() {
    btnStart.onclick = start
    btnCall.onclick = call
    btnHangUp.onclick = hangUp
}

function start() {
    if (!isWebRTCSupported()) {
        console.error("WebRTC is not supported in your browser.");
        return
    }

    const constraints = {video: true, audio: false}
    navigator.mediaDevices.getUserMedia(constraints)
        .then(onGetMediaStreamSuccess)
        .catch(onGetMediaStreamError);
}

function onGetMediaStreamSuccess(stream) {
    console.log("onGetMediaStreamSuccess: ", stream)
    localStream = stream
    localVideo.srcObject = stream
}

function onGetMediaStreamError(error) {
    console.error("onGetMediaStreamError: ", error)
}

function call() {
    /*
        the standard way to establish a WebRTC connection is to use a signaling server to exchange session description protocol (SDP) messages between the two peers.

        media negotiation process:

                localRtcPeerConnection creates an offer and sets it as the local description by createOffer method and setLocalDescription method.
                localRtcPeerConnection sends the offer to the remote peer by signaling server.
                remoteRtcPeerConnection receives the offer and sets it as the remote description by setRemoteDescription method.
                remoteRtcPeerConnection creates an answer by createAnswer method.
                remoteRtcPeerConnection sets the answer as the local description by setLocalDescription method.
                remoteRtcPeerConnection sends the answer to the local peer by signaling server.

        network traversal process:

                localRtcPeerConnection gets local candidates by onicecandidate event.
                localRtcPeerConnection generates ICE candidates and sends them to the remote peer by signaling server.
                remoteRtcPeerConnection receives the ICE candidates and adds them by addIceCandidate method.
                remoteRtcPeerConnection gets local candidates by onicecandidate event.
                remoteRtcPeerConnection generates ICE candidates and sends them to the local peer by signaling server.
                the local and remote peers exchange ICE candidates until they have a direct connection.
                once the peers have a direct connection, they can exchange media.

    but here we omit the signaling server and directly exchange the SDP messages between the two peers.
     */

    localRtcPeerConnection = new RTCPeerConnection();
    remoteRtcPeerConnection = new RTCPeerConnection();

    localRtcPeerConnection.onicecandidate = (ev) => {
        console.log("localRtcPeerConnection.onicecandidate: ", ev)
        if (ev.candidate) {
            remoteRtcPeerConnection.addIceCandidate(ev.candidate)
                .catch((error) => {
                    console.error("remoteRtcPeerConnection.addIceCandidate error: ", error)
                })
        }
    }

    localRtcPeerConnection.oniceconnectionstatechange = (ev) => {
        console.log("localRtcPeerConnection.oniceconnectionstatechange state: ", this.iceConnectionState)
        console.log("localRtcPeerConnection.oniceconnectionstatechange event: ", ev)
    }

    remoteRtcPeerConnection.onicecandidate = (ev) => {
        console.log("remoteRtcPeerConnection.onicecandidate: ", ev)
        if (ev.candidate) {
            localRtcPeerConnection.addIceCandidate(ev.candidate)
                .catch((error) => {
                    console.error("localRtcPeerConnection.addIceCandidate error: ", error)
                })
        }
    }

    remoteRtcPeerConnection.oniceconnectionstatechange = (ev) => {
        console.log("remoteRtcPeerConnection.oniceconnectionstatechange state: ", this.iceConnectionState)
        console.log("remoteRtcPeerConnection.oniceconnectionstatechange event: ", ev)
    }

    // ontrack event is fired when a remote track is added to the RTCPeerConnection.
    // Here when media streams sent by local peer are received by remote peer, ontrack event is fired.
    remoteRtcPeerConnection.ontrack = onGetRemoteStream;

    localStream.getTracks().forEach(track => {
        localRtcPeerConnection.addTrack(track, localStream);
    });

    const offerOptions = {
        offerToReceiveAudio: false,
        offerToReceiveVideo: true
    };

    localRtcPeerConnection.createOffer(offerOptions)
        .then(onGetLocalDescription)
        .catch((error) => {
            console.error("localRtcPeerConnection.createOffer error: ", error)
        });
}

function onGetRemoteStream(ev) {
    if (remoteVideo.srcObject !== ev.streams[0]) {
        remoteVideo.srcObject = ev.streams[0];
    }
}

function onGetLocalDescription(desc) {
    offerSdpTextarea.value = desc.sdp

    localRtcPeerConnection.setLocalDescription(desc)
        .then(() => {
            console.log("localRtcPeerConnection.setLocalDescription success: ", desc)
        })
        .catch((error) => {
            console.error("localRtcPeerConnection.setLocalDescription error: ", error)
        });

    remoteRtcPeerConnection.setRemoteDescription(desc)
        .then(() => {
            console.log("remoteRtcPeerConnection.setRemoteDescription success: ", desc)
        })
        .catch((error) => {
            console.error("remoteRtcPeerConnection.setRemoteDescription error: ", error)
        });

    remoteRtcPeerConnection.createAnswer().then(onGetAnswerDescription)
        .catch((error) => {
            console.error("remoteRtcPeerConnection.createAnswer error: ", error)
        });
}

function onGetAnswerDescription(desc) {
    answerSdpTextarea.value = desc.sdp

    remoteRtcPeerConnection.setLocalDescription(desc)
        .then(() => {
            console.log("remoteRtcPeerConnection.setLocalDescription success: ", desc)
        })
        .catch((error) => {
            console.error("remoteRtcPeerConnection.setLocalDescription error: ", error)
        });

    localRtcPeerConnection.setRemoteDescription(desc)
        .then(() => {
            console.log("localRtcPeerConnection.setRemoteDescription success: ", desc)
        })
        .catch((error) => {
            console.error("localRtcPeerConnection.setRemoteDescription error: ", error)
        });
}

function hangUp() {
    localRtcPeerConnection.close();
    remoteRtcPeerConnection.close();
    localRtcPeerConnection = null;
    remoteRtcPeerConnection = null;
}