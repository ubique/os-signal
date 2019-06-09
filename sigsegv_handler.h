#include <setjmp.h>
#include <signal.h>

struct sigsegv_handler {
        static int set_sigsegv_handler();

private:
        static jmp_buf jmp;
        static void deref_handler(int);
        static void handler(int, siginfo_t *, void *);

        sigsegv_handler();
};
