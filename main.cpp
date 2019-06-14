#include <csignal>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <ucontext.h>
#include <cmath>
#include <algorithm>
#include <climits>
#include <sys/ucontext.h>
#include <csetjmp>
#include <vector>
#include <unistd.h>

const size_t SIZE = 1;
const size_t RANGE = 10;
jmp_buf env;
static const std::vector<std::pair<const char*, const int>> REGISTERS = {
        {"R8",      REG_R8},
        {"R9",      REG_R9},
        {"R10",     REG_R10},
        {"R11",     REG_R11},
        {"R12",     REG_R12},
        {"R13",     REG_R13},
        {"R14",     REG_R14},
        {"R15",     REG_R15},
        {"RDI",     REG_RDI},
        {"RSI",     REG_RSI},
        {"RBP",     REG_RBP},
        {"RBX",     REG_RBX},
        {"RDX",     REG_RDX},
        {"RAX",     REG_RAX},
        {"RCX",     REG_RCX},
        {"RSP",     REG_RSP},
        {"RIP",     REG_RIP},
        {"EFL",     REG_EFL},
        {"CSGSFS",  REG_CSGSFS},
        {"ERR",     REG_ERR},
        {"TRAPNO",  REG_TRAPNO},
        {"OLDMASK", REG_OLDMASK},
        {"CR2",     REG_CR2}
};

void write_str(const char *s){
    write(1,s,strlen(s));
}
void writer_helper(intptr_t num) {
    if (num) {
        writer_helper(num >> 4);
        long long digit = num & ((1ll << 4) - 1);
        char symbol = ((digit > 9 ? 'a' - 10 : '0') + digit);
        write(1, &symbol, 1);
    }
}
void write_num(uintptr_t num){
    if(num == 0){
        write_str("0");
    }
    else writer_helper(num);
}

void defaultHandler(int sig) {
    if (sig == SIGSEGV) {
        siglongjmp(env, 1);
    }
}
void dumpMem(intptr_t addr){
    write_str("MEMORY DUMP : ");
    write_str("\n");
    long long left = std::max(0LL,(long long)(addr - RANGE * SIZE));
    long long right = std::min(std::numeric_limits<long long>::max(),(long long) (addr + RANGE * SIZE));
    for(long long address = left; address < right ; address += SIZE){
        sigset_t setSig;
        sigemptyset(&setSig);
        sigaddset(&setSig,SIGSEGV);
        sigprocmask(SIG_UNBLOCK,&setSig, nullptr);
        struct sigaction sa{};
        sa.sa_handler = defaultHandler;
        if(sigaction(SIGSEGV,&sa, nullptr) == -1){
            write_str("Error occured during sigaction");
            write_str("\n");
            exit(EXIT_FAILURE);
        }
        if (setjmp(env) != 0)
        {
            write_str("failed to dump\n");
        } else {
            write_num(static_cast<intptr_t>(address));
            write_str("\n");
        }

    }
}
void dumpReg(ucontext_t *context){
    write_str("General purposed registers:\n");
    auto greg = context->uc_mcontext.gregs;
    for(auto [name,index] : REGISTERS){
        write_str(name);
        write_str(": 0x");
        write_num(greg[index]);
        write_str("\n");
    }
}

void handler (int signal ,siginfo_t * info,void* context ){
    if(info->si_signo == SIGSEGV){
        write_str(strsignal(signal));
        write_str("\n");
           if(info->si_addr == nullptr){
            write_str("Address: NULL");
            write_str("\n");
        }
        else {
            write_str("Address : ");
            auto addr = reinterpret_cast<uintptr_t>(info->si_addr);
            write_str("0x");
            write_num(addr);
            write_str("\n");
        }
        if(info->si_code == SEGV_MAPERR){
            write_str("Reason was : The address doesn't match the object");
            write_str("\n");
        }
        if(info->si_code == SEGV_ACCERR){
            write_str("Reason was : No permission for the mapped object");
            write_str("\n");
        }
        if(info->si_addr != nullptr) dumpMem(reinterpret_cast<uintptr_t>(info->si_addr));
        dumpReg(static_cast<ucontext_t*>(context));
    }
    exit(EXIT_FAILURE);
}
void first_test(){
    int *p = nullptr;
    (*p)++;
}
void second_test(){
    char a[10];
    for(auto p = a; ; p++){
        (*p)++;
    }
}
int main() {
    struct sigaction sa{};
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    if(sigaction(SIGSEGV,&sa,nullptr) == -1){
        write_str("sigaction error");
        exit(EXIT_FAILURE);
    }
//    write_str("testing with nullptr : ");
//    write_str("\n");
//    first_test();
    write_str("testing with bounds : " );
    write_str("\n");
    second_test();
    return 0;
}
