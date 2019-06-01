#ifndef OS_SIGNAL_SIGNALHANDLER_H
#define OS_SIGNAL_SIGNALHANDLER_H

#include <iostream>
#include <map>
#include <sys/ucontext.h>
#include <cstring>
#include <signal.h>
#include <climits>
#include <csetjmp>


class SignalHandler {
public:
    static void handler(int signo, siginfo_t* siginfo, void* context);

private:
    static void printErr(const std::string& message);

    static void helper(int sig, siginfo_t* info, void* context);

    static void dumpAddress(long long address);

    static void dumpMemory(void* address);

    static void dumpRegisters(ucontext_t* ucontext);

    static const std::map<std::string, int> REGISTERS;

    static const int MEMORY_RANGE = 20;
};


#endif //OS_SIGNAL_SIGNALHANDLER_H
