#include "core/utils.h"
#include <stdint.h>
#include <iostream>

namespace xtdraw {
static constexpr uint8_t ExtractBits(uint32_t in, uint8_t lsb, uint8_t count) {
    const uint32_t mask = ((1 << count) - 1) << lsb;
    return (in & mask) >> lsb;
}

std::string EncodeUTF8(uint32_t keycode) {
    char result[5];
    if (keycode <= 0x7F) {
        result[0] = ExtractBits(keycode, 0, 7);
        result[1] = '\0';
    } else if (keycode <= 0x7FF) {
        result[0] = 0b11000000 | ExtractBits(keycode, 6, 5);
        result[1] = 0b10000000 | ExtractBits(keycode, 0, 6);
        result[2] = '\0';
    } else if (keycode <= 0xFFFF) {
        result[0] = 0b11100000 | ExtractBits(keycode, 12, 4);
        result[1] = 0b10000000 | ExtractBits(keycode, 6, 6);
        result[2] = 0b10000000 | ExtractBits(keycode, 0, 6);
        result[3] = '\0';
    } else if (keycode <= 0x10FFFF) {
        result[0] = 0b11110000 | ExtractBits(keycode, 18, 3);
        result[1] = 0b10000000 | ExtractBits(keycode, 12, 6);
        result[2] = 0b10000000 | ExtractBits(keycode, 6, 6);
        result[3] = 0b10000000 | ExtractBits(keycode, 0, 6);
        result[4] = '\0';
    }
    return result;
}

uint32_t ReadUTF8(std::istream &f, uint8_t first_char) {
    size_t   bytes_to_read = 0;
    uint32_t result        = 0;

    if ((first_char & 0b10000000) == 0) {
        // Byte 0xxx_xxxx is a standard ASCII byte, nothing more to read
        return static_cast<uint32_t>(first_char);
    } else if ((first_char & 0b11000000) == 0b10000000) {
        // Byte 10xx_xxxx is a consecutive UTF-8 byte, but never the first one.
        // If we found it, something is wrong with the data.
    } else if ((first_char & 0b11100000) == 0b11000000) {
        // Byte 110x_xxxx indicates that we take 5 bits from this one, then
        // 6 bits from next byte (having a format of 10xx_xxxx).
        result        = first_char & 0b00011111;
        bytes_to_read = 1;
    } else if ((first_char & 0b11110000) == 0b11100000) {
        // Byte 1110_xxxx indicates that we take 4 bits from this one, then
        // 12 bits from next two bytes.
        result        = first_char & 0b00001111;
        bytes_to_read = 2;
    } else if ((first_char & 0b11111000) == 0b11110000) {
        // Byte 1111_0xxx indicates that we take 3 bits from this one, then
        // 18 bits from the next three bytes
        result        = first_char & 0b00000111;
        bytes_to_read = 3;
    }
    if (f.eof()) {
        return 0;
    }

    if (bytes_to_read == 0) {
        // We will eventually hit a valid byte
        return ReadUTF8(f, ReadByteFromFile(f));
    } else {
        uint8_t data[4];
        f.read(reinterpret_cast<char *>(data), bytes_to_read);
        for (size_t ix = 0; ix < bytes_to_read; ix++) {
            result = (result << 6) | (data[ix] & 0b00111111);
        }
    }

    return result;
}

uint8_t ReadByteFromFile(std::istream &f) {
    uint8_t b;
    f.read(reinterpret_cast<char *>(&b), 1);
    return b;
}

}  // namespace xtdraw
