//
// Created by max on 23.05.19.
//

#include <array>
#include <iostream>

#include "../Handler.h"


int main() {
    Handler handler{};

    srand(time(0));
    int i = rand();
    int *a = &i;
    while (true) {
        std::cout << *a << std::endl;
        a = reinterpret_cast<int *>(*a);
    }
}
