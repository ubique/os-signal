#include <sys/param.h>
#include <sys/ucontext.h>
#include <iostream>
#include <iomanip>
#include  <cstring>
#include <unistd.h>
#include <limits>
#include <signal.h>
#include <map>
static const size_t RADIUS = 8;

void print_char(uint8_t k) {
    write(1, &k, 1);
}

void print_str(const char *s) {
    write(1, s, strlen(s));
}

void print_s(uint8_t val) {
    char c;
    c = val < 10 ? val + '0' : val + 'A' - 10;
    write(1, &c, 1);
}

void print_byte(uint8_t k) {
    print_s(k / 16);
    print_s(k % 16);
}


void print_number(uint64_t num, size_t sz) {
    print_str("0x");
    for (int i = sz; i >= 1; --i) {
        print_byte(0xFF & (num >> (8 * (i - 1))));
    }
}


void printError(const char *message) {
    print_str(message);
}


void printHeader(const char *message) {
    print_str(">>>>>>>>>>>");
    print_str(message);
    print_str("<<<<<<<<<<<<<<<\n");
}

void printLine() {
    print_str("------------------------------------------------\n");
}
const std::map<const char *, uint64_t> registers = {
        {"R8",      REG_R8},
        {"R9",      REG_R9},
        {"R10",     REG_R10},
        {"R11",     REG_R11},
        {"R12",     REG_R12},
        {"R13",     REG_R13},
        {"R14",     REG_R14},
        {"R15",     REG_R15},
        {"RAX",     REG_RAX},
        {"RBX",     REG_RBX},
        {"RCX",     REG_RCX},
        {"RDX",     REG_RDX},
        {"RDI",     REG_RDI},
        {"RBP",     REG_RBP},
        {"RIP",     REG_RIP},
        {"RSP",     REG_RSP},
        {"RSI",     REG_RSI},
        {"ERR",     REG_ERR},
        {"EFL",     REG_EFL},
        {"CSGSFS",  REG_CSGSFS},
        {"CR2",     REG_CR2},
        {"OLDMASK", REG_OLDMASK},
        {"TRAPNO",  REG_TRAPNO},
};

void mem_dump(void* address_v) {
    size_t address = reinterpret_cast<size_t>(address_v);
    char *mem = reinterpret_cast<char *>(address_v);
    printHeader("MEM_DUMP");
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        print_str("Unable to dump memory\n");
        return;
    }
    char * start = reinterpret_cast<char *>(std::max((long long) 0, (long long) ((char *) address - 16 * sizeof(char))));
    char * end_ = reinterpret_cast<char *>(std::min(LONG_LONG_MAX, (long long) ((char *) address + 16 * (sizeof(char)))));

    for (char * i = start; i < end_; i++) {
        if(i == mem){
            print_str("->");
        }
        if (write(pipefd[1], i, 1) != -1) {
            print_number((static_cast<uint64_t>(*i) & 0xFFu), 1);
        } else {
            print_str("bad");
        }
        if(i == mem){
            print_str("<-");
        }
       print_str(" ");
    }
}

void registers_dump(ucontext_t *context) {
    printHeader("REGISTERS");
    printLine();
    for (const auto &reg : registers) {
        print_str(reg.first);
        print_str(" | ");
        print_number(context->uc_mcontext.gregs[reg.second], 8);
        print_str("\n");
        printLine();
    }
}

void handler_sigsegv(int sig_num, siginfo_t *sig_info, void *context) {
    size_t address = reinterpret_cast<size_t>(sig_info->si_addr);
    if (sig_info->si_signo == SIGSEGV) {
        print_str("!--->SIGSEGV_FAULT<---!\n");
        print_str("No access to ");
        print_number(address, 8);
        print_str("\n");
        if (sig_info->si_code == SEGV_ACCERR) {
            print_str("No permission for mapped object.\n");
        } else if (sig_info->si_code == SEGV_MAPERR) {
            print_str("Not mapped to object.\n");
        } else {
            print_str("Unknown ERROR\n");
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
    print_str("]CHOOSE PROGRAMME:\n");
    print_str("1 -- OUT OF BOUNDS\n");
    print_str("2 -- NULL\n");
    int num;
    std::cin >> num;
    switch (num) {
        case 1: {
            char *s = (char *) ("CT-MEM");
            s[50] = '!';
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
