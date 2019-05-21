//
// Created by ifkbhit on 21.05.19.
//

#pragma once

#include <csignal>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <ucontext.h>
#include <cmath>
#include <algorithm>
#include <climits>
#include <sys/ucontext.h>
#include <csetjmp>
#include <iostream>
#include "../table/TableDouble.h"

class SIGSEGVHandler {

private:

    using byte = char;
#define __ifInvalidAccess if (siginfo->si_signo == SIGSEGV)

    int signum;
    siginfo_t* siginfo;
    void* context;

    static jmp_buf buf;

    SIGSEGVHandler(int signum, siginfo_t* siginfo, void* context);

    static void jump(int signum, siginfo_t* siginfo, void* context);
    static bool attachFunction(void (* pFunction)(int, siginfo_t*, void*));
    static void catchSignal(int signum, siginfo_t* siginfo, void* context);


    void showCode();
    void dumpGeneralRegisters();
    void dumpMemory(size_t range);


public:

    static size_t memoryDumpRange;

    static bool attach(const size_t dumpRange = 15);

};

