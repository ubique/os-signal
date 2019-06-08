//
// Created by rsbat on 5/21/19.
//

#ifndef OS_SIGNAL_SIGSEGV_HANDLER_H
#define OS_SIGNAL_SIGSEGV_HANDLER_H

#include <bits/types/siginfo_t.h>
#include <signal.h>
#include <sys/ucontext.h>
#include <string>
#include <vector>
#include <iostream>
#include <csetjmp>
#include <iomanip>
#include <limits>

static const char* registers[] = {"R8",
                                  "R9",
                                  "R10",
                                  "R11",
                                  "R12",
                                  "R13",
                                  "R14",
                                  "R15",
                                  "RDI",
                                  "RSI",
                                  "RBP",
                                  "RBX",
                                  "RDX",
                                  "RAX",
                                  "RCX",
                                  "RSP",
                                  "RIP",
                                  "EFL",
                                  "CSGSFS",
                                  "ERR",
                                  "TRAPNO",
                                  "OLDMASK",
                                  "CR2"};

static const size_t DUMP_RANGE = 16;

static jmp_buf buf;

void print_address(size_t address) {
    std::cerr << "0x" << std::setw(16) << std::setfill('0') << std::right << address;
}

void noop_handler(int signum, siginfo_t* info, void* context) {
    longjmp(buf, 1);
}

void sigsegv_handler(int sig_num, siginfo_t* info, void* context) {
    auto* ucontext = reinterpret_cast<ucontext_t*>(context);
    mcontext_t mcontext = ucontext->uc_mcontext;
    greg_t* regs = mcontext.gregs;

    // not using std::showbase because of std::uppercase
    std::cerr << std::hex << std::uppercase;
    auto address = reinterpret_cast<size_t>(info->si_addr);

    std::cerr << "Memory access violation at ";
    print_address(address);
    std::cerr << std::endl;

    if (info->si_code == SEGV_MAPERR) {
        std::cerr << "Address not mapped to object";
    } else if (info->si_code == SEGV_ACCERR) {
        std::cerr << "Invalid permissions for mapped object";
    } else if (info->si_code == SEGV_BNDERR) {
        std::cerr << "Failed address bound checks";
    } else if (info->si_code == SEGV_PKUERR) {
        std::cerr << "Access was denied by memory protection keys";
    } else {
        std::cerr << "Reason is unknown";
    }
    std::cerr << std::endl << std::endl;

    
    // dump registers
    std::cerr << "Registers:" << std::endl;
    for (int i = 0; i < NGREG; i++) {
        std::cerr << std::setw(10) << std::setfill(' ') << std::left << registers[i] << ": ";
        print_address(regs[i]);
        std::cerr << std::endl;
    }
    std::cerr << std::endl;


    // dump memory
    struct sigaction act{};
    act.sa_flags = SA_SIGINFO | SA_NODEFER;
    act.sa_handler = reinterpret_cast<__sighandler_t>(noop_handler);

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGSEGV);
    sigprocmask(SIG_UNBLOCK, &set, nullptr);

    sigaction(SIGSEGV, &act, nullptr);

    char* ptr = reinterpret_cast<char*>(info->si_addr);
    char* begin = ptr - DUMP_RANGE < ptr ? ptr - DUMP_RANGE : nullptr;
    char* end = ptr + DUMP_RANGE > ptr ? ptr + DUMP_RANGE : std::numeric_limits<char*>::max();

    std::cerr << "Memory from ";
    print_address(reinterpret_cast<size_t>(begin));
    std::cerr << " to ";
    print_address(reinterpret_cast<size_t>(end));
    std::cerr << std::endl;

    for (char* i = begin; i <= end; i++) {
        if (setjmp(buf) != 0) {
            std::cerr << "??";
        } else {
            char ch = *i;
            std::cerr << std::setw(2) << std::setfill('0') << (static_cast<uint32_t>(ch) & 0xFFu);
        }
        std::cerr << ' ';
    }
    std::cerr << std::endl;

    for (char* i = begin; i < ptr; i++) {
        std::cerr << "   ";
    }
    std::cerr << "^^" << std::endl;


    exit(EXIT_FAILURE);
}

#endif //OS_SIGNAL_SIGSEGV_HANDLER_H
