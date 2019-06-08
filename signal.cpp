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
#include <vector>


static const int MEMORY_DUMP_RANGE = 20;
const std::vector<std::string> registers = {
        "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15",
        "RDI", "RSI","RBP", "RBX", "RDX", "RAX", "RCX", "RSP", "RIP",
        "EFL", "CSGSFS", "ERR", "TRAPNO", "OLDMASK", "CR2"};
static jmp_buf jmpBuf;

void sigsegvAddressHandler(int signum, siginfo_t* siginfo, void* context) {
    if (siginfo->si_signo == SIGSEGV) {
        siglongjmp(jmpBuf, 1);
    }
}

void dumpMemory(void* address) {
    std::cout << "\nMemory dump: " << "\n";
    const size_t step = sizeof(char);
    const long long from = std::max(0LL, (long long) ((char*) address - MEMORY_DUMP_RANGE * step));
    const long long to = std::min(LONG_LONG_MAX, (long long) ((char*) address + MEMORY_DUMP_RANGE * step));

    for (long long addr = from; addr < to; addr += step) {
        sigset_t setSignal;
        sigemptyset(&setSignal);
        sigaddset(&setSignal, SIGSEGV);
        sigprocmask(SIG_UNBLOCK, &setSignal, nullptr);

        struct sigaction sigAction{};
        sigAction.sa_sigaction = sigsegvAddressHandler;
        sigAction.sa_flags = SA_SIGINFO;

        if (sigaction(SIGSEGV, &sigAction, nullptr) < 0) {
            std::cerr << "Signal action failed: " << strerror(errno) << std::endl;
            exit(EXIT_FAILURE);
        }
        const char* p = (const char*)addr;
        std::cout << "0x" << addr;
        if (setjmp(jmpBuf) != 0) {
            std::cout << " Couldn't dump\n";
        } else {
            std::cout  << std::hex << +*reinterpret_cast<char*>(addr) << " " << (int)p[0] <<"\n";
        }
    }
}

void dumpRegisters(ucontext_t* ucontext) {
    std::cout << "\nGeneral purposes registers:\n";
    greg_t* gregs = ucontext->uc_mcontext.gregs;
    for (greg_t curReg = REG_R8; curReg != NGREG; curReg++) {
        std::cout << " " << registers[curReg] << ": 0x" << ucontext->uc_mcontext.gregs[curReg] << "\n";
    }
}

void sigsegvHandler(int signo, siginfo_t* siginfo, void* context) {
    if (siginfo->si_signo == SIGSEGV) {
        std::cout << "Signal aborted: " << strsignal(signo) << "\n";
        switch (siginfo->si_code) {
            case SEGV_MAPERR: {
                std::cout << "\nCause: Address is not mapped to object" << "\n";
                break;
            }
            case SEGV_ACCERR: {
                std::cout << "\nCause: Invalid permission for mapped object" << "\n";
                break;
            }
            default:
                std::cout << "\nUnsupported code: " << siginfo->si_code << std::endl;
        }

        auto address = reinterpret_cast<intptr_t>(siginfo->si_addr);
        std::cout << "Address: 0x" << std::hex << address << "\n";

        dumpRegisters(reinterpret_cast<ucontext_t*> (context));
        dumpMemory(siginfo->si_addr);
    }
    exit(EXIT_FAILURE);
}

void test1() {
    char* arr = const_cast<char*>("123");
    arr[21] = '4';
}

void test2() {
    int* x = nullptr;
    *x = 21;
}

int main(int argc, char **argv) {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = sigsegvHandler;
    action.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &action, nullptr) < 0) {
        std::cerr << "Sigaction failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
//    test1();
    test2();
    exit(EXIT_SUCCESS);
}