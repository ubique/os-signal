#include <stdio.h>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <unistd.h>

#include "context.hpp"
#include "sigsegv_handler.hpp"

sigsegv_handler::sigsegv_handler()
{
    // static class
}

int sigsegv_handler::get_ready() {
    struct sigaction sa; // mask of signals which should be blocked
    sa.sa_flags = SA_SIGINFO | SA_NODEFER;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = &handle;
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("Can't change sigsegv action");
        return -1;
    }
    return 0;
}

void write_string(const char * s) {
    write(STDERR_FILENO, s, strlen(s));
}

void sigsegv_handler::handle(int, siginfo_t * siginf, void * context)
{
    write_string("! SIGSEGV happened !\n");

    ucontext_t * p = (ucontext_t *)context;
    write_string("=== REG ===\n");
    auto reg_name = regs.begin();
    for (int i = 0; i < NGREG; ++i) {
        if ((reg_name = regs.find(i)) != regs.end()) {
            const std::string out_val = reg_name->second + " = " + std::to_string(p->uc_mcontext.gregs[i]) + "\n";
            write_string(out_val.c_str());
        }
    }

    write_string("=== MEM ===\n");
    // https://www.geeksforgeeks.org/non-blocking-io-with-pipes-in-c/
    // https://stackoverflow.com/questions/7134590/how-to-test-if-an-address-is-readable-in-linux-userspace-app
    int pipefd[2];
    try_create_pipe(pipefd);
    uint8_t buf;
    for (int i = int(std::numeric_limits<int8_t>::min()); i < int(std::numeric_limits<int8_t>::max()); ++i) {
        auto mem_value = static_cast<char *>(siginf->si_addr) + i;
        auto pipest_write = write(pipefd[1], mem_value, 1);
        if (i == 0) {
            write_string("[");
        }
        if (pipest_write == -1) {
            write_string(i == 0 ? "XX] " : "XX ");
            continue;
        }
        auto pipest_read = read(pipefd[0], &buf, 1);
        if (pipest_read == -1) {
            write_string("XX ");
        } else {
            const std::string bt = std::to_string(buf) + " ";
            write_string(bt.c_str());
        }
        if (i == 0) {
            write_string("]");
        }
    }
    write_string("\n");
    exit(EXIT_FAILURE);
}

void sigsegv_handler::try_create_pipe(int * pipefd)
{
    int pipest = pipe(pipefd);
    if (pipest == -1) {
        write_string("Can't pipe\n");
        exit(EXIT_FAILURE);
    }
}
