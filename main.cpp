#include <sys/mman.h>

#include <iostream>
#include <cstring>
#include <errno.h>

#include "segvhandler.h"

void test_nullptr() {
    *((char*)nullptr) = 0;
}

void test_boundary() {
    size_t len = 4096;

    void* mem = mmap(0, len, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);

    if (mem == MAP_FAILED) {
        fprintf(stderr, "Could not allocate memory: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    memset(mem, 0xa5, len);

    *((char*)mem - 1) = 1;
}

void usage(char* name) {
    printf("Usage: %s boundary|nullptr\n", name);
    exit(EXIT_FAILURE);
}

int main(int argc, char**argv) {
    handler_setup();

    if (argc < 2) {
        usage(argv[0]);
    }

    if (strcmp("boundary",argv[1]) == 0) {
        test_boundary();
    }

    if (strcmp("nullptr",argv[1]) == 0) {
        test_nullptr();
    }

    usage(argv[0]);
}
