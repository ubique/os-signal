#include <signal.h>
#include <cstring>
#include <map>
#include <csetjmp>
#include <limits>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <ucontext.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


using namespace std;

const map<const char *, int> registers = {{"R8",      REG_R8},
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


void checkErr(int value, const char *message) {
    if (value == -1) {
        perror(message);
        exit(EXIT_FAILURE);
    }
}

jmp_buf jmp;

void helper(int, siginfo_t *info, void *) {
    if (info->si_signo == SIGSEGV) {
        siglongjmp(jmp, 1);
    }
}

void write(const char *data) {
    size_t result;
    size_t size = strlen(data);
    while (size > 0 && (result = static_cast<size_t>(write(STDERR_FILENO, data, size)))) {
        if (result < 0 && errno == EINTR) {
            continue;
        } else if (result < 0) {
            exit(EXIT_FAILURE);
        }
        size -= result;
        data += result;
    }
}

void to_hex(char* res, size_t value, size_t len_in_bytes = 8) {
    len_in_bytes *= 2;
    res[0] = '0';
    res[1] = 'x';
    for (size_t i = 0; i < len_in_bytes; i++) {
        size_t ch = value & 0xf;
        value >>= 4;
        res[2 + len_in_bytes - i - 1] = (ch < 10) ? ('0' + ch) : ('A' + ch - 10);
    }
}

void write(int val){
    char buf[1024];
    to_hex(buf, val);
    write(buf);
}

void sigHandler(int, siginfo_t *info, void *context) {
    if (info->si_signo == SIGSEGV) {
        write("registers here: \n");
        char buf[1024];
        for (auto &reg :registers) {
            char const * ch = reg.first;
            if (std::strcmp(ch, "") != 0) {
                write(ch);
                write("=");
                to_hex(buf, ((ucontext_t*)context)->uc_mcontext.gregs[reg.second]);
                write(buf);
                write("\n");
            }
        }

        for (char i = -16; i <= 16; i += sizeof(char)) {
            auto address = (char *) info->si_addr + i;

            if ((int64_t) address < 0 || (int64_t) address > INT64_MAX) {
                continue;
            }


            struct sigaction action{};
            action.sa_flags = SA_SIGINFO;
            action.sa_sigaction = helper;


            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set, SIGSEGV);
            sigprocmask(SIG_UNBLOCK, &set, nullptr);

            checkErr(sigaction(SIGSEGV, &action, nullptr), "Can't sigaction");


            if (setjmp(jmp) != 0) {
                write((uint64_t) address);
                write(" unknown\n");
            } else {
                write((uint64_t) address);
                write(" ");
                write(*address);
                write("\n");
            }
        }

        exit(EXIT_FAILURE);
    }
}


int main() {
    struct sigaction action{};
    memset(&action, 0, sizeof(action));
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = sigHandler;
    checkErr(sigaction(SIGSEGV, &action, nullptr), "Can't sigaction");

    char * test = "abacaba";
    test[8] = 'a';
    exit(EXIT_SUCCESS);
}