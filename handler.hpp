#ifndef HANDLER_HPP
#define HANDLER_HPP

#include <csetjmp>
#include <string>

#include <signal.h>
#include <ucontext.h>
#include <stdexcept>

class handler {

public:

    handler();

    ~handler() = default;

    void cause_segfault();

    static void dump_registers(ucontext_t *ucontext);

    static void dump_memory(void *address);
    
private:

    static void handle(int signal, siginfo_t *siginfo, void *context);

    static void handle_inner(int, siginfo_t *, void *);

    static jmp_buf jmp;

};

#endif // HANDLER_HPP
