#include <iostream>
#include <cstring>
#include <climits>

#include <csignal>
#include <csetjmp>

const int MEMORY_DUMP_RANGE = 20 * sizeof(char);
const char *registers[] = {"R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15",
                           "RDI", "RSI", "RBP", "RBX", "RDX", "RAX", "RCX", "RSP", "RIP", "EFL", "CSGSFS",
                           "ERR", "TRAPNO", "OLDMASK", "CR2"};

static jmp_buf jmpBuf;

void sigsegvAddressHandler(int num, siginfo_t *siginfo, void *context)
{
    if (siginfo->si_signo == SIGSEGV)
        siglongjmp(jmpBuf, 1);
}

void sigsegvHandler(int num, siginfo_t *siginfo, void *context)
{
    if (siginfo->si_signo == SIGSEGV)
    {
        std::cout << "Aborted: " << strsignal(num) << std::endl;

        int signal_code = siginfo->si_code;
        if (signal_code == SEGV_MAPERR)
        {
            std::cout << "Reason: nothing is mapped to address\n";
        }
        else if (signal_code == SEGV_ACCERR)
        {
            std::cout << "Reason: access error\n";
        }
        else
        {
            std::cout << "Error code: " << signal_code << std::endl;
        }

        auto signal_addr = siginfo->si_addr;
        std::cout << "Address: " << signal_addr << std::endl;

        std::cout << "Registers dump: \n";
        for (size_t i = 0; i < NGREG; ++i)
            std::cout << "\t" << registers[i] << ": 0x" << std::hex << ((ucontext_t *)context)->uc_mcontext.gregs[i] << std::endl;

        std::cout << "Memory dump: \n";

        const long long from = std::max(0LL, (long long)((char *)signal_addr - MEMORY_DUMP_RANGE));
        const long long to = std::min(LONG_LONG_MAX, (long long)((char *)signal_addr + MEMORY_DUMP_RANGE));

        for (long long i = from; i < to; i += sizeof(char))
        {
            sigset_t setSignal;
            sigemptyset(&setSignal);
            sigaddset(&setSignal, SIGSEGV);
            sigprocmask(SIG_UNBLOCK, &setSignal, nullptr);
            struct sigaction sAction
            {
            };
            sAction.sa_sigaction = sigsegvAddressHandler;
            sAction.sa_flags = SA_SIGINFO;
            if (sigaction(SIGSEGV, &sAction, nullptr) == -1)
            {
                perror("Error occurred during sigaction");
                exit(-1);
            }
            std::cout << "\t" << (void *)i << ": ";
            if (setjmp(jmpBuf) != 0)
            {
                std::cout << "failed to dump" << std::endl;
            }
            else
            {
                std::cout << std::hex << (int)((const char *)i)[0] << std::endl;
            }
        }
    }
    exit(0);
}

int main(int argc, char *argv[], char *envp[])
{
    struct sigaction action {};
    action.sa_sigaction = sigsegvHandler;
    action.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &action, nullptr) == -1)
    {
        perror("Error occurred during sigaction");
        return -1;
    }
    std::cout << "1. Nullptr test\n"
                 "2. Left-bound test\n"
                 "3. Right-bound test\n";
    std::cout << "Choose number: ";

    unsigned short ans = -1;
    std::cin >> ans;
    char *test;
    switch (ans)
    {
    case 1:
        test = nullptr;
        *test = 1;
        break;
    case 2:
        test = (char *)"data";
        *(--test) = 'A';
        break;
    case 3:
        test = (char *)"data";
        test[5] = '1';
        break;
    default:
        std::cout << "Incorrect input: expected number of option\n";
    }
    return 0;
}