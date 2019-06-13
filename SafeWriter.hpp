#include <unistd.h>
#include <cstring>
#include <cstdint>

namespace SafeWriter {
    void writeHalf(uint8_t byte) {
        char c = byte < 10 ? byte + '0' : byte + 'A' - 10;
        write(1, &c, 1);
    }

    void writeByte(uint8_t byte) {
        writeHalf(byte / 16);
        writeHalf(byte % 16);
    }

    void safeWrite(uint64_t value) {
        for (int i = 7; i >= 0; --i) {
            writeByte(0xFF & (value >> (8 * i)));
        }
    }

    void safeWrite(const char* string) {
        write(1, string, strlen(string));
    }
}