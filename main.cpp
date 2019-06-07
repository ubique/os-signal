//
// Created by roman on 07.06.19.
//
#include <iostream>
#include <signal.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <limits>
#include <iomanip>
#include <sys/mman.h>
#include <malloc.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;


gregset_t *gregset;

template<typename T>
void print_register(const string &name, T reg) {
    cout << name << ": " << "0x" << std::hex
         << std::setfill('0') << std::setw(sizeof(void *) * 2) << (*gregset)[reg] << endl;
}

static constexpr size_t radius = 32;

static const char *no_permission = "####";
static const char *undefined = "****";
static const char *delimeter = ", ";
static const char *l_ar = "-->";
static const char *r_ar = "<--";

static void handler(int sig, siginfo_t *info, void *ucontext) {
    cout << "SIGSEGV caught" << endl;
    cout << "Memory location which caused fault: " << info->si_addr << endl;
//    print_register("RAX", )
    auto *context = (ucontext_t *) ucontext;
    gregset = &context->uc_mcontext.gregs;
    print_register("RAX", REG_RAX);
    print_register("RBX", REG_RBX);
    print_register("RCX", REG_RCX);
    print_register("RDX", REG_RDX);
    print_register("RBP", REG_RBP);
    print_register("RSP", REG_RSP);
    print_register("RSI", REG_RSI);
    print_register("RDI", REG_RDI);
    print_register("R8", REG_R8);
    print_register("R9", REG_R9);
    print_register("R10", REG_R10);
    print_register("R11", REG_R11);
    print_register("R12", REG_R12);
    print_register("R13", REG_R13);
    print_register("R14", REG_R14);
    print_register("R15", REG_R15);
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        cerr << "Pipe error: " << strerror(errno) << endl;
        cerr << "Cannot dump memory" << endl;
        exit(EXIT_FAILURE);
    }
    cout << "Memory dump:" << endl;
    auto adr = (size_t) info->si_addr;
    auto ptr = (char*)info->si_addr;
    auto lower = (char *) (adr >= radius ? adr - radius : 0);
    auto upper = (char *) (std::numeric_limits<size_t>::max() - radius >= adr ?
                           adr + radius :
                           std::numeric_limits<size_t>::max());
    for (auto i = lower; i < upper; ++i) {
        if (i == ptr) {
            write(1, l_ar, strlen(l_ar));
        }
        if (write(pipefd[1], i, 1) != -1) {
            auto s_stream = std::ostringstream();
            s_stream << "0x" << std::setfill('0') << std::setw(2) << std::hex
                     << (unsigned int) (unsigned char) *i;
            string buf(s_stream.str());
            write(1, buf.data(), buf.size());

        } else {
            if (errno == EFAULT) {
                write(1, no_permission, strlen(no_permission));
            } else {
                write(1, undefined, strlen(undefined));
            }
        }
        if (i == ptr) {
            write(1, r_ar, strlen(r_ar));
        }
        if (i != upper) {
            write(1, delimeter, strlen(delimeter));
        }
        if ((i - lower) % 16 == 15) {
            write(1, "\n", 1);
        }
    }
    exit(EXIT_FAILURE);

}

int main(int argc, char *argv[]) {
    struct sigaction act{};
    act.sa_flags = SA_SIGINFO;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = handler;
    if (sigaction(SIGSEGV, &act, nullptr) == -1) {
        cerr << "Error during sigaction: " << strerror(errno);
        exit(EXIT_FAILURE);
    }
    //do not expect any errors from sys calls))))
    int pagesize = sysconf(_SC_PAGE_SIZE); //4096 expected
    char* buffer = (char*)memalign(pagesize, 4 * pagesize);
    for (int i = 0; i < 4 * pagesize; ++i) {
        buffer[i] = i%256;
    }
    mprotect(buffer + pagesize, pagesize, PROT_NONE);
    buffer[4097] = 'h';
    return 0;
}
