//
// Created by vitalya on 09.06.19.
//

#include <cstdio>
#include "sigsegv_handler.h"

int main() {
    sigsegv_handler handler(&handle);

    char* null = 0;
    *null = 10;
}