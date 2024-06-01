'use strict'

const elements = {
    // video
    videoPlayer: undefined,
    videoConstraintsDiv: undefined,
    videoFilterSelect: undefined,
    snapshotButton: undefined,
    recordingButton: undefined,
    playRecordingButton: undefined,
    downloadRecordingButton: undefined,
    snapshotCanvas: undefined,

    // recoding video
    recordingVideoPlayer: undefined,
    recordingVideoConstraintsDiv: undefined,
}

const globalState = {
    isRecording: false,
    stream: undefined,
    mediaRecorder: undefined,
    buffer: [],
}

const avConstraints = {
    video: true,
    audio: true
}

window.onload = function () {
    start();
}

function isWebRTCSupported() {
    return navigator.mediaDevices && navigator.mediaDevices.getDisplayMedia;
}

function start() {
    if (!isWebRTCSupported()) {
        console.error("WebRTC is not supported in your browser.")
        return
    }
    console.log("WebRTC is supported in your browser.")
    findElements();
    setUpListeners();
    getAVDevices();
}

function findElements() {
    elements.videoPlayer = document.querySelector("video#videoPlayer")
    elements.videoFilterSelect = document.querySelector("select#videoFilter")
    elements.videoConstraintsDiv = document.querySelector("div#videoConstraints")
    elements.snapshotCanvas = document.querySelector("canvas#snapshotCanvas")
    elements.snapshotButton = document.querySelector("button#snapshotBtn")
    elements.recordingButton = document.querySelector("button#recordBtn")
    elements.playRecordingButton = document.querySelector("button#playRecordingBtn")
    elements.downloadRecordingButton = document.querySelector("button#downloadRecordingBtn")

    elements.recordingVideoPlayer = document.querySelector("video#recordingVideoPlayer")
    elements.recordingVideoConstraintsDiv = document.querySelector("div#recordingVideoConstraints")
}

function setUpListeners() {
    elements.videoFilterSelect.onchange = function () {
        elements.videoPlayer.className = elements.videoFilterSelect.value;
    }

    elements.snapshotButton.onclick = function () {
        elements.snapshotCanvas.width = elements.videoPlayer.videoWidth;
        elements.snapshotCanvas.height = elements.videoPlayer.videoHeight;
        elements.snapshotCanvas.getContext("2d").drawImage(elements.videoPlayer, 0, 0, elements.snapshotCanvas.width, elements.snapshotCanvas.height);
    }

    elements.recordingButton.onclick = function () {
        startRecording()
    };
    elements.playRecordingButton.onclick = function () {
        playRecordingVideo()
    };
    elements.downloadRecordingButton.onclick = function () {
        downloadRecordingVideo()
    };
}

function onGetMediaStream(stream) {
    globalState.stream = stream;

    elements.videoPlayer.srcObject = null;
    elements.videoConstraintsDiv.textContent = "";

    // video only
    if (avConstraints.video && !avConstraints.audio) {
        let videoTrack = stream.getVideoTracks()[0];
        elements.videoConstraintsDiv.textContent = JSON.stringify(videoTrack.getSettings(), null, 2);
        elements.videoPlayer.srcObject = stream;
        return navigator.mediaDevices.enumerateDevices();
    }

    // audio and video
    let audioTrack = stream.getAudioTracks()[0];
    let videoTrack = stream.getVideoTracks()[0];
    let bothSettings = {
        audioTrack: audioTrack ? audioTrack.getSettings() : null,
        videoTrack: videoTrack.getSettings()
    }
    elements.videoConstraintsDiv.textContent = JSON.stringify(bothSettings, null, 2);
    elements.videoPlayer.srcObject = stream;
    return navigator.mediaDevices.enumerateDevices();
}

function onGetDevices(devices) {
    if (!devices.length) {
        console.log("No devices found. devices: ", devices)
        return
    }
    devices.forEach(device => {
        console.log(device.kind + ": label = " + device.label + ": id = " + device.deviceId + ": groupId = " + device.groupId);
    });
}

function onGetDevicesError(error) {
    console.log("onGetDevicesError:", error);
}

function getAVDevices() {
    navigator.mediaDevices
        .getDisplayMedia(avConstraints)// getDisplayMedia 是获取屏幕的音视频流，也会触发用户授权
        .then(onGetMediaStream)
        .then(onGetDevices)
        .catch(onGetDevicesError)
}

function startRecording() {
    if (globalState.isRecording) {
        globalState.isRecording = false
        elements.recordingButton.textContent = "Start recording"
        elements.playRecordingButton.disabled = false
        elements.downloadRecordingButton.disabled = false
        stopRecording()
        return
    }

    globalState.isRecording = true
    elements.recordingButton.textContent = "Stop recording"
    elements.playRecordingButton.disabled = true
    elements.downloadRecordingButton.disabled = true
    doRealRecoding()
}

function stopRecording() {
    if (globalState.mediaRecorder) {
        globalState.mediaRecorder.stop();
    }
}

function doRealRecoding() {
    if (!globalState.stream) {
        console.error("not stream assigned.")
        return;
    }

    globalState.buffer = [];
    const options = {
        mimeType: 'video/webm;codecs=vp8'
    };

    if (!MediaRecorder.isTypeSupported(options.mimeType)) {
        console.error(`${options.mimeType} is not supported!`);
        return;
    }

    try {
        globalState.mediaRecorder = new MediaRecorder(globalState.stream, options);
    } catch (e) {
        console.error('Failed to create MediaRecorder:', e);
        return;
    }
    globalState.mediaRecorder.ondataavailable = handleRecordedData
    globalState.mediaRecorder.start(10/*多长时间回调一次数据*/);
}

function handleRecordedData(recordedData) {
    if (recordedData && recordedData.data && recordedData.data.size > 0) {
        globalState.buffer.push(recordedData.data)
    }
}

function downloadRecordingVideo() {
    if (!globalState.buffer || globalState.buffer.length <= 0) {
        console.error('No video has been recorded.');
        return;
    }

    const blob = new Blob(globalState.buffer, {type: 'video/webm'});
    const url = window.URL.createObjectURL(blob);
    const aElement = document.createElement('a');
    aElement.href = url;
    aElement.style.display = 'none';
    aElement.download = 'recodedVideo.webm';
    aElement.click();
}

function playRecordingVideo() {
    if (!globalState.buffer || globalState.buffer.length <= 0) {
        console.error('No video has been recorded.');
        return;
    }

    const blob = new Blob(globalState.buffer, {type: 'video/webm'});
    elements.recordingVideoPlayer.src = window.URL.createObjectURL(blob);
    elements.recordingVideoPlayer.srcObject = null;
    elements.recordingVideoPlayer.controls = true;
    elements.recordingVideoPlayer.play();
}