//
// Created by ifkbhit on 12.06.19.
//

#pragma once

#include <unistd.h>
#include <cstring>

class Output {
private:
    int _fd;

    using byte = uint8_t;
    byte toHex(byte b);


public:
    explicit Output(int fd = STDERR_FILENO);
    Output& putString(const char*);
    Output& putHex(unsigned long int hex);
    Output& ws(size_t = 1);
    Output& newLine(size_t = 1);
    Output& putByte(byte b);
};

