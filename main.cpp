#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h> 
#include <limits>

void out_sym(uint8_t k) {
    char c;
    if (k < 10)
        c = char(k + '0');
    else
        c = char(k - 10 + 'a');
    write(1, &c, 1);
}

void out_byte(uint8_t k) {
    out_sym(k / 16);
    out_sym(k % 16);
}

void write_string(const char *s) {
    write(1, s, strlen(s));
}
void out(const char *reg, uint64_t value) {
    write_string(reg);
    write_string(" ");
    for (int i = 7; i >= 0; i--) {
        out_byte((value >> (8 * i)) & 0xff);
    }   
    write_string("\n");
}

void action(int sig, siginfo_t *info, void *context) {
    struct ucontext_t *p = static_cast<ucontext_t*>(context);
    out("R8", p->uc_mcontext.gregs[REG_R8]);
    out("R9", p->uc_mcontext.gregs[REG_R9]);
    out("R10", p->uc_mcontext.gregs[REG_R10]);
    out("R11", p->uc_mcontext.gregs[REG_R11]);
    out("R12", p->uc_mcontext.gregs[REG_R12]);
    out("R13", p->uc_mcontext.gregs[REG_R13]);
    out("R14", p->uc_mcontext.gregs[REG_R14]);
    out("R15", p->uc_mcontext.gregs[REG_R15]);
    out("RDI", p->uc_mcontext.gregs[REG_RDI]);
    out("RSI", p->uc_mcontext.gregs[REG_RSI]);
    out("RBP", p->uc_mcontext.gregs[REG_RBP]);
    out("RBX", p->uc_mcontext.gregs[REG_RBX]);
    out("RDX", p->uc_mcontext.gregs[REG_RDX]);
    out("RAX", p->uc_mcontext.gregs[REG_RAX]);
    out("RCX", p->uc_mcontext.gregs[REG_RCX]);
    out("RSP", p->uc_mcontext.gregs[REG_RSP]);
    out("RIP", p->uc_mcontext.gregs[REG_RIP]);
    out("EFL", p->uc_mcontext.gregs[REG_EFL]);
    out("CSGSFS", p->uc_mcontext.gregs[REG_CSGSFS]);
    out("ERR", p->uc_mcontext.gregs[REG_ERR]);
    out("TRAPNO", p->uc_mcontext.gregs[REG_TRAPNO]);
    out("OLDMASK", p->uc_mcontext.gregs[REG_OLDMASK]);
    out("CR2", p->uc_mcontext.gregs[REG_CR2]);
    int mpipe[2];
    int status = pipe(mpipe);
    //https://stackoverflow.com/questions/7134590/how-to-test-if-an-address-is-readable-in-linux-userspace-app
    if (status == -1) {
        write_string("Can't pipe\n");
        exit(0);
    }
    for (int i = -128; i < 128; i++) {
        char *t = static_cast<char*>(info->si_addr) + i;
        int status = write(mpipe[1], t, 1);
        if (i == 0) {
            write_string("[");
        } else {
            write_string(" ");
        }
        if (status == -1) {
            write_string("??");
        } else {
            uint8_t value;
            int status = read(mpipe[0], &value, 1);
            if (status == -1) {
                write_string("??");
            } else {
                out_byte(value);
            }
        }
        if (i == 0) {
            write_string("] ");
        } else {
            write_string("  ");
        }
        if ((i & 15) == 15) {
            write_string("\n");
        }
    }
    exit(1);
}

struct sigaction sigact;

int main() {
    sigact.sa_flags = SA_SIGINFO;
    int status = sigemptyset(&sigact.sa_mask);
    if (status == -1) {
        fprintf(stderr, "Can't make empty set of signals: %s\n", strerror(errno));
        return 0;
    }
    sigact.sa_sigaction = action;
    status = sigaction(SIGSEGV, &sigact, NULL);
    if (status == -1) {
        fprintf(stderr, "Can't make sigaction call: %s\n", strerror(errno));
        return 0;
    }
    //test 0 non writable memory
    //char* p = reinterpret_cast<char*>(main);
    //*p = 8;
    //test 1 null dereference
    //char *nll = NULL;
    //*nll = 5;
    //test 2 out of memory
    // int a[5];
    // for (int j = 0; ; j++) {
    //     a[j] = j;
    //     std::cout << a[j] << "\n";
    // }
    //test 3
    //char *nll = reinterpret_cast<char*>(std::numeric_limits<long long>::max());
    //*nll = 5;
    return 0;
}