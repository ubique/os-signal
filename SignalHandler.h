#ifndef OS_SIGNAL_SIGNALHANDLER_H
#define OS_SIGNAL_SIGNALHANDLER_H

#include <vector>
#include <signal.h>


struct SignalHandler {
public:
    static void handler(int signo, siginfo_t* siginfo, void* context);

private:
    SignalHandler() = default;

    static void printErr(const char* message);

    static void dumpAddress(int pip[], char* address,int i);

    static void dumpMemory(char* address);

    static void dumpRegisters(ucontext_t* ucontext);

    static const std::vector<std::pair<const char*, int>> REGISTERS;

    static const int MEMORY_RANGE = 16;
};


#endif //OS_SIGNAL_SIGNALHANDLER_H
