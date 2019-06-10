#include <iostream>
#include <signal.h>
#include <cstring>
#include <map>
#include <csetjmp>
#include <limits>
#include <vector>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <ucontext.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


using namespace std;

const map<string, int> registers = {{"R8",      REG_R8},
                                    {"R9",      REG_R9},
                                    {"R10",     REG_R10},
                                    {"R11",     REG_R11},
                                    {"R12",     REG_R12},
                                    {"R13",     REG_R13},
                                    {"R14",     REG_R14},
                                    {"R15",     REG_R15},
                                    {"RAX",     REG_RAX},
                                    {"RBP",     REG_RBP},
                                    {"RBX",     REG_RBX},
                                    {"RCX",     REG_RCX},
                                    {"RDI",     REG_RDI},
                                    {"RDX",     REG_RDX},
                                    {"RIP",     REG_RIP},
                                    {"RSI",     REG_RSI},
                                    {"RSP",     REG_RSP},
                                    {"CR2",     REG_CR2},
                                    {"EFL",     REG_EFL},
                                    {"CSGSFS",  REG_CSGSFS},
                                    {"ERR",     REG_ERR},
                                    {"OLDMASK", REG_OLDMASK},
                                    {"TRAPNO",  REG_TRAPNO}};


void checkErr(int value, const char *message) {
    if (value == -1) {
        cerr << message << endl;
        exit(EXIT_FAILURE);
    }
}

jmp_buf jmp;

void helper(int, siginfo_t * info, void *) {
    if (info->si_signo == SIGSEGV) {
        siglongjmp(jmp, 1);
    }
}

void write(const char* data, size_t size) {
    size_t result;
    while (size > 0 && (result = static_cast<size_t>(write(STDERR_FILENO, data, size)))) {
        if (result < 0 && errno == EINTR) {
            continue;
        } else if (result < 0) {
            exit(EXIT_FAILURE);
        }
        size -= result;
        data += result;
    }
}

void write(std::string const& pointer) {
    write(pointer.c_str(), pointer.size());
}

std::string to_hex(size_t value) {
    std::stringstream stream;
    stream << std::hex << value;
    std::string res = stream.str();
    return "0x" + res + (res.size() == 1 ? "0" : "");
}

std::string to_hex(char* value){
    return "0x" + string(value);
}

void sigHandler(int, siginfo_t *info, void *context) {
    if (info->si_signo == SIGSEGV) {
        write("registers here: \n");
        for (auto &reg :registers) {
            write(string(reg.first.c_str()) + " = " +  to_hex(((ucontext_t*)context)->uc_mcontext.gregs[reg.second]) + "\n");
        }

        for (char i = -16; i <= 16; i += sizeof(char)) {
            auto address = (char *) info->si_addr + i;

            if ((int64_t) address < 0 || (int64_t) address > INT64_MAX) {
                continue;
            }


            struct sigaction action{};
            action.sa_flags = SA_SIGINFO;
            action.sa_sigaction = helper;


            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set, SIGSEGV);
            sigprocmask(SIG_UNBLOCK, &set, nullptr);

            checkErr(sigaction(SIGSEGV, &action, nullptr), "Can't sigaction");



            if (setjmp(jmp) != 0) {
                std::cout << std::hex << (void *) address << " unknown" << '\n';
            } else {
                std::cout << std::hex << (void *) address << " " << (int) *address << '\n';
            }
        }

        exit(EXIT_FAILURE);
    }
}


int main() {
    struct sigaction action{};
    memset(&action, 0, sizeof(action));
    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = sigHandler;
    checkErr(sigaction(SIGSEGV, &action, nullptr), "Can't sigaction");

    exit(EXIT_SUCCESS);
}