#include <cstdio>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <utility>

#include <unistd.h>

void write_safe(char c)
{
	write(STDOUT_FILENO, &c, 1);
}

void write_str_safe(const char* str)
{
	write(STDOUT_FILENO, str, std::strlen(str));
}

void write_byte_safe(uint8_t byte)
{
	const auto lower = 0xF & (byte >> 4);
	write_safe(lower < 10 ? lower + '0' : lower - 10 + 'A');
	const auto higher = 0xF & byte;
	write_safe(higher < 10 ? higher + '0' : higher - 10 + 'A');
}

void write_word_safe(uint16_t word)
{
	write_byte_safe(0xFF & (word >> 8));
	write_byte_safe(0xFF & word);
}

void write_dword_safe(uint32_t word)
{
	write_word_safe(0xFFFF & (word >> 16));
	write_word_safe(0xFFFF & word);
}

void write_qword_safe(uint64_t word)
{
	write_dword_safe(0xFFFFFFFF & (word >> 32));
	write_dword_safe(0xFFFFFFFF & word);
}

void dump_registers(const ucontext_t* context)
{
	constexpr static std::pair<const char*, uint64_t> regs[] = {
		{"R8      ", REG_R8},
		{"R9      ", REG_R9},
		{"R10     ", REG_R10},
		{"R11     ", REG_R11},
		{"R12     ", REG_R12},
		{"R13     ", REG_R13},
		{"R14     ", REG_R14},
		{"R15     ", REG_R15},
		{"RDI     ", REG_RDI},
		{"RSI     ", REG_RSI},
		{"RBP     ", REG_RBP},
		{"RSP     ", REG_RSP},
		{"RAX     ", REG_RAX},
		{"RBX     ", REG_RBX},
		{"RCX     ", REG_RCX},
		{"RDX     ", REG_RDX},
		{"RIP     ", REG_RIP},
		{"EFL     ", REG_EFL},
		{"CSGSFS  ", REG_CSGSFS},
		{"ERR     ", REG_ERR},
		{"TRAPNO  ", REG_TRAPNO},
		{"OLDMASK ", REG_OLDMASK},
		{"CR2     ", REG_CR2}
	};
	for (const auto& reg : regs) {
		write_str_safe(reg.first);
		write_qword_safe(context->uc_mcontext.gregs[reg.second]);
		write_safe('\n');
	}
}

bool dump_memory(const siginfo_t* info, int range)
{
	int pipefd[2];
	if (pipe(pipefd) == -1) {
		return false;
	}

	const auto hrange = range / 2;
	for (auto i = -hrange; i < hrange; i++) {
		
		if (i == 0) {
			write_safe('>');
		}
		else {
			write_safe(' ');
		}

		const auto mem = static_cast<char*>(info->si_addr) + i;
		if (write(pipefd[1], mem, 1) == -1) {
			write_str_safe("??");
		}
		else {
			uint8_t value;
			if (read(pipefd[0], &value, 1) == -1) {
				write_str_safe("??");
			}
			else {
				write_byte_safe(value);
			}
		}

		if (i == 0) {
			write_str_safe("< ");
		}
		else {
			write_str_safe("  ");
		}
		if ((i & 7) == 7) {
			write_safe('\n');
		}
	}
	return true;
}

void handler(int sig, siginfo_t* info, void* context) {
	const auto ucontext = static_cast<ucontext_t*>(context);
	dump_registers(ucontext);
	if (!dump_memory(info, 64))
	{
		write_str_safe("cannot dump memory");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}

int main()
{
	struct sigaction sigact;
	sigact.sa_flags = SA_SIGINFO;
	if (sigemptyset(&sigact.sa_mask) == -1) {
		std::perror("sigemptyset");
		return EXIT_FAILURE;
	}
	sigact.sa_sigaction = handler;
	if (sigaction(SIGSEGV, &sigact, NULL) == -1) {
		std::perror("sigaction");
		return EXIT_FAILURE;
	}

    return EXIT_SUCCESS;
}