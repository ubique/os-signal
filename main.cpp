#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/mman.h>

using namespace std;

void write16(char a) {
    if (a < 10) {
        char b = a + '0';
        write(STDERR_FILENO, &b, 1);
    } else {
        char b = a + 'a' - 10;
        write(STDERR_FILENO, &b, 1);
    }
}

void write_char(char x) {
    unsigned char y = x;
    char a = y / 16;

    write16(a);
    a = y % 16;
    write16(a);

    write(STDERR_FILENO, " ", 1);
}

void output(const char *data) {
    for (size_t i = 0; i < strlen(data); ++i) {
        write_char(*(data + i));
    }
    write(STDERR_FILENO, "\n", 1);
}

void output(const greg_t *data) {
    for (size_t i = 0; i < sizeof(data); ++i) {
        write_char(*((char *) (data + i)));
    }
    write(STDERR_FILENO, "\n", 1);
}

struct sigaction sigsegv;
struct sigaction sigsegv_error;

jmp_buf saved_state;

void sigsegv_err_function(int sig, siginfo_t *info, void *ucontext) {
    longjmp(saved_state, 100);
}

void my_printer(int signal, siginfo_t *info, void *ucontext) {
    sigsegv_error.sa_sigaction = &sigsegv_err_function;
    sigsegv_error.sa_flags = SA_SIGINFO | SA_NODEFER;
    if (sigaction(SIGSEGV, &sigsegv_error, NULL) < 0) {
        perror("ERROR IN SIGNAL. VERY BAD");
        exit(1);
    }

    char *address = (char *) info->si_addr;

    write(STDERR_FILENO, "RAM\n", 4);
    for (int i = -5; i <= 5; ++i) {
        if (setjmp(saved_state) == 0) {
            write_char(*(address + i));
        } else {
            write(STDERR_FILENO, "$$ ", 3);
        }
    }

    write(STDERR_FILENO, "\nRegisters\n", 11);
    greg_t *regs = ((ucontext_t *) ucontext)->uc_mcontext.gregs;
    for (int i = 0; i < NGREG; ++i) {
        if (setjmp(saved_state) == 0) {
            output(regs + i);
        } else {
            output("$$");
        }
    }
    exit(1);
}

int main() {
    sigsegv.sa_sigaction = &my_printer;
    sigsegv.sa_flags = SA_SIGINFO | SA_NODEFER;
    if (sigaction(SIGSEGV, &sigsegv, NULL) < 0) {
        perror("SIGACTION ERROR");
        return 1;
    }
    char *p = NULL;
    printf("%c\n",*p);
    return 0;
}
