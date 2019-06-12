//
// Created by max on 23.05.19.
//

#include <iostream>
#include <memory.h>
#include <iomanip>
#include <signal.h>
#include <csetjmp>
#include <limits>
#include <unistd.h>

#include "Handler.h"
#include "helper.h"


Handler::Handler(Handler::foo function) {
    sigact.sa_flags = SA_SIGINFO;
    sigact.sa_sigaction = function;
    sigset_t sigset1{};
    sigemptyset(&sigset1);
    sigaddset(&sigset1, SIGSEGV);
    sigact.sa_mask = sigset1;
    checker(sigaction(SIGSEGV, &sigact, nullptr), "Sigaction failed");

}


jmp_buf jmpBuf;

void write_num(size_t num) {
    static size_t size = sizeof(size_t) * 2;
    char number[size];
    memset(number, 0, size);
    for (int i = 0; i < size / 2; ++i) {
        uint8_t numb = num;
        uint8_t numb_top = numb & 15;
        number[size - 1 - 2 * i] = (numb_top >= 10 ? numb_top - 10 + 'a' : numb_top + '0');
        number[size - 2 - 2 * i] = (((numb >> 4) >= 10) ? (numb >> 4) - 10 + 'a' : (numb >> 4) + '0');

        num = num >> 8;
    }
    ::write(1, number, size);
}

void write_byte(unsigned char ch) {
    uint8_t numb = ch;
    uint8_t numb_top = numb & 15;
    uint8_t ch1 = numb_top >= 10 ? numb_top - 10 + 'a' : numb_top + '0';
    uint8_t ch2 = (((numb >> 4) >= 10) ? (numb >> 4) - 10 + 'a' : (numb >> 4) + '0');
    ::write(1, &(ch2), 1);
    ::write(1, &(ch1), 1);


}

void write_str(const char *str) {
    ::write(1, str, strlen(str));
}

void m_hnd(UNUSED int sig, siginfo_t *siginfo, UNUSED void *ucontext) {
    if (siginfo->si_signo == SIGSEGV) {
        siglongjmp(jmpBuf, 1);
    }
}

void hnd(int sig, siginfo_t *siginfo, void *ucontext) {
    write_str("Handler start\n");
    auto address = siginfo->si_addr;
    write_str(strsignal(sig));
    write_str(" on: ");
    write_num((size_t) address);
    write_str("\n");
    write_str("General purpose registers:\n");

    write_str("     R8 ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_R8]);
    write_str("\n");
    write_str("     R9 ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_R9]);
    write_str("\n");
    write_str("    R10 ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_R10]);
    write_str("\n");
    write_str("    R11 ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_R11]);
    write_str("\n");
    write_str("    R12 ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_R12]);
    write_str("\n");
    write_str("    R13 ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_R13]);
    write_str("\n");
    write_str("    R14 ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_R14]);
    write_str("\n");
    write_str("    R15 ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_R15]);
    write_str("\n");
    write_str("    RAX ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_RAX]);
    write_str("\n");
    write_str("    RBP ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_RBP]);
    write_str("\n");
    write_str("    RSP ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_RSP]);
    write_str("\n");
    write_str("    RSI ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_RSI]);
    write_str("\n");
    write_str("    RIP ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_RIP]);
    write_str("\n");
    write_str("    RDX ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_RDX]);
    write_str("\n");
    write_str("    RDI ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_RDI]);
    write_str("\n");
    write_str("    RCX ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_RCX]);
    write_str("\n");
    write_str("    RBX ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_RBX]);
    write_str("\n");
    write_str("    ERR ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_ERR]);
    write_str("\n");
    write_str("    EFL ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_EFL]);
    write_str("\n");
    write_str("    CR2 ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_CR2]);
    write_str("\n");
    write_str(" TRAPNO ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_TRAPNO]);
    write_str("\n");
    write_str("OLDMASK ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_OLDMASK]);
    write_str("\n");
    write_str(" CSGSFS ");
    write_num(reinterpret_cast<ucontext_t *>(ucontext)->uc_mcontext.gregs[REG_CSGSFS]);
    write_str("\n");
    if (address == nullptr) {
        write_str("nullptr\n");
        exit(EXIT_FAILURE);
    }

    auto pointer = reinterpret_cast<size_t >(address);

    size_t size = 30;
    size_t left_border = (pointer > size ? pointer - size : 0);
    size_t max = std::numeric_limits<size_t>::max();
    size_t right_border = (max - pointer < size ? max : pointer + size);


    sigset_t sset;
    sigemptyset(&sset);
    sigaddset(&sset, SIGSEGV);
    write_str("Range: ");
    write_num(left_border);
    write_str(" | ");
    write_num(right_border);
    write_str("\n");
    for (size_t ptr = left_border; ptr < right_border; ptr += sizeof(char)) {
        sigprocmask(SIG_UNBLOCK, &sset, nullptr);
        Handler handler{m_hnd};
        if (ptr == pointer) {
            write_str("[!!] ");
        } else if (setjmp(jmpBuf) != 0) {
            write_str("!! ");
        } else {
            write_byte(*reinterpret_cast<unsigned char *>(ptr));
            write_str(" ");
        }
    }

    write_str("\n");
    exit(EXIT_FAILURE);
}


