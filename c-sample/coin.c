//
// Created by Okada, Takahiro on 2018/01/21.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "log.h"
#include "http.h"

#define BUF_LEN 256
#define LOCAL
#ifdef LOCAL
const static char host[BUF_LEN] = "127.0.0.1";
const static char path[BUF_LEN] = "/";
#else
const static char host[BUF_LEN] = "rinkeby.infura.io";
const static char path[BUF_LEN] = "/<YOUR_ID>";
#endif
const static char my_addr[BUF_LEN] = "<YOUR_ADDR>";
const static char contract_addr[BUF_LEN] = "<YOUR_CONTRACT_ADDR>";
static char msg[100];

void post_to_eth(char* req_body, size_t req_body_len, char* resp_body) {
    // HTTPS POST
    http_request_t request;
    memset(&request, 0, sizeof(request));
    memcpy(request.host, host, sizeof(host));
    memcpy(request.url, path, sizeof(path));
    char params[REQUEST_BUF_SIZE];

    memcpy(request.body, req_body, req_body_len);
    request.content_length = (int) req_body_len;

    http_response_t response;
#ifdef LOCAL
    request.port = 8545;
    int success = http_post(&request, &response, false);
#else
    int success = http_post(&request, &response, true);
#endif
    if (success == HTTP_SUCCESS) {
//        LOG("response:\n %s", response.body);
        strcpy(resp_body, response.body);
    } else {
        LOG("cannot get data from server");
        strcpy(resp_body, "error");
    }
}

void convert_to_bytes(char *src, char *dst, char len) {
    for (int i = 0; i<len; i++) {
        sprintf(dst, "%s%x", dst, src[i]);
    }
    sprintf(dst, "%s%s", dst, "00000000000000000000000000000000"); // padding for 16bytes string
}

void post_check_web3_client_version() {
    char contents[REQUEST_BUF_SIZE];
    char resp[REQUEST_BUF_SIZE];
    memset(contents, 0, REQUEST_BUF_SIZE);
    sprintf(contents, "{\"jsonrpc\":\"2.0\",\"method\":\"web3_clientVersion\",\"params\":[],\"id\":67}");
    post_to_eth(contents, strlen(contents), resp);

    LOG("post_check_web3_client_version result: %s",resp);
}

// In Solidity,
//      function buyCoin() public payable
void post_buy_coin() {
    char contents[REQUEST_BUF_SIZE];
    char resp[REQUEST_BUF_SIZE];
    char params[REQUEST_BUF_SIZE];
    memset(contents, 0, REQUEST_BUF_SIZE);
    char* data="0x091e9a60";
    sprintf(params, "[{\"from\":\"%s\",\"to\":\"%s\",\"data\":\"%s\"}]",
            my_addr, contract_addr, data);
    sprintf(contents, "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":%s,\"id\":0}", params);
    post_to_eth(contents, strlen(contents), resp);

    LOG("post_buy_coin result: %s",resp);
}

// In Solidity,
//     function payFromPlug(string _key, uint256 _coin) public
void post_pay_from_plug(char* key, char key_len) {
    char contents[REQUEST_BUF_SIZE];
    char resp[REQUEST_BUF_SIZE];
    char params[REQUEST_BUF_SIZE];
    memset(contents, 0, REQUEST_BUF_SIZE);
    char* func="0x91662a2d";
    char first_param[REQUEST_BUF_SIZE];
    memset(first_param, 0, REQUEST_BUF_SIZE);
    convert_to_bytes(key, first_param, key_len);
    char* second_param="0000000000000000000000000000000000000000000000000000000000000123";
    sprintf(params, "[{\"from\":\"%s\",\"to\":\"%s\",\"data\":\"%s%s%s\"}]",
            my_addr, contract_addr, func, first_param, second_param);
    sprintf(contents, "{\"jsonrpc\":\"2.0\",\"method\":\"eth_sendTransaction\",\"params\":%s,\"id\":0}", params);
    post_to_eth(contents, strlen(contents), resp);

    LOG("post_pay_from_plug result: %s",resp);
}

int exec_coin_payment(char* key, char key_len) {
#ifdef LOCAL
#else
    http_secure_init((char*)host, (char*)path);
#endif
    // just for test
    post_check_web3_client_version();

    //post_buy_coin();
    post_pay_from_plug(key, key_len);


    http_secure_clean();

    return EXIT_SUCCESS;

}
