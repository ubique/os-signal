#include <iostream>
#include "sigsegv_handler.h"

using namespace std;

int main()
{
    sigsegv_handler::set_sigsegv_handler();
    // your code goes here
    return 0;
}
