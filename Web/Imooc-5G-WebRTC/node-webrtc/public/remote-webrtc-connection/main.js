'use strict'

// ===============================================================================
// import modules
// ===============================================================================
let http = require('http');
let https = require('https');
let fs = require('fs');
let serveIndex = require('serve-index');
let express = require('express');
let {Server} = require('socket.io');
let log4js = require('log4js');

// ===============================================================================
// configure log4js
// ===============================================================================
log4js.configure({
    appenders: {
        console: {type: 'console'},
        file: {
            type: 'file',
            filename: 'app.log',
            layout: {
                type: 'pattern',
                pattern: '%r %p - %m',
            }
        }
    },
    categories: {
        default: {
            appenders: ['console', 'file'],
            level: 'debug'
        }
    }
});
const logger = log4js.getLogger();


// ===============================================================================
// create express app
// ===============================================================================
const app = express();
app.use(serveIndex('./public', {}));
app.use(express.static('public'));

// ===============================================================================
// start server server
// ===============================================================================
const httpServer = http.createServer(app);
// bind socket.io to http server
const socketIo = new Server(httpServer, {
    // options
});

// Commands (Shared between server and client)
const Command = {
    JOIN: "join",
    ON_JOINED: "on_joined",
    ON_FULL: "on_full",
    ON_PEER_JOINED: "on_peer_joined",

    LEAVE: "leave",
    ON_LEAVE: "on_leave",
    ON_PEER_LEAVE: "on_peer_leave",
}

/*
connection:
    socket.to(room).emit('joined', room, socket.id); // 发给房间内的所有人（除自己外）。
    socket.broadcast.emit('joined', room, socket.id); / / 发给全部站点内的所有人（除自己外）。
    socketIo.in(room).emit('joined', room, socket.id) // 发给房间内的所有人（包括自己）。
 */
socketIo.sockets.on('connection', (socket) => {
    logger.log('a user connected: ', socket.id);

    socket.on('message', (room, data) => {
        logger.log(socket.id + ' sent a message [' + JSON.stringify(data) + '] to room [' + room + ']');
        // all the members but excepting yourself in the room will receive the message
        socket.to(room).emit('message', room, data)
    });

    // 该函数应该加锁
    socket.on(Command.JOIN, (room) => {
        logger.log(socket.id + ' wants to join the room: ' + room);
        socket.join(room)

        // After the socket joins the room, log the rooms
        const rooms = socketIo.sockets.adapter.rooms;
        const createdRoom = Array.from(rooms).find(([key, value]) => key === room)
        logger.log('got or created the room: ', createdRoom);
        if (!createdRoom) {
            return;
        }
        const sockets = createdRoom[1]
        const userCount = sockets.size
        logger.log('the number of user in room is: ' + userCount);

        // 在这里可以控制进入房间的人数，现在一个房间最多 2个人，为了便于客户端控制，如果是多人的话，应该将目前房间里人的个数当做数据下发下去。
        if (userCount < 3) {
            logger.log(socket.id + ' joined room: ' + room);
            socket.emit(Command.ON_JOINED, room, socket.id);
            if (userCount > 1) {
                socket.to(room).emit(Command.ON_PEER_JOINED, room, socket.id);
            }
        } else {
            socket.leave(room);
            socket.emit(Command.ON_FULL, room, socket.id);
        }
    });

    socket.on(Command.LEAVE, (room) => {
        const createdRoom = Array.from(socketIo.sockets.adapter.rooms).find(([key, value]) => key === room)
        const sockets = createdRoom[1]
        const usersCount = sockets.size
        logger.log(socket.id + 'left room: ' + room);
        logger.log('the number of user in room is: ' + (usersCount - 1));
        socket.leave(room);
        socket.to(room).emit(Command.ON_PEER_LEAVE, room, socket.id)
        socket.emit(Command.ON_LEAVE, room, socket.id);
    });
});

httpServer.listen(8080, "0.0.0.0");
logger.log('HTTP Server is running on: http://localhost:8080');

// ===============================================================================
// start https server
// ===============================================================================
if (fs.existsSync('../../cert/server.key') && fs.existsSync('../../cert/server.cert')) {
    const httpsServer = https.createServer({
        key: fs.readFileSync('../../cert/server.key'),
        cert: fs.readFileSync('../../cert/server.cert')
    }, app);
    httpsServer.listen(8443, "0.0.0.0");
    logger.log('HTTPS Server is running on: https://localhost:8443');
} else {
    logger.log('HTTPS Server not running, missing server.key and server.cert');
}