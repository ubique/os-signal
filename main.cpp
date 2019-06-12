#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ucontext.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <vector>

const int PAGESIZE = 4096;

std::string to_hex(size_t value) {
    std::stringstream stream;
    stream << std::hex << value;
    std::string res = stream.str();
    return "0x" + res + (res.size() == 1 ? "0" : "");
}

void write_to_stderr(const char* pointer, size_t len) {
    size_t result;
    while (len && (result = write(STDERR_FILENO, pointer, len))) {
        if (result < 0 && errno == EINTR) {
            continue;
        }
        if (result < 0) {
            exit(EXIT_FAILURE);
        }
        len -= result;
        pointer += result;
    }
}

void write_to_stderr(std::string const& pointer) {
    write_to_stderr(pointer.c_str(), pointer.size());
}

std::string get_register_name(int reg) {
    switch (reg) {
    case REG_R8:
        return "R8";
    case REG_R9:
        return "R9";
    case REG_R10:
        return "R10";
    case REG_R11:
        return "R11";
    case REG_R12:
        return "R12";
    case REG_R13:
        return "R13";
    case REG_R14:
        return "R14";
    case REG_R15:
        return "R15";
    case REG_RDI:
        return "RDI";
    case REG_RSI:
        return "RSI";
    case REG_RBP:
        return "RBP";
    case REG_RBX:
        return "RBX";
    case REG_RDX:
        return "RDX";
    case REG_RAX:
        return "RAX";
    case REG_RCX:
        return "RCX";
    case REG_RSP:
        return "RSP";
    case REG_RIP:
        return "RIP";
    case REG_EFL:
        return "EFL";
    case REG_CSGSFS:
        return "CSGSFS";
    case REG_ERR:
        return "ERR";
    case REG_TRAPNO:
        return "TRAPNO";
    case REG_OLDMASK:
        return "OLDMASK";
    case REG_CR2:
        return "CR2";
    default:
        return "";
    }
}


void sig_handler(int sig, siginfo_t* info, void* ucontext) {
    write_to_stderr("Segmentation fault at " + to_hex((size_t)info->si_addr) + "\n");
    write_to_stderr("Registers:\n");
    for (size_t i = 0; i < __NGREG; i++) {
        std::string reg = get_register_name(i);
        if (reg != "") {
            write_to_stderr(reg + "=" + to_hex(((ucontext_t*)ucontext)->uc_mcontext.gregs[i]) + '\n');
        }
    }
    for (size_t i = 0; i < __NGREG; i++) {
        std::string reg = get_register_name(i);
        if (reg != "") {
            write_to_stderr(reg + "=" + to_hex(((ucontext_t*)ucontext)->uc_mcontext.gregs[i]) + '\n');
        }
    }

    if (info->si_addr != nullptr) {
        size_t start = 1;
        if ((size_t)info->si_addr > 64) {
            start = (size_t)info->si_addr - 64;
        }
        size_t finish = std::numeric_limits<size_t>::max();
        if ((size_t)info->si_addr < std::numeric_limits<size_t>::max() - 64) {
            finish = (size_t)info->si_addr + 64;
        }
        write_to_stderr("Memory dump:\n");
        int fd = open("/dev/random", O_WRONLY);
        size_t j = 0;
        for(char* pos = (char*)start; pos != (char*)finish; pos++, j++) {
            if (j % 8 == 0) {
                 write_to_stderr(std::to_string((ptrdiff_t)pos - (ptrdiff_t)info->si_addr) + ":\t");
            }
            std::string data = "????";
            if (write(fd, pos, 1) >= 0) {
               data = to_hex((size_t)(*pos) & 0xff);
            }
            write_to_stderr(data + " ");
            if (j % 8 == 7) {
                write_to_stderr("\n");
            }
        }
    }
    exit(EXIT_FAILURE);
}


int main() {
    struct sigaction act{};
    if (sigemptyset(&act.sa_mask) < 0) {
        perror("sigemptyset failed");
        return EXIT_FAILURE;
    }
    act.sa_sigaction = sig_handler;
    act.sa_flags = SA_SIGINFO | SA_NODEFER;
    if (sigaction(SIGSEGV, &act, nullptr) < 0) {
        perror("sigaction failed");
        return EXIT_FAILURE;
    }
    char* a;
    char* alligned;
    a = (char *)malloc(2 * PAGESIZE + 1024 - 1);
    if (a == nullptr) {
        perror("malloc failed");
        return EXIT_FAILURE;
    }
    alligned = (char *)(((size_t) a + PAGESIZE - 1) & ~(PAGESIZE - 1));
    for (size_t i = 0; i < 2 * PAGESIZE; i++) {
        alligned[i] = 'A' + i % 26;
    }
    if (mprotect(alligned, PAGESIZE, PROT_NONE) < 0) {
        perror("mprotect");
        return EXIT_FAILURE;
    }
    alligned[PAGESIZE - 1] = 'y';
    free(a);
}
