"use strict";

const Log = require('./utils/log');
const Coin = require('./eth/coin');
const Wifi = require('./network/wifi');
const Gpio = require('./peripheral/gpio');
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
        const privatekey = json["privatekey"];
        const _this = this;
        this.coin = new Coin(address, password, privatekey);

        this.wifi = new Wifi();
        this.gpio = new Gpio();

        this.interval = setInterval(() => {
            if (_this.gpio.isPlugin()==1) {
                clearInterval(_this.interval);
                _this.wifi.setup(); // get key from outlet
                _this.wifi.on('key', (key) => {
                    log.debug("send coin to: " + key);
                    _this.coin.sendCoin(key);
                });
            }
        }, 5000);
    }
}

let app = new App();
