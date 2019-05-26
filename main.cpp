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

typedef long long LL;

const char* register_to_string(int reg) {
    switch (reg) {
        case 0:
            return "REG_R8";
        case 1:
            return "REG_R9";
        case 2:
            return "REG_R10";
        case 3:
            return "REG_R11";
        case 4:
            return "REG_R12";
        case 5:
            return "REG_R13";
        case 6:
            return "REG_R14";
        case 7:
            return "REG_R15";
        case 8:
            return "REG_RDI";
        case 9:
            return "REG_RSI";
        case 10:
            return "REG_RBP";
        case 11:
            return "REG_RBX";
        case 12:
            return "REG_RDX";
        case 13:
            return "REG_RAX";
        case 14:
            return "REG_RCX";
        case 15:
            return "REG_RSP";
        case 16:
            return "REG_RIP";
        case 17:
            return "REG_EFL";
        case 18:
            return "REG_CSGSFS";
        case 19:
            return "REG_ERR";
        case 20:
            return "REG_TRAPNO";
        case 21:
            return "REG_OLDMASK";
        case 22:
            return "REG_CR2";
        default:
            return "REG_UNKNOWN";
    }
}

static jmp_buf jbuf;

void handler_sigsegv_address(int signum, siginfo_t* siginfo, void* context) {
    if (siginfo->si_signo == SIGSEGV) {
        siglongjmp(jbuf, 1);
    }
}

void address_dump(LL address) {
    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGSEGV);
    sigprocmask(SIG_UNBLOCK, &signal_set, nullptr);

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = handler_sigsegv_address;
    act.sa_flags = SA_SIGINFO;

    if (sigaction(SIGSEGV, &act, nullptr) < 0) {
        std::cerr << "Sigaction failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    const char* p = (const char*)address;
    if (setjmp(jbuf) == 0) {
        printf("ADDRESS_0x%p %x\n", (void*)address, (int)p[0]);
    } else {
        printf("ADDRESS_0x%p (bad)\n", (void*)address);
    }
}

void memory_dump(void* address) {
    std::cout << "MEMORY DUMP" << std::endl;
    LL from = std::max((LL)0, (LL)((char*)address - 20 * sizeof(char)));
    LL to = std::min(LONG_LONG_MAX, (LL)((char*)address + 20 * (sizeof(char))));
    for (LL i = from; i < to; i += sizeof(char)) {
        address_dump(i);
    }
}

void registers_dump(ucontext_t* context) {
    std::cout << "REGISTERS" << std::endl;
    for (int reg = 0; reg < NGREG; ++reg) {
        printf("%-10s\t0x%x\n", register_to_string(reg), (unsigned int)context->uc_mcontext.gregs[reg]);
    }
}

void handler_sigsegv(int signum, siginfo_t* siginfo, void* context) {
    if (siginfo->si_signo == SIGSEGV) {
        std::cout << "ERROR. Segmentation fault. Tried to access " << siginfo->si_addr << ". ";
        switch (siginfo->si_code) {
            case SEGV_MAPERR:
                std::cout << "Address not mapped to object." << std::endl;
                break;
            case SEGV_ACCERR:
                std::cout << "Invalid permissions for mapped object." << std::endl;
                break;
            default:
                std::cout << "Unknown error." << std::endl;
        }
        std::cout << "----------LOG---------" << std::endl;
        registers_dump((ucontext_t*)context);
        memory_dump(siginfo->si_addr);
    }
    exit(EXIT_FAILURE);
}

void test1() {
    char* c = (char*)"LETSROCK";
    std::cout << "String starts from: " << (void*)c << std::endl;
    c[9] = 'A';
}

void test2() {
    int* x = nullptr;
    *x = 10;
}

int main(int argc, char **argv) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = handler_sigsegv;
    action.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &action, nullptr) < 0) {
        std::cerr << "Sigaction failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    test1();
    //test2();
    exit(EXIT_SUCCESS);
}