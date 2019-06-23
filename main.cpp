#include <signal.h>
#include <string.h>
#include <setjmp.h>
#include <sys/mman.h>
#include <signal.h>
#include <unistd.h>

#include <iostream>
#include <vector>

const int STDOUT = 1, STDERR = 2;

const char *regs_name[] = {
        "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15",
        "RDI", "RSI", "RBP", "RBX", "RDX", "RAX", "RCX", "RSP", "RIP", "EFL",
        "CSGSFS", "ERR", "TRAPNO", "OLDMASK", "CR2"};

void write(const char *str) {
    write(STDOUT, str, strlen(str));
}

void write_error(const char *str) {
    write(STDERR, str, strlen(str));
}

void write_hex_string(size_t n) {
    char *buffer = new char[sizeof(size_t) * 2 + 3];
    unsigned short k = 2;
    buffer[0] = '0';
    buffer[1] = 'x';

    for (int i = sizeof (size_t) - 1; i >= 0; --i) {
        uint8_t bt = 0xFF & (n >> (i * 8));
        uint8_t number = bt / 16;
        buffer[k++] = static_cast<char>(number + (number < 10 ? '0' : 'A' - 10));
        number = bt % 16;
        buffer[k++] = static_cast<char>(number + (number < 10 ? '0' : 'A' - 10));
    }
    buffer[k] = '\0';
    write(buffer);
    delete[] buffer;
}

void reg_dump(void *ucontext) {
    write("Registers:\n");
    mcontext_t mcontext = ((ucontext_t *)ucontext)->uc_mcontext;
    for (int i = 0; i < __NGREG; i++) {
        write(regs_name[i]);
        write(" : ");
        write_hex_string((size_t)  mcontext.gregs[i]);
        write("\n");
    }
}

sigjmp_buf jb;
void mem_handler(int sig) {
    siglongjmp(jb, 1);
}

const int env_size = 16;
void memory_dump(char* addr) {
    write("Memory : \n");

    addr -= env_size;

    for (int i = 0; i < env_size * 2; i++) {
        char pref[] = "   ";
        if (i == env_size) {
            memcpy(pref, "-> ", 2);
        }

        sigset_t sigset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGSEGV);
        sigprocmask(SIG_UNBLOCK, &sigset, NULL);

        struct sigaction action;
        memset(&action, 0, sizeof(struct sigaction));
        action.sa_flags = SA_NODEFER;
        action.sa_handler = mem_handler;
        action.sa_mask = sigset;
        if (sigaction(SIGSEGV, &action, NULL) < 0) {
            write_error("sigaction");
            _exit(1);
        }

        if (setjmp(jb) == 0) {
            write(pref);
            write("address = ");
            write_hex_string((size_t) addr);
            write("data = ");
            write_hex_string(*addr);
            write("\n");
        } else {
            write(pref);
            write("can't get data from this address = ");
            write_hex_string((size_t) addr);
            write("\n");
        }
        addr++;
    }
}

void handler(int sig, siginfo_t *info, void *ucontext) {
    write("Segmentation fault\n");
    write("Address with error: ");
    write_hex_string((size_t) info->si_addr);
    write("\n");
    memory_dump((char *) info->si_addr);
    reg_dump(ucontext);
    _exit(0);
}
int main() {

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = handler;

    if (sigaction(SIGSEGV, &action, NULL) < 0) {
        perror("sigaction");
        return 1;
    }


    return 0;
}
