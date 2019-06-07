#include <sys/ucontext.h>
#include <bits/types/siginfo_t.h>
#include <iostream>
#include <memory.h>
#include <limits>
#include "handler.h"

static jmp_buf jmpbuf;

const std::vector<std::pair<std::string, int>> registers{
        {"R8",      REG_R8},
        {"R9",      REG_R9},
        {"R10",     REG_R10},
        {"R11",     REG_R11},
        {"R12",     REG_R12},
        {"R13",     REG_R13},
        {"R14",     REG_R14},
        {"R15",     REG_R15},
        {"RAX",     REG_RAX},
        {"RBP",     REG_RBP},
        {"RSP",     REG_RSP},
        {"RSI",     REG_RSI},
        {"RIP",     REG_RIP},
        {"RDX",     REG_RDX},
        {"RDI",     REG_RDI},
        {"RCX",     REG_RCX},
        {"RBX",     REG_RBX},
        {"TRAPNO",  REG_TRAPNO},
        {"OLDMASK", REG_OLDMASK},
        {"CSGSFS",  REG_CSGSFS},
        {"ERR",     REG_ERR},
        {"EFL",     REG_EFL},
        {"CR2",     REG_CR2},
};

handler::handler(void (*foo)(int, siginfo_t *, void *)) {
    struct sigaction act{};
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = foo;
    sigset_t sig;
    sigemptyset(&sig);
    sigaddset(&sig, SIGSEGV);
    act.sa_mask = sig;

    if (sigaction(SIGSEGV, &act, nullptr) == -1) {
        perror("");
    }

}

void help_handler(int sig, siginfo_t *siginfo, void *context) {
    if (siginfo->si_signo == SIGSEGV) {
        siglongjmp(jmpbuf, 1);
    }
}

void hand(int sig, siginfo_t *siginfo, void *context) {
    std::cout << strsignal(sig) << "on address: " << siginfo->si_addr << "\n";
    std::cout << "general purpose registers:" << "\n";
    for (auto &reg : registers) {
        std::cout << reg.first << ": " << reinterpret_cast<ucontext_t *>(context)->uc_mcontext.gregs[reg.second]
                  << "\n";
    }
    if (siginfo->si_addr == nullptr) {
        std::cout << "nullptr" << "\n";
        exit(EXIT_FAILURE);
    }
    size_t pointer = reinterpret_cast<size_t >(siginfo->si_addr);

    const size_t size = 20;
    size_t left = pointer > size ? pointer - size : 0;
    size_t max = std::numeric_limits<size_t>::max();
    size_t right = max - pointer < size ? max : pointer + size;

    sigset_t sset;
    sigemptyset(&sset);
    sigaddset(&sset, SIGSEGV);

    std::cout << "from " << left << " to " << right << " \n";
    for (size_t i = left; i < right; i += sizeof(char)) {
        sigprocmask(SIG_UNBLOCK, &sset, nullptr);
        handler handler1(&help_handler);
        if (i == pointer) {
            std::cout << " !! ";
        } else if (setjmp(jmpbuf) != 0) {
            std::cout << " ?? ";
        } else {
            std::cout << " " << *reinterpret_cast<char *>(i) << " ";
        }
    }
    exit(EXIT_FAILURE);

}