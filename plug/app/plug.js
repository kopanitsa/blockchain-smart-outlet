"use strict";

const Log = require('./utils/log');
const Coin = require('./eth/coin');
const Wifi = require('./network/wifi');
const fs = require('fs');
let log = new Log();

// error handling
process.on('uncaughtException', function(err) {
    log.error(err);
});

// main code
class App {
    constructor() {
        const file = fs.readFileSync(__dirname + '/../data/id.json', 'utf8');
        const json = JSON.parse(file);
        const address = json["address"];
        const password = json["password"];
        this.coin = new Coin(address, password);
        this.wifi = new Wifi();
        this.wifi.setup(); // get key from outlet

        this.wifi.on('key', (key) => {
            this.coin.sendCoin(key);
        });
    }
}

let app = new App();
