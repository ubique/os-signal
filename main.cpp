
#include <iostream>
#include <cstdlib>
#include "handler/SIGSEGVHandler.h"
#include <string>


typedef void (* testFunc)();

void usage() {
    std::string help = "Usage:\n"
                       "    (executable) testName [memoryDumpRange]\n"
                       "\n"
                       "    where:\n"
                       "        testName is\n"
                       "            maperr - 'Address not mapped to object' signal code for SIGSEGV\n"
                       "            accerr - 'Invalid permissions for mapped object' signal code for SIGSEGV\n"
                       "        memoryDumpRange is\n"
                       "            number of range for dump memory. Will dump [address - memoryDumpRange, address + memoryDumpRange]. Default 18\n\n";
    std::cout << help;
}


void accerr() {
    char* c = const_cast<char*>("abc");
    std::cout << "3 element array. Let's set 4th element to 'd'\n";
    c[4] = 'd';
}

void maperr() {
    int* c = reinterpret_cast<int*>(NULL);
    std::cout << "What is 'NULL' contains?\n" << *c << " " << std::endl;
}


testFunc getTestFunc(char* name) {
    if (strcmp(name, "maperr") == 0) {
        return maperr;
    }
    if (strcmp(name, "accerr") == 0) {
        return accerr;
    }
    return nullptr;
}

int main(int argc, char** argv) {

    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        usage();
        return 0;
    }

    int range = 18;

    if (argc == 2 || argc == 3) {
        auto function = getTestFunc(argv[1]);
        if (function == nullptr) {
            std::cerr << "Undefined test with name: " << argv[1] << std::endl;
            return 0;
        }
        if (argc == 3) {
            range = std::stoi(std::string(argv[2]));
            if (range < 0) {
                std::cerr << "Range must be positive\n";
                return 0;
            }
        }

        if (!SIGSEGVHandler::attach(range)) {
            std::cout << "Cannot attach handler\n";
            return 0;
        }

        function();
    } else {
        std::cerr << "Wrong input args. Try --help.\n";
    }


    return 0;
}