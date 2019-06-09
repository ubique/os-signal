#include <csignal>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <ucontext.h>
#include <cmath>
#include <algorithm>
#include <climits>
#include <sys/ucontext.h>
#include <csetjmp>
#include <map>

void error(std::string const &message) {
    perror(message.data());
    exit(EXIT_FAILURE);
}

const std::map<std::string, int> registers = {{"R8",      REG_R8},
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
                                              {"CSGSFS",  REG_CSGSFS},
                                              {"EFL",     REG_EFL},
                                              {"ERR",     REG_ERR},
                                              {"OLDMASK", REG_OLDMASK},
                                              {"TRAPNO",  REG_TRAPNO}};

jmp_buf jump_buf;

void helper(int, siginfo_t *siginfo, void *) {
    if (siginfo->si_signo == SIGSEGV) {
        siglongjmp(jump_buf, 1);
    }
}

void handler(int signum, siginfo_t *siginfo, void *context) {
    if (siginfo->si_signo == SIGSEGV) {
        std::cout << strsignal(signum) << " on " << siginfo->si_addr << '\n';

        std::cout << "registers:\n";
        auto *ucontext = reinterpret_cast<ucontext_t *>(context);

        for (auto &reg : registers) {
            std::cout << reg.first << " = " << std::hex << "0x"
                      << static_cast<int>(ucontext->uc_mcontext.gregs[reg.second])
                      << '\n';
        }

        std::cout << "memory:\n";
        for (int64_t offset = -16; offset < 16; offset += sizeof(char)) {
            auto address = (char *) siginfo->si_addr + offset;

            if ((int64_t) address < 0 || (int64_t) address > INT64_MAX) {
                continue;
            }

            sigset_t signal_set;
            sigemptyset(&signal_set);
            sigaddset(&signal_set, SIGSEGV);
            sigprocmask(SIG_UNBLOCK, &signal_set, nullptr);

            struct sigaction sigact{};
            sigact.sa_sigaction = helper;
            sigact.sa_flags = SA_SIGINFO;

            if (sigaction(SIGSEGV, &sigact, nullptr) < 0) {
                error("Can't sigaction");
            }

            if (setjmp(jump_buf) == 0) {
                std::cout << std::hex << (void *) address << " " << (int) *address << '\n';
            } else {
                std::cout << std::hex << (void *) address << " unknown" << '\n';
            }
        }

        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = handler;
    action.sa_flags = SA_SIGINFO;

    if (sigaction(SIGSEGV, &action, nullptr) < 0) {
        error("Can't sigaction");
    }

    char *test = "abacaba";
    test[8] = 'a';

    exit(EXIT_SUCCESS);
}