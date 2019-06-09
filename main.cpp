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

const size_t SIZE = 1;
const size_t RANGE = 10;
jmp_buf env;
static const std::vector<std::pair<std::string, int>> REGISTERS = {
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
void defaultHandler(int sig) {
    if (sig == SIGSEGV) {
        siglongjmp(env, 1);
    }
}
void dumpMem(char * addr){
    std::cout<<"MEMORY DUMP : "<<"\n";
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
            std::cerr<<"Error occured during sigaction"<<"\n";
            exit(EXIT_FAILURE);
        }
        if (setjmp(env) != 0)
        {
            std::cout << "failed to dump\n";
        } else {
            std::cout  << +*reinterpret_cast<char*>(address)<<"\n";
        }

    }
}
void dumpReg(ucontext_t *context){
    std::cout<<"General purposed registers:\n";
    auto greg = context->uc_mcontext.gregs;
    for(auto [name,index] : REGISTERS){
        std::cout<< name << ": 0x"<<std::hex<<greg[index] << "\n";
    }
}
void handler (int signal ,siginfo_t * info,void* context ){
    if(info->si_signo == SIGSEGV){
        std::cout<<"Segmentation fault occured"<<"\n";
        if(info->si_addr == nullptr){
            std::cout<<"Address: NULL"<<"\n";
        }
        else {
            std::cout<<info->si_addr<<"\n";
        }
        if(info->si_code == SEGV_MAPERR){
            std::cout<<"Reason was : The address doesn't match the object"<<"\n";
        }
        if(info->si_code == SEGV_ACCERR){
            std::cout<<"Reason was : No permission for the mapped object"<<"\n";
        }
        if(info->si_addr != nullptr) dumpMem(static_cast<char*>(info->si_addr));
        dumpReg((ucontext_t*) context);
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
        std::cerr<<"Error occured during sigaction"<<"\n";
        exit(EXIT_FAILURE);
    }
    std::cout<<"testing with nullptr : "<<"\n";
    first_test();
//    std::cout<<"testing with bounds : " << "\n";
  //  second_test();
    return 0;
}
