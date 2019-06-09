//
// Created by utusi on 09.06.19.
//

#include <iostream>
#include <cstring>
#include <climits>
#include <csignal>
#include <csetjmp>

static jmp_buf buf;
void sigsegv_address_handler(int code, siginfo_t* siginfo, void* context_of_programm) {
    if (siginfo -> si_signo == SIGSEGV) {
        siglongjmp(buf, 1);
    }
}

void unblock() {
    sigset_t setSignal;
    sigemptyset(&setSignal);
    sigaddset(&setSignal, SIGSEGV);
    sigprocmask(SIG_UNBLOCK, &setSignal, nullptr);
}

void dumping_memory(void* address) {
    const long long range = 16;
    const long long from = std::max(0LL, (long long) (static_cast<char*>(address) - range));
    const long long to = std::min(LONG_LONG_MAX, (long long) (static_cast<char*>(address) + range));
    for (long long i = from; i < to; i++) {
        unblock();
        //change handler
        struct sigaction sAction {};
        sAction.sa_sigaction = sigsegv_address_handler;
        sAction.sa_flags = SA_SIGINFO;
        if (sigaction(SIGSEGV, &sAction, nullptr) == -1) {
            std::cout << "Error in sigaction" << std::endl;
            exit(EXIT_FAILURE);
        }
        if (sigsetjmp(buf, 0) != 0) {
            std::cout << "Can't dump memory. Error in setjmp" << std::endl;
        } else {
            std::cout << "Address: " << ((void*)i) << ". Value: " << std::hex << (int)((const char*)i)[0] << std::endl;
        }
    }
}

const char* regs_name[] = {"R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15", "RDI", "RSI","RBP", "RBX",
                           "RDX", "RAX", "RCX", "RSP", "RIP", "EFL", "CSGSFS","ERR", "TRAPNO", "OLDMASK", "CR2"};

void dumping_registries(ucontext_t *context) {
    for (size_t reg = 0; reg < NGREG; reg++) {
        std::cout << regs_name[reg] << ": " << "0x" << std::hex << context->uc_mcontext.gregs[reg] << std::endl;
    }
}

void handler(int code, siginfo_t *siginfo, void *context_of_program) {
    std::cout << "Handler of signal" << std::endl << "Name of signal is \"" << strsignal(code) << "\"" << std::endl;
    if (siginfo->si_signo != SIGSEGV) {
        exit(EXIT_FAILURE);
    }
    if (siginfo->si_code == SEGV_MAPERR) {
        std::cout << "The address does not match the object" << std::endl;
    } else if (siginfo->si_code == SEGV_ACCERR) {
        std::cout << "Rights to the reflected object are incorrect" << std::endl;
    } else {
        exit(EXIT_FAILURE);
    }
    std::cout << "Address is " << (siginfo->si_addr != nullptr ? siginfo->si_addr : "null pointer") << std::endl;
    std::cout << "General registries dump:" << std::endl;
    dumping_registries((ucontext_t *) context_of_program);
    if (siginfo -> si_addr != nullptr) {
        std::cout << "Dumping of memory" << std::endl;
        dumping_memory(siginfo->si_addr);
    } else {
        std::cout << "Can't dump a memory. Cause null pointer" << std::endl;
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

void print_info() {
    std::cout << "This program handle a sigsegv" << std::endl;
    std::cout << "Print a command:" << std::endl;
    std::cout << "Out of range: out" << std::endl;
    std::cout << "Null pointer: null" << std::endl;
}

int main(int argc, char* argv[], char* envp[]) {
    print_info();
    struct sigaction action {};
    action.sa_sigaction = handler;
    action.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &action, nullptr) == -1) {
        std::cout << "Error in sigaction" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string cmd;
    std::cin >> cmd;
    if (cmd == "out") {
        char* str = "Hello"; // len is 6. (with \0)
        str[7] = 'i';
    } else if (cmd == "null") {
        char* ptr = nullptr;
        (*ptr) = 'd';
    } else {
        std::cout << "Incorrect command" << std::endl;
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}