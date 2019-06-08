//
// Created by Георгий Розовский on 03/06/2019.
//

#include <csignal>
#include <unistd.h>
#include <cstring>
#include <csetjmp>
#include <iostream>

sigjmp_buf jmpbuf;

char const *regs[] = {"R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15", "RDI", "RSI", "RBP", "RBX", "RDX", "RAX",
                      "RCX", "RSP", "RIP", "RFL", "CSGSFS", "ERR", "TRAPNO", "OLDMASK", "CR2"};

void my_error(char const *message) {
    fprintf(stderr, "%s", message);
    exit(EXIT_FAILURE);
}

char get_digit(uint64_t a) {
    if (a < 10) {
        return static_cast<char>('0' + a);
    }
    return static_cast<char>('a' + (a % 10));
}

void write_str(char const *s, int path = STDERR_FILENO) {
    ssize_t cur = 0;
    ssize_t len = static_cast<ssize_t>(strlen(s));
    ssize_t tmp;
    while (cur < len) {
        tmp = write(path, s + cur, static_cast<size_t>(len - cur));
        if (tmp == -1) {
            my_error("Error was occurred while writing");
        }
        cur += tmp;
    }
}

char *to_string(uint64_t a) {
    char *ans = new char[19];
    ans[0] = '0';
    ans[1] = 'x';
    for (size_t i = 0; i < 16; ++i) {
        ans[2 + i] = get_digit(a % 16);
        a = a / 16;
    }
    ans[18] = '\0';
    return ans;
}

char *byte_to_string(uint8_t a) {
    char *ans = new char[3];
    ans[0] = get_digit(static_cast<uint64_t >(a % 16));
    ans[1] = get_digit(static_cast<uint64_t >(a / 16));
    ans[2] = '\0';
    return ans;
}


void write_reg(mcontext_t *mcontext, size_t reg) {
    write_str(regs[static_cast<size_t >(reg)]);
    write_str(" = ");
    write_str(to_string(static_cast<uint64_t>(mcontext->gregs[reg])));
    write_str("\n");
}