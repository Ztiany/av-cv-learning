'use strict'

const userName = document.querySelector('input#username');
const inputRoom = document.querySelector('input#room');
const btnConnect = document.querySelector('button#connect');
const btnLeave = document.querySelector('button#leave');
const outputArea = document.querySelector('textarea#output');
const inputArea = document.querySelector('textarea#input');
const btnSend = document.querySelector('button#send');

let socket;
let room;

btnConnect.onclick = () => {
    //connect
    socket = io();// init socket.io

    // receive message
    socket.on('joined', (room, id) => {
        btnConnect.disabled = true;
        btnLeave.disabled = false;
        inputArea.disabled = false;
        btnSend.disabled = false;
    });

    socket.on('left', (room, id) => {
        btnConnect.disabled = false;
        btnLeave.disabled = true;
        inputArea.disabled = true;
        btnSend.disabled = true;

        socket.disconnect();
    });

    socket.on('message', (room, id, data) => {
        outputArea.scrollTop = outputArea.scrollHeight;// 窗口总是显示最后的内容
        console.log("received message [" + data + "] from [" + id + "]");
        outputArea.value = outputArea.value + data + '\r';
    });

    socket.on('disconnect', (socket) => {
        btnConnect.disabled = false;
        btnLeave.disabled = true;
        inputArea.disabled = true;
        btnSend.disabled = true;
    });

    // send message
    room = inputRoom.value;
    socket.emit('join', room);
}

btnSend.onclick = () => {
    let data = inputArea.value;
    data = userName.value + ':' + data;
    socket.emit('message', room, data);
    inputArea.value = '';
}

btnLeave.onclick = () => {
    room = inputRoom.value;
    socket.emit('leave', room);
}

inputArea.onkeypress = (event) => {
    // event = event || window.event;
    if (event.keyCode === 13/* 回车发送消息 */) {
        let data = inputArea.value;
        data = userName.value + ':' + data;
        socket.emit('message', room, data);
        inputArea.value = '';
        event.preventDefault(); //阻止默认行为
    }
}