//
// Created by ifkbhit on 21.05.19.
//

#include "SIGSEGVHandler.h"


jmp_buf SIGSEGVHandler::buf;
size_t SIGSEGVHandler::memoryDumpRange = 15;

void SIGSEGVHandler::dumpGeneralRegisters() {
    TableDouble regDump("General registers", "Register", "Value");
    std::vector<std::string> regSuffixes = {
            "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15",
            "RDI", "RSI",
            "RBP",
            "RBX", "RDX", "RAX", "RCX",
            "RSP", "RIP",
            "EFL",
            "CSGSFS", "ERR", "TRAPNO", "OLDMASK",
            "CR2"
    };
    for (greg_t currentRegister = REG_R8; currentRegister != NGREG; currentRegister++) {

        regDump.addValue(regSuffixes[currentRegister],
                         (((ucontext_t*) context)->uc_mcontext.gregs[currentRegister]));
    }
    regDump.print(std::cerr);
}

void SIGSEGVHandler::showCode() {
    std::cerr << "Caused by: ";
    switch (siginfo->si_code) {
        case SEGV_MAPERR:
            std::cerr << "Address doesn't map to object\n";
            return;
        case SEGV_ACCERR:
            std::cerr << "Invalid permissions for mapped object\n";
            return;
    }
    std::cerr << "???\nUnsupported code: " << siginfo->si_code << std::endl;

}

void SIGSEGVHandler::catchSignal(int signum, siginfo_t* siginfo, void* context) {
    __ifInvalidAccess {
        SIGSEGVHandler handler(signum, siginfo, context);
        handler.showCode();
        handler.dumpGeneralRegisters();
        handler.dumpMemory(SIGSEGVHandler::memoryDumpRange);

    }
    exit(1);
}

SIGSEGVHandler::SIGSEGVHandler(int signum, siginfo_t* siginfo, void* context) : signum(signum), siginfo(siginfo),
                                                                                context(context) {

    std::cerr << "Handle SIGSEGV signal\n"
                 "Access to " << static_cast<void*>(siginfo->si_addr) << std::endl;
}

bool SIGSEGVHandler::attachFunction(void (* pFunction)(int, siginfo_t*, void*)) {
    struct sigaction act{};

    act.sa_sigaction = pFunction;
    act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &act, nullptr) < 0) {
        perror("attaching (sigaction)");
        return false;
    }
    return true;
}

void SIGSEGVHandler::jump(int signum, siginfo_t* siginfo, void* context) {
    __ifInvalidAccess {
        siglongjmp(buf, 1);
    }
}

void SIGSEGVHandler::dumpMemory(size_t range) {
    auto address = (long long) ((byte*) siginfo->si_addr);
    auto step = sizeof(byte);
    TableDouble tableDouble("Memory (range " + std::to_string(range) + ")", "Address", "Value");
    long long to = LONG_LONG_MAX - range * step > address ? (address + range * step) : LONG_LONG_MAX;

    for (auto addr = std::max(0ll, (long long) (address - range * step));
         addr <= to; addr += step) {
        sigset_t signalSet;
        sigemptyset(&signalSet);
        sigaddset(&signalSet, SIGSEGV);
        sigprocmask(SIG_UNBLOCK, &signalSet, nullptr);

        if (!attachFunction(SIGSEGVHandler::jump)) {
            return;
        }

        auto markAddr = addr == address ? "SIGSEGV " : "";

        std::stringstream value;

        if (setjmp(buf) != 0) {
            value << "cannot be dumped";
        } else {
            value << std::hex << static_cast<unsigned int>(reinterpret_cast<const byte*>(addr)[0]);
        }
        tableDouble.addValue(reinterpret_cast<void*>(addr), value.str(), markAddr);

    }
    tableDouble.print(std::cerr);
}

bool SIGSEGVHandler::attach(const size_t dumpRange) {
    memoryDumpRange = dumpRange;
    return attachFunction(SIGSEGVHandler::catchSignal);
}
