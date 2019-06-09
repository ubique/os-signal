#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ucontext.h>
#include <cstring>
#include <setjmp.h>

std::string regs[] = {
        "R8",
        "R9",
        "R10",
        "R11",
        "R12",
        "R13",
        "R14",
        "R15",
        "RDI",
        "RSI",
        "RBP",
        "RBX",
        "RDX",
        "RAX",
        "RCX",
        "RSP",
        "RIP",
        "EFL",
        "CSGSFS",
        "ERR",
        "TRAPNO",
        "OLDMASK",
        "CR2"
};
jmp_buf buf;


void addrHandler(int sig, siginfo_t *info, void *ucontext) {
    siglongjmp(buf, 1);
}

void handler(int sig, siginfo_t *info, void *ucontext) {
    if (info->si_signo == SIGSEGV) {
        std::cout << "SIGSEGV detected at " << info->si_addr << std::endl;
        switch (info->si_code) {
            case SEGV_MAPERR: {
                std::cout << "Address not mapped to object" << std::endl;
                break;
            }
            case SEGV_ACCERR: {
                std::cout << "Invalid permissions for mapped object" << std::endl;
                break;
            }
            case SEGV_BNDERR: {
                std::cout << "Failed address bound checks" << std::endl;
                break;
            }
            case SEGV_PKUERR: {
                std::cout << "Access was denied by memory protection keys" << std::endl;
                break;
            }
            default: {
                std::cout << "Something bad happened" << std::endl;
                break;
            }
        }
        // registers
        std::cout << "---REGISTERS---" << std::endl;
        auto context = ((ucontext_t *) ucontext)->uc_mcontext;
        for (int i = 0; i < NGREG; ++i) {
            std::cout << regs[i] << "   " << "0x" << std::hex << context.gregs[i] << std::endl;
        }
        // memory
        std::cout << std::dec << "---MEMORY---" << std::endl;
        auto memAddr = (size_t) info->si_addr;
        size_t start = 0;
        size_t end = SIZE_MAX;
        size_t range = 25;
        if (memAddr > range) {
            start = memAddr - range;
        }
        if (end - memAddr > range) {
            end = memAddr + range;
        }
        std::cout << "from " << start << " to " << end << std::hex << std::endl;
        for (size_t i = start; i <= end; i += sizeof(char)) {
            sigset_t sigset;
            sigemptyset(&sigset);
            sigaddset(&sigset, SIGSEGV);
            sigprocmask(SIG_UNBLOCK, &sigset, nullptr);
            struct sigaction act{};
            act.sa_flags = SA_SIGINFO;
            act.sa_sigaction = addrHandler;
            if (sigaction(SIGSEGV, &act, nullptr) < 0) {
                std::cerr << "sigaction error" << strerror(errno) << std::endl;
                exit(EXIT_FAILURE);
            }
            if (setjmp(buf)) {
                std::cout << "0x" << i << " " << "--";
            } else {
                auto data = ((const char *) i)[0];
                std::cout << "0x" << i << " " << (uint32_t) data;
            }
            std::cout << std::endl;
        }

    }
    exit(EXIT_FAILURE);
}

int main() {
    struct sigaction act{};
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handler;
    if (sigaction(SIGSEGV, &act, nullptr) < 0) {
        std::cerr << "sigaction error" << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }
    // test1
//    int *a = nullptr;
//    *a+=228;


    // test2
    const char *a = "a";
    *(const_cast<char *>(a)) = 'b';

    return 0;
}