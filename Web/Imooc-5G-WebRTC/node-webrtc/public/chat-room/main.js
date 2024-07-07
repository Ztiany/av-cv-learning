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
const httpServerIO = new Server(httpServer, {
    // options
});

// connection
httpServerIO.sockets.on('connection', (socket) => {
    logger.log('a user connected: ', socket.id);

    socket.on('message', (room, data) => {
        logger.log(socket.id + ' sent a message [' + data + '] to room [' + room + ']');
        // all the members but excepting yourself in the room will receive the message
        // socket.to(room).emit('message', room, data)
        httpServerIO.in(room).emit('message', room, socket.id, data)
    });

    // 该函数应该加锁
    socket.on('join', (room) => {
        logger.log(socket.id + ' wants to join the room: ' + room);
        socket.join(room)

        // After the socket joins the room, log the rooms
        const rooms = httpServerIO.sockets.adapter.rooms;
        const createdRoom = Array.from(rooms).find(([key, value]) => key === room)
        logger.log('got or created the room: ', createdRoom);
        if (!createdRoom) {
            return;
        }
        const sockets = createdRoom[1]
        const users = sockets.size
        logger.log('the number of user in room is: ' + users);

        // 在这里可以控制进入房间的人数，现在一个房间最多 2个人，为了便于客户端控制，如果是多人的话，应该将目前房间里人的个数当做数据下发下去。
        if (users < 3) {
            logger.log(socket.id + ' joined room: ' + room);
            socket.emit('joined', room, socket.id);
            if (users > 1) {
                socket.to(room).emit('other_join', room);
            }
        } else {
            socket.leave(room);
            socket.emit('full', room, socket.id);
        }
        //socket.to(room).emit('joined', room, socket.id); // 发给房间内的所有人（除自己外）。
        //httpServerIO.in(room).emit('joined', room, socket.id) // 发给房间内的所有人（包括自己）。
        //socket.broadcast.emit('joined', room, socket.id); / / 发给全部站点内的所有人（除自己外）。
    });

    socket.on('leave', (room) => {
        const createdRoom = Array.from(httpServerIO.sockets.adapter.rooms).find(([key, value]) => key === room)
        const sockets = createdRoom[1]
        const usersCount = sockets.size
        logger.log(socket.id + 'leaved room: ' + room);
        logger.log('the number of user in room is: ' + (usersCount - 1));
        socket.leave(room);
        socket.to(room).emit('bye', room, socket.id)
        socket.emit('leaved', room, socket.id);
    });
});

httpServer.listen(8080, "0.0.0.0");
logger.log('HTTP Server is running on: http://localhost:8080');

// ===============================================================================
// start https server
// ===============================================================================
if (fs.existsSync('./cert/server.key') && fs.existsSync('./cert/server.cert')) {
    const httpsServer = https.createServer({
        key: fs.readFileSync('server.key'),
        cert: fs.readFileSync('server.cert')
    }, app);
    httpsServer.listen(8443, "0.0.0.0");
    logger.log('HTTPS Server is running on: https://localhost:8443');
} else {
    logger.log('HTTPS Server not running, missing server.key and server.cert');
}