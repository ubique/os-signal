#include <signal.h>
#include <sys/ucontext.h>

#include <vector>
#include <utility>
#include <string>
#include <iostream>
#include <climits>
#include <csetjmp>
#include <cstring>
#include <unistd.h>

static const long long RANGE = 15;

static const size_t CHAR_SIZE = sizeof(char);

static jmp_buf jmpBuf;

namespace safe {
    void out(const char *str) {
        write(STDOUT_FILENO, str, strlen(str));
    }

    void endl() {
        write(STDOUT_FILENO, "\n", strlen("\n"));
    }

    void part(uint8_t val) {
        const char c = val + (val < 10 ? '0' : 'A' - 10);
        write(STDOUT_FILENO, &c, 1);
    }

    void out(uint8_t byte) {
        part(byte / 16);
        part(byte % 16);
    }

    void hex(uint64_t value) {
        for (int i = 7; i >= 0; i--) {
            out(0xFF & (value >> (8 * i)));
        }
    }
}


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

const char* getCodeInfo(int code) {
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
    safe::out("____________________________________\n\n");
    safe::out("General registers:\n");
    greg_t *gregs = ucontext->uc_mcontext.gregs;

    for (auto &r : REGISTERS) {
        safe::out("\t\t");
        safe::out(r.first.c_str());
        safe::out(r.first.length() <= 3 ? "\t\t:\t" : "\t:\t");
        safe::hex(gregs[r.second]);
        safe::endl();
    }
    safe::out("____________________________________\n\n");
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
        safe::out("\t\tCannot dump\n");
    } else {
        safe::out("\t\t0x");
        safe::hex(+*reinterpret_cast<char*>(addr));
        safe::endl();
    }
}

void dumpMemory(char *addr) {
    safe::out("Memory: \n");

    const long long left = std::max(0LL, reinterpret_cast<long long> (addr - RANGE * CHAR_SIZE));
    const long long right = std::min(LONG_LONG_MAX, reinterpret_cast<long long> (addr + RANGE * CHAR_SIZE));

    for (auto address = left; address < right; address += CHAR_SIZE) {
        dumpByAddress(address);
    }
    safe::out("____________________________________\n\n");
}


void handler(int sig, siginfo_t *info, void *uncontext) {
    if (sig == SIGSEGV) {
        safe::out("Segmentation fault :)");
        safe::out("\n____________________________________\n\n");
        safe::out("Memory address: ");
        safe::out((info->si_addr == nullptr ? "NULL" : static_cast<const char *>(info->si_addr)));
        safe::endl();
        safe::out("Cause: ");
        safe::out(getCodeInfo(info->si_code));
        safe::endl();

        dumpRegisters(static_cast<ucontext_t *> (uncontext));

        if (info->si_addr != nullptr) {
            dumpMemory(static_cast<char *>(info->si_addr));
        }
    }

    _exit(EXIT_FAILURE);
}


int main() {
    struct sigaction sigact;
    memset(&sigact, 0, sizeof(struct sigaction));
    sigact.sa_flags = SA_SIGINFO;
    sigact.sa_sigaction = handler;

    if (sigaction(SIGSEGV, &sigact, nullptr) == -1) {
        perror("Sigaction failed");
        _exit(EXIT_FAILURE);
    }

    char* test = const_cast<char*>("Hell");
    test[6] = 'o';
}