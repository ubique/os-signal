//
// Created by Павел Пономарев on 2019-06-06.
//

#include "Handler.h"
#include <iostream>
#include <cmath>
#include <climits>
#include <string.h>


void Handler::handleSignal(int signum, siginfo_t* info, void* context) {
    if (info->si_signo == SIGSEGV) {
        std::cout << "Tried access " << info->si_addr << std::endl;
        switch (info->si_code) {
            case SEGV_ACCERR:
                std::cout << "SEGV_ACCERR" << std::endl;
                break;
            case SEGV_MAPERR:
                std::cout << "SEGV_MAPPER" << std::endl;
                break;
            default:
                std::cout << "Unknown si_code" << std::endl;
        }

        std::cout << "Dumping registers..." << std::endl;
        auto rContext = reinterpret_cast<ucontext_t*>(context);
        dumpRegisters(rContext);
        std::cout << "Registers dumped." << std::endl;

        std::cout << "Dumping memory..." << std::endl;
        dumpMemory(info->si_addr);
        std::cout << "Memory dumped." << std::endl;
    }
    exit(EXIT_FAILURE);
}

void Handler::dumpRegisters(ucontext_t* context) {
    for (int r = 0; r < 23; ++r) {
        std::cout << "Register " << r <<
        static_cast<unsigned int>(context->uc_mcontext.gregs[r]) << std::endl;
    }
}

void Handler::dumpMemory(void* address) {
    if (address == nullptr) {
        std::cout << "Address is nullptr" << std::endl;
        exit(EXIT_FAILURE);
    }
    long long from = std::max(0ll, (long long) (static_cast<char*>(address) - sizeof(char) * MEMORY_SIZE));
    long long to = std::min(LONG_LONG_MAX, (long long) (static_cast<char*>(address) + sizeof(char) * MEMORY_SIZE));

    for (long long curAddr = from; curAddr < to; ++curAddr) {
        sigset_t signalSet;
        sigemptyset(&signalSet);
        sigaddset(&signalSet, SIGSEGV);
        sigprocmask(SIG_UNBLOCK, &signalSet, nullptr);

        struct sigaction act{};

        act.sa_sigaction = sigsegvHandlerAddress;
        act.sa_flags = SA_SIGINFO;

        if (sigaction(SIGSEGV, &act, nullptr) != 0) {
            std::cerr << "Sigaction failed: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
        const char* p = (const char*) curAddr;
        std::cout << "ADDRESS_0x" << (void*) curAddr << " ";
        if (setjmp(mJbuf) != 0) {
            std::cout << "bad :(";
        } else {
            std::cout << int (p[0]);
        }
        std::cout << std::endl;
    }
}

void Handler::sigsegvHandlerAddress(int signum, siginfo_t* siginfo, void* context) {
    if (siginfo->si_signo == SIGSEGV) {
        siglongjmp(mJbuf, 1);
    }
}
