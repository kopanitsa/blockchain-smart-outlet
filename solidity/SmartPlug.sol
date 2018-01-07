pragma solidity ^0.4.7;

contract SmartPlug {
    bool DEBUG = false;

    mapping(address => uint256) public coinBalance; // remaining coin of each devices
    mapping(bytes32 => address) reserved; // list of reserved outlet
    mapping(bytes32 => uint256) remainingTime; // list of remaining supply time for each reservation

    event log(string);
    event logUI256(uint256);
    event startSupply(address);

    function SmartPlug() public {

    }

    // 1. Outlet sends encrypted hash at first
    function sendOutletHash(bytes32 _hash) public {
        reserved[_hash] = msg.sender;
    }

    // 2. Plug sends key and coin.
    //    The contract check key has been registered as hash.
    //    If registered, transfer coin and notify "startSupply" event to outlet.
    function payFromPlug(string _key, uint256 _coin) public {
        bytes32 encrypted = keccak256(_key);
        address targetOutlet = reserved[encrypted];
        if (targetOutlet == 0) {
            log("ERROR: Cannot find ready outlet");
            return;
        }

        if ((int256)(coinBalance[msg.sender] - _coin) < 0) {
            log("ERROR: You dont have enough coin to supply!");
            return;
        }
        coinBalance[msg.sender]   -= _coin;
        coinBalance[targetOutlet] += _coin;

        if (DEBUG) {
            remainingTime[encrypted] = now + 20 * 1 seconds;
        } else {
            remainingTime[encrypted] = now + 60 * 2 minutes;
        }

        startSupply(targetOutlet);
    }

    // 3. Outlet ask that charging time is expired or not.
    //    If expired, reserved variable is cleared.
    function expired(bytes32 _hash) public returns(bool) {
        if (remainingTime[_hash] <= now) {
            reserved[_hash] = 0;
            return true;
        }
        return false;
    }

    // 4. Buy coin with Wei. Rate is 1coin/1Wei now.
    function buyCoin() public payable {
        coinBalance[msg.sender] += msg.value;
    }

    // 5. Withdraw earned coin as Wei.
    function withdraw(uint256 amount) public payable returns (bool) {
        if ((int)(coinBalance[msg.sender] - amount) < 0) {
            return false;
        }
        coinBalance[msg.sender] -= amount;
        msg.sender.transfer(amount);
        return true;
    }

}