#include "SignalHandler.h"

const std::map<std::string, int> SignalHandler::REGISTERS = {
        {"R8",      REG_R8},
        {"R9",      REG_R9},
        {"R10",     REG_R10},
        {"R11",     REG_R11},
        {"R12",     REG_R12},
        {"R13",     REG_R13},
        {"R14",     REG_R14},
        {"R15",     REG_R15},
        {"RAX",     REG_RAX},
        {"RCX",     REG_RCX},
        {"RDX",     REG_RDX},
        {"RSI",     REG_RSI},
        {"RDI",     REG_RDI},
        {"RIP",     REG_RIP},
        {"RSP",     REG_RSP},
        {"CR2",     REG_CR2},
        {"RBP",     REG_RBP},
        {"RBX",     REG_RBX},
        {"EFL",     REG_EFL},
        {"ERR",     REG_ERR},
        {"CSGSFS",  REG_CSGSFS},
        {"TRAPNO",  REG_TRAPNO},
        {"OLDMASK", REG_OLDMASK},
};

static jmp_buf jmpBuf;

void SignalHandler::printErr(const std::string& message) {
    fprintf(stderr, "ERROR %s: %s\n", message.c_str(), strerror(errno));

}

void SignalHandler::helper(int sig, siginfo_t* info, void* context) {
    if (info->si_signo == SIGSEGV) {
        siglongjmp(jmpBuf, 1);
    }
}

void SignalHandler::dumpAddress(long long address) {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGSEGV);
    sigprocmask(SIG_UNBLOCK, &sigset, nullptr);

    struct sigaction action{};
    action.sa_sigaction = helper;
    action.sa_flags = SA_SIGINFO;

    if (sigaction(SIGSEGV, &action, nullptr) < 0) {
        printErr("Signal action failed");
        exit(EXIT_FAILURE);
    }

    if (setjmp(jmpBuf) != 0) {
        std::cout << " Unable to dump\n";
    } else {
        std::cout << " " << std::hex << +*reinterpret_cast<char*>(address) << "\n";
    }
}

void SignalHandler::dumpMemory(void* address) {
    std::cout << "\nMemory nearby: " << "\n";
    if (address == nullptr) {
        std::cout << " NULL\n";
        exit(EXIT_FAILURE);
    }

    const size_t size = sizeof(char);
    const long long from = std::max(0LL, (long long) ((char*) address - MEMORY_RANGE * size));
    const long long to = std::min(LONG_LONG_MAX, (long long) ((char*) address + MEMORY_RANGE * size));

    for (long long i = from; i < to; i += size) {
        dumpAddress(i);
    }
}

void SignalHandler::dumpRegisters(ucontext_t* ucontext) {
    std::cout << "\nGeneral purposes registers:\n";
    greg_t* gregs = ucontext->uc_mcontext.gregs;

    for (const auto& reg : REGISTERS) {
        std::cout << " " << reg.first << ": " << gregs[reg.second] << "\n";
    }
}

void SignalHandler::handler(int signo, siginfo_t* siginfo, void* context) {
    if (siginfo->si_signo == SIGSEGV) {
        std::cout << "Signal aborted: " << strsignal(signo) << "\n";
        switch (siginfo->si_code) {
            case SEGV_MAPERR: {
                std::cout << "\nCause: Address is not mapped to object" << "\n";
                break;
            }
            case SEGV_ACCERR: {
                std::cout << "\nCause: Invalid permission" << "\n";
                break;
            }
            default:
                std::cout << "\nCause: Something bad happen" << "\n";
        }

        auto address = reinterpret_cast<intptr_t>(siginfo->si_addr);
        std::cout << "Address: " << std::hex << address << "\n";

        dumpRegisters(reinterpret_cast<ucontext_t*> (context));
        dumpMemory(siginfo->si_addr);
    }
    exit(EXIT_FAILURE);
}