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
#include "../Output.h"

class SIGSEGVHandler {

private:
    Output output;
    using byte = uint8_t;
#define __ifInvalidAccess if (siginfo->si_signo == SIGSEGV)

    int signum;
    siginfo_t* siginfo;
    void* context;

    SIGSEGVHandler(int, siginfo_t*, void*);

    static bool attachFunction(void (*)(int, siginfo_t*, void*));

    static void catchSignal(int, siginfo_t*, void*);


    void showCode();

    void dumpGeneralRegisters();

    void dumpMemory(int);

    void dumpReg(const char*, unsigned long int, bool = false);


public:

    static size_t memoryDumpRange;

    static bool attach(size_t = 15);

};

