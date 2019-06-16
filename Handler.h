#pragma once

#include <sys/ucontext.h>
#include <map>
#include <limits>
#include <csetjmp>
#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

using namespace std;

class Handler {
	public:
		static void handler(int, siginfo_t *, void *);
		static void dump_registers(ucontext_t *);
		static void dump_memory(void *);
		static void print_error(const char *);
		static void jump_handler(int, siginfo_t *, void *);
		static void write_string(const char *);
		static void write_byte(uint8_t);
		static void write_number(uint64_t);
		static void write_char(char);

		static void find_borders(size_t &, size_t &, uintptr_t);

		static const int RADIUS = 15;
	private:
		static map <string, int> regs;
};
