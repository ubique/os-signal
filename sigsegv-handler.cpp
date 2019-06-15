#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <signal.h> 
#include <unistd.h>

const int LINES = 8;
const int ITEMS = 16;


void print_error(const char* reason) {
    std::cerr << "sigsegv-handler: " << reason;
    if (errno) {
        std::cerr << ": " << std::strerror(errno);
    }
    std::cerr << std::endl;
}


void sprint_str(const char* value) {
    write(STDERR_FILENO, value, strlen(value));
}


void sprint_hex(uint8_t value) {
    char out;
    if (value < 10) {
        out = char(48 + value);
    } else {
        out = char(87 + value);
    }
    write(STDERR_FILENO, &out, 1);
}


void sprint_u64(uint64_t value) {
    for (int i = 16; i-- > 0;) {
        sprint_hex(value >> (i * 2) & 0xf);
    }
}

void sprint_reg(const char* reg_name, int reg, struct ucontext_t* uc) {
    sprint_str(reg_name);
    sprint_str(": ");
    sprint_u64(uc->uc_mcontext.gregs[reg]);
    sprint_str("\n");
}


void dump(int signal, siginfo_t* siginfo, void *context) {
    struct ucontext_t *uc = static_cast<ucontext_t*>(context);

    sprint_str("SIGSEGV handler triggered. That's the debug information:\n\nRegister values\n");

    sprint_reg("RDI",     REG_RDI,     uc);
    sprint_reg("RSI",     REG_RSI,     uc);
    sprint_reg("RBP",     REG_RBP,     uc);
    sprint_reg("RBX",     REG_RBX,     uc);
    sprint_reg("RDX",     REG_RDX,     uc);
    sprint_reg("RAX",     REG_RAX,     uc);
    sprint_reg("RCX",     REG_RCX,     uc);
    sprint_reg("RSP",     REG_RSP,     uc);
    sprint_reg("RIP",     REG_RIP,     uc);
    sprint_reg("R8",      REG_R8,      uc);
    sprint_reg("R9",      REG_R9,      uc);
    sprint_reg("R10",     REG_R10,     uc);
    sprint_reg("R11",     REG_R11,     uc);
    sprint_reg("R12",     REG_R12,     uc);
    sprint_reg("R13",     REG_R13,     uc);
    sprint_reg("R14",     REG_R14,     uc);
    sprint_reg("R15",     REG_R15,     uc);
    sprint_reg("EFL",     REG_EFL,     uc);
    sprint_reg("CSGSFS",  REG_CSGSFS,  uc);
    sprint_reg("ERR",     REG_ERR,     uc);
    sprint_reg("TRAPNO",  REG_TRAPNO,  uc);
    sprint_reg("OLDMASK", REG_OLDMASK, uc);
    sprint_reg("CR2",     REG_CR2,     uc);

    int dummy[2];
    if (pipe(dummy) == -1) {
        sprint_str("fatal: can't create dummy pipes");
        exit(EXIT_FAILURE);
    }

    char *base_addr = static_cast<char*>(siginfo->si_addr);

    sprint_str("\nShort memory dump near ");
    sprint_u64((uint64_t)base_addr);
    sprint_str(":\n");

    for (int line = -LINES; line < LINES; line++) {
        sprint_u64((uint64_t)(base_addr + line * ITEMS));
        for (int cell = 0; cell < ITEMS; cell++) {
            char *addr = base_addr + line * ITEMS + cell;

            if (line == 0 && cell == 0) {
                sprint_str(">");
            } else {
                sprint_str(" ");
            }

            if (write(dummy[1], addr, 1) == -1) {
                sprint_str("..");
                continue;
            }

            uint8_t value;
            if (read(dummy[0], &value, 1) == -1) {
                sprint_str("..");
                continue;
            }

            sprint_hex(value >> 4 & 0xf);
            sprint_hex(value & 0xf);
        }

        sprint_str("\n");
    }

    exit(EXIT_FAILURE);
}

int main() {
    struct sigaction sigact;
    sigact.sa_flags = SA_SIGINFO;
    if (sigemptyset(&sigact.sa_mask) == -1) {
        print_error("Can't create signal empty set");
        return EXIT_FAILURE;
    }
    sigact.sa_sigaction = dump;
    if (sigaction(SIGSEGV, &sigact, nullptr) == -1) {
        print_error("Can't attach signal action");
        return EXIT_FAILURE;
    }

    // Insert your code here.

    return EXIT_SUCCESS;
}
