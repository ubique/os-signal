#include <iostream>
#include <cstring>
#include <climits>

#include <csignal>
#include <csetjmp>

const int MEMORY_DUMP_RANGE = 20 * sizeof(char);
const char* regStr[] = {"R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15",
"RDI", "RSI", "RBP", "RBX", "RDX", "RAX", "RCX", "RSP", "RIP", "EFL", "CSGSFS",
"ERR", "TRAPNO", "OLDMASK", "CR2"};

static jmp_buf jmpBuf;


void sigsegvAddressHandler(int num, siginfo_t* siginfo, void* context)
{
    if (siginfo -> si_signo == SIGSEGV)
    {
        siglongjmp(jmpBuf, 1);
    }
}

void dumpReg(ucontext_t* context)
{
    std::cout << "Registers dump \n";
    for (size_t reg = 0; reg < NGREG; ++reg)
    {
        std::cout << regStr[reg] << ": 0x" << std::hex << context->uc_mcontext.gregs[reg] << '\n';
    }
}

void dumpMem(void* address)
{
    std::cout << "Memory dump \n";
    const long long from = std::max(0LL, (long long) ((char*) address - MEMORY_DUMP_RANGE));
    const long long to = std::min(LONG_LONG_MAX, (long long) ((char*) address + MEMORY_DUMP_RANGE));
    for (long long i = from; i < to; i += sizeof(char))
    {
        sigset_t setSignal;
        sigemptyset(&setSignal);
        sigaddset(&setSignal, SIGSEGV);
        sigprocmask(SIG_UNBLOCK, &setSignal, nullptr);
        struct sigaction sAction {};
        sAction.sa_sigaction = sigsegvAddressHandler;
        sAction.sa_flags = SA_SIGINFO;
        if (sigaction(SIGSEGV, &sAction, nullptr) == -1)
        {
            std::cout << "Error occurred during sigaction\n";
            exit(-1);
        }
        std::cout << (void*) i << ' ';
        if (setjmp(jmpBuf) != 0)
        {
            std::cout << "failed to dump\n";
        } else {
            std::cout  << std::hex << (int)((const char*)i)[0] <<"\n";
        }
    }
}

void sigsegvHandler(int num, siginfo_t* siginfo, void* context)
{
    if (siginfo -> si_signo == SIGSEGV)
    {
        std::cout << "Signal rejected: " << strsignal(num) << '\n';
        if (siginfo -> si_code == SEGV_MAPERR)
        {
            std::cout << "Reason: nothing is mapped to address\n";
        } else if(siginfo -> si_code == SEGV_ACCERR) {
            std::cout << "Reason: access error\n";
        } else {
            std::cout << "Error code: " << siginfo -> si_code << '\n';
        }
        std::cout << "Address: " << siginfo -> si_addr << '\n';
        dumpReg((ucontext_t*) context);
        dumpMem(siginfo -> si_addr);
    }
    exit(0);
}

int main(int argc, char* argv[], char *envp[])
{
    struct sigaction action {};
    action.sa_sigaction = sigsegvHandler;
    action.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSEGV, &action, nullptr) == -1)
    {
        std::cout << "Error occurred during sigaction\n";
        return -1;
    }
    std::cout << "Choose one of the options: \n";
    std::cout << "1. Left-bound test\n2. Right-bound test\n3. nullptr test\n";
    unsigned short ans = 0;
    std::cin >> ans;
    if (ans == 1)
    {
        char* test = (char*) "check";
        *(--test) = 'A';
    } else if (ans == 2) {
        char* test = (char*) "check";
        test[6] = '1';
    } else if (ans == 3) {
        unsigned short* test = nullptr;
        *test = 1;
    } else {
        std::cout << "Incorrect input: expected number of option\n";
    }
    return 0;
}
