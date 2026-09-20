#ifndef XTDRAW_UTILS_H
#define XTDRAW_UTILS_H

#include <stdint.h>

#include <string>

namespace xtdraw {
std::string EncodeUTF8(uint32_t keycode);
uint32_t    ReadUTF8(std::istream &f, uint8_t first_char);
uint8_t     ReadByteFromFile(std::istream &f);
}  // namespace xtdraw

#endif
