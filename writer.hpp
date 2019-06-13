#ifndef WRITER_HPP_
#define WRITER_HPP_

#include <stdio.h>
#include <unistd.h>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <type_traits>

void write_string(const char* s) { write(STDERR_FILENO, s, strlen(s)); }

void write_byte(uint8_t bt) {
    const char sym = static_cast<char>((bt < 10) ? bt + '0' : bt - 10 + 'A');
    write(STDERR_FILENO, &sym, 1);
}

template <typename T,
          class = typename std::enable_if<std::is_integral<T>::value>::type>
void write_hex_number(T num) {
    write_string("0x");
    for (int i = 0; i < sizeof num; ++i) {
        write_byte(((num >> (i << 3)) & 0xFF) / 16);
        write_byte(((num >> (i << 3)) & 0xFF) % 16);
    }
}

void write_register(const char* reg, greg_t value) {
    write_string(reg);
    write_string(" = ");
    write_hex_number(value);
    write_string("\n");
}

#endif  // WRITER_HPP_
