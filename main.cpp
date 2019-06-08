#include "sigsegv_handler.h"

#include <signal.h>
#include <iostream>


int main() {
    struct sigaction act{};
    act.sa_flags = SA_SIGINFO | SA_NODEFER;
    act.sa_handler = reinterpret_cast<__sighandler_t>(sigsegv_handler);

    sigaction(SIGSEGV, &act, nullptr);

    const char* test = "abc";

    *(const_cast<char*>(test)) = 'd';
}