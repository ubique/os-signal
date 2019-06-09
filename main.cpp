#include <iostream>
#include <cstring>
#include <limits>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

using std::cin;
using std::cout;
using std::endl;
using std::cerr;
using std::string;

void check(int var_to_check, const char *msg) {
    if (var_to_check == -1) {
        std::perror(msg);
        exit(EXIT_FAILURE);
    }
}

gregset_t *gregset;

const int LOWER_BOUND = -32;
const int HIGHER_BOUND = 64;
const int ROW_LEN = 8;

void write_str(const char *str) {
    write(1, str, strlen(str));
}

char get_digit(uint64_t a) {
    return static_cast<char>(a < 10 ? '0' + a : 'a' + (a % 10));
}

template<typename T>
void write_reg(const string &reg_name, T reg) {
    write_str(reg_name.data());
    write_str(": ");

    char *formatted_str = new char[19];
    uint64_t x = (*gregset)[reg];
    formatted_str[0] = '0';
    formatted_str[1] = 'x';
    for (size_t i = 0; i < 16; ++i) {
        formatted_str[2 + i] = get_digit(x % 16);
        x /= 16;
    }
    formatted_str[18] = '\0';
    write_str(formatted_str);
    write_str("\n");
}

void write_c(uint8_t x) {
    char c = (x < 10) ? x + '0' : x - 10 + 'a';
    write(1, &c, 1);
}

void my_action(int sig, siginfo_t *siginfo, void *context) {
    auto *ctx = (ucontext_t *) context;
    gregset = &ctx->uc_mcontext.gregs;

    write_reg("R8", REG_R8);
    write_reg("R9", REG_R9);
    write_reg("R10", REG_R10);
    write_reg("R11", REG_R11);
    write_reg("R12", REG_R12);
    write_reg("R13", REG_R13);
    write_reg("R14", REG_R14);
    write_reg("R15", REG_R15);
    write_reg("RDI", REG_RDI);
    write_reg("RSI", REG_RSI);
    write_reg("RBP", REG_RBP);
    write_reg("RBX", REG_RBX);
    write_reg("RDX", REG_RDX);
    write_reg("RAX", REG_RAX);
    write_reg("RCX", REG_RCX);
    write_reg("RSP", REG_RSP);
    write_reg("RIP", REG_RIP);
    write_reg("EFL", REG_EFL);
    write_reg("CSGSFS", REG_CSGSFS);
    write_reg("ERR", REG_ERR);
    write_reg("TRAPNO", REG_TRAPNO);
    write_reg("OLDMASK", REG_OLDMASK);
    write_reg("CR2", REG_CR2);

    int pepe[2];
    uint8_t value;
    int status = pipe(pepe);

    if (status == -1) {
        write_str("Can't dump\n");
        exit(0);
    }
    for (int i = LOWER_BOUND; i < HIGHER_BOUND; i++) {
        write_str(i == 0 ? ">" : " ");

        status = write(pepe[1], (char *) (siginfo->si_addr) + i, 1);
        if (status == -1) {
            write_str("??");
        } else {
            status = read(pepe[0], &value, 1);
            if (status != -1) {
                write_c(value / 16);
                write_c(value % 16);
            } else {
                write_str("??");
            }
        }
        write_str(i == 0 ? "<" : " ");
        if ((i + 1) % ROW_LEN == 0) {
            write_str("\n");
        }
    }
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    struct sigaction sa{};
    sa.sa_flags = SA_SIGINFO;
    int status = sigemptyset(&sa.sa_mask);

    check(status, "Error in creation of empty set of signals\n");
    sa.sa_sigaction = my_action;

    status = sigaction(SIGSEGV, &sa, NULL);
    check(status, "Error in generating sigact call\n");
    return 0;
}