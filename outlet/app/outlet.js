"use strict";

const fs = require('fs');
const createKeccakHash = require('keccak');
const Log = require('./utils/log');
const Coin = require('./eth/coin');
const Gpio = require('./peripheral/gpio');
let log = new Log();

const key = '/home/pi/key.txt';

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
        let _this = this;

        this.coin = new Coin(address, password);
        this.coin.on('contract', (enabled) => {
            if (enabled) {
                _this.gpio.contractApproved();
            }
        });

        this.gpio = new Gpio();
        this.gpio.on('switch', (enabled) => {
            if (enabled) {
                const hashpair = _this.generateKeyAndHash();
                _this.coin.sendOutletHash('0x' + hashpair.datahash);
                fs.writeFile(key, hashpair.datakey);
            } else {
                log.debug("start sta");
            }
        });
        setInterval(() => {
            this.gpio.checkSwitch()
        }, 1000);
    }

    generateKeyAndHash() {
        const l = 16;
        const c = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        const cl = c.length;
        let key = "";
        for(let i=0; i<l; i++){
            key += c[Math.floor(Math.random()*cl)];
        }
        let hash = createKeccakHash('keccak256').update(key).digest('hex');

        return {datakey: key, datahash: hash};
    }
}

let app = new App();
