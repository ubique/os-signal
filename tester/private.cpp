//
// Created by max on 23.05.19.
//

#include <sys/mman.h>
#include <iostream>
#include "../Handler.h"

int main() {
    Handler handler{};
    void *ptr = mmap(nullptr, 100, PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr != MAP_FAILED) {
        std::cout << "MMAP success" << std::endl;
        *(static_cast<char *>(ptr)+3) += 1;
    } else {
        std::cout << "MMAP fail" << std::endl;
    }
}