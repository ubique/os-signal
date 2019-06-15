#include "Handler.h"
#include <iostream>
#include <string.h>
#include <cstring>

void fall() {
    int* p = nullptr;
    *p = 13;
}

void fall2() {
    char* str = const_cast<char*>("Hello, my friend");
    str[16] = '1';
}

int main() {
    struct sigaction action{};
    action.sa_sigaction = Handler::handleSignal;
    action.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &action, nullptr) != 0) {
        std::cout << "Error occurred in sigaction: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    //fall();
    fall2();

    exit(EXIT_SUCCESS);
}