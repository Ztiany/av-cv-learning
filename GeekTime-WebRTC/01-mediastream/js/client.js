'use strict'

const videoPlay = document.querySelector("video#player");

function gotMediaStream(stream) {
    const videoTrack = stream.getVideoTracks()[0];

    window.stream = stream;
    videoPlay.srcObject = stream;
}

function handleError(err) {
    console.log('getUserMedia error:', err);
}

function start() {
    if (!navigator.mediaDevices || !navigator.mediaDevices.getUserMedia) {
        console.log('getUserMedia is not supported!');
        return;
    }

    let constraints = {
        video: {
            width: 640,
            height: 480,
            frameRate: 15,
            facingMode: 'environment'
        },
        audio: false
    };

    navigator.mediaDevices.getUserMedia(constraints)
        .then(gotMediaStream)
        .catch(handleError);
}

start();