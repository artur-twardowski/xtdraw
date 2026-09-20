#ifndef CHARSET_H
#define CHARSET_H

#include <stdint.h>
#include <vector>
#include <string>

namespace xtdraw {

enum struct LineWeight : unsigned int { THIN = 0, THICK, DOUBLE, NONE };

enum struct BoxDrawing { BLK_2x2, BLK_2x3, BLK_2x4_BRAILLE };

static inline const uint8_t ATTR_SOLID           = 0x00;
static inline const uint8_t ATTR_DASH2           = 0x01;
static inline const uint8_t ATTR_DASH3           = 0x02;
static inline const uint8_t ATTR_DASH4           = 0x03;
static inline const uint8_t ATTR_ROUNDED_CORNERS = 0x04;

struct BoxDrawingSpec {
    LineWeight north{LineWeight::NONE}, south{LineWeight::NONE}, east{LineWeight::NONE},
        west{LineWeight::NONE};
    uint8_t attributes{0};
};

enum struct UnicodeVersion {
    V1_0 = 0,
    V13_0
};

uint32_t GetBoxDrawingCharacter(const BoxDrawingSpec &spec);

uint32_t GetBlockDrawingCharacter(uint8_t pixels, BoxDrawing set);
bool     CharacterToPixels(uint8_t &pixels, BoxDrawing set, uint32_t ch);
void GetCharacterSubset(std::vector<uint32_t> &chars, size_t set_index);
void GetCharacterSubset(std::vector<uint32_t> &chars, bool &double_width,
                        std::string &subset_name, size_t set_index);
bool IsCharacterSetDoubleWidth(size_t set_index);
uint32_t GetCharFromSubset(uint16_t set_index, uint16_t char_index);

}
#endif
