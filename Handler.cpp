//
// Created by max on 23.05.19.
//

#include <iostream>
#include <memory.h>
#include <iomanip>
#include <signal.h>
#include <csetjmp>
#include <limits>

#include "Handler.h"
#include "helper.h"

const std::vector<std::pair<std::string, long long>> Handler::registers{
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

Handler::Handler(Handler::foo function) {
//    Printer::println(std::cout, "Handler::handler");
    sigact.sa_flags = SA_SIGINFO;
    sigact.sa_sigaction = function;
    sigset_t sigset1{};
    sigemptyset(&sigset1);
    sigaddset(&sigset1, SIGSEGV);
    sigact.sa_mask = sigset1;
    checker(sigaction(SIGSEGV, &sigact, nullptr), "Sigaction failed");

}


jmp_buf jmpBuf;


void m_hnd(UNUSED int sig, siginfo_t *siginfo, UNUSED void *ucontext) {
    if (siginfo->si_signo == SIGSEGV) {
        siglongjmp(jmpBuf, 1);
    }
}

void hnd(int sig, siginfo_t *siginfo, void *ucontext) {
    std::cout << "Handler start" << std::endl;
    auto address = siginfo->si_addr;
    std::cout << strsignal(sig) << " on: " << address << std::endl;
    std::cout << "General purpose registers:" << std::endl;
    for (const auto &aaa : Handler::registers) {
        std::cout << std::setfill(' ') << std::setw(7) << aaa.first << " " << std::hex << std::setfill('0')
                  << std::setw(sizeof(greg_t) * 8 / 4)
                  << reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[aaa.second] << std::endl;
    }

    if (address == nullptr) {
        std::cout << "nullptr" << std::endl;
        exit(EXIT_FAILURE);
    }
    Printer::println(std::cout, "Not null");

    auto pointer = reinterpret_cast<size_t >(address);

    size_t size = 30;
    size_t left_border = (pointer > size ? pointer - size : 0);
    size_t max = std::numeric_limits<size_t>::max();
    size_t right_border = (max - pointer < size ? max : pointer + size);


    sigset_t sset;
    sigemptyset(&sset);
    sigaddset(&sset, SIGSEGV);
    Printer::println(std::cout, "sigset_t set");
    std::cout << "Range: " << left_border << " | " << right_border << std::endl;
    for (size_t ptr = left_border; ptr < right_border; ptr += sizeof(char)) {
        sigprocmask(SIG_UNBLOCK, &sset, nullptr);
        Handler handler{m_hnd};
        if (ptr == pointer) {
            std::cout << "[!!] ";
        } else if (setjmp(jmpBuf) != 0) {
            std::cout << "!! ";
        } else {
            std::cout << std::hex << (int) *reinterpret_cast<char *>(ptr) << " ";
        }
    }

    std::cout << std::endl;
    exit(EXIT_FAILURE);
}


