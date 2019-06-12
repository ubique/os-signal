//
// Created by max on 10.06.19.
//

#include "../Handler.h"
#include <iostream>

int main() {
    Handler handler{};
    int unused[1] = {28};
    char a[2];
    for (char *i = a;; ++i) {
        *i = 42;
    }
}