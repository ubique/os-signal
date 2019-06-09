#include <iostream>
#include "Handler.h"

using namespace std;

void printError(const char *error) {
	perror(error);
	exit(EXIT_FAILURE);
}

void generateError() {
	char *str = const_cast<char *>("Hello, World!");
	str[15] = ' ';
}

int main() {
        struct sigaction sa{};
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = Handler::handler;

        if (sigaction(SIGSEGV, &sa, nullptr) == -1) {
		printError("sigaction");
	}

	generateError();

	return 0;
}
