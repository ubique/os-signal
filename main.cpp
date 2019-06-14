#include <cstdio>
#include <signal.h>
#include <unistd.h>
#include <sys/ucontext.h>
#include <cstring>
#include <vector>
#include <cstdint>
#include <cstdlib>

#define RANGE 64

void write_char(uint8_t ch) {
    char c = ch < 10 ? '0' + ch : 'A' + ch - 10;
    write(1, &c, 1);
}

void write_byte(uint8_t byte) {
    write_char(byte / 16);
    write_char(byte % 16);
}

void write_num(uint64_t num) {
    for (int i = 7; i >= 0; --i) {
        write_byte((num >> (i * 8)) & 0xFF);
    }
}

void write_str(const char *str) {
    write(1, str, strlen(str));
}

void sigsegv_handler(int signo, siginfo_t *siginfo, void *ucontext) {
    if (siginfo->si_signo == SIGSEGV) {
        const std::vector<std::pair<const char*, uint64_t >> registers = {
                {"R8",      REG_R8},
                {"R9",      REG_R9},
                {"R10",     REG_R10},
                {"R11",     REG_R11},
                {"R12",     REG_R12},
                {"R13",     REG_R13},
                {"R14",     REG_R14},
                {"R15",     REG_R15},
                {"RDI",     REG_RDI},
                {"RSI",     REG_RSI},
                {"RBP",     REG_RBP},
                {"RBX",     REG_RBX},
                {"RDX",     REG_RDX},
                {"RAX",     REG_RAX},
                {"RCX",     REG_RCX},
                {"RSP",     REG_RSP},
                {"RIP",     REG_RIP},
                {"EFL",     REG_EFL},
                {"CSGSFS",  REG_CSGSFS},
                {"ERR",     REG_ERR},
                {"TRAPNO",  REG_TRAPNO},
                {"OLDMASK", REG_OLDMASK},
                {"CR2",     REG_CR2}
        };

        write_str("REGISTERS:\n");
        auto *context = static_cast<ucontext_t *>(ucontext);
        for (const auto &reg : registers) {
            write_str(" ");
            write_str(reg.first);
            write_str(" = ");
            write_num(context->uc_mcontext.gregs[reg.second]);
            write_str("\n");
        }

        write_str("MEMORY:\n");
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            write_str("Can not dump\n");
            exit(EXIT_FAILURE);
        }

        char *addr = static_cast<char *>(siginfo->si_addr);
        for (int i = -RANGE; i < RANGE; ++i) {
            if (i == 0) {
                write_str("[");
            } else {
                write_str(" ");
            }
            uint8_t value;
            if (write(pipefd[1], addr + i, 1) == -1 || read(pipefd[0], &value, 1) == -1) {
                write_str("?");
            } else {
                write_byte(value);
            }
            if (i == 0) {
                write_str("]");
            } else {
                write_str(" ");
            }

            if ((i + 1) % 8 == 0) {
                write_str("\n");
            }

        }
    }

    exit(EXIT_FAILURE);
}

int main() {
    struct sigaction action{};
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = sigsegv_handler;
    if (sigaction(SIGSEGV, &action, nullptr) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    char *hello = const_cast<char *>("Hello");
    hello[4] = 'a';

    return 0;
}