#include "coin.h"
#include "log.h"

int main(int argc, char *argv[]){
    LOG("start");
    return exec_coin_payment("1234567890abcdef", 16);
}

