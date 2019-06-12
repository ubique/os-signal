#ifndef OS_SIGNAL_HANDLER_H
#define OS_SIGNAL_HANDLER_H

#include <signal.h>
#include <csetjmp>

void hand(int sig, siginfo_t *siginfo, void *ucontext);
void help_handler(int sig, siginfo_t* siginfo, void* context);

class handler {
public:
    handler(void (*foo) (int, siginfo_t*, void*));
};


#endif //OS_SIGNAL_HANDLER_H
