#include "Handler.h"

static jmp_buf jmp;

std::map <string, int> Handler::regs = {
	{"R8",      REG_R8}, {"R9",      REG_R9}, {"R10",     REG_R10},
	{"R11",     REG_R11}, {"R12",     REG_R12}, {"R13",     REG_R13},
	{"R14",     REG_R14}, {"R15",     REG_R15}, {"RAX",     REG_RAX},
	{"RCX",     REG_RCX}, {"RDX",     REG_RDX}, {"RSI",     REG_RSI},
	{"RDI",     REG_RDI}, {"RIP",     REG_RIP}, {"RSP",     REG_RSP},
	{"CR2",     REG_CR2}, {"RBP",     REG_RBP},{"RBX",     REG_RBX},
	{"EFL",     REG_EFL}, {"ERR",     REG_ERR}, {"CSGSFS",  REG_CSGSFS},
	{"TRAPNO",  REG_TRAPNO}, {"OLDMASK", REG_OLDMASK}
};

void Handler::dump_memory(void *address) {
	if (address == nullptr) {
		write_string("address is null");
		return;
	}
	
	write_string("Memory dump:\n");

	size_t addr = reinterpret_cast<size_t> (address);
	size_t left;
	size_t right;

	find_borders(left, right, addr);

	for (size_t i = left; i < right; i++) {
		sigset_t sigset_;
		sigemptyset(&sigset_);
		sigaddset(&sigset_, SIGSEGV);
		sigprocmask(SIG_UNBLOCK, &sigset_, nullptr);

		struct sigaction act;
		act.sa_flags = SA_SIGINFO;
		act.sa_sigaction = jump_handler;

		if (sigaction(SIGSEGV, &act, nullptr) == -1) {
			print_error("sigaction");
		}

		if (setjmp(jmp) == -1) {
			print_error("Memory dump failed");
		}
		else {
			write_string("0x");
			write_number(static_cast<uint64_t>(i));
			write_string("\n");
		}
	}
	write_string("\n");
}

void Handler::write_byte(uint8_t num) {
	write_char(num / 16);
	write_char(num % 16);
}

void Handler::write_char(char c) {
	char ch;
	
	if (c < 10) {
		ch = c + '0';
	}
	else {
		ch = c - 10 + 'a';
	}

	write(1, &ch, 1);
}

void Handler::write_number(uint64_t n) {
	if (n == 0) {
		write_string("0");
		return;
	}
	for (int i = 7; i >= 0; i--) {
		uint8_t byte = (n >> (i * 8)) & 0xFF;
		if (byte != 0) {
			write_byte(byte);
		}
	}
}

void Handler::write_string(const char *str) {
	write(1, str, strlen(str));
}

void Handler::jump_handler(int signo, siginfo_t *siginfo, void *ucontext) {
	siglongjmp(jmp, 1);
}

void Handler::find_borders(size_t &left, size_t &right, size_t addr) {
	size_t l = addr - RADIUS;
	if (l < 0) {
		l = 0;
	}

	size_t max = numeric_limits<size_t>::max();
	size_t r = addr + RADIUS;
	if (r > max) {
		r = max;
	}

	left = l;
	right = r;
}

void Handler::dump_registers(ucontext_t *ucontext) {
	write_string("Registers dump:\n");
	for (map <string, int>::iterator it = regs.begin(); it != regs.end(); it++) {
		write_string(it->first.c_str());
		write_string(": 0x");
		write_number(ucontext->uc_mcontext.gregs[it->second]);
		write_string("\n");
	}
}

void Handler::handler(int signo, siginfo_t *siginfo, void *ucontext) {
	if (siginfo->si_signo == SIGSEGV) {
		write_string("SIGSEGV occurred at address: ");
		if (siginfo->si_addr == nullptr) {
			write_string("null");
		}
		else {
			uint64_t address = reinterpret_cast<uint64_t>(siginfo->si_addr);
			write_string("0x");
			write_number(address);
		}
		write_string("\n\n");
		dump_memory(static_cast<char *>(siginfo->si_addr));
		dump_registers(reinterpret_cast<ucontext_t *> (ucontext));
	}

	_exit(EXIT_FAILURE);
}

void Handler::print_error(const char *error) {
	write_string(error);
	write_string("\n");
	_exit(EXIT_FAILURE);
}
