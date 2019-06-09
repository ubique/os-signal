//
// Created by vitalya on 09.06.19.
//

#include "sigsegv_handler.h"

//#include <signal.h>
#include <csetjmp>
#include <iostream>
#include <map>
#include <memory.h>
#include <sys/ucontext.h>
#include <string>
#include <signal.h>
#include <limits>

static jmp_buf jmpbuf;

static constexpr const size_t offset = 16; /* bytes */
static constexpr size_t MAX_MEM = std::numeric_limits<size_t>::max();

static const std::map<std::string, int> registers = {
        {"CR2",     REG_CR2},
        {"ERR",     REG_ERR},
        {"CSGSFS",  REG_CSGSFS},
        {"EFL",     REG_EFL},
        {"OLDMASK", REG_OLDMASK},
        {"RAX",     REG_RAX},
        {"RBX",     REG_RBX},
        {"RCX",     REG_RCX},
        {"RDX",     REG_RDX},
        {"RDI",     REG_RDI},
        {"RBP",     REG_RBP},
        {"RIP",     REG_RIP},
        {"RSP",     REG_RSP},
        {"RSI",     REG_RSI},
        {"R8",      REG_R8},
        {"R9",      REG_R9},
        {"R10",     REG_R10},
        {"R11",     REG_R11},
        {"R12",     REG_R12},
        {"R13",     REG_R13},
        {"R14",     REG_R14},
        {"R15",     REG_R15},
        {"TRAPNO",  REG_TRAPNO},
};

static void handle1(int sig, siginfo_t *siginfo, void *context) {
    longjmp(jmpbuf, 1);
}

void handle(int sig, siginfo_t* siginfo, void* context) {
    if (siginfo->si_signo != SIGSEGV) {
        return;
    }
    std::cout << strsignal(sig) << "on address: " << siginfo->si_addr << std::endl;
    std::cout << "general registers:" << std::endl;
    auto ucontext = reinterpret_cast<ucontext_t *>(context);
    for (auto& reg : registers) {
        std::cout << reg.first << ": " << ucontext->uc_mcontext.gregs[reg.second] << std::endl;
    }

    size_t address = reinterpret_cast<size_t>(siginfo->si_addr);
    size_t left_bound = address > offset ? address - offset : 0;
    size_t right_bound = address < MAX_MEM - offset ? address + offset : MAX_MEM;

    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGSEGV);

    std::cout << "memory dump from " << left_bound << " to " << right_bound << ": " << std::endl;
    for (size_t i = left_bound; i < right_bound; ++i) {
        sigprocmask(SIG_UNBLOCK, &sigset, nullptr);
        sigsegv_handler handler(handle1);

        char *ptr = reinterpret_cast<char*>(i);
        if (setjmp(jmpbuf)) {
            std::cout << "?? ";
        } else if (i == address) {
            std::cout << "[HERE] ";
        } else {
            std::cout << std::hex << std::uppercase << int(*ptr) << " ";
        }
    }
    std::cout << std::endl;
    exit(EXIT_FAILURE);
}

sigsegv_handler::sigsegv_handler(void (*handle_func) (int, siginfo_t*, void*)) {
    struct sigaction sig_act{};
    sig_act.sa_sigaction = handle_func;
    sig_act.sa_flags = (SA_SIGINFO | SA_NOMASK);

    sigset_t sig;
    sigemptyset(&sig);
    sigaddset(&sig, SIGSEGV);
    sig_act.sa_mask = sig;

    if (sigaction(SIGSEGV, &sig_act, nullptr) == -1) {
        perror("Unable to handle signal");
    }
}
