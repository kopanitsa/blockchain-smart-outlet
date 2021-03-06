"use strict";

// contracts
const abi = [
    {
        "constant": true,
        "inputs": [
            {
                "name": "x",
                "type": "bytes32"
            }
        ],
        "name": "bytes32ToString",
        "outputs": [
            {
                "name": "",
                "type": "string"
            }
        ],
        "payable": false,
        "stateMutability": "view",
        "type": "function"
    },
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
        "inputs": [],
        "payable": false,
        "stateMutability": "nonpayable",
        "type": "constructor"
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
    }
];

const address_contract = "0x018968e41da1364e328499613a7e5a22904ad513";

const Web3 = require('web3');
const web3 = new Web3(new Web3.providers.HttpProvider("https://rinkeby.infura.io/CPJOZqFCV8WOx1zOCRXg"));
let plugContract = new web3.eth.Contract(abi, address_contract);

const Log = require('../utils/log');
let log = new Log();

class Coin {
    constructor(address, password, privatekey) {
        this.address = address;
        this.password = password;
        this.privatekey = privatekey;
        plugContract.options.from = this.address;
        plugContract.options.gasPrice = '200000000000000'; // default gas price in wei
        plugContract.options.gas = 5000000; // provide as fallback always 5M gas

        web3.eth.accounts.wallet.add({
            privateKey: this.privatekey,
            address: this.address
        });
    }

    sendCoin(key) {
        this.execPayFromPlug(key, 10000);
    }

    execPayFromPlug(key, coin) {
        plugContract.methods.payFromPlug(key, coin).send(
            {from: this.address, gasPrice: 20000000000, gas: 3000000},
            (error, result)=> {
                log.debug("payFromPlug result:" + result);
                log.debug("payFromPlug error:" + error);
            }).on('transactionHash', (hash) => {
            log.debug("transactionHash: " + hash);
        }).on('receipt', (receipt) => {
            log.debug(receipt);
        }).on('error', (error) => {
            log.error(error);
        });
    }

}

module.exports = Coin;