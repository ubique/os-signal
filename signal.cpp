#include <iostream>
#include <signal.h>
#include <zconf.h>
#include <cstring>
#include <map>
#include <csetjmp>
#include <limits>
#include <vector>

using namespace std;

char num_buffer[20];
jmp_buf jmp;


void check_error(int value, const char *message) {
    if (value == -1) {
        perror(message);
        exit(EXIT_FAILURE);
    }
}


void write_string(const char *s) {
    char *current_pos = const_cast<char *>(s);

    int tmp = write(STDERR_FILENO, current_pos, strlen(current_pos));
    if (tmp == -1) {
        exit(EXIT_FAILURE);
    }
    while (tmp != strlen(current_pos)) {
        current_pos += tmp;
        tmp = write(STDERR_FILENO, current_pos, strlen(current_pos));
        if (tmp == -1) {
            exit(EXIT_FAILURE);
        }
    }
}

void write_int(int x) {
    if (x < 0) {
        write_string("-");
        x *= -1;
    }

    int pos = 18;
    if (x == 0) {
        num_buffer[pos] = '0';
        write_string(num_buffer + pos);
        return;
    }

    while (x) {
        num_buffer[pos--] = '0' + x % 10;
        x /= 10;
    }
    write_string(num_buffer + pos + 1);

}

void write_sizet(size_t x) {
    int pos = 18;
    if (x == 0) {
        num_buffer[pos] = '0';
        write_string(num_buffer + pos);
        return;
    }

    while (x) {
        num_buffer[pos--] = '0' + x % 10;
        x /= 10;
    }
    write_string(num_buffer + pos + 1);
}



void write_register(const char *reg, int num_reg, ucontext_t *ucontext) {
    write_string(reg);
    write_string(" = ");
    int reg_int = static_cast<int>(ucontext->uc_mcontext.gregs[num_reg]);
    write_int(reg_int);
    write_string("\n");

}

void write_registers(ucontext_t *ucontext) {
    write_register("R8",  REG_R8, ucontext);
    write_register("R9",  REG_R9, ucontext);
    write_register("R10", REG_R10, ucontext);
    write_register("R11", REG_R11, ucontext);
    write_register("R12", REG_R12, ucontext);
    write_register("R13", REG_R13, ucontext);
    write_register("R14", REG_R14, ucontext);
    write_register("R15", REG_R15, ucontext);
    write_register("RAX", REG_RAX, ucontext);
    write_register("RBP", REG_RBP, ucontext);
    write_register("RBX", REG_RBX, ucontext);
    write_register("RCX", REG_RCX, ucontext);
    write_register("RDI", REG_RDI, ucontext);
    write_register("RDX", REG_RDX, ucontext);
    write_register("RIP", REG_RIP, ucontext);
    write_register("RSI", REG_RSI, ucontext);
    write_register("RSP", REG_RSP, ucontext);
    write_register("CR2", REG_CR2, ucontext);
    write_register("EFL", REG_EFL, ucontext);
    write_register("CSGSFS",  REG_CSGSFS,  ucontext);
    write_register("ERR",     REG_ERR,     ucontext);
    write_register("OLDMASK", REG_OLDMASK, ucontext);
    write_register("TRAPNO",  REG_TRAPNO,  ucontext);
}



void sig_helper(int, siginfo_t *, void *) {
    siglongjmp(jmp, 1);
}

void sigact_handler(int, siginfo_t *info, void *context) {
    auto *ucontext = reinterpret_cast<ucontext_t *>(context);

    write_registers(ucontext);

    char *addr = (char *) info->si_addr;

    write_string("address = ");
    write_sizet((size_t) addr);
    write_string("\n");


    char *left = addr < (char *) 16 ? nullptr : addr - 16;
    char *right = addr > numeric_limits<char *>::max() - 16 ? numeric_limits<char *>::max() : addr + 16;


    struct sigaction sa{};
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sig_helper;


    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGSEGV);
    sigprocmask(SIG_UNBLOCK, &set, nullptr);

    check_error(sigaction(SIGSEGV, &sa, nullptr), "sigaction");


    for (char *i = left; i <= right; i += 4) {
        if (setjmp(jmp) != 0) {
            write_string("???");
        } else {
            int tmp = write(STDERR_FILENO, i, 4);
            if (tmp == -1) {
                write_string("???");
            }
        }
        write_string("\n");
    }

    exit(EXIT_FAILURE);
}


int main() {
    struct sigaction sa{};
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sigact_handler;
    check_error(sigaction(SIGSEGV, &sa, nullptr), "sigaction");


    return 0;
} 