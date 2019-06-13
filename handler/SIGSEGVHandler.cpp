//
// Created by ifkbhit on 21.05.19.
//

#include "SIGSEGVHandler.h"
#include "../Output.h"

size_t SIGSEGVHandler::memoryDumpRange = 15;

void SIGSEGVHandler::dumpGeneralRegisters() {
    output.putString("General registers").newLine();
    dumpReg("R8", (((ucontext_t*) context)->uc_mcontext.gregs[REG_R8]));
    dumpReg("R9", (((ucontext_t*) context)->uc_mcontext.gregs[REG_R9]));
    dumpReg("R10", (((ucontext_t*) context)->uc_mcontext.gregs[REG_R10]));
    dumpReg("R11", (((ucontext_t*) context)->uc_mcontext.gregs[REG_R11]));
    dumpReg("R12", (((ucontext_t*) context)->uc_mcontext.gregs[REG_R12]));
    dumpReg("R13", (((ucontext_t*) context)->uc_mcontext.gregs[REG_R13]));
    dumpReg("R14", (((ucontext_t*) context)->uc_mcontext.gregs[REG_R14]));
    dumpReg("R15", (((ucontext_t*) context)->uc_mcontext.gregs[REG_R15]));
    dumpReg("RDI", (((ucontext_t*) context)->uc_mcontext.gregs[REG_RDI]));
    dumpReg("RSI", (((ucontext_t*) context)->uc_mcontext.gregs[REG_RSI]));
    dumpReg("RBP", (((ucontext_t*) context)->uc_mcontext.gregs[REG_RBP]));
    dumpReg("RBX", (((ucontext_t*) context)->uc_mcontext.gregs[REG_RBX]));
    dumpReg("RDX", (((ucontext_t*) context)->uc_mcontext.gregs[REG_RDX]));
    dumpReg("RAX", (((ucontext_t*) context)->uc_mcontext.gregs[REG_RAX]));
    dumpReg("RCX", (((ucontext_t*) context)->uc_mcontext.gregs[REG_RCX]));
    dumpReg("RSP", (((ucontext_t*) context)->uc_mcontext.gregs[REG_RSP]));
    dumpReg("RIP", (((ucontext_t*) context)->uc_mcontext.gregs[REG_RIP]));
    dumpReg("EFL", (((ucontext_t*) context)->uc_mcontext.gregs[REG_EFL]));
    dumpReg("CSGSFS", (((ucontext_t*) context)->uc_mcontext.gregs[REG_CSGSFS]));
    dumpReg("ERR", (((ucontext_t*) context)->uc_mcontext.gregs[REG_ERR]));
    dumpReg("TRAPNO", (((ucontext_t*) context)->uc_mcontext.gregs[REG_TRAPNO]));
    dumpReg("OLDMASK", (((ucontext_t*) context)->uc_mcontext.gregs[REG_OLDMASK]));
    dumpReg("CR2", (((ucontext_t*) context)->uc_mcontext.gregs[REG_CR2]));
    output.newLine(2);
}

void SIGSEGVHandler::showCode() {
    output.putString("Caused by: ");
    switch (siginfo->si_code) {
        case SEGV_MAPERR:
            output.putString("Address doesn't map to object").newLine(3);
            return;
        case SEGV_ACCERR:
            output.putString("Invalid permissions for mapped object").newLine(3);
            return;
    }
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
    output = Output();
    output.putString("Handle SIGSEGV signal")
            .newLine()
            .putString("Access to ")
            .putHex((unsigned long int) static_cast<void*>(siginfo->si_addr))
            .newLine(2);

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


void SIGSEGVHandler::dumpMemory(int range) {
    output.putString("Memory dump").newLine();
    int pipeArray[2];
    auto status = pipe(pipeArray);
    if (status == -1) {
        output.putString("Cannot create pipe for dump memory").newLine();
        return;
    }
    size_t inBlock = 5;
    size_t count = 0;
    for (int current = -range; current <= range; current++) {
        char* addr = static_cast<char*>(siginfo->si_addr) + current;
        auto validAddr = write(pipeArray[1], addr, 1);
        if (validAddr != -1) {
            output.putHex((unsigned long int) addr).ws();
            byte value;
            auto dumpStatus = read(pipeArray[0], &value, 1);
            if (dumpStatus == -1) {
                output.putString("NDEF");
            } else {
                output.putByte(value);
                if(current == 0){
                    output.putString("[!!]");
                } else {
                    output.ws(4);
                }
                output.ws(2);
            }

        } else {
            output.putString("  ER  ");
        }
        count++;
        output.ws();
        if(count % inBlock == 0){
            count = 0;
            output.newLine();
        }

    }

    output.newLine(2);
}

bool SIGSEGVHandler::attach(const size_t dumpRange) {
    memoryDumpRange = dumpRange;
    return attachFunction(SIGSEGVHandler::catchSignal);
}

void SIGSEGVHandler::dumpReg(const char* name, unsigned long int value, bool newBlock) {
    size_t blockSize = 10;
    output.putString(name).ws(blockSize - strlen(name)).putHex(value).newLine(newBlock ? 2 : 1);
}
