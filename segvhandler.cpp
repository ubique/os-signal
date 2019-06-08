#include <signal.h>

#include <cstring>
#include <iostream>
#include <errno.h>
#include <setjmp.h>
#include <limits>

#include "segvhandler.h"

static jmp_buf jmp;

constexpr size_t ALTSTACK_SIZE = 4096;
constexpr size_t DUMP_WINDOW = 16; 

static char const * const reg_names[] = {
    "REG_R8",
    "REG_R9",
    "REG_R10",
    "REG_R11",
    "REG_R12",
    "REG_R13",
    "REG_R14",
    "REG_R15",
    "REG_RDI",
    "REG_RSI",
    "REG_RBP",
    "REG_RBX",
    "REG_RDX",
    "REG_RAX",
    "REG_RCX",
    "REG_RSP",
    "REG_RIP",
    "REG_EFL",
    "REG_CSGSFS",
    "REG_ERR",
    "REG_TRAPNO",
    "REG_OLDMASK",
    "REG_CR2"
};

int setup_altstack() {
    stack_t newstack;
    newstack.ss_sp = malloc(ALTSTACK_SIZE);
    newstack.ss_size = ALTSTACK_SIZE;
    newstack.ss_flags = 0;

    return sigaltstack(&newstack, nullptr);
}

int register_handler(void (*handler)(int, siginfo_t*, void*)) {
    int err;
    sigset_t act_set;
    struct sigaction sa;

    sigemptyset(&act_set);
    sigaddset(&act_set, SIGSEGV);

    sa.sa_sigaction = handler;
    sa.sa_mask = act_set;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;

    err = sigprocmask(SIG_UNBLOCK, &act_set, nullptr);
    
    if (err < 0) {
        return -1;
    }
    
    err = sigaction(SIGSEGV, &sa, nullptr);

    if (err < 0) {
        return -2;
    }

    return 0;
}

void helper_handler(int sig, siginfo_t * info, void *context) {
    longjmp(jmp, 1);
}

void sigsegv_handler(int sig, siginfo_t * info, void *context) {
    int ret;
    char* addr = (char*) info->si_addr;
    greg_t* regs = ((ucontext_t*)context)->uc_mcontext.gregs;

    printf("SIGSEGV at %p\n", addr);
    for (int i = 0; i < NGREG; ++i) {
        printf("  %s: %016llx\n", reg_names[i], regs[i]);
    }

    char* ptr_max = std::numeric_limits<char*>::max();
    char* start = (addr > (char*)DUMP_WINDOW) ? addr - DUMP_WINDOW : nullptr;
    char* end = (addr < ptr_max - DUMP_WINDOW) ? addr + DUMP_WINDOW : ptr_max;

    printf("Dumping from %p to %p\n", start, end);

    for (char* i = start; i < end; ++i) {
        ret = register_handler(helper_handler);
        
        if (ret < 0) {
            printf("Could not regiter handler: %s\n", strerror(errno));
        }

        printf(i == addr ? "!" : " ");

        if (setjmp(jmp)) {
            printf("--");
        } else {
            printf("%02hhx", *i);
        }
    }
    printf("\n");
    exit(1);

}

void handler_setup() {
    setup_altstack();
    register_handler(sigsegv_handler);
}
