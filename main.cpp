#include <iostream>
#include "Handler.h"

using namespace std;

void print_error(const char *error) {
	write(1, error, strlen(error));
	_exit(EXIT_FAILURE);
}

void generate_error() {
	char *str = const_cast<char *>("Hello, World!");
	str[15] = ' ';
}

int main() {
        struct sigaction sa{};
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = Handler::handler;

        if (sigaction(SIGSEGV, &sa, nullptr) == -1) {
		print_error("sigaction");
	}

	generate_error();

	return 0;
}
