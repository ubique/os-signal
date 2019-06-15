//
// Created by Павел Пономарев on 2019-06-06.
//

#ifndef OS_SIGNAL_HANDLER_H
#define OS_SIGNAL_HANDLER_H

#include <sys/param.h>
#include <sys/ucontext.h>
#include <signal.h>
#include <csetjmp>
#include <unistd.h>
#include <cstring>
#include <cstdint>


class Handler {
public:
    static void handleSignal(int, siginfo_t*, void*);
private:
    static void dumpRegisters(ucontext_t*);
    static void dumpMemory(void*);

    static const int MEMORY_SIZE = 20;
};


#endif //OS_SIGNAL_HANDLER_H
