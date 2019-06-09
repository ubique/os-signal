#include <signal.h>
#include <sys/ucontext.h>

#include <vector>
#include <utility>
#include <string>
#include <iostream>
#include <climits>
#include <csetjmp>
#include <cstring>

static const long long RANGE = 50;

static jmp_buf jmpBuf;


// general registers from <ucontext.h>
// pairs: <name, number in gregset array>
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

std::string getCodeInfo(int code) {
    switch (code) {
        case SEGV_MAPERR:
            return "Address is not mapped to an object";
        case SEGV_ACCERR:
            return "Invalid permissions for mapped object";
        case SEGV_BNDERR:
            return "Failed address bound checks";
        case SEGV_PKUERR:
            return "Access was denied by memory protection keys";
        default:
            return "¯\\_(ツ)_/¯";
    }
}

void dumpRegisters(ucontext_t *ucontext) {
    std::cout << "____________________________________\n" << std::endl;
    std::cout << "General registers:\n";
    greg_t *gregs = ucontext->uc_mcontext.gregs;

    for (auto &r : REGISTERS) {
        std::cout << "\t\t" << r.first << " : " << gregs[r.second] << std::endl;
    }
    std::cout << "____________________________________\n" << std::endl;
}


void helpHandler(int sig) {
    if (sig == SIGSEGV) {
        siglongjmp(jmpBuf, 1);
    }
}

void dumpByAddress(long long addr) {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGSEGV);
    sigprocmask(SIG_UNBLOCK, &sigset, nullptr);

    struct sigaction sigact;
    memset(&sigact, 0, sizeof(struct sigaction));
    sigact.sa_handler = helpHandler;

    if (sigaction(SIGSEGV, &sigact, nullptr) == -1) {
        perror("Sigaction failed");
        return;
    }

    if (setjmp(jmpBuf)) {
        std::cout << "\t\tCannot dump" << std::endl;
    } else {
        std::cout << "\t\t0x" << std::hex << +*reinterpret_cast<char*>(addr) << "\n";
    }
}

void dumpMemory(char *addr) {
    std::cout << "Memory: " << std::endl;

    const size_t charSize = sizeof(char);
    const long long left = std::max(0LL, reinterpret_cast<long long> (addr - RANGE * sizeof(char)));
    const long long right = std::min(LONG_LONG_MAX, reinterpret_cast<long long> (addr + RANGE * sizeof(char)));

    for (auto address = left; address < right; address += charSize) {
        dumpByAddress(address);
    }
    std::cout << "____________________________________\n" << std::endl;
}


void handler(int sig, siginfo_t *info, void *uncontext) {
    if (sig == SIGSEGV) {
        std::cout << "Segmentation fault :)" << std::endl;
        std::cout << "\n____________________________________\n" << std::endl;
        std::cout << "Memory address: " << (info->si_addr == nullptr ? "NULL" : info->si_addr) << std::endl;
        std::cout << "Cause: " << getCodeInfo(info->si_code) << std::endl;

        dumpRegisters(static_cast<ucontext_t *> (uncontext));

        if (info->si_addr != nullptr) {
            dumpMemory(static_cast<char *>(info->si_addr));
        }
    }

    exit(EXIT_FAILURE);
}


int main() {
    struct sigaction sigact;
    memset(&sigact, 0, sizeof(struct sigaction));
    sigact.sa_flags = SA_SIGINFO;
    sigact.sa_sigaction = handler;

    if (sigaction(SIGSEGV, &sigact, nullptr) == -1) {
        perror("Sigaction failed");
        exit(EXIT_FAILURE);
    }

    char* test = const_cast<char*>("Hell");
    test[6] = 'o';
}