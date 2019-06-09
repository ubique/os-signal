//
// Created by vitalya on 09.06.19.
//

#ifndef OS_SIGNAL_SIGSEGV_HANDLER_H
#define OS_SIGNAL_SIGSEGV_HANDLER_H

#include <bits/types/siginfo_t.h>

void handle(int sig, siginfo_t* siginfo, void* context);

class sigsegv_handler {
public:
    explicit sigsegv_handler(void (*handle_func) (int, siginfo_t*, void*));
};

#endif //OS_SIGNAL_SIGSEGV_HANDLER_H
