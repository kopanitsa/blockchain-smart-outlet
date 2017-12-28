"use strict";

const log4js = require('log4js');
log4js.configure({
    appenders: {
        out: { type: 'stdout' },
        app: { type: 'file', filename: 'application.log' }
    },
    categories: {
        default: {
            appenders: [ 'out', 'app' ],
            level: 'debug'
        }
    }
});
const logger = log4js.getLogger('log');

let Log = function() {
    logger.level = 'debug';
};

Log.prototype.error = function(msg) {
    logger.error(msg);
};

Log.prototype.warn = function(msg) {
    logger.warn(msg);
};

Log.prototype.debug = function(msg) {
    logger.debug(msg);
};

module.exports = Log;
