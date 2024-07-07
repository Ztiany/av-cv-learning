'use strict'

let audioSourceSelect;
let audioOutputSelect;
let videoSourceSelect;

window.onload = function () {
    start();
}

function isWebRTCSupported() {
    return navigator.mediaDevices && navigator.mediaDevices.enumerateDevices;
}

function findElements() {
    audioSourceSelect = document.querySelector("select#audioSource")
    audioOutputSelect = document.querySelector("select#audioOutput")
    videoSourceSelect = document.querySelector("select#videoSource")
}

function onGetDevices(devices) {
    if (!devices.length) {
        console.log("No devices found. devices: ", devices)
        return
    }

    devices.forEach(device => {
        /*
        非 https 网站无法获取设备的 label，解决方案：
            # 谷歌
            chrome://flags/#unsafely-treat-insecure-origin-as-secure
            # edge
            edge://flags/#unsafely-treat-insecure-origin-as-secure

            将上述地址打开，在 Insecure origins treated as secure 添加 http://localhost:8080，然后重启浏览器。具体参考 <https://blog.csdn.net/qq_35385687/article/details/120736610>。
         */
        console.log(device.kind + ": label = " + device.label + ": id = " + device.deviceId + ": groupId = " + device.groupId);

        let option = document.createElement("option")
        option.value = device.deviceId
        if (device.kind === "audioinput") {
            option.text = device.label || "microphone " + (audioSourceSelect.length + 1)
            audioSourceSelect.appendChild(option)
        } else if (device.kind === "audiooutput") {
            option.text = device.label || "speaker " + (audioOutputSelect.length + 1)
            audioOutputSelect.appendChild(option)
        } else if (device.kind === "videoinput") {
            option.text = device.label || "camera " + (videoSourceSelect.length + 1)
            videoSourceSelect.appendChild(option)
        }
    });
}

function onGetDevicesError(error) {
    console.log(error);
}

function getAVDevices() {
    navigator.mediaDevices.enumerateDevices()
        .then(onGetDevices)
        .catch(onGetDevicesError)
}

function start() {
    if (!isWebRTCSupported()) {
        console.error("WebRTC is not supported in your browser")
        return
    }

    findElements();
    getAVDevices();
}