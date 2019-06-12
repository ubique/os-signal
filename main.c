#define _GNU_SOURCE

#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <unitypes.h>

void write_num_hex(const uint8_t num) {
    uint8_t symb = num + (num < 10 ? '0' : 'a' - 10);
    write(1, &symb, 1);
}

void write_byte_hex(const uint8_t byte) {
    write_num_hex(byte >> (unsigned) 4);
    write_num_hex(byte & (unsigned) 0xF);
}

void write_ull_hex(const uint64_t ull) {
    write(1, "0x", strlen("0x"));
    for (int i = 0; i < 8; ++i) write_byte_hex((ull >> ((unsigned) (7 - i) << (unsigned) 3)) & (unsigned) 0xFF);
}

void write_string(const char* string) {
    write(1, string, strlen(string));
}

void write_stderr(const char* message) {
    write(2, message, strlen(message));
}

void address_dump(void* addr, int p[2]) {
    write_ull_hex((uint64_t) (addr));
    write_string(" ");
    if (write(p[1], addr, 1) == -1) {
        write_string("bad");
    } else {
        uint8_t val;
        if (read(p[0], &val, 1) == -1) {
            write_string("bad");
        } else {
            write_byte_hex(val);
        }
    }
    write_string("\n");
}

void memory_dump(void* addr) {
    write_string("\tMEMORY DUMP\n");
    int p[2];
    if (pipe(p) == -1) {
        write_stderr("Failed to create pipe.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = -15; i <= 16; ++i) address_dump(addr + i, p);
}

void write_register(const char* name, const uint64_t value) {
    write_string(name);
    write_string(" ");
    write_ull_hex(value);
    write_string("\n");
}

void registers_dump(ucontext_t* context) {
    write_string("\tREGISTERS DUMP\n");
    write_register("REG_R8     ", context->uc_mcontext.gregs[REG_R8]);
    write_register("REG_R9     ", context->uc_mcontext.gregs[REG_R9]);
    write_register("REG_R10    ", context->uc_mcontext.gregs[REG_R10]);
    write_register("REG_R11    ", context->uc_mcontext.gregs[REG_R11]);
    write_register("REG_R12    ", context->uc_mcontext.gregs[REG_R12]);
    write_register("REG_R13    ", context->uc_mcontext.gregs[REG_R13]);
    write_register("REG_R14    ", context->uc_mcontext.gregs[REG_R14]);
    write_register("REG_R15    ", context->uc_mcontext.gregs[REG_R15]);
    write_register("REG_RDI    ", context->uc_mcontext.gregs[REG_RDI]);
    write_register("REG_RSI    ", context->uc_mcontext.gregs[REG_RSI]);
    write_register("REG_RBP    ", context->uc_mcontext.gregs[REG_RBP]);
    write_register("REG_RBX    ", context->uc_mcontext.gregs[REG_RBX]);
    write_register("REG_RDX    ", context->uc_mcontext.gregs[REG_RDX]);
    write_register("REG_RAX    ", context->uc_mcontext.gregs[REG_RAX]);
    write_register("REG_RCX    ", context->uc_mcontext.gregs[REG_RCX]);
    write_register("REG_RSP    ", context->uc_mcontext.gregs[REG_RSP]);
    write_register("REG_RIP    ", context->uc_mcontext.gregs[REG_RIP]);
    write_register("REG_EFL    ", context->uc_mcontext.gregs[REG_EFL]);
    write_register("REG_CSGSFS ", context->uc_mcontext.gregs[REG_CSGSFS]);
    write_register("REG_ERR    ", context->uc_mcontext.gregs[REG_ERR]);
    write_register("REG_TRAPNO ", context->uc_mcontext.gregs[REG_TRAPNO]);
    write_register("REG_OLDMASK", context->uc_mcontext.gregs[REG_OLDMASK]);
    write_register("REG_CR2    ", context->uc_mcontext.gregs[REG_CR2]);
    write_string("\n");
}

const char* signal_code_to_reason(const int si_code) {
    switch (si_code) {
        case SEGV_MAPERR:
            return "Address not mapped to object.";
        case SEGV_ACCERR:
            return "Invalid permissions for mapped object.";
        default:
            return "Unknown error.";
    }
}

void main_info(siginfo_t* siginfo) {
    write_string("\tMAIN INFO\n");
    write_string("ERROR: Segmentation fault.\n");
    write_string("Tried to access ");
    write_ull_hex((uint64_t) (siginfo->si_addr));
    write_string("\n");
    write_string(signal_code_to_reason(siginfo->si_code));
    write_string("\n");
    write_string("\n");
}

void sigsegv_handler(int signum, siginfo_t* siginfo, void* context) {
    if (siginfo->si_signo == SIGSEGV) {
        main_info(siginfo);
        registers_dump((ucontext_t*) context);
        memory_dump(siginfo->si_addr);
    }
    exit(EXIT_FAILURE);
}

void set_sigaction() {
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = sigsegv_handler;
    act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &act, NULL) < 0) {
        write_stderr("Sigaction failed.");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char** argv) {
    set_sigaction();

    // test
    char* c = (char*) "TEST";
    c[5] = 'G';

    return EXIT_SUCCESS;
}
