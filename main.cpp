#include <unistd.h>
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ucontext.h>
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

char get_char(uint8_t num) {
    if (num < 10) {
        return '0' + num;
    } else {
        return 'A' + num - 10;
    }
}

void to_hex(char* buffer, size_t value, size_t len_in_bytes) {
    len_in_bytes *= 2;
    buffer[0] = '0';
    buffer[1] = 'x';
    for (size_t i = 0; i < len_in_bytes; i++) {
       buffer[2 + len_in_bytes - i - 1] = get_char(value & 0xf);
       value >>= 4;
    }
}

void to_str(char* buffer, int value) {
    size_t start = 0;
    if (value < 0) {
        value = -value;
        buffer[0] = '-';
        start++;
    }
    int cp = value;
    size_t lg = 0;
    if (value == 0) {
        lg = 1;
    }
    while (cp) {
        cp /= 10;
        lg++;
    }
    for (size_t i = 0; i < lg; i++) {
       buffer[start + lg - i - 1] = '0' + value % 10;
       value /= 10;
    }
    buffer[start + lg] = '\0';
}

void write_to_stderr(const char* pointer) {
    ssize_t result;
    size_t len = strlen(pointer);
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

const char* get_register_name(greg_t reg) {
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

void print_error(const char* message) {
    write_to_stderr(message);
    write_to_stderr(": ");
    write_to_stderr(std::strerror(errno));
    write_to_stderr("\n");
}


void sig_handler(int sig, siginfo_t* info, void* ucontext) {
    write_to_stderr("Segmentation fault at ");
    char buffer[1024];
    to_hex(buffer, (size_t)info->si_addr, 8);
    write_to_stderr(buffer);
    write_to_stderr("\nRegisters:\n");
    for (size_t i = 0; i < __NGREG; i++) {
        const char * reg = get_register_name(i);
        if (std::strcmp(reg, "") != 0) {
            write_to_stderr(reg);
            write_to_stderr("=");
            to_hex(buffer, ((ucontext_t*)ucontext)->uc_mcontext.gregs[i], 8);
            write_to_stderr(buffer);
            write_to_stderr("\n");
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
        int fd[2];
        if (pipe(fd) < 0) {
            print_error("Pipe failed");
            exit(EXIT_FAILURE);
        }
        size_t j = 0;
        for(char* pos = (char*)start; pos != (char*)finish; pos++, j++) {
            if (j % 8 == 0) {
                to_str(buffer, (ptrdiff_t)pos - (ptrdiff_t)info->si_addr);
                write_to_stderr(buffer);
                write_to_stderr(":\t");
            }
            std::strcpy(buffer, "????");
            if (write(fd[1], pos, 1) >= 0) {
               to_hex(buffer, (size_t)(*pos) & 0xff, 1);
            }
            write_to_stderr(buffer);
            write_to_stderr(" ");
            if (j % 8 == 7) {
                write_to_stderr("\n");
            }
        }
        if (close(fd[0]) < 0) {
            print_error("Can't close pipe");
        }
        if (close(fd[1]) < 0) {
            print_error("Can't close pipe");
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
