#include <signal.h>

struct sigsegv_handler {
        static int set_sigsegv_handler();

private:
        static void handler(int, siginfo_t *, void *);
        static void write_num(long long);
        static void write_str(const char *);

        sigsegv_handler();
};
