//
// Created by Noname Untitled on 05.06.19.
//

#include "Handler.h"

std::map<const char *, int> Handler::REGISTERS = {
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

void Handler::dumpMemory(void *address) {
    Utils::printString("Memory dumping... \n");

    int descriptors[2];

    if (address == nullptr) {
        Utils::printString("Address is NULL\n");
        _exit(EXIT_FAILURE);
    }

    if (pipe(descriptors) == -1) {
        Utils::printString("Unable to dump memory!\n");
    }

    Utils::printString("Checking memory nearby...\n");

    size_t size = sizeof(char);
    long long from = std::max(0ll, (long long) ((unsigned char *) address - MEMORY_NEARBY * size));
    long long to = std::min(LONG_LONG_MAX, (long long) ((unsigned char *) address + MEMORY_NEARBY * size));

    for (long long i = from; i < to; i += 2) {
        if (write(descriptors[1], (unsigned char *) i, 1) != -1) {
            Utils::printNumber((*((unsigned char *) i) & 0xFFu), 1);
        } else {
            Utils::printString("Bad memory");
        }
        Utils::printChar(' ');
    }

    Utils::printChar('\n');
}

void Handler::dumpRegisters(ucontext_t *ucontext) {
    Utils::printString("Registers dumping...\n");

    for (const auto &item : REGISTERS) {
        Utils::printChar(' ');
        Utils::printString(item.first);
        Utils::printString(": ");
        Utils::printHex(ucontext->uc_mcontext.gregs[item.second]);
        Utils::printChar('\n');
    }
}

void Handler::handler(int signalNumber, siginfo_t *siginfo, void *context) {
    Utils::printString(strsignal(signalNumber));
    Utils::printString(" detected!\n");

    if (siginfo->si_signo == SIGSEGV) {
        switch (siginfo->si_code) {
            case SEGV_MAPERR: {
                Utils::printString("Reason: Address doesn't relate to the object\n");
                break;
            }
            case SEGV_ACCERR: {
                Utils::printString("Reason: Invalid rights to access the object\n");
                break;
            }
            default: {
                Utils::printString("Unknown reason!!! WTF!!!\n");
                _exit(EXIT_FAILURE);
            }
        }

        dumpMemory(siginfo->si_addr);
        dumpRegisters(reinterpret_cast<ucontext_t *> (context));

    } else {
        Utils::printString("Signal handle by default behavior\n");
    }

    _exit(EXIT_FAILURE);
}
