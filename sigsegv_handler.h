//
// Created by Anton Shelepov on 2019-06-09.
//

#ifndef OS_SIGNAL_SIGSEGV_HANDLER_H
#define OS_SIGNAL_SIGSEGV_HANDLER_H

#include <sys/param.h>
#include <sys/ucontext.h>
#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <limits>


static const size_t RADIUS = 16;


void print_string(const char* str) {
    write(STDERR_FILENO, str, strlen(str));
}

static inline void normalize_hex(size_t hex, size_t width, char* buf) {
    buf[0] = '0';
    buf[1] = 'x';
    for (size_t i = width + 1; i >= 2; i--) {
        uint8_t val = hex & 0xf;
        char dig;
        if (val < 10) {
            dig = '0' + val;
        } else {
            dig = 'a' + (val - 10);
        }
        buf[i] = dig;
        hex >>= 4;
    }
    buf[2 + width] = '\0';
}

static inline void print_address(size_t address) {
    char buf[128];
    normalize_hex(address, 16, buf);
    print_string(buf);
}

static inline void print_register(const char* name, size_t reg) {
    print_string(name);
    print_string(": ");
    print_address(reg);
    print_string("\n");
}

static inline const char* get_cause(siginfo_t* info) {
    if (info->si_code == SEGV_MAPERR) {
        return "Address not mapped to object";
    }
    if (info->si_code == SEGV_ACCERR) {
        return "Invalid permissions for mapped object";
    }
    if (info->si_code == SEGV_BNDERR) {
        return "Failed address bound checks";
    }
    if (info->si_code == SEGV_PKUERR) {
        return "Access was denied by memory protection keys";
    }

    return "Undefined";
}

static inline void dump_registers(void* context) {
    print_string("General purpose registers dump: \n");
    auto ucontext = reinterpret_cast<ucontext_t*>(context);
    greg_t* regs = ucontext->uc_mcontext.gregs;

    print_register("RAX", regs[REG_RAX]);
    print_register("RBX", regs[REG_RBX]);
    print_register("RDX", regs[REG_RDX]);
    print_register("RBP", regs[REG_RBP]);
    print_register("RSP", regs[REG_RSP]);
    print_register("RDI", regs[REG_RDI]);
    print_register("RSI", regs[REG_RSI]);

    print_register("R9", regs[REG_R9]);
    print_register("R10", regs[REG_R10]);
    print_register("R11", regs[REG_R11]);
    print_register("R12", regs[REG_R12]);
    print_register("R13", regs[REG_R13]);
    print_register("R14", regs[REG_R14]);
    print_register("R15", regs[REG_R15]);
}

static inline void dump_memory(size_t address) {
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        print_string("Couldn't dump memory\n");
        return;
    }

    auto ptr = reinterpret_cast<char*>(address);
    auto begin = reinterpret_cast<char*>(address >= RADIUS ? address - RADIUS : 0);
    auto end = reinterpret_cast<char*>(std::numeric_limits<size_t>::max() - RADIUS >= address
                                       ? address + RADIUS : std::numeric_limits<size_t>::max());

    print_string("Memory dump: \n");
    char buffer[128];
    for (auto i = begin; i < end; i++) {
        if (write(pipefd[1], i, 1) != -1) {
            normalize_hex((static_cast<uint32_t>(*i) & 0xFFu), 2, buffer);
            print_string(buffer);
        } else {
            print_string("????");
        }
        print_string(" ");
    }
    print_string("\n");
    for (auto i = begin; i < ptr; i++) {
        print_string("     ");
    }
    print_string("^ Memory violation");
}

static inline  void sigsegv_handler(int sig, siginfo_t* info, void* context) {
    auto address = reinterpret_cast<size_t>(info->si_addr);
    print_string("Memory violation occurred at address: ");
    print_address(address);
    print_string("\n");

    print_string("Cause: ");
    print_string(get_cause(info));
    print_string("\n");

    dump_registers(context);
    dump_memory(address);

    exit(EXIT_FAILURE);
}

#endif //OS_SIGNAL_SIGSEGV_HANDLER_H

