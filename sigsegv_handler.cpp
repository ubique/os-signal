#include <unistd.h>
#include <cstring>
#include <cstdio>
#include "sigsegv_handler.h"

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

void sigsegv_handler::write_str(const char *s) {
    write(1, s, strlen(s));
}

void sigsegv_handler::write_num(long long a) {
    if (a < 0) {
        write_str("-");
        a = -a;
    }
    if (a == 0) {
        write_str("0");
        return;
    }
    char num[25];
    num[24] = '\0';
    int i = 23;
    while (a > 0) {
        num[i] = '0' + (a % 10);
        a /= 10;
        i--;
    }
    write_str(num + i + 1);
}

void sigsegv_handler::handler(int, siginfo_t *siginfo, void *context) {
    write_str("--REGISTERS--\n");
    ucontext_t *ucontext = (ucontext_t *) context;
    for (int i = 0; i < NGREG; i++) {
        write_str("reg[");
        write_num(i);
        write_str("] = ");
        write_num(ucontext->uc_mcontext.gregs[i]);
        write_str("\n");
    }

    int pipefd[2];
    if (pipe(pipefd) < 0) {
        write_str("Cannot create pipe");
    }

    write_str("--MEMORY--\n");
    char *addr = (char *) siginfo->si_addr;
    for (int shift = -16; shift <= 16; shift++) {
        char *i = addr + shift;
        char mem;
        if (shift < 0 && i > addr)
            continue;
        if (shift > 0 && i < addr)
            continue;
        if (i == addr)
            write_str("here: ");
        if (write(pipefd[1], i, 1) < 0)
            write_str("Cannot read memory");
        else if (read(pipefd[0], &mem, 1) < 0)
            write_str("Cannot read memory");
        else
            write_num(mem);
        write_str("\n");
    }
    _exit(1);
}
