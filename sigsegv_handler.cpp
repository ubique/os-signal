//
// Created by vitalya on 09.06.19.
//

#include "sigsegv_handler.h"

//#include <signal.h>
#include <csetjmp>
#include <iostream>
#include <map>
#include <memory.h>
#include <sys/ucontext.h>
#include <string>
#include <signal.h>
#include <limits>
#include <zconf.h>

namespace {

    static jmp_buf jmpbuf;

    constexpr const size_t offset = 16; /* bytes */
    constexpr size_t MAX_MEM = std::numeric_limits<size_t>::max();

    const std::map<const char*, uint64_t> registers = {
            {"CR2",     REG_CR2},
            {"ERR",     REG_ERR},
            {"CSGSFS",  REG_CSGSFS},
            {"EFL",     REG_EFL},
            {"OLDMASK", REG_OLDMASK},
            {"RAX",     REG_RAX},
            {"RBX",     REG_RBX},
            {"RCX",     REG_RCX},
            {"RDX",     REG_RDX},
            {"RDI",     REG_RDI},
            {"RBP",     REG_RBP},
            {"RIP",     REG_RIP},
            {"RSP",     REG_RSP},
            {"RSI",     REG_RSI},
            {"R8",      REG_R8},
            {"R9",      REG_R9},
            {"R10",     REG_R10},
            {"R11",     REG_R11},
            {"R12",     REG_R12},
            {"R13",     REG_R13},
            {"R14",     REG_R14},
            {"R15",     REG_R15},
            {"TRAPNO",  REG_TRAPNO},
    };

    void write_string(const char *s) {
        write(STDOUT_FILENO, s, strlen(s));
    }

    char to_hex(uint8_t x) {
        return x < 10 ? ('0' + x) : ('A' + x - 10);
    }

    void write_byte(uint8_t byte) {
        char c[2];
        c[0] = to_hex(byte / 0x10);
        c[1] = to_hex(byte & 0xf);
        write(STDOUT_FILENO, &c, 2);
    }

    void write_number(uint64_t number) {
        for (int i = sizeof(number) - 1; i >= 0; --i) {
            write_byte((number >> (i * 8)) & 0xff);
        }
    }

    template<typename T>
    typename std::enable_if<std::is_integral<T>::value, T>::type
    write(T arg) {
        write_number(arg);
        return arg;
    }

    template<typename T>
    typename std::enable_if<std::is_pointer<T>::value, T>::type
    write(T arg) {
        write_string(arg);
        return arg;
    }

    template<typename T>
    void print(T arg) {
        write(arg);
    }

    template<typename T, typename... Args>
    void print(T arg, Args... args) {
        write(arg);
        print(args...);
    }
}

static void handle1(int sig, siginfo_t *siginfo, void* context) {
    longjmp(jmpbuf, 1);
}

void handle(int sig, siginfo_t* siginfo, void* context) {
    if (siginfo->si_signo != SIGSEGV) {
        return;
    }
    print(strsignal(sig), "!\n");
    print("general registers:\n");
    auto ucontext = reinterpret_cast<ucontext_t *>(context);
    for (const auto& reg : registers) {
        print(reg.first, ": ", ucontext->uc_mcontext.gregs[reg.second], "\n");
    }

    size_t address = reinterpret_cast<size_t>(siginfo->si_addr);
    size_t left_bound = address > offset ? address - offset : 0;
    size_t right_bound = address < MAX_MEM - offset ? address + offset : MAX_MEM;

    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGSEGV);

    print("memory dump from ", left_bound, " to ", right_bound, ": \n");
    for (size_t i = left_bound; i < right_bound; ++i) {
        sigprocmask(SIG_UNBLOCK, &sigset, nullptr);
        sigsegv_handler handler(handle1);

        char *ptr = reinterpret_cast<char*>(i);
        if (setjmp(jmpbuf)) {
            print("?? ");
        } else if (i == address) {
            print("[HERE] ");
        } else {
            print(int(*ptr));
        }
    }
    print("\n");
    exit(EXIT_FAILURE);
}

sigsegv_handler::sigsegv_handler(void (*handle_func) (int, siginfo_t*, void*)) {
    struct sigaction sig_act{};
    sig_act.sa_sigaction = handle_func;
    sig_act.sa_flags = (SA_SIGINFO | SA_NOMASK);

    sigset_t sig;
    sigemptyset(&sig);
    sigaddset(&sig, SIGSEGV);
    sig_act.sa_mask = sig;

    if (sigaction(SIGSEGV, &sig_act, nullptr) == -1) {
        perror("Unable to handle signal");
    }
}
