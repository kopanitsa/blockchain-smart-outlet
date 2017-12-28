# blockchain-smart-outlet
Outlet which can pay encrypted coin automatically. It's a study for blockchain, smart contract, iot and wifi network.

### Summary

- This is a blockchain (Smart Contract) + IoT project to enable automatic payment from a plug to anoutlet.
- Sequence is like below.
    1. User put the plug to the outlet, and push outlet switch.
    2. Outlet recognizes the event, and send `hash` to a smart contract on Ethereum blockchain.
    3. Plug connects to Outlet via Wi-Fi (Outlet have Wi-Fi AP function), and get `key`.
    4. Plug send the `key` to the smart contract. The smart contract check `hash` and `key` is paired or not.
    5. If `key` and `hash` is paired, the smart contract send event to the outlet. At the same time, plug's coins move to the outlet.
    6. After payment, the outlet supply power to the plug before expiring timer.

![summary](https://user-images.githubusercontent.com/891384/34401292-472630b4-ebdd-11e7-96f7-f4f9b80bbdc5.png)


### Environment

|  | Version | Memo |
|:-----------|:------------|:------------|
|Raspberry Pi Zero W|November 2017|for plug| 
|Raspberry Pi 3| November 2017|for Outlet|
|Node.js| v8.9.3| |
|npm| 5.3.0| |
|Geth| 1.7.3-stable-4bb3c89d| |
|Solidity| 0.4.19+commit.c4cbbb05.Emscripten.clang| |
|Test net| rinkeby| |
|hostapd| v2.6| for Outlet's AP|


