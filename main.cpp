//
// Created by daniil on 09.06.19.
//
#include "SigsegvHandler.h"

int main() {
    SigsegvHandler::setSigsegv();

    //test
    char* test = "aca";
    test[3] = 't';
    return 0;
}