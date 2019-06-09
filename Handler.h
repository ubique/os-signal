//
// Created by Noname Untitled on 05.06.19.
//

#pragma once

#include <iostream>
#include <cstring>
#include <string>
#include <map>
#include <limits>
#include <csetjmp>

#include <signal.h>

#include <sys/ucontext.h>

#define MEMORY_NEARBY 20

class Handler {
public:
    static void handler(int, siginfo_t*, void*);

private:
    static void helper(int, siginfo_t*, void*);

    static void dumpMemory(void*);

    static void dumpRegisters(ucontext_t*);

    static std::map<std::string, int> REGISTERS;

};



