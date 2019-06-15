//
// Created by Noname Untitled on 15.06.19.
//

#pragma once

#include <unistd.h>
#include <cstdint>
#include <cstring>

class Utils {
public:
    static void printChar(unsigned char);

    static void printString(const char *);

    static void printHex(unsigned char);

    static void printNumber(unsigned char, size_t);
};