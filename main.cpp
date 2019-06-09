#include <csignal>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <cmath>
#include <csetjmp>
#include <limits.h>
void printError(const std::string &message) {
    std::cerr << message << ":" << strerror(errno) << std::endl;
}

template<typename T>
void print(const T &message) {
    std::cout << message;
}

template<typename T>
void println(const T &message) {
    std::cout << message << std::endl;
}

void printHeader(const std::string &message) {
    std::cout << ">>>>>>>>>>>" << message << "<<<<<<<<<<<<<<<" << std::endl;
}

void printLine() {
    std::cout << "-----------------------------" << std::endl;
}

const char *get_reg(int reg) {
    switch (reg) {
        case 0:
            return "REG_R8";
        case 1:
            return "REG_R9";
        case 2:
            return "REG_R10";
        case 3:
            return "REG_R11";
        case 4:
            return "REG_R12";
        case 5:
            return "REG_R13";
        case 6:
            return "REG_R14";
        case 7:
            return "REG_R15";
        case 8:
            return "REG_RDI";
        case 9:
            return "REG_RSI";
        case 10:
            return "REG_RBP";
        case 11:
            return "REG_RBX";
        case 12:
            return "REG_RDX";
        case 13:
            return "REG_RAX";
        case 14:
            return "REG_RCX";
        case 15:
            return "REG_RSP";
        case 16:
            return "REG_RIP";
        case 17:
            return "REG_EFL";
        case 18:
            return "REG_CSGSFS";
        case 19:
            return "REG_ERR";
        case 20:
            return "REG_TRAPNO";
        case 21:
            return "REG_OLDMASK";
        case 22:
            return "REG_CR2";
        default:
            return "REG_UNKNOWN";
    }
}


void print_register(int r, ucontext_t *context) {
    printf("%-15s\t | 0x%x \n", get_reg(r), (unsigned int) context->uc_mcontext.gregs[r]);
    printLine();
}

static jmp_buf jbuf;

void print_register(int r);

void handler(int signum, siginfo_t *siginfo, void *context) {
    if (siginfo->si_signo == SIGSEGV) {
        siglongjmp(jbuf, 1);
    }
}

void address_dump(long long address) {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGSEGV);
    sigprocmask(SIG_UNBLOCK, &sigset, nullptr);

    struct sigaction action{};
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = handler;
    action.sa_flags = SA_SIGINFO;

    if (sigaction(SIGSEGV, &action, nullptr) < 0) {
        printError("Sigaction failed");
        exit(EXIT_FAILURE);
    }
    const char *p = (const char *) address;
    if (setjmp(jbuf) == 0) {
        printf("ADDRESS %p | %x\n", (void *) address, (int) p[0]);
    } else {
        printf("ADDRESS %p | bad\n", (void *) address);
    }
    printLine();
}

void mem_dump(void *address) {
    long long start, end_;
    printHeader("MEM_DUMP");
    start = std::max((long long) 0, (long long) ((char *) address - 20 * sizeof(char)));
    end_ = std::min(LONG_LONG_MAX, (long long) ((char *) address + 20 * (sizeof(char))));
    for (long long i = start; i < end_; i += sizeof(char)) {
        address_dump(i);
    }
}

void registers_dump(ucontext_t *context) {
    printHeader("REGISTERS");
    printLine();
    for (int r = 0; r < NGREG; ++r) {
        print_register(r, context);
    }
}


void handler_sigsegv(int sig_num, siginfo_t *sig_info, void *context) {
    if (sig_info->si_signo == SIGSEGV) {
        println("!--->SIGSEGV_FAULT<---!");
        print("No access to ");
        std::cout << sig_info->si_addr << std::endl;
        if (sig_info->si_code == SEGV_ACCERR) {
            println("No permission for mapped object.");
        } else if (sig_info->si_code == SEGV_MAPERR) {
            println("Not mapped to object.");
        } else {
            println("Unknown ERROR");
        }
    }
    registers_dump((ucontext_t *) context);
    mem_dump(sig_info->si_addr);
    exit(EXIT_FAILURE);
}


int main(int argc, char **argv) {
    struct sigaction action{};
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = handler_sigsegv;
    action.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &action, nullptr) < 0) {
        printError("Sigaction error");
        exit(EXIT_FAILURE);
    }
    std::cout << R"BLOCK(CHOOSE PROGRAMME:
1 -- OUT OF BOUNDS
2 -- NULL)BLOCK";
    std::cout << std::endl;
    int num;
    std::cin >> num;
    switch (num) {
        case 1: {
            char *s = (char *) ("CT-MEM");
            s[6] = '!';
            break;
        }
        case 2: {
            int *ptr = nullptr;
            *ptr = 5;
            break;
        }
        default: {
            std::cout << "??????????????????" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}
