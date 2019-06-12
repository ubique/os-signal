#include <limits>
#include <unistd.h>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include "handler.h"

static jmp_buf jmpbuf;

void write_string(const char *str) {
    write(1, str, strlen(str));
}

void write_char(char ch) {
    write(1, &ch, 1);
}

void write_hex(uint8_t num) {
    write_char(num >= 10 ? 'A' + num - 10 : '0' + num);
}

void write_num(uint64_t num) {
    for (int i = 0; i < 8; ++i) {
        write_hex(num >> 8);
        write_hex(num >> 4);
        num >>= 8;
    }
}

void write_reg(const char *name, uint64_t reg) {
    write_string(name);
    write_string(": ");
    write_num(reg);
    write_string("\n");
}

void write_regs(void *cont) {
    auto *context = reinterpret_cast<ucontext_t *>(cont);
    write_reg("R8", context->uc_mcontext.gregs[REG_R8]);
    write_reg("R9", context->uc_mcontext.gregs[REG_R9]);
    write_reg("R10", context->uc_mcontext.gregs[REG_R10]);
    write_reg("R11", context->uc_mcontext.gregs[REG_R11]);
    write_reg("R12", context->uc_mcontext.gregs[REG_R12]);
    write_reg("R13", context->uc_mcontext.gregs[REG_R13]);
    write_reg("R14", context->uc_mcontext.gregs[REG_R14]);
    write_reg("R15", context->uc_mcontext.gregs[REG_R15]);
    write_reg("RAX", context->uc_mcontext.gregs[REG_RAX]);
    write_reg("RBP", context->uc_mcontext.gregs[REG_RBP]);
    write_reg("RSP", context->uc_mcontext.gregs[REG_RSP]);
    write_reg("RSI", context->uc_mcontext.gregs[REG_RSI]);
    write_reg("RIP", context->uc_mcontext.gregs[REG_RIP]);
    write_reg("RDX", context->uc_mcontext.gregs[REG_RDX]);
    write_reg("RDI", context->uc_mcontext.gregs[REG_RDI]);
    write_reg("RCX", context->uc_mcontext.gregs[REG_RCX]);
    write_reg("RBX", context->uc_mcontext.gregs[REG_RBX]);
    write_reg("TRAPNO", context->uc_mcontext.gregs[REG_TRAPNO]);
    write_reg("OLDMASK", context->uc_mcontext.gregs[REG_OLDMASK]);
    write_reg("CSGSFS", context->uc_mcontext.gregs[REG_CSGSFS]);
    write_reg("ERR", context->uc_mcontext.gregs[REG_ERR]);
    write_reg("EFL", context->uc_mcontext.gregs[REG_EFL]);
    write_reg("CR2", context->uc_mcontext.gregs[REG_CR2]);
}

handler::handler(void (*foo)(int, siginfo_t *, void *)) {
    struct sigaction act{};
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = foo;
    sigset_t sig;
    sigemptyset(&sig);
    sigaddset(&sig, SIGSEGV);
    act.sa_mask = sig;

    if (sigaction(SIGSEGV, &act, nullptr) == -1) {
        perror("");
    }

}

void help_handler(int sig, siginfo_t *siginfo, void *context) {
    if (siginfo->si_signo == SIGSEGV) {
        siglongjmp(jmpbuf, 1);
    }
}

void hand(int sig, siginfo_t *siginfo, void *context) {
    write_string(strsignal(sig));
    write_string("on address: ");
    write_num((uint64_t) siginfo->si_addr);
    write_string("\n");
    write_string("general purpose registers:\n");
    write_regs(context);

    if (siginfo->si_addr == nullptr) {
        write_string("nullptr\n");
        exit(EXIT_FAILURE);
    }
    size_t pointer = reinterpret_cast<size_t >(siginfo->si_addr);

    const size_t size = 20;
    size_t left = pointer > size ? pointer - size : 0;
    size_t max = std::numeric_limits<size_t>::max();
    size_t right = max - pointer < size ? max : pointer + size;

    sigset_t sset;
    sigemptyset(&sset);
    sigaddset(&sset, SIGSEGV);

    write_string("from ");
    write_num(left);
    write_string(" to ");
    write_num(right);
    write_string(" \n");

    for (size_t i = left; i < right; i += sizeof(char)) {
        sigprocmask(SIG_UNBLOCK, &sset, nullptr);
        handler handler1(&help_handler);
        if (i == pointer) {
            write_string(" !! ");
        } else if (setjmp(jmpbuf) != 0) {
            write_string(" ?? ");
        } else {
            write_string(" ");
            write_char(*reinterpret_cast<char *>(i));
            write_string(" ");
        }
    }
    exit(EXIT_FAILURE);

}