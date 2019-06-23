#include <iostream>
#include <cstring>
#include <climits>
#include <csignal>
#include <csetjmp>
#include <unistd.h>
#include "signal.h"

using std::cin, std::cout, std::endl, std::cerr;

int main(int argc, char* argv[], char *envp[])
{
    struct sigaction action {};
    action.sa_sigaction = handler;
    action.sa_flags = SA_SIGINFO;

    if (sigaction(SIGSEGV, &action, nullptr) == -1)
    {
        cerr << "Sigaction failed" << endl;
        exit(0);
    }

    cout << "Choose one of the options:" << endl;
    cout << "1. Left-bound test" << endl << "2. Right-bound test" << endl << "3. nullptr test" << endl;

    int ans;
    cin >> ans;

    if        (ans == 1) {
        char* test = (char*) "check";
        *(--test) = 'A';
    } else if (ans == 2) {
        char* test = (char*) "check";
        test[6] = '1';
    } else if (ans == 3) {
        unsigned short* test = nullptr;
        *test = 1;
    } else {
        cout << "Incorrect input: expected number of option" << endl;
    }
    return 0;
}