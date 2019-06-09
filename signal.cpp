#include <iostream>
#include <signal.h>
#include <zconf.h>
#include <cstring>
#include <map>
#include <csetjmp>
#include <limits>
#include <vector>

using namespace std;


const map<string, int> registers = {{"R8",      REG_R8},
                                    {"R9",      REG_R9},
                                    {"R10",     REG_R10},
                                    {"R11",     REG_R11},
                                    {"R12",     REG_R12},
                                    {"R13",     REG_R13},
                                    {"R14",     REG_R14},
                                    {"R15",     REG_R15},
                                    {"RAX",     REG_RAX},
                                    {"RBP",     REG_RBP},
                                    {"RBX",     REG_RBX},
                                    {"RCX",     REG_RCX},
                                    {"RDI",     REG_RDI},
                                    {"RDX",     REG_RDX},
                                    {"RIP",     REG_RIP},
                                    {"RSI",     REG_RSI},
                                    {"RSP",     REG_RSP},
                                    {"CR2",     REG_CR2},
                                    {"EFL",     REG_EFL},
                                    {"CSGSFS",  REG_CSGSFS},
                                    {"ERR",     REG_ERR},
                                    {"OLDMASK", REG_OLDMASK},
                                    {"TRAPNO",  REG_TRAPNO}};


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

jmp_buf jmp;

void sig_helper(int, siginfo_t *, void *) {
    siglongjmp(jmp, 1);
}

void sigact_handler(int, siginfo_t *info, void *context) {
    auto *ucontext = reinterpret_cast<ucontext_t *>(context);


    for (auto &reg :registers) {
        write_string(reg.first.c_str());
        write_string(" = ");
        int reg_int = static_cast<int>(ucontext->uc_mcontext.gregs[reg.second]);
        write_string(to_string(reg_int).c_str());
        write_string("\n");
    }


    char *addr = (char *) info->si_addr;

    write_string("address = ");
    write_string(to_string((size_t)addr).c_str());
    write_string("\n");


    char *left  = addr < (char*)16 ? nullptr : addr - 16;
    char *right = addr > numeric_limits<char *>::max() - 16 ? numeric_limits<char *>::max() : addr + 16;


    struct sigaction sa{};
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sig_helper;


    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGSEGV);
    sigprocmask(SIG_UNBLOCK, &set, nullptr);

    check_error(sigaction(SIGSEGV, &sa, nullptr), "sigaction");


    for (char *i = left ; i <= right; i +=4) {
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