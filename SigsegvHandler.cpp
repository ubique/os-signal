//
// Created by daniil on 09.06.19.
//

#include <limits>
#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <bits/signum.h>
#include <iostream>
#include "SigsegvHandler.h"
#include <map>
#include <string.h>

jmp_buf jmp;

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

void SigsegvHandler::myHandle(int sig, siginfo_t *siginfo, void *ucontext) {
    if (siginfo->si_signo == SIGSEGV) {
        siglongjmp(jmp, 1);
    }
}

void SigsegvHandler::handle(int s, siginfo_t *siginfo, void *context) {
    if (siginfo->si_signo == SIGSEGV) {
        std::cout << strsignal(s) << " on " << siginfo->si_addr << std::endl;
        std::cout << "___REGISTERS___" << std::endl;
        ucontext_t *ucontext = (ucontext_t *) context;
        for (auto &reg: registers) {
            std::cout << reg.first << " = " << ucontext->uc_mcontext.gregs[reg.second] << std::endl;
        }

        std::cout << "___MEMORY___" << std::endl;
        char *addr = (char *) siginfo->si_addr;
        char *maxc = std::numeric_limits<char *>::max();
        char *left, *right;
        if (addr > (char*) NUM) left = addr - NUM;
        else left = NULL;
        if (addr < maxc - NUM) right = addr + NUM;
        else right = maxc;
        sigset_t sset;
        sigemptyset(&sset);
        sigaddset(&sset, SIGSEGV);
        for (char *i = left; i <= right; i++) {
            sigprocmask(SIG_UNBLOCK, &sset, nullptr);
            struct sigaction act{};
            act.sa_flags = SA_SIGINFO;
            act.sa_sigaction = &myHandle;
            sigset_t sigset1{};
            sigemptyset(&sigset1);
            sigaddset(&sigset1, SIGSEGV);
            act.sa_mask = sigset1;
            if (sigaction(SIGSEGV, &act, NULL) < 0) {
                perror("Cannot change signal action");
                exit(EXIT_FAILURE);
            }
            if (setjmp(jmp)) {
                std::cout << "can't read memory" << std::endl;
            } else {
                if (i == addr) {
                    std::cout << "[";
                }
                std::cout << (int) *i;
                if (i == addr) {
                    std::cout << "]";
                }
                std::cout << std::endl;
            }
        }
        exit(EXIT_FAILURE);
    }
}

int SigsegvHandler::setSigsegv() {
    struct sigaction act{};
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &handle;
    sigset_t sigset1{};
    sigemptyset(&sigset1);
    sigaddset(&sigset1, SIGSEGV);
    act.sa_mask = sigset1;
    if (sigaction(SIGSEGV, &act, NULL) < 0) {
        perror("Can't change signal action");
        return 1;
    }
    return 0;
}

