#pragma once

#include <iostream>
#include <sys/ucontext.h>
#include <map>
#include <limits>
#include <csetjmp>
#include <signal.h>
#include <string>

using namespace std;

class Handler {
	public:
		static void handler(int, siginfo_t*, void*);
		static void dump_registers(ucontext_t *);
		static void dump_memory(void *);
		static void print_error(const char *);
		static void jump_handler(int, siginfo_t *, void *);

		static void find_borders(size_t &, size_t &, size_t);

		static const int RADIUS = 15;
	private:
		static map <string, int> regs;
};
