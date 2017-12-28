"use strict";

const Log = require('../utils/log');
const util = require('util');
const events = require('events');

let log = new Log();

const SSR = 2;
const SW_OUT = 19;
const SW_IN = 26;

const underContract = false;

class Gpio {
    constructor() {
        util.inherits(Gpio, events.EventEmitter);

        this.fs = require('fs');
        this.prevValue = 0;

        this.gpioExport(SSR);
        this.gpioDirection(SSR, 'out');
        this.writeGpio(SSR, 0);

        this.gpioExport(SW_OUT);
        this.gpioDirection(SW_OUT, 'out');
        this.writeGpio(SW_OUT, 1);

        this.gpioExport(SW_IN);
        this.gpioDirection(SW_IN, 'in');
    }

    checkSwitch() {
        let value = this.readGpio(SW_IN, 'utf8');
        if (this.prevValue ==0 && parseInt(value) == 1 && !underContract) {
            this.prevValue = 1;
            log.debug("start initial power supply");
            this.writeGpio(SSR, 1);
            this.emit('switch', true);

            this.initialPowerTimer = setTimeout(() => {
                this.writeGpio(SSR, 0);
                this.emit('switch', false);
            }, 30 * 1000);
        } else if (parseInt(value) == 0) {
            this.prevValue = 0;
        }
    }

    contractApproved() {
        log.debug("finish initial power supply");
        clearTimeout(this.initialPowerTimer);
        this.writeGpio(SSR, 1);
    }

    contractFinished() {
        this.writeGpio(SSR, 0);
    }

    gpioExport(pin) {
        this.fs.writeFileSync('/sys/class/gpio/export', pin);
    }

    gpioUnexport(pin) {
        this.fs.writeFileSync('/sys/class/gpio/unexport', pin);
    }

    gpioDirection(pin, dir) {
        this.fs.writeFileSync('/sys/class/gpio/gpio'+pin+'/direction', dir);
    }

    writeGpio(pin, value) {
        this.fs.writeFileSync('/sys/class/gpio/gpio'+pin+'/value', value);
    }

    readGpio(pin) {
        return this.fs.readFileSync('/sys/class/gpio/gpio'+pin+'/value');
    }
}

module.exports = Gpio;