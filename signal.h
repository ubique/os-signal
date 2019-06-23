#pragma once

static const int MEMORY_DUMP_RANGE = 16;
static const char* regStr[23] = {"R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15",
"RDI", "RSI", "RBP", "RBX", "RDX", "RAX", "RCX", "RSP", "RIP", "EFL", "CSGSFS",
"ERR", "TRAPNO", "OLDMASK", "CR2"};
static jmp_buf jmpBuf;

void sigsegv_address_handler(int, siginfo_t*, void*);
void dumping_registries(ucontext_t*);
void dumping_memory(void*);
void handler(int, siginfo_t*, void*);
void print(const char*);
void print(long long);
void printHex(long long);