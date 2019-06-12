//
// Created by max on 23.05.19.
//

#ifndef OS_HANDLER_H
#define OS_HANDLER_H


#include <signal.h>
#include <vector>
#include <string>

void hnd(int, siginfo_t *, void *);

class Handler {
    using foo = void (*)(int, siginfo_t *, void *);
public:

    explicit Handler(foo function = &hnd);

private:

    struct sigaction sigact{};
};

#endif //OS_HANDLER_H
