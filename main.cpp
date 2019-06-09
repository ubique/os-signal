#include <iostream>
#include <signal.h>
#include <sys/ucontext.h>
#include <setjmp.h>
#include <cstring>
#include <vector>
#include <limits>

static const std::vector<std::pair<std::string, greg_t>> REGISTERS = {
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

static const size_t RANGE = 25;
static jmp_buf jmpBuf;

void jump_handler(int signo, siginfo_t *siginfo, void *ucontext) {
    siglongjmp(jmpBuf, 1);
}

void dump_memory(const char *addr) {
    std::cout << "Dumping memory: " << std::endl;

    size_t point = reinterpret_cast<size_t>(addr);
    size_t left = (point > RANGE ? point - RANGE : 0);
    size_t max = std::numeric_limits<size_t>::max();
    size_t right = (max - point < RANGE ? max : point + RANGE);

    for (size_t i = left; i < right; i += sizeof(char)) {
        std::cout << "\t";

        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGSEGV);
        sigprocmask(SIG_UNBLOCK, &sigset, nullptr);

        struct sigaction action;
        action.sa_flags = SA_SIGINFO;
        action.sa_sigaction = jump_handler;

        if (sigaction(SIGSEGV, &action, nullptr) == -1) {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }
        if (setjmp(jmpBuf) != 0) {
            std::cout << "Can not dump" << std::endl;
        } else {
            std::cout << "0x" << std::hex << (int) *reinterpret_cast<char *>(i) << std::endl;
        }
    }
}

void sigsegv_handler(int signo, siginfo_t *siginfo, void *ucontext) {
    if (siginfo->si_signo == SIGSEGV) {
        std::cout << "SIGSEGV was sent" << std::endl;
        std::cout << "Address: " << (siginfo->si_addr == nullptr ? "NULL" : siginfo->si_addr)  << std::endl;

        auto *context = static_cast<ucontext_t *>(ucontext);
        std::cout << "Registers: " << std::endl;
        for (const auto &reg : REGISTERS) {
            std::cout << "\t";
            std::cout << reg.first << " : " << std::hex << "0x" << context->uc_mcontext.gregs[reg.second] << std::endl;
        }

        if (siginfo->si_addr != nullptr) {
            dump_memory(static_cast<char *>(siginfo->si_addr));
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