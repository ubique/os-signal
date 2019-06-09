//
// Created by Noname Untitled on 05.06.19.
//

#include "Handler.h"

static jmp_buf env;

std::map<std::string, int> Handler::REGISTERS = {
        {"R8",      REG_R8},
        {"R9",      REG_R9},
        {"R10",     REG_R10},
        {"R11",     REG_R11},
        {"R12",     REG_R12},
        {"R13",     REG_R13},
        {"R14",     REG_R14},
        {"R15",     REG_R15},
        {"RAX",     REG_RAX},
        {"RCX",     REG_RCX},
        {"RDX",     REG_RDX},
        {"RSI",     REG_RSI},
        {"RDI",     REG_RDI},
        {"RIP",     REG_RIP},
        {"RSP",     REG_RSP},
        {"CR2",     REG_CR2},
        {"RBP",     REG_RBP},
        {"RBX",     REG_RBX},
        {"EFL",     REG_EFL},
        {"ERR",     REG_ERR},
        {"CSGSFS",  REG_CSGSFS},
        {"TRAPNO",  REG_TRAPNO},
        {"OLDMASK", REG_OLDMASK},
};

void Handler::helper(int sigIgnored, siginfo_t *info, void *contextIgnored) {
    if (info->si_signo == SIGSEGV) {
        siglongjmp(env, 1);
    }
}

void Handler::dumpMemory(void *address) {
    std::cout << "Memory dumping... " << std::endl;

    if (address == nullptr) {
        std::cout << "Address is NULL" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Checking memory nearby..." << std::endl;

    size_t size = sizeof(char);
    long long from = std::max(0ll, (long long) ((char *) address - MEMORY_NEARBY * size));
    long long to = std::min(std::numeric_limits<long long>::max(),
                            (long long) ((char *) address + MEMORY_NEARBY * size));

    for (long long item = from; item < to; item += size) {
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGSEGV);
        sigprocmask(SIG_UNBLOCK, &sigset, nullptr);

        struct sigaction action{};
        action.sa_sigaction = helper;
        action.sa_flags = SA_SIGINFO;

        if (sigaction(SIGSEGV, &action, nullptr) != 0) {
            perror("Dump address");
            exit(EXIT_FAILURE);
        }

        if (setjmp(env) != 0) {
            std::cout << "Unable to dump address" << std::endl;
        } else {
            std::cout << " " << *reinterpret_cast<char *>(item) << " ";
        }
    }

    std::cout << std::endl;
}

void Handler::dumpRegisters(ucontext_t *ucontext) {
    std::cout << "Registers dumping..." << std::endl;

    greg_t *gRegs = ucontext->uc_mcontext.gregs;

    for (const auto &item : REGISTERS) {
        std::cout << " " << item.first << ": " << gRegs[item.second] << std::endl;
    }
}

void Handler::handler(int signalNumber, siginfo_t *siginfo, void *context) {
    std::cout << "Detected: " << strsignal(signalNumber) << std::endl;

    if (siginfo->si_signo == SIGSEGV) {
        switch (siginfo->si_code) {
            case SEGV_MAPERR: {
                std::cout << "Reason: Address doesn't relate to the object" << std::endl;
                break;
            }
            case SEGV_ACCERR: {
                std::cout << "Reason: Invalid rights to access the object" << std::endl;
                break;
            }
            default: {
                std::cout << "Unknown reason!!! WTF!!!" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        dumpMemory(siginfo->si_addr);
        dumpRegisters(reinterpret_cast<ucontext_t *> (context));

    } else {
        std::cout << "Signal handle by default behavior" << std::endl;
    }

    exit(EXIT_FAILURE);
}
