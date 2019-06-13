//
// Created by ifkbhit on 12.06.19.
//

#include <cstdint>
#include "Output.h"


Output& Output::ws(size_t count) {
    for (size_t i = 0; i < count; i++) {
        putString(" ");
    }
    return *this;
}

Output& Output::newLine(size_t count) {
    for (size_t i = 0; i < count; i++) {
        putString("\n");
    }
    return *this;
}

Output& Output::putString(const char* string) {
    write(_fd, string, strlen(string));
    return *this;
}

Output::Output(int fd) : _fd(fd) {}


Output& Output::putByte(Output::byte byte) {
    auto left = toHex(byte >> (unsigned) 4);
    auto right = toHex(byte >> (unsigned) 0xF);
    write(_fd, &left, 1);
    write(_fd, &right, 1);
    return *this;
}

Output::byte Output::toHex(Output::byte b) {
    return b + (b < 10 ? '0' : 'a' - 10);
}

Output& Output::putHex(unsigned long int hex) {
    putString("0x");
    for (int i = 0; i < 8; i++) {
        putByte((hex >> ((unsigned) (7 - i) << (unsigned) 3)) & (unsigned) 0xFF);
    }
    return *this;
}
