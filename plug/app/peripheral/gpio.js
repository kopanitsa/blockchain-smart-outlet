"use strict";

const Log = require('../utils/log');
const util = require('util');
const events = require('events');

let log = new Log();

const PLUG_IN = 0;

const underContract = false;

class Gpio {
    constructor() {
        util.inherits(Gpio, events.EventEmitter);

        this.fs = require('fs');
        this.prevValue = 0;

        try {
            this.gpioUnexport(PLUG_IN);
        } catch (e) {
            // do nothing
        }

        this.gpioExport(PLUG_IN);
        this.gpioDirection(PLUG_IN, 'in');
    }

    isPlugin() {
        return parseInt(this.readGpio(PLUG_IN, 'utf8'));
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