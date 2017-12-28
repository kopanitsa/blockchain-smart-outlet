"use strict";

// contracts
const abi = [
    {
        "constant": true,
        "inputs": [
            {
                "name": "",
                "type": "address"
            }
        ],
        "name": "coinBalance",
        "outputs": [
            {
                "name": "",
                "type": "uint256"
            }
        ],
        "payable": false,
        "stateMutability": "view",
        "type": "function"
    },
    {
        "constant": false,
        "inputs": [],
        "name": "buyCoin",
        "outputs": [],
        "payable": true,
        "stateMutability": "payable",
        "type": "function"
    },
    {
        "constant": false,
        "inputs": [
            {
                "name": "_hash",
                "type": "bytes32"
            }
        ],
        "name": "expired",
        "outputs": [
            {
                "name": "",
                "type": "bool"
            }
        ],
        "payable": false,
        "stateMutability": "nonpayable",
        "type": "function"
    },
    {
        "anonymous": false,
        "inputs": [
            {
                "indexed": false,
                "name": "",
                "type": "string"
            }
        ],
        "name": "log",
        "type": "event"
    },
    {
        "anonymous": false,
        "inputs": [
            {
                "indexed": false,
                "name": "",
                "type": "uint256"
            }
        ],
        "name": "logUI256",
        "type": "event"
    },
    {
        "constant": false,
        "inputs": [
            {
                "name": "_key",
                "type": "string"
            },
            {
                "name": "_coin",
                "type": "uint256"
            }
        ],
        "name": "payFromPlug",
        "outputs": [],
        "payable": false,
        "stateMutability": "nonpayable",
        "type": "function"
    },
    {
        "constant": false,
        "inputs": [
            {
                "name": "_hash",
                "type": "bytes32"
            }
        ],
        "name": "sendOutletHash",
        "outputs": [],
        "payable": false,
        "stateMutability": "nonpayable",
        "type": "function"
    },
    {
        "anonymous": false,
        "inputs": [
            {
                "indexed": false,
                "name": "",
                "type": "address"
            }
        ],
        "name": "startSupply",
        "type": "event"
    },
    {
        "constant": false,
        "inputs": [
            {
                "name": "amount",
                "type": "uint256"
            }
        ],
        "name": "withdraw",
        "outputs": [
            {
                "name": "",
                "type": "bool"
            }
        ],
        "payable": true,
        "stateMutability": "payable",
        "type": "function"
    },
    {
        "inputs": [],
        "payable": false,
        "stateMutability": "nonpayable",
        "type": "constructor"
    }
];
const address_contract = "0x71A2897b2B222225855240c10114FdE25865b3fB";

const Web3 = require('web3');
const net = require('net');
let web3 = new Web3('/home/pi/.ethereum/rinkeby/geth.ipc', net);
let plugContract = new web3.eth.Contract(abi, address_contract);

const Log = require('../utils/log');
let log = new Log();

class Coin {
    constructor(address, password) {
        this.address = address;
        this.password = password;
        plugContract.options.from = this.address;
        plugContract.options.gasPrice = '20000000000000'; // default gas price in wei
        plugContract.options.gas = 5000000; // provide as fallback always 5M gas

        this.startWatching();
    }

    startWatching() {
        var subscription = web3.eth.subscribe('logs', {
            address: address_contract,
            topics: [web3.utils.sha3('log(string)')]
        }, (error, result) => {
            if (error)
                log.error(error);
            log.debug("result: " + web3.utils.hexToAscii(result.data));
        }).on("data", function(result){
            log.debug("data: " + web3.utils.hexToAscii(result.data));
        }).on("changed", function(result){
            log.debug("changed: " + web3.utils.hexToAscii(result.data));
        });
    }

    sendCoin(key) {
        web3.eth.personal.unlockAccount(this.address, this.password, 60)
            .then((response) => {
                log.debug("unlock:" + response);
                this.execPayFromPlug(key, 10000);
            }).catch((error) => {
            console.log(error);
        });
    }

    execPayFromPlug(key, coin) {
        plugContract.methods.payFromPlug(key, coin).send(
            {from: this.address, gasPrice: 200000000000, gas: 3000000},
            (error, result)=> {
                log.debug("payFromPlug result:" + result);
                log.debug("payFromPlug error:" + error);
            }).on('transactionHash', (hash) => {
            log.debug("transactionHash: " + hash);
        }).on('receipt', (receipt) => {
            log.debug("receipt: " + receipt);
        }).on('error', (error) => {
            log.error(error);
        });
    }

}

module.exports = Coin;