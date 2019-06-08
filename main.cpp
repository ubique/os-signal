#include <iostream>
#include "Handler.h"

void fall() {
    int* p = nullptr;
    *p = 13;
}

int main() {
    struct sigaction action{};
    action.sa_sigaction = Handler::handleSignal;
    action.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &action, nullptr) != 0) {
        std::cout << "Error occurred in sigaction: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    fall();

    exit(EXIT_SUCCESS);
}