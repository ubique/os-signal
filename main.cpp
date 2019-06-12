#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ucontext.h>
#include <cstring>
#include <setjmp.h>
#include <unistd.h>

char regs[23][10] = {
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

void print(const char *str) {
    write(STDERR_FILENO, str, strlen(str));
}

void println(const char *str) {
    print(str);
    print("\n");
}

void getHex(size_t val, char *res) {
    size_t size = 16;
    res[0] = '0';
    res[1] = 'x';
    res[2 + size] = '\0';
    size_t i = size + 1;
    while (i != 1 && val != 0) {
        size_t digit = val % 16;
        if (digit < 10) {
            res[i] = '0' + digit;
        } else {
            res[i] = 'a' + (digit - 10);
        }
        val /= 16;
        i--;
    }
}

void printhex(size_t val) {
    char v[32];
    for (char &i : v) {
        i = '0';
    }
    getHex(val, v);
    print(v);
}

void printreg(const char *reg, size_t val) {
    print(reg);
    print("  ");
    printhex(val);
    println("");


}

void addrHandler(int sig, siginfo_t *info, void *ucontext) {
    siglongjmp(buf, 1);
}

void handler(int sig, siginfo_t *info, void *ucontext) {
    if (info->si_signo == SIGSEGV) {
        print("SIGSEGV caught");
        switch (info->si_code) {
            case SEGV_MAPERR: {
                println("Address not mapped to object");
                break;
            }
            case SEGV_ACCERR: {
                println("Invalid permissions for mapped object");
                break;
            }
            case SEGV_BNDERR: {
                println("Failed address bound checks");
                break;
            }
            case SEGV_PKUERR: {
                println("Access was denied by memory protection keys");
                break;
            }
            default: {
                println("Something bad happened");
                break;
            }
        }
        // registers
        println("---REGISTERS---");
        auto context = ((ucontext_t *) ucontext)->uc_mcontext;
        for (int i = 0; i < NGREG; ++i) {
            printreg(regs[i], context.gregs[i]);
        }
        // memory
        println("---MEMORY---");
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
        for (size_t i = start; i <= end; i += sizeof(char)) {
            sigset_t sigset;
            sigemptyset(&sigset);
            sigaddset(&sigset, SIGSEGV);
            sigprocmask(SIG_UNBLOCK, &sigset, nullptr);
            struct sigaction act{};
            act.sa_flags = SA_SIGINFO;
            act.sa_sigaction = addrHandler;
            if (sigaction(SIGSEGV, &act, nullptr) < 0) {
                print("sigaction error: ");
                println(strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (setjmp(buf)) {
                printhex(i);
                print(" --");
            } else {
                auto data = ((const char *) i)[0];
                printhex(i);
                print(" ");
                printhex((uint32_t) data);
            }
            println("");
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