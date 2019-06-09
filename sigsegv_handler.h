//
// Created by Anton Shelepov on 2019-06-09.
//

#ifndef OS_SIGNAL_SIGSEGV_HANDLER_H
#define OS_SIGNAL_SIGSEGV_HANDLER_H

#include <sys/param.h>
#include <sys/ucontext.h>
#include <iostream>
#include <iomanip>
#include <zconf.h>
#include <limits>


static const size_t RADIUS = 16;

static inline void print_address(size_t address) {
    std::cerr << "0x" << std::setw(16) << std::setfill('0') << std::right << address;
}

static inline void print_register(std::string const& name, size_t reg) {
    std::cerr << name << ": ";
    print_address(reg);
    std::cerr << std::endl;
}

static inline std::string get_cause(siginfo_t* info) {
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
    std::cerr << "General purpose registers dump: " << std::endl;
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
        std::cerr << "Couldn't dump memory" << std::endl;
        return;
    }

    auto ptr = reinterpret_cast<char*>(address);
    auto begin = reinterpret_cast<char*>(address >= RADIUS ? address - RADIUS : 0);
    auto end = reinterpret_cast<char*>(std::numeric_limits<size_t>::max() - RADIUS >= address
            ? address + RADIUS : std::numeric_limits<size_t>::max());

    std::cerr << "Memory dump: " << std::endl;
    for (auto i = begin; i < end; i++) {
        if (write(pipefd[1], i, 1) != -1) {
            std::cerr << "0x" << std::setw(2) << std::setfill('0') << (static_cast<uint32_t>(*i) & 0xFFu);
        } else {
            std::cerr << "??";
        }
        std::cerr << ' ';
    }
    std::cerr << std::endl;
    for (auto i = begin; i < ptr; i++) {
        if (write(pipefd[1], i, 1) != -1) {
            std::cerr << "     ";
        } else {
            std::cerr << "   ";
        }
    }
    std::cerr << "^ Memory violation";
}

static inline  void sigsegv_handler(int sig, siginfo_t* info, void* context) {
    auto address = reinterpret_cast<size_t>(info->si_addr);
    std::cerr << std::hex;

    std::cerr << "Memory violation occurred at address: ";
    print_address(address);
    std::cerr << std::endl;

    std::cerr << "Cause: " << get_cause(info) << std::endl;

    dump_registers(context);
    dump_memory(address);

    exit(EXIT_FAILURE);
}

#endif //OS_SIGNAL_SIGSEGV_HANDLER_H
