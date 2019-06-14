//
// Created by rsbat on 5/21/19.
//

#ifndef OS_SIGNAL_SIGSEGV_HANDLER_H
#define OS_SIGNAL_SIGSEGV_HANDLER_H

#include <signal.h>
#include <sys/ucontext.h>
#include <iomanip>
#include <limits>
#include <unistd.h>
#include <cstring>

namespace {

    const char *registers[] = {"R8",
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

    const size_t DUMP_RANGE = 16;

    void print_char(char x) {
        write(1, &x, 1);
    }

    void print_byte(char x) {
        char hi = static_cast<uint8_t>(x) / 16;
        char lo = static_cast<uint8_t>(x) % 16;
        print_char(hi < 10  ? '0' + hi : 'A' + hi - 10);
        print_char(lo < 10  ? '0' + lo : 'A' + lo - 10);
    }

    void print_string(const char* s) {
        write(1, s, strlen(s));
    }

    void print_address(size_t address) {
        print_string("0x");
        for (int i = 14; i >= 0; i -= 2) {
            print_byte((address >> i) & 0xFF);
        }
    }
}

void sigsegv_handler(int sig_num, siginfo_t* info, void* context) {
    auto* ucontext = reinterpret_cast<ucontext_t*>(context);
    mcontext_t mcontext = ucontext->uc_mcontext;
    greg_t* regs = mcontext.gregs;

    auto address = reinterpret_cast<size_t>(info->si_addr);

    print_string("Memory access violation at ");
    print_address(address);
    print_char('\n');

    if (info->si_code == SEGV_MAPERR) {
        print_string("Address not mapped to object\n\n");
    } else if (info->si_code == SEGV_ACCERR) {
        print_string("Invalid permissions for mapped object\n\n");
    } else if (info->si_code == SEGV_BNDERR) {
        print_string("Failed address bound checks\n\n");
    } else if (info->si_code == SEGV_PKUERR) {
        print_string("Access was denied by memory protection keys\n\n");
    } else {
        print_string("Reason is unknown\n\n");
    }
    
    // dump registers
    print_string("Registers:\n");
    for (int i = 0; i < NGREG; i++) {
        print_string(registers[i]);
        print_string(": ");
        print_address(regs[i]);
        print_char('\n');
    }
    print_char('\n');

    // dump memory
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("Could not create pipe");
        exit(EXIT_FAILURE);
    }

    char* ptr = reinterpret_cast<char*>(info->si_addr);
    char* begin = ptr - DUMP_RANGE < ptr ? ptr - DUMP_RANGE : nullptr;
    char* end = ptr + DUMP_RANGE > ptr ? ptr + DUMP_RANGE : std::numeric_limits<char*>::max();

    print_string("Memory from ");
    print_address(reinterpret_cast<size_t>(begin));
    print_string(" to ");
    print_address(reinterpret_cast<size_t>(end));
    print_char('\n');

    for (char* i = begin; i <= end; i++) {
        char ch;
        if (write(pipefd[1], i, 1) == -1) {
            print_string("??");
        } else if (read(pipefd[0], &ch, 1) == -1) {
            print_string("??");
        } else {
            print_byte(ch);
        }

        print_char(' ');
    }
    print_char('\n');

    for (char* i = begin; i < ptr; i++) {
        print_string("   ");
    }
    print_string("^^\n");


    exit(EXIT_FAILURE);
}

#endif //OS_SIGNAL_SIGSEGV_HANDLER_H