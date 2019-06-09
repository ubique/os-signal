//
// Created by daniil on 09.06.19.
//
#include "SigsegvHandler.h"

int main() {
    SigsegvHandler::setSigsegv();

    int unused[1] = {28};
    char a[2];
    for (char* i = a; ; ++i) {
        *i = 42;
    }

    return 0;
}