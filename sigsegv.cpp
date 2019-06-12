//
// Created by dumpling on 09.06.19.
//

#include <iostream>
#include <signal.h>
#include <cstring>
#include <unistd.h>
#include <limits>
#include <assert.h>

static const size_t RANGE = 32;

void print_err(const char *message) {
    std::cerr << "\033[31m" << message;
    if (errno) {
        std::cerr << ": " << std::strerror(errno);
    }
    std::cerr << "\033[0m" << std::endl;
}

void write_str(const char *msg) {
    write(STDERR_FILENO, msg, strlen(msg));
}

void write_half_byte(uint8_t half_byte) {
    char c;
    if (half_byte < 10) {
        c = '0' + half_byte;
    } else {
        c = 'a' + (half_byte - 10);
    }

    write(STDERR_FILENO, &c, 1);
}

void write_byte(uint8_t byte) {
    write_half_byte(byte / 16);
    write_half_byte(byte % 16);
}

void write_size_t(size_t addr) {
    for (size_t i = ((size_t) 1 << (SIZE_WIDTH - 8)); i > 0; i >>= 8) {
        write_byte(addr / i);
        addr %= i;
    }
}

const char *reg_to_str(int reg) {
    switch (reg) {
        case REG_RAX:
            return "RAX";
        case REG_RBX:
            return "RBX";
        case REG_RCX:
            return "RCX";
        case REG_RDX:
            return "RDX";
        case REG_RDI:
            return "RDI";
        case REG_RSI:
            return "RSI";
        case REG_RBP:
            return "RBP";
        case REG_RSP:
            return "RSP";
        case REG_R8:
            return "R8 ";
        case REG_R9:
            return "R9 ";
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
        default:
            return "";
    }
}

void handler(int sig, siginfo_t *info, void *ucontext) {
    auto *context = static_cast<ucontext_t *>(ucontext);
    auto addr = reinterpret_cast<size_t>(info->si_addr);

    write_str("Segmentation fault at ");
    write_size_t(addr);
    write_str("\nRegisters:\n");
    for (int i = 0; i < __NGREG; ++i) {
        const char *reg = reg_to_str(i);
        if (strcmp(reg, "") != 0) {
            write_str(reg);
            write_str(" : 0x");
            write_size_t(context->uc_mcontext.gregs[i]);
            write_str("\n");
        }
    }


    if (info->si_addr != nullptr) {
        size_t st = addr > RANGE ? addr - RANGE : 1;
        size_t fin = addr < SIZE_MAX - RANGE ? addr + RANGE : SIZE_MAX;

        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            write_str("\nCan't dump memory\n");
            _exit(EXIT_FAILURE);
        }

        int j = 0;
        write_str("\nMemory from 0x");
        write_size_t(st);
        write_str(" to 0x");
        write_size_t(fin);
        write_str(":\n");
        for (auto p = reinterpret_cast<char *>(st); p != reinterpret_cast<char *>(fin); ++p) {
            if (p == info->si_addr) {
                write_str("\033[31m");
            }

            if (write(pipe_fd[1], p, 1) != -1) {
                char tmp = *p;
                auto s = static_cast<uint8_t>(static_cast<u_char>(tmp));
                write_byte(s);
            } else {
                write_str("??");
            }

            if (p == info->si_addr) {
                write_str("\033[0m");
            }

            write_str(" ");
            if (++j % 8 == 0) {
                write_str("\n");
            }
        }
    }
    _exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

    struct sigaction act{};
    if (sigemptyset(&act.sa_mask) < 0) {
        print_err("Sigemptyset failed");
        return EXIT_FAILURE;
    }
    act.sa_sigaction = handler;
    act.sa_flags = SA_SIGINFO | SA_NODEFER;
    if (sigaction(SIGSEGV, &act, nullptr) == -1) {
        print_err("Sigaction failed");
        return EXIT_FAILURE;
    }

    if (argc == 2) {
        auto arg = std::string(argv[1]);
        if (arg == "test_const_char") {
            char *a = (char *) "Hello world!";
            a[5] = 'm';
        }

        if (arg == "test_nullptr") {
            char *a = nullptr;
            int b = *a;
        }
    } else {
        char *a = (char *) "Hi";
        a[2] = 'm';
    }

}