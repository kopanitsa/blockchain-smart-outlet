"use strict";

const http = require('http');
const fs = require('fs');
const Log = require('./utils/log');
let log = new Log();
const ip = '192.168.1.1';
const key = '/home/pi/key.txt';

http.createServer(function (req, res) {
    const key = fs.readFileSync(key, 'utf8');
    res.writeHead(200, {'Content-Type': 'text/plain'});
    res.end(key);
}).listen(8000, ip);

