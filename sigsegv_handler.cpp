#include <iostream>
#include <limits>
#include <unistd.h>

#include "sigsegv_handler.hpp"
#include "writer.hpp"

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

void sigsegv_handler::handle(int, siginfo_t * siginf, void * context)
{
    write_string("! SIGSEGV happened !\n");
    dump_registers(context);
    dump_memory(siginf);
    _exit(EXIT_FAILURE);
}

void sigsegv_handler::dump_registers(void * context) {
    ucontext_t * p = (ucontext_t *)context;
    write_string("=== REG ===\n");
    write_register("R8", p->uc_mcontext.gregs[REG_R8]);
    write_register("R9", p->uc_mcontext.gregs[REG_R9]);
    write_register("R10", p->uc_mcontext.gregs[REG_R10]);
    write_register("R11", p->uc_mcontext.gregs[REG_R11]);
    write_register("R12", p->uc_mcontext.gregs[REG_R12]);
    write_register("R13", p->uc_mcontext.gregs[REG_R13]);
    write_register("R14", p->uc_mcontext.gregs[REG_R14]);
    write_register("R15", p->uc_mcontext.gregs[REG_R15]);
    write_register("RDI", p->uc_mcontext.gregs[REG_RDI]);
    write_register("RSI", p->uc_mcontext.gregs[REG_RSI]);
    write_register("RBP", p->uc_mcontext.gregs[REG_RBP]);
    write_register("RBX", p->uc_mcontext.gregs[REG_RBX]);
    write_register("RDX", p->uc_mcontext.gregs[REG_RDX]);
    write_register("RAX", p->uc_mcontext.gregs[REG_RAX]);
    write_register("RCX", p->uc_mcontext.gregs[REG_RCX]);
    write_register("RSP", p->uc_mcontext.gregs[REG_RSP]);
    write_register("RIP", p->uc_mcontext.gregs[REG_RIP]);
    write_register("EFL", p->uc_mcontext.gregs[REG_EFL]);
    write_register("CSGSFS", p->uc_mcontext.gregs[REG_CSGSFS]);
    write_register("ERR", p->uc_mcontext.gregs[REG_ERR]);
    write_register("TRAPNO", p->uc_mcontext.gregs[REG_TRAPNO]);
    write_register("OLDMASK", p->uc_mcontext.gregs[REG_OLDMASK]);
    write_register("CR2", p->uc_mcontext.gregs[REG_CR2]);
}

void sigsegv_handler::dump_memory(siginfo_t * siginf) {
    write_string("=== MEM ===\n");
    int pipefd[2];
    try_create_pipe(pipefd);
    uint8_t buf;
    for (int i = -127; i < 128; ++i) {
        const auto mem_value = static_cast<char *>(siginf->si_addr) + i;
        auto pipest_write = write(pipefd[1], mem_value, 1);
        if (i == 0) {
            write_string("[");
        }
        if (pipest_write == -1) {
            write_string(i == 0 ? "XX] " : "XX ");
            continue;
        }
        const auto pipest_read = read(pipefd[0], &buf, 1);
        if (pipest_read == -1) {
            write_string("XX ");
        } else {
            write_hex_number(buf);
            write_string(" ");
        }
        if (i == 0) {
            write_string("]");
        }
    }
    write_string("\n");
}

void sigsegv_handler::try_create_pipe(int * pipefd)
{
    const auto pipest = pipe(pipefd);
    if (pipest == -1) {
        write_string("Can't pipe\n");
        _exit(EXIT_FAILURE);
    }
}
