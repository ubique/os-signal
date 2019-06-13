#include <csignal>
#include <iostream>

#include "sigsegv_handler.hpp"

void test_nullptr() {
    int* p = nullptr;
    *p = 100500;
}

void test_raise() { std::raise(SIGSEGV); }

void test_buf() {
    char buf[2];
    for (auto ptr = buf;; ++ptr) {
        *ptr += 'F';
    }
}

int main() {
    sigsegv_handler::get_ready();

    test_nullptr();
    // test_raise();
    // test_buf();

    return EXIT_SUCCESS;
}
