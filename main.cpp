#include <iostream>
#include <csignal>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <csetjmp>
#include <zconf.h>

using std::cout;

std::vector<std::pair<std::string, greg_t>> registers = {
        {"R8      : ", REG_R8},
        {"R9      : ", REG_R9},
        {"R10     : ", REG_R10},
        {"R11     : ", REG_R11},
        {"R12     : ", REG_R12},
        {"R13     : ", REG_R13},
        {"R14     : ", REG_R14},
        {"R15     : ", REG_R15},
        {"RDI     : ", REG_RDI},
        {"RSI     : ", REG_RSI},
        {"RBP     : ", REG_RBP},
        {"RBX     : ", REG_RBX},
        {"RDX     : ", REG_RDX},
        {"RAX     : ", REG_RAX},
        {"RCX     : ", REG_RCX},
        {"RSP     : ", REG_RSP},
        {"RIP     : ", REG_RIP},
        {"EFL     : ", REG_EFL},
        {"CSGSFS  : ", REG_CSGSFS},
        {"ERR     : ", REG_ERR},
        {"TRAPNO  : ", REG_TRAPNO},
        {"OLDMASK : ", REG_OLDMASK},
        {"CR2     : ", REG_CR2}
};

gregset_t *gregset;

const int R = 64;

static jmp_buf jmpBuf;

void processSiCode(int code) {
    switch (code) {
        case SEGV_ACCERR:
            cout << "Rights to the reflected object are incorrect\n";
            return;
        case SEGV_MAPERR:
            cout << "Address does not match the object\n";
            return;
        case SEGV_BNDERR:
            cout << "Error checking the boundaries of the address\n";
            return;
        case SEGV_PKUERR:
            cout << "Access denied by memory protection bits\n";
            return;
        default:
            cout << "Unknown\n";
            return;
    }
}

void helpHandler(int, siginfo_t *siginfo, void *) {
    if (siginfo->si_signo == SIGSEGV)
        siglongjmp(jmpBuf, 1);
}

void dumpMemory(void *address) {
    auto left = std::max(0LL, (long long) ((char *) address - R));
    auto right = std::min(LONG_LONG_MAX, (long long) ((char *) address + R));
    for (long long i = left; i <= right; ++i) {
        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGSEGV);
        sigprocmask(SIG_UNBLOCK, &sigset, nullptr);

        struct sigaction action;
        action.sa_sigaction = helpHandler;
        action.sa_flags = SA_SIGINFO;

        if (sigaction(SIGSEGV, &action, nullptr) == -1) {
            perror("address sigaction");
            return;
        }

        cout << "0x" << i << ' ';
        if (setjmp(jmpBuf) != 0) {
            std::cout << "error during dump\n";
//            return;
        } else {
             cout << std::hex << +*reinterpret_cast<char *>(i) << '\n';
        }
    }
}

void sigsegvHandler(int sig, siginfo_t *si, void *context) {
    if (si->si_signo == SIGSEGV) {
        cout << "Signal : " << strsignal(sig) << '\n';
        processSiCode(si->si_code);

        if (si->si_addr != nullptr) {
            cout << "Address : " << si->si_addr << '\n';
            dumpMemory(si->si_addr);
        } else {
            std::cerr << "Address is NULL\n";
        }

        gregset = &static_cast<ucontext_t *>(context)->uc_mcontext.gregs;
        for (auto &i: registers)
            cout << i.first << gregset[i.second] << '\n';
    }
    exit(EXIT_FAILURE);
}


int main() {
    struct sigaction action;
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = sigsegvHandler;
    if (sigaction(SIGSEGV, &action, nullptr) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // any code causing errors

    char a[1];

    for (auto ptr = a; ; ++ptr) {
        ++*ptr;
    }
    return 0;
}
