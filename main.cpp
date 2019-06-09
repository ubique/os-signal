//
// Created by Noname Untitled on 02.06.19.
//

#include "Handler.h"

void outOfBoundsFunction() {
    char *message = const_cast<char *>("Some message");
    message[20] = 'F';
}

void nullPointerFunction() {
    int *nullPointer = nullptr;
    std::cout << *nullPointer;
}

int main() {
    struct sigaction handleAction{};
    handleAction.sa_sigaction = Handler::handler;
    handleAction.sa_flags = SA_SIGINFO;

    if (sigaction(SIGSEGV, &handleAction, nullptr) != 0) {
        perror("Signal action binding");
        exit(EXIT_FAILURE);
    }

    std::cout << "Enter case number\nOut of Bounds case: 1\nNull Pointer case: 2" << std::endl;

    int selectedCase;
    std::cin >> selectedCase;

    switch (selectedCase) {
        case 1: {
            outOfBoundsFunction();
            exit(EXIT_SUCCESS);
        }
        case 2: {
            nullPointerFunction();
            exit(EXIT_SUCCESS);
        }
        default: {
            std::cout << "Unknown case" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}