"use strict";

const http = require('http');
const events = require('events');
const util = require('util');
const fs = require('fs');
const ifconfig = require('wireless-tools/ifconfig');
const wpa_supplicant = require('wireless-tools/wpa_supplicant');
const setTimeoutPromise = util.promisify(setTimeout);
const outlet_server_url = 'http://192.168.1.1:8000/';

const Log = require('../utils/log');
let log = new Log();
const Timeout = {long: 120 * 1000, mid: 20 * 1000, short: 3 * 1000};


class Wifi {
    constructor() {
        this.outletkey = "";
        util.inherits(Wifi, events.EventEmitter);
        ifconfig.status('wlan0', (err, status) => {
            log.debug(status);
        });

        const file = fs.readFileSync(__dirname + '/../../data/wifi.json', 'utf8');
        const json = JSON.parse(file);
        this.ssid = json["ssid"];
        this.password = json["password"];
    }

    setup() {
        log.debug("start");
        this.disconnect().then(()=> {
            return this.disconnect2();
        }).then(()=> {
            return this.connectOutlet();
        }).then(()=> {
            return this.waitabit();
        }).then(()=> {
            return this.getKeyFromOutlet();
        }).then(()=> {
            return this.disconnect();
        }).then(()=> {
            return this.waitabit();
        }).then(()=> {
            return this.connectMain();
        }).then(()=> {
            return this.waitabit();
        }).then(()=> {
            return this.sendEvent();
        }).catch((err) => {
            log.warn(err);
            this.fallback();
        });

    }

    waitabit() {
        return setTimeoutPromise(Timeout.mid);
    }

    connectOutlet() {
        const options = {
            interface: 'wlan0',
            ssid: 'SmartOutlet',
            passphrase: 'smartoutlet',
            driver: 'nl80211,wext'
        };
        return new Promise((resolve, reject) => {
            log.debug("try to connect outlet wifi");
            wpa_supplicant.enable(options, (err) => {
                if (err) {
                    reject("connectOutlet:" + err);
                }
                log.debug("success to connect outlet");
                ifconfig.status('wlan0', (err, status) => {
                    log.debug(status);
                });
                resolve();
            });
        });
    }

    connectMain() {
        const options = {
            interface: 'wlan0',
            ssid: this.ssid,
            passphrase: this.password,
            driver: 'nl80211,wext'
        };
        return new Promise((resolve, reject) => {
            log.debug("try to connect main wifi");
            wpa_supplicant.enable(options, (err) => {
                if (err) {
                    reject("connectMain:" + err);
                }
                log.debug("connected to main");
                ifconfig.status('wlan0', (err, status) => {
                    log.debug(status);
                });
                resolve();
            });
        });
    }

    disconnect() {
        return new Promise((resolve, reject) => {
            wpa_supplicant.disable("wlan0", (err) => {
                if (err) {
                    log.debug("disconnect: " + err);
                }
                resolve();
            });
        });
    }

    disconnect2() {
        return new Promise((resolve, reject) => {
            wpa_supplicant.disable2("wlan0", (err) => {
                if (err) {
                    log.debug("disconnect: " + err);
                }
                resolve();
            });
        });
    }

    showConfig() {
        return new Promise((resolve, reject) => {
            ifconfig.status((err, status) => {
                log.debug(status);
            });
            resolve();
        });
    }

    getKeyFromOutlet() {
        let _this = this;
        return new Promise((resolve, reject) => {
            log.debug("try to get outlet key");
            http.get(outlet_server_url, (res) => {
                let body = '';
                res.setEncoding('utf8');
                res.on('data', (chunk) => {
                    body += chunk;
                });
                res.on('end', (res) => {
                    _this.outletkey = body;
                    log.debug("outlet key: " + body);
                    resolve();
                });
            }).on('error', (e) => {
                reject("http error:" + e);
            });
        });
    }

    sendEvent() {
        let _this = this;
        return new Promise((resolve, reject) => {
           log.debug("try to emit event");
            _this.emit('key', _this.outletkey);
            resolve();
        });
    }

    fallback() {
        log.error("start network fallback");
        this.disconnect().then(()=> {
            return this.disconnect2();
        }).then(()=> {
            return this.connectMain();
        });
    }


}

module.exports = Wifi;