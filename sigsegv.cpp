//
// Created by dumpling on 09.06.19.
//

#include <iostream>
#include <signal.h>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <limits>
#include <algorithm>
#include <iomanip>

static const size_t RANGE = 32;

std::string get_hex(size_t n, bool is_reg = false) {
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(is_reg ? 16 : 2) << std::hex << n;
    std::string hex(stream.str());

    return hex;
}

void print_err(const std::string &message) {
    std::cerr << "\033[31m" << message;
    if (errno) {
        std::cerr << ": " << std::strerror(errno);
    }
    std::cerr << "\033[0m" << std::endl;
}

void write(const std::string &msg) {

    int size = msg.length();

    int total = 0;
    while (total < size) {
        int was_send = write(STDERR_FILENO, msg.data() + total, size - total);
        if (was_send == -1) {
            throw std::runtime_error("Write failed");
        }

        total += was_send;
    }
}

std::string reg_to_str(int reg) {
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

    write("Segmentation fault at " + get_hex(addr) + "\n");
    write("Registers:\n");
    for (int i = 0; i < __NGREG; ++i) {
        std::string reg = reg_to_str(i);
        if (!reg.empty()) {
            write(reg + " : 0x" + get_hex(context->uc_mcontext.gregs[i], true) + '\n');
        }
    }


    if (info->si_addr != nullptr) {
        size_t st = addr > RANGE ? addr - RANGE : 1;
        size_t fin = addr < SIZE_MAX - RANGE ? addr + RANGE : SIZE_MAX;

        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            throw std::runtime_error("Can't dump memory");
        }

        int j = 0;
        write("\nMemory from " + get_hex(st) + " to " + get_hex(fin) + ":\n");
        for (auto p = reinterpret_cast<char *>(st); p != reinterpret_cast<char *>(fin); ++p) {
            if (p == info->si_addr) {
                write("\033[31m");
            }

            if (write(pipe_fd[1], p, 1) != -1) {
                char tmp = *p;
                auto s = get_hex(static_cast<uint>(static_cast<u_char>(tmp)));
                write(s);
            } else {
                write("??");
            }

            if (p == info->si_addr) {
                write("\033[0m");
            }

            write(" ");
            if (++j % 8 == 0) {
                write("\n");
            }
        }
    }
    exit(EXIT_FAILURE);
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