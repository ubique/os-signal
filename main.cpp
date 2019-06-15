//
// Created by anastasia on 09.06.19.
//

#include <cstring>
#include <csetjmp>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <gmpxx.h>
#include <unistd.h>

const int NUM = 8;

static jmp_buf jmp;

void outSymbol(uint8_t k) {
    char c;
    if (k < 10) {
        c = '0' + k;
    } else {
        c = 'a' + k - 10;
    }
    write(STDOUT_FILENO, &c, 1);
}

void outByte(uint8_t b) {
    outSymbol(b / 16);
    outSymbol(b % 16);
}

void printString(const char *s) {
    write(STDOUT_FILENO, s, strlen(s));
}

void printNum(uint64_t num) {
    for (int i = 7; i >= 0; i--) {
        outByte((num >> (i * 8)) & 0xff);
    }
}

void reg(const char *reg, uint64_t num) {
    printString(reg);
    printString(" = ");
    printNum(num);
    printString("\n");
}

static void jmp_handler() {
    longjmp(jmp, 1);
}

static void sigsegv_handler(int s, siginfo_t *siginfo, void *context) {
    if (siginfo->si_signo == SIGSEGV) {
        printString("registers:\n");
        auto *ucontext = (ucontext_t *) context;
        reg("R8", ucontext->uc_mcontext.gregs[REG_R8]);
        reg("R9", ucontext->uc_mcontext.gregs[REG_R9]);
        reg("R10", ucontext->uc_mcontext.gregs[REG_R10]);
        reg("R11", ucontext->uc_mcontext.gregs[REG_R11]);
        reg("R12", ucontext->uc_mcontext.gregs[REG_R12]);
        reg("R13", ucontext->uc_mcontext.gregs[REG_R13]);
        reg("R14", ucontext->uc_mcontext.gregs[REG_R14]);
        reg("R15", ucontext->uc_mcontext.gregs[REG_R15]);
        reg("RDI", ucontext->uc_mcontext.gregs[REG_RDI]);
        reg("RSI", ucontext->uc_mcontext.gregs[REG_RSI]);
        reg("RBP", ucontext->uc_mcontext.gregs[REG_RBP]);
        reg("RBX", ucontext->uc_mcontext.gregs[REG_RBX]);
        reg("RDX", ucontext->uc_mcontext.gregs[REG_RDX]);
        reg("RAX", ucontext->uc_mcontext.gregs[REG_RAX]);
        reg("RCX", ucontext->uc_mcontext.gregs[REG_RCX]);
        reg("RSP", ucontext->uc_mcontext.gregs[REG_RSP]);
        reg("RIP", ucontext->uc_mcontext.gregs[REG_RIP]);
        reg("EFL", ucontext->uc_mcontext.gregs[REG_EFL]);
        reg("CSGSFS", ucontext->uc_mcontext.gregs[REG_CSGSFS]);
        reg("ERR", ucontext->uc_mcontext.gregs[REG_ERR]);
        reg("TRAPNO", ucontext->uc_mcontext.gregs[REG_TRAPNO]);
        reg("OLDMASK", ucontext->uc_mcontext.gregs[REG_OLDMASK]);
        reg("CR2", ucontext->uc_mcontext.gregs[REG_CR2]);

        struct sigaction act{};
        act.sa_flags = SA_NODEFER;
        act.sa_handler = reinterpret_cast<__sighandler_t>(&jmp_handler);
        if (sigaction(SIGSEGV, &act, nullptr) < 0) {
            perror("Cannot change signal action");
            exit(EXIT_FAILURE);
        }

        printString("memory:\n");
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
                perror("Can't read memory\n");
            } else {
                if (i == addr) {
                    printString("here: ");
                }
                printNum(reinterpret_cast<uint64_t>(i));
                //std::cout << std::hex << (void *)i << ' ' << (int) *i << std::endl;
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
        perror( "Can't make empty set of signals\n");
        return 0;
    }
    act.sa_sigaction = &sigsegv_handler;
    if (sigaction(SIGSEGV, &act, nullptr)== -1) {
        perror("Can't change signal action\n");
    }

    return 0;
};