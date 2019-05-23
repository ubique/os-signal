//
// Created by max on 23.05.19.
//

#include <array>
#include <iostream>

#include "../Handler.h"


int main() {
    Handler handler{};

    int mas[1];

    int i = 1;
    int *a = mas;
    *a = 0;
    std::cout << i << std::endl;
    while (--a) {
        std::cout << --*a << std::endl;
    }
    std::cout << mas[0] << std::endl;
}
