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

    void dump_registers(ucontext_t *ucontext);

    struct handler_exception : public std::runtime_error {

        handler_exception(std::string const &message, int error = 0) : std::runtime_error(message), error(error) {};

        int error;

    };

private:

    static void handle(int signal, siginfo_t *siginfo, void *context);

    static jmp_buf jmp;

};

#endif // HANDLER_HPP
