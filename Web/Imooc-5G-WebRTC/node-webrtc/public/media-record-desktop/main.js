'use strict'

let http = require('http');
let https = require('https');
let fs = require('fs');
let serveIndex = require('serve-index');
let express = require('express');

const app = express();
app.use(serveIndex('./public', {}));
app.use(express.static('public'));

// start http server
const httpServer = http.createServer(app);
httpServer.listen(8080, "0.0.0.0", () => {
    console.log('HTTP Server running on port 8080');
});

// start https server
if (fs.existsSync('../../cert/server.key') && fs.existsSync('../../cert/server.cert')) {
    const httpsServer = https.createServer({
        key: fs.readFileSync('../../cert/server.key'),
        cert: fs.readFileSync('../../cert/server.cert')
    }, app);
    httpsServer.listen(8443, "0.0.0.0", () => {
        console.log('HTTPS Server running on port 8443');
    });
} else {
    console.log('HTTPS Server not running, missing server.key and server.cert');
}