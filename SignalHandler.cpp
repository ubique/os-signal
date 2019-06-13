#include <unistd.h>
#include <cstring>
#include <climits>
#include <csetjmp>
#include <sys/ucontext.h>
#include <cstdlib>

#include "SignalHandler.h"
#include "SafeWriter.hpp"

using namespace SafeWriter;

const std::vector<std::pair<const char*, int>> SignalHandler::REGISTERS = {
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


void SignalHandler::printErr(const char* message) {
    safeWrite(message);
}


void SignalHandler::dumpAddress(int pip[], char* address, int i) {
    safeWrite(" ");
    uint8_t value;
    char* cur = address + i;

    if (cur == address) {
        safeWrite("> ");
    }

    if (write(pip[1], cur, 1) < 0 || read(pip[0], &value, 1) < 0) {
        safeWrite("Unknown");
    } else {
        safeWrite(value);
    }

    safeWrite("\n");
}

void SignalHandler::dumpMemory(char* address) {
    int pip[2];
    if (pipe(pip) < 0) {
        printErr("Unable to pipe");
        _exit(EXIT_FAILURE);
    }

    safeWrite("\nMemory nearby:\n");
    if (address == nullptr) {
        safeWrite(" NULL\n");
        _exit(EXIT_FAILURE);
    }

    for (int i = -MEMORY_RANGE; i <= MEMORY_RANGE; ++i) {
        dumpAddress(pip, address, i);
    }
}

void SignalHandler::dumpRegisters(ucontext_t* ucontext) {
    safeWrite("\nGeneral purposes registers:\n");
    greg_t* gregs = ucontext->uc_mcontext.gregs;

    for (const auto& reg : REGISTERS) {
        safeWrite(" ");
        safeWrite(reg.first);
        safeWrite(": ");
        safeWrite(gregs[reg.second]);
        safeWrite("\n");
    }
}

void SignalHandler::handler(int signo, siginfo_t* siginfo, void* context) {
    if (siginfo->si_signo == SIGSEGV) {
        safeWrite("Signal aborted: ");
        safeWrite(strsignal(signo));
        safeWrite("\n");

        switch (siginfo->si_code) {
            case SEGV_MAPERR: {
                safeWrite("\nCause: Address is not mapped to object\n");
                break;
            }
            case SEGV_ACCERR: {
                safeWrite("\nCause: Invalid permission\n");
                break;
            }
            default:
                safeWrite("\nCause: Something bad happen\n");
        }

        auto address = reinterpret_cast<intptr_t>(siginfo->si_addr);

        safeWrite("Address: ");
        safeWrite(address);
        safeWrite("\n");

        dumpRegisters(reinterpret_cast<ucontext_t*> (context));
        dumpMemory(static_cast<char*>(siginfo->si_addr));
        _exit(EXIT_SUCCESS);
    }
    _exit(EXIT_FAILURE);
}