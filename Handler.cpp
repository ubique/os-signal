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
	cout << "Memory dump:" << endl;

	if (address == nullptr) {
		print_error("address in nullptr");
	}

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
			cerr << "Memory dump failed" << endl;
		}
		else {
			cout << "0x" << hex << (int) *reinterpret_cast<char *>(i) << endl;
		}
	}
	cout << endl;
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
	cout << "Registers dump:" << endl;
	for (map <string, int>::iterator it = regs.begin(); it != regs.end(); it++) {
		cout << (*it).first << ": " << "0x" << hex << ucontext->uc_mcontext.gregs[(*it).second] << endl;
	}
}

void Handler::handler(int signo, siginfo_t *siginfo, void *ucontext) {
	if (siginfo->si_signo == SIGSEGV) {
		cout << "SIGSEGV occurred at address: " << ((siginfo->si_addr == nullptr)? "null" : siginfo->si_addr) << endl << endl;
		dump_memory(siginfo->si_addr);
		dump_registers(reinterpret_cast<ucontext_t *> (ucontext));
	}

	exit(EXIT_FAILURE);
}

void Handler::print_error(const char *error) {
	cerr << error << endl;
	exit(EXIT_FAILURE);
}
