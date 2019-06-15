//
// Created by Павел Пономарев on 2019-06-06.
//

#include "Handler.h"
#include <iostream>
#include <cmath>
#include <climits>
#include <string.h>

namespace Writer {
    void partialWrite(uint8_t n) {
        char c = n < 10 ? n + '0' : n + 'A' - 10;
        write(1, &c, 1);
    }

    void writeByte(uint8_t byte) {
        partialWrite(byte / 16);
        partialWrite(byte % 16);
    }

    void writeSafe(const char* chars) {
        write(1, chars, strlen(chars));
    }

    void writeSafe(uint64_t num) {
        for (int i = 7; i >= 0; --i) {
            writeByte(0xFF && (num >> (8 * i)));
        }
    }

    void writeSafeRegister(const char* reg, greg_t greg) {
        writeSafe(reg);
        writeSafe(" --- ");
        writeSafe(greg);
        writeSafe("\n");
    }
};



void Handler::handleSignal(int signum, siginfo_t* info, void* context) {
    if (info->si_signo == SIGSEGV) {
        Writer::writeSafe("Aborted: ");
        Writer::writeSafe(strsignal(signum));
        Writer::writeSafe("\n");
        switch (info->si_code) {
            case SEGV_ACCERR:
                Writer::writeSafe("SEGV_ACCERR\n");
                break;
            case SEGV_MAPERR:
                Writer::writeSafe("SEGV_MAPPER\n");
                break;
            default:
                Writer::writeSafe("Unknown si_code\n");

        }

        Writer::writeSafe("Dumping registers...\n");
        auto rContext = reinterpret_cast<ucontext_t*>(context);
        dumpRegisters(rContext);
        Writer::writeSafe("Registers dumped.\n");

        Writer::writeSafe("Dumping memory...\n");
        dumpMemory(info->si_addr);
        Writer::writeSafe("Memory dumped.\n");
    }
    _exit(EXIT_FAILURE);
}

void Handler::dumpRegisters(ucontext_t* context) {
    Writer::writeSafeRegister("R8", context->uc_mcontext.gregs[REG_R8]);
    Writer::writeSafeRegister("R9", context->uc_mcontext.gregs[REG_R9]);
    Writer::writeSafeRegister("R10", context->uc_mcontext.gregs[REG_R10]);
    Writer::writeSafeRegister("R11", context->uc_mcontext.gregs[REG_R11]);
    Writer::writeSafeRegister("R12", context->uc_mcontext.gregs[REG_R12]);
    Writer::writeSafeRegister("R13", context->uc_mcontext.gregs[REG_R13]);
    Writer::writeSafeRegister("R14", context->uc_mcontext.gregs[REG_R14]);
    Writer::writeSafeRegister("R15", context->uc_mcontext.gregs[REG_R15]);
    Writer::writeSafeRegister("RDI", context->uc_mcontext.gregs[REG_RDI]);
    Writer::writeSafeRegister("RSI", context->uc_mcontext.gregs[REG_RSI]);
    Writer::writeSafeRegister("RBP", context->uc_mcontext.gregs[REG_RBP]);
    Writer::writeSafeRegister("RBX", context->uc_mcontext.gregs[REG_RBX]);
    Writer::writeSafeRegister("RDX", context->uc_mcontext.gregs[REG_RDX]);
    Writer::writeSafeRegister("RAX", context->uc_mcontext.gregs[REG_RAX]);
    Writer::writeSafeRegister("RCX", context->uc_mcontext.gregs[REG_RCX]);
    Writer::writeSafeRegister("RSP", context->uc_mcontext.gregs[REG_RSP]);
    Writer::writeSafeRegister("RIP", context->uc_mcontext.gregs[REG_RIP]);
    Writer::writeSafeRegister("EFL", context->uc_mcontext.gregs[REG_EFL]);
    Writer::writeSafeRegister("CSGSFS", context->uc_mcontext.gregs[REG_CSGSFS]);
    Writer::writeSafeRegister("ERR", context->uc_mcontext.gregs[REG_ERR]);
    Writer::writeSafeRegister("TRAPNO", context->uc_mcontext.gregs[REG_TRAPNO]);
    Writer::writeSafeRegister("OLDMASK", context->uc_mcontext.gregs[REG_OLDMASK]);
    Writer::writeSafeRegister("CR2", context->uc_mcontext.gregs[REG_CR2]);
}

void Handler::dumpMemory(void* address) {
    int pipefd[2];
    uint8_t buffer;
    const auto p = pipe(pipefd);
    if (p == -1) {
        Writer::writeSafe("Can't pipe\n");
        _exit(EXIT_FAILURE);
    }
    if (address == nullptr) {
        Writer::writeSafe("address is nullptr\n");
        _exit(EXIT_FAILURE);
    }

    for (int i = -MEMORY_SIZE; i < MEMORY_SIZE; ++i) {
        uint8_t n;
        char* t = ((char*)address) + i;
        if (t == address) {
            Writer::writeSafe("--> ");
        }

        if (write(pipefd[1], t, 1) < 0 || read(pipefd[0], &buffer, 1) < 0) {
            Writer::writeSafe("????");
        } else {
            Writer::writeSafe(n);
        }
        Writer::writeSafe("\n");


    }

}
