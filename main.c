#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <setjmp.h>

#define MIN(o1, o2) (((o1)<(o2))?(o1):(o2))
#define MAX(o1, o2) (((o1)>(o2))?(o1):(o2))

#define FATAL_ERROR(msg) {perror(msg);exit(EXIT_FAILURE);}

static jmp_buf jmpbuf;

const char* register_to_name(const int reg) {
    static const char* name[] = {"R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15", "RDI", "RSI", "RBP", "RBX",
                                 "RDX", "RAX", "RCX", "RSP", "RIP", "EFL", "CSGSFS", "ERR", "TRAPNO", "OLDMASK",
                                 "CR2", "UNKNOWN"};
    if (0 <= reg && reg <= 22) return name[reg];
    else return name[23];
}

void sigsegv_address_handler(int signum, siginfo_t* siginfo, void* context) {
    if (siginfo->si_signo == SIGSEGV) siglongjmp(jmpbuf, 1);
}

void address_dump(void* addr) {
    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGSEGV);
    sigprocmask(SIG_UNBLOCK, &signal_set, NULL);

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = sigsegv_address_handler;
    act.sa_flags = SA_SIGINFO;

    if (sigaction(SIGSEGV, &act, NULL) < 0) FATAL_ERROR("Sigaction failed.")

    if (setjmp(jmpbuf) == 0) {
        printf("%p\t%x\n", addr, ((const char*) addr)[0]);
    } else {
        printf("%p\t(bad)\n", addr);
    }
}

void memory_dump(void* addr) {
    printf("\tMEMORY DUMP\n");
    long long from = MAX((long long) 1, (long long) ((char*) addr - 15 * sizeof(char)));
    long long to = MIN(LLONG_MAX, (long long) ((char*) addr + 16 * (sizeof(char))));
    for (long long i = from; i < to; i += sizeof(char)) { address_dump((void*) i); }
    printf("\n");
}

void registers_dump(ucontext_t* context) {
    printf("\tREGISTERS DUMP\n");
    for (int reg = 0; reg < NGREG; ++reg) {
        printf("REG_%-8s\t0x%x\n", register_to_name(reg), (unsigned int) context->uc_mcontext.gregs[reg]);
    }
    printf("\n");
}

const char* signal_code_to_reason(const int si_code) {
    switch (si_code) {
        case SEGV_MAPERR:
            return "Address not mapped to object.";
        case SEGV_ACCERR:
            return "Invalid permissions for mapped object.";
        default:
            return "Unknown error.";
    }
}

void main_info(siginfo_t* siginfo) {
    printf("\tMAIN INFO\n");
    printf("ERROR. Segmentation fault.\n");
    printf("Tried to access %p.\n", siginfo->si_addr);
    printf("%s\n", signal_code_to_reason(siginfo->si_code));
    printf("\n");
}

void sigsegv_handler(int signum, siginfo_t* siginfo, void* context) {
    if (siginfo->si_signo == SIGSEGV) {
        main_info(siginfo);
        registers_dump((ucontext_t*) context);
        memory_dump(siginfo->si_addr);
    }
    exit(EXIT_FAILURE);
}

void set_sigaction() {
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = sigsegv_handler;
    act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &act, NULL) < 0) FATAL_ERROR("Sigaction failed.")
}

int main(int argc, char** argv) {
    set_sigaction();

    // test
    char* c = (char*) "TEST";
    c[5] = 'G';

    return 0;
}
