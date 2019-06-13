#include <iostream>
#include <cstring>

#include "SignalHandler.h"

void printErr(const std::string& message) {
    fprintf(stderr, "ERROR %s: %s\n", message.c_str(), strerror(errno));
}

void testNull() {
    int* ptr = nullptr;
    *ptr = 42;
}

void testOutOfBound() {
    char* tmp = (char*) ("Hello");
    tmp[6] = '!';

}

int main() {
    struct sigaction action{};
    action.sa_sigaction = SignalHandler::handler;
    action.sa_flags = SA_SIGINFO;

    if (sigaction(SIGSEGV, &action, nullptr) < 0) {
        printErr("Signal action failed");
        exit(EXIT_FAILURE);
    }


    std::cout << "Test null - type \"1\", out-of-bound - type \"2\".\n";

    int choice;
    std::cin >> choice;
    switch (choice) {
        case 1: {
            testNull();
            break;
        }
        case 2: {
            testOutOfBound();
            break;
        }
        default: {
            std::cout << "Unknown test\n";
            exit(EXIT_FAILURE);
        }
    }

    exit(EXIT_SUCCESS);
}