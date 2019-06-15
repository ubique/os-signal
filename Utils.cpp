//
// Created by Noname Untitled on 15.06.19.
//

#include "Utils.h"

void printHelper(unsigned char);

void Utils::printChar(unsigned char ch) {
    write(1, &ch, 1);
}

void Utils::printString(const char *str) {
    write(1, str, strlen(str));
}

void Utils::printHex(unsigned char bt) {
    printHelper(bt / 16);
    printHelper(bt % 16);
}

void Utils::printNumber(unsigned char num, size_t size) {
    printString("0x");
    for (int i = size; i >= 1; --i) {
        printHex(0xFFu & (num >> (sizeof(char) * 8 * (i - 1))));
    }
}

void printHelper(unsigned char value) {
    char c;
    c = value < 10 ? value + '0' : value + 'A' - 10;
    write(1, &c, 1);
}
