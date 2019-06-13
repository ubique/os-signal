#ifndef SIGSEGV_HANDLER_HPP_
#define SIGSEGV_HANDLER_HPP_

#include <csignal>

struct sigsegv_handler {
    static int get_ready();

private:
    sigsegv_handler();

    static void handle(int, siginfo_t * siginf, void * context);
    static void dump_registers(void * context);
    static void dump_memory(siginfo_t * siginf);

    static void try_create_pipe(int * pipefd);
};

#endif // SIGSEGV_HANDLER_HPP_
