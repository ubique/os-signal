//
// Created by daniil on 09.06.19.
//

#ifndef OS_SIGNAL_SIGSEGVHANDLER_H
#define OS_SIGNAL_SIGSEGVHANDLER_H


#include <csetjmp>
#include <bits/types/siginfo_t.h>

class SigsegvHandler {
    static jmp_buf jmp;

    static void myHandle(int);

    static void handle(int, siginfo_t *, void *);

public:
    static int setSigsegv();
};


#endif //OS_SIGNAL_SIGSEGVHANDLER_H
