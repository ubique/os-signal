#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/ucontext.h>
#include <sys/mman.h>

#include <iostream>
#include <vector>

const char *regs_name[] = {
        "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15",
        "RDI", "RSI", "RBP", "RBX", "RDX", "RAX", "RCX", "RSP", "RIP", "EFL",
        "CSGSFS", "ERR", "TRAPNO", "OLDMASK", "CR2"};

void reg_dump(void *ucontext) {

    printf("Registers:\n");
    mcontext_t mcontext = ((ucontext_t *)ucontext)->uc_mcontext;
    for (int i = 0; i < __NGREG; i++) {
        printf(" %s : %u\n", regs_name[i], (unsigned int) mcontext.gregs[i]);
    }
}

sigjmp_buf jb;
void mem_handler(int sig) {
    siglongjmp(jb, 1);
}

const int env_size = 16;
void memory_dump(char* addr) {

    printf("Memory : \n");

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
            perror("sigaction");
            exit(1);
        }

        if (setjmp(jb) == 0) {
            printf("%sadress = %p   data = %c\n", pref, addr, *addr);
        } else {
            printf("%scan't get data from this address = %p\n", pref, addr);
        }
        addr++;
    }
}

void handler(int sig, siginfo_t *info, void *ucontext) {

    std::cout << "Segmentation fault" << std::endl;
    std::cout << "Address with error: " << info->si_addr << std::endl;

    memory_dump((char *) info->si_addr);
    reg_dump(ucontext);
    exit(0);
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