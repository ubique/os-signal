#include <iostream>
#include <limits>
#include <cstdio>
#include <cstdlib>
#include "sigsegv_handler.h"

using namespace std;

jmp_buf sigsegv_handler::jmp;

int sigsegv_handler::set_sigsegv_handler() {
    struct sigaction act;
    act.sa_flags = SA_SIGINFO | SA_NODEFER;
    act.sa_sigaction = &handler;
    if (sigaction(SIGSEGV, &act, NULL) < 0) {
        perror("Cannot change signal action");
        return -1;
    }
    return 0;
}

void sigsegv_handler::deref_handler(int sig) {
    longjmp(jmp, 1);
}

void sigsegv_handler::handler(int, siginfo_t *siginfo, void *context) {
    cout << "--REGISTERS--" << endl;
    ucontext_t *ucontext = (ucontext_t *) context;
    for (int i = 0; i < NGREG; i++) {
        cout << "reg[" << i << "] = " << ucontext->uc_mcontext.gregs[i] << endl;
    }

    struct sigaction act;
    act.sa_flags = SA_NODEFER;
    act.sa_handler = &deref_handler;
    if (sigaction(SIGSEGV, &act, NULL) < 0) {
        perror("Cannot change signal action");
        exit(EXIT_FAILURE);
    }

    cout << "--MEMORY--" << endl;
    char *addr = (char *) siginfo->si_addr;
    char *max_ = numeric_limits<char *>::max();
    char *left = (addr > (char *) 16 ? addr - 16 : NULL);
    char *right = (addr < max_ - 16 ? addr + 16 : max_);
    for (char *i = left ; i <= right; i++) {
        if (setjmp(jmp)) {
            cout << "cannot read memory" << endl;
        } else {
            if (i == siginfo->si_addr)
                cout << "here: ";
            cout << (int) *i << endl;
        }
    }
    exit(EXIT_FAILURE);
}
