//
// Created by anastasia on 09.06.19.
//

#include <cstring>
#include <csetjmp>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <gmpxx.h>
#include <map>

const std::map<std::string, int> registers = {{"R8",      REG_R8},
                                              {"R9",      REG_R9},
                                              {"R10",     REG_R10},
                                              {"R11",     REG_R11},
                                              {"R12",     REG_R12},
                                              {"R13",     REG_R13},
                                              {"R14",     REG_R14},
                                              {"R15",     REG_R15},
                                              {"RAX",     REG_RAX},
                                              {"RBP",     REG_RBP},
                                              {"RBX",     REG_RBX},
                                              {"RCX",     REG_RCX},
                                              {"RDI",     REG_RDI},
                                              {"RDX",     REG_RDX},
                                              {"RIP",     REG_RIP},
                                              {"RSI",     REG_RSI},
                                              {"RSP",     REG_RSP},
                                              {"CR2",     REG_CR2},
                                              {"CSGSFS",  REG_CSGSFS},
                                              {"EFL",     REG_EFL},
                                              {"ERR",     REG_ERR},
                                              {"OLDMASK", REG_OLDMASK},
                                              {"TRAPNO",  REG_TRAPNO}};

const int NUM = 8;

static jmp_buf jmp;
static void jmp_handler() {
    longjmp(jmp, 1);
}
static void sigsegv_handler(int s, siginfo_t *siginfo, void *context) {
    if (siginfo->si_signo == SIGSEGV) {
        std::cout << "registers:" << std::endl;
        auto *ucontext = (ucontext_t *) context;
        for (auto &reg: registers) {
            std::cout << "reg[" << reg.first << "] = " << std::hex << "0x" << static_cast<int>(ucontext->uc_mcontext.gregs[reg.second]) << std::endl;
        }

        struct sigaction act{};
        act.sa_flags = SA_NODEFER;
        act.sa_handler = reinterpret_cast<__sighandler_t>(&jmp_handler);
        if (sigaction(SIGSEGV, &act, nullptr) < 0) {
            perror("Cannot change signal action");
            exit(EXIT_FAILURE);
        }

        std::cout << "memory:" << std::endl;
        auto *addr = (char *) siginfo->si_addr;
        char *maxx = std::numeric_limits<char *>::max();
        char *left, *right;
        if (addr > (char*) NUM)
            left = addr - NUM;
        else
            left = nullptr;
        if (addr < maxx - NUM)
            right = addr + NUM;
        else
            right = maxx;
        for (char *i = left; i <= right; i++) {
            act.sa_flags = SA_NODEFER;
            act.sa_handler = reinterpret_cast<__sighandler_t>(&jmp_handler);
            if (sigaction(SIGSEGV, &act, nullptr) < 0) {
                perror("Cannot change signal action");
                exit(EXIT_FAILURE);
            }
            if (setjmp(jmp)) {
                std::cerr << "Can't read memory" << std::endl;
            } else {
                if (i == addr) {
                    std::cout << "here: ";
                }
                std::cout << std::hex << (void *)i << ' ' << (int) *i << std::endl;
            }
        }
        exit(EXIT_FAILURE);
    }
}

int main() {
    struct sigaction act{};
    memset(&act, 0, sizeof(act));
    act.sa_flags = SA_SIGINFO;
    if (sigemptyset(&act.sa_mask) == -1) {
        perror( "Can't make empty set of signals");
        return 0;
    }
    act.sa_sigaction = &sigsegv_handler;
    if (sigaction(SIGSEGV, &act, nullptr)== -1) {
        perror("Can't change signal action");
    }

    return 0;
};