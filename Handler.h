//
// Created by Noname Untitled on 05.06.19.
//

#pragma once

#include <iostream>
#include <map>
#include <limits>

#include <signal.h>

#include <sys/param.h>
#include <sys/ucontext.h>

#include "Utils.h"

#define MEMORY_NEARBY 20

class Handler {
public:
    static void handler(int, siginfo_t*, void*);

private:
    static void dumpMemory(void*);

    static void dumpRegisters(ucontext_t*);

    static std::map<const char*, int> REGISTERS;
};



