#include "handler.hpp"

#include <iostream>
#include <iomanip>
#include <cstring>

#include <signal.h>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace console {

    std::string _ERROR = "\033[31;1m";
    std::string _HELP = "\033[32;1m";
    std::string _DEFAULT = "\033[0m";
    std::string _BOLD = "\033[1m";
    std::string _INFO = "\033[34;1m";

    // @formatter:off
    std::string USAGE = "SIGSEGV handler + regdump v.1.0.0\n"
                        "Usage: ./segfault (no arguments)";
    // @formatter:on

    int report(std::string const &message, int err = 0) {
        std::cerr << _ERROR << message;
        if (err != 0) {
            std::cerr << std::strerror(errno);
        }
        std::cerr << std::endl << _DEFAULT;
        return 0;
    }

    void notify(std::string const &message) {
        std::cout << _INFO << message << std::endl << _DEFAULT;
    }

}

handler::handler() {
    struct sigaction action{};

    action.sa_flags = SA_NODEFER | SA_SIGINFO;
    action.sa_sigaction = &handle;

    if (sigaction(SIGSEGV, &action, nullptr) < 0) {
        throw handler_exception("Could not set signal handler", errno);
    }
}

void handler::cause_segfault() {
    char mem[0];
    mem[0] /= 0;
}

void handler::dump_registers(ucontext_t *ucontext) {
    static std::unordered_map<std::string, int> registers{
            {"R8",      REG_R8},
            {"R9",      REG_R8},
            {"R10",     REG_R8},
            {"R11",     REG_R8},
            {"R12",     REG_R8},
            {"R13",     REG_R8},
            {"R14",     REG_R8},
            {"R15",     REG_R8},
            {"RAX",     REG_R8},
            {"RBP",     REG_R8},
            {"RBX",     REG_R8},
            {"RCX",     REG_R8},
            {"RDI",     REG_R8},
            {"RDX",     REG_R8},
            {"RIP",     REG_RIP},
            {"RSI",     REG_RSI},
            {"RSP",     REG_RSP},
            {"CR2",     REG_CR2},
            {"CSGSFS",  REG_CSGSFS},
            {"EFL",     REG_EFL},
            {"ERR",     REG_ERR},
            {"OLDMASK", REG_OLDMASK},
            {"TRAPNO",  REG_TRAPNO}
    };

    console::notify("REGISTERS");
    for (auto const &pair : registers) {
        std::ios state(nullptr);
        state.copyfmt(std::cout);

        std::cout << std::setw(8) << std::setfill(' ') << pair.first;
        std::cout.copyfmt(state);

        std::cout << ": " << std::hex << ucontext->uc_mcontext.gregs[pair.second] << std::endl;
        std::cout.copyfmt(state);
    }
}

void handler::handle(int signal, siginfo_t *siginfo, void *context) {
    if (siginfo->si_signo == SIGSEGV) {

    }
}

int main() {

    handler().cause_segfault();

}