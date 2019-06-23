#include <cstring>
#include <climits>
#include <csignal>
#include <csetjmp>
#include <unistd.h>

#include "signal.h"

void sigsegv_address_handler(int num, siginfo_t* siginfo, void* context) {
    if (siginfo -> si_signo == SIGSEGV) {
        siglongjmp(jmpBuf, 1);
    }
}

void dumping_registries(ucontext_t* context)
{
    print("Registers dumping:\n");
    for (size_t reg = 0; reg < NGREG; reg++) {
        print(regStr[reg]);
        print(": 0x");
        print(context->uc_mcontext.gregs[reg]);
        print("\n");
    }
}

void dumping_memory(void* address)
{
    print("Memory dumping:\n");
    long long shift = (long long)((char*) address - MEMORY_DUMP_RANGE);
    long long from = 0;

    if (shift > 0) {
        from = shift;
    }

    shift += MEMORY_DUMP_RANGE;
    long long to = LONG_LONG_MAX;

    if (shift < LONG_LONG_MAX - MEMORY_DUMP_RANGE) {
        to = shift + MEMORY_DUMP_RANGE;
    }

    for (long long i = from; i < to; i++)
    {
        sigset_t setSignal;
        sigemptyset(&setSignal);
        sigaddset(&setSignal, SIGSEGV);
        sigprocmask(SIG_UNBLOCK, &setSignal, nullptr);

        struct sigaction sAction {};
        sAction.sa_sigaction = sigsegv_address_handler;
        sAction.sa_flags = SA_SIGINFO;

        if (sigaction(SIGSEGV, &sAction, nullptr) == -1) {
            print("Sigaction failure\n");
            _exit(-1);
        }
        print("0x");
        print(i);
        print(" ");
        if (setjmp(jmpBuf) != 0) {
            print("Dumping failure\n");
        } else {
            print((int)((const char*)i)[0]);
            print("\n");
        }
    }
}

void handler(int num, siginfo_t* siginfo, void* context)
{
    if (siginfo -> si_signo == SIGSEGV)
    {
        print("Signal rejected: ");
        print((const char*) strsignal(num));
        print("\n");

        if (siginfo -> si_code == SEGV_MAPERR) {
            print("Reason: nothing is mapped to address\n");
        } else if(siginfo -> si_code == SEGV_ACCERR) {
            print("Reason: access error\n");
        } else {
            print("Error code: ");
            print(siginfo -> si_code);
            print("\n");
        }
        print("Address: ");
        if(siginfo -> si_addr == nullptr) {
            print("nullptr. No memory dump needed\n");
        } else {
            print("0x");
            print((intptr_t) siginfo -> si_addr);
            print("\n");
        }
        dumping_registries((ucontext_t*) context);
        if (siginfo -> si_addr != nullptr) {
            dumping_memory(siginfo -> si_addr);
        }
    }
    _exit(1);
}

void print(const char* string) {
    write(1, string, strlen(string));
}

void print(long long num) {
    if (num) {
        printHex(num);
    } else {
        print("0");
    }
}

void printHex(long long num) {
    if (num) {
        printHex(num >> 4);
        char symbol = num & 15;
        if (symbol < 10) {
            symbol += '0';
        } else {
            symbol += 'a' - 10;
        }
        write(1, &symbol, 1);
    }
}