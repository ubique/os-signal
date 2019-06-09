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

constexpr const size_t offset = 16; /* bytes */
constexpr char* MAX_MEM = std::numeric_limits<char*>::max();

const std::map<std::string, int> registers = {
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

namespace {
    void handle1(int sig) {
        longjmp(jmpbuf, 1);
    }
}

void handle(int sig, siginfo_t* siginfo, void* context) {
    if (siginfo->si_signo != SIGSEGV) {
        return;
    }
    std::cout << strsignal(sig) << "on address: " << siginfo->si_addr << std::endl;
    std::cout << "GENERAL REGISTERS:" << std::endl;
    auto ucontext = reinterpret_cast<ucontext_t *>(context);
    for (auto& reg : registers) {
        std::cout << reg.first << ": " << ucontext->uc_mcontext.gregs[reg.second] << std::endl;
    }

    char* address = static_cast<char*>(siginfo->si_addr);
    char* left_bound = address > (char*)offset ? address - offset : nullptr;
    char* right_bound = address < MAX_MEM - offset ? address + offset : MAX_MEM;

    std::cout << "MEMORY DUMP:" << std::endl;
    for (char* ptr = left_bound; ptr < right_bound; ++ptr) {
        struct sigaction sig_act{};
        sig_act.sa_flags = SA_NOMASK;
        sig_act.sa_handler = &handle1;

        if (sigaction(SIGSEGV, &sig_act, NULL) < 0) {
            perror("Unable to change signal action");
            exit(EXIT_FAILURE);
        }
        if (setjmp(jmpbuf)) {
            std::cerr << "Unable to read nearby memory" << std::endl;
        } else {
            if (ptr == address) {
                std::cout << "[";
            }
            std::cout << (int)*ptr;
            if (ptr == address) {
                std::cout << "]";
            }
            std::cout << std::endl;
        }
    }
    exit(EXIT_FAILURE);
}

sigsegv_handler::sigsegv_handler() {
    struct sigaction sig_act{};
    sig_act.sa_sigaction = &handle;
    sig_act.sa_flags = (SA_SIGINFO | SA_NOMASK);

    sigset_t sig;
    sigemptyset(&sig);
    sigaddset(&sig, SIGSEGV);
    sig_act.sa_mask = sig;

    if (sigaction(SIGSEGV, &sig_act, nullptr) == -1) {
        perror("Unable to handle signal");
    }
}
