//
// Created by Anton Shelepov on 2019-06-09.
//

#include <sys/param.h>
#include <signal.h>
#include "sigsegv_handler.h"

int main() {
    struct sigaction action{};
    action.sa_flags = SA_SIGINFO;

    if (sigemptyset(&action.sa_mask) == -1) {
        std::cerr << "Couldn't create empty set of signals: " << strerror(errno) << std::endl;
        return 0;
    }

    action.sa_sigaction = sigsegv_handler;

    if (sigaction(SIGSEGV, &action, nullptr) == -1) {
        std::cerr << "Couldn't set sigsegv handler: " << strerror(errno) << std::endl;
        return 0;
    }

    int arr[3];
    for (int i = 0;; i++) {
        arr[i] = i;
    }
}