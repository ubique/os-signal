//
// Created by vitalya on 09.06.19.
//

#include "sigsegv_handler.h"

int main() {
    sigsegv_handler handler1();
    int var[1];
    char a[1];

    for (auto ptr = a; ; ++ptr) {
        *ptr += 42;
    }
}