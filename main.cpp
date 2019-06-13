//
// Created by roman on 07.06.19.
//
#include <iostream>
#include <signal.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <limits>
#include <iomanip>
#include <sys/mman.h>
#include <malloc.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

template<typename T>
void print_hex_num(T num, int len) {
    write(STDOUT_FILENO, "0x", 2);
    uint8_t ones = 15;
    for (int i = 0; i < len; ++i) {
        char show = (num >> (len - 1 - i) * 4) & ones;
        show += show < 10 ? '0' : 'A' - 10;
        write(STDOUT_FILENO, &show, 1);
    }
}

template<typename T>
void print_register(const char* name, T reg, gregset_t *gregset) {
    write(STDOUT_FILENO, name, strlen(name));
    write(STDOUT_FILENO, ": ", 2);
    print_hex_num((*gregset)[reg], sizeof(void *) * 2);
    write(STDOUT_FILENO, "\n", 1);
}

void print_er(const char *str) {
    write(STDERR_FILENO, str, strlen(str));
    write(STDERR_FILENO, "\n", 1);
}

void print(const char *str) {
    write(STDOUT_FILENO, str, strlen(str));
    write(STDOUT_FILENO, "\n", 1);
}

static constexpr size_t radius = 32;

static const char *no_permission = "####";
static const char *undefined = "****";
static const char *delimeter = ", ";
static const char *l_ar = "-->";
static const char *r_ar = "<--";

static void handler(int sig, siginfo_t *info, void *ucontext) {
    print("SIGSEGV caught");
    auto *context = (ucontext_t *) ucontext;
    gregset_t* gregset= &context->uc_mcontext.gregs;
    print_register("RAX", REG_RAX, gregset);
    print_register("RBX", REG_RBX, gregset);
    print_register("RCX", REG_RCX, gregset);
    print_register("RDX", REG_RDX, gregset);
    print_register("RBP", REG_RBP, gregset);
    print_register("RSP", REG_RSP, gregset);
    print_register("RSI", REG_RSI, gregset);
    print_register("RDI", REG_RDI, gregset);
    print_register("R8", REG_R8, gregset);
    print_register("R9", REG_R9, gregset);
    print_register("R10", REG_R10, gregset);
    print_register("R11", REG_R11, gregset);
    print_register("R12", REG_R12, gregset);
    print_register("R13", REG_R13, gregset);
    print_register("R14", REG_R14, gregset);
    print_register("R15", REG_R15, gregset);
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        print_er("Pipe error");
        print_er("Cannot dump memory");
        _exit(EXIT_FAILURE);
    }
    cout << "Memory dump:" << endl;
    auto adr = (size_t) info->si_addr;
    auto ptr = (char *) info->si_addr;
    auto lower = (char *) (adr >= radius ? adr - radius : 0);
    auto upper = (char *) (std::numeric_limits<size_t>::max() - radius >= adr ?
                           adr + radius :
                           std::numeric_limits<size_t>::max());
    for (auto i = lower; i < upper; ++i) {
        if (i == ptr) {
            write(STDOUT_FILENO, l_ar, strlen(l_ar));
        }
        if (write(pipefd[1], i, 1) != -1) {
            print_hex_num(*i, 2);
        } else {
            if (errno == EFAULT) {
                write(STDOUT_FILENO, no_permission, strlen(no_permission));
            } else {
                write(STDOUT_FILENO, undefined, strlen(undefined));
            }
        }
        if (i == ptr) {
            write(STDOUT_FILENO, r_ar, strlen(r_ar));
        }
        if (i != upper) {
            write(STDOUT_FILENO, delimeter, strlen(delimeter));
        }
        if ((i - lower) % 16 == 15) {
            write(STDOUT_FILENO, "\n", 1);
        }
    }
    _exit(EXIT_FAILURE);

}

int main(int argc, char *argv[]) {
    struct sigaction act{};
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = handler;
    if (sigaction(SIGSEGV, &act, nullptr) == -1) {
        cerr << "Error during sigaction: " << strerror(errno);
        exit(EXIT_FAILURE);
    }
    //do not expect any errors from sys calls))))
    int pagesize = sysconf(_SC_PAGE_SIZE); //4096 expected
    char *buffer = (char *) memalign(pagesize, 4 * pagesize);
    for (int i = 0; i < 4 * pagesize; ++i) {
        buffer[i] = i % 256;
    }
    mprotect(buffer + pagesize, pagesize, PROT_NONE);
    buffer[4097] = 'h';
    return 0;
}
