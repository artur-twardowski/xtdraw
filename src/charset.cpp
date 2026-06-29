#include <assert.h>
#include <stdlib.h>

#include <array>
#include <iostream>
#include <map>

#include "charset.h"

uint32_t GetBoxDrawingCharacter(const box_drawing_spec_t &s) {
    // clang-format off
    static constexpr uint32_t VERTICAL_LINES[] = {
        /* Thin */   0x2502, 0x254e, 0x2506, 0x250a,
        /* Thick */  0x2503, 0x254f, 0x2507, 0x250b,
        /* Double */ 0x2551, 0x2551, 0x2551, 0x2551
    };
    static constexpr uint32_t HORIZONTAL_LINES[] = {
        /* Thin */   0x2500, 0x254c, 0x2504, 0x2508,
        /* Thick */  0x2501, 0x254d, 0x2505, 0x2509,
        /* Double */ 0x2550, 0x2550, 0x2550, 0x2550
    };
    static constexpr uint32_t CORNERS[] = {
        0x257a, 0x2576, 0x257a, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
        0x0000, 0x0000, 0x0000, 0x2510, 0x0000, 0x0000, 0x0000, 0x2511, 0x0000, 0x0000, 0x0000, 0x0000, 0x250c, 0x250d, 0x0000, 0x0000, 
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
    };
    // clang-format on
    static constexpr size_t N_VERTICAL_LINES =
        sizeof(VERTICAL_LINES) / sizeof(VERTICAL_LINES[0]);
    static constexpr size_t N_HORIZONTAL_LINES =
        sizeof(HORIZONTAL_LINES) / sizeof(HORIZONTAL_LINES[0]);
    static constexpr size_t N_CORNERS = sizeof(CORNERS) / sizeof(CORNERS[0]);
    static_assert(N_VERTICAL_LINES == 12);
    static_assert(N_HORIZONTAL_LINES == 12);
    static_assert(N_CORNERS == 256);
    // Either horizontal or vertical, with the same thickness
    if (s.north == s.south && s.east == s.west && s.north != line_weight_t::NONE &&
        s.east == line_weight_t::NONE) {
        size_t index = static_cast<size_t>(s.north) * 4 + (s.attributes & 0x03);
        assert(index < N_VERTICAL_LINES);
        return VERTICAL_LINES[index];
    } else if (s.north == s.south && s.east == s.west && s.east != line_weight_t::NONE &&
               s.north == line_weight_t::NONE) {
        size_t index = static_cast<size_t>(s.east) * 4 + (s.attributes & 0x03);
        assert(index < N_HORIZONTAL_LINES);
        return HORIZONTAL_LINES[index];
    } else {
        size_t index = (static_cast<size_t>(s.north) << 6) |
                       (static_cast<size_t>(s.south) << 4) |
                       (static_cast<size_t>(s.west) << 2) | static_cast<size_t>(s.east);
        assert(index < N_CORNERS);
        uint32_t ch = CORNERS[index];
        if (ch) {
            // If rounded corners have been selected, substitute the value if needed
            if (s.attributes & ATTR_ROUNDED_CORNERS) {
                if (ch == 0x250c) {  // left top
                    return 0x256d;
                } else if (ch == 0x2510) {  // right top
                    return 0x256e;
                }
            }
            return ch;
        } else {
            std::cout << "DDD" << std::hex << index << std::dec;
        }
    }
    return '?';
}

static constexpr uint32_t BLOCKS_2x2[] = {0x0020, 0x2598, 0x259d, 0x2580, 0x2596, 0x258c,
                                          0x259e, 0x259b, 0x2597, 0x259a, 0x2590, 0x259c,
                                          0x2584, 0x2599, 0x259f, 0x2588};

static constexpr auto BLOCKS_2x3{[]() constexpr {
    std::array<uint32_t, 64> result;
    // The following ones are not in the order:
    result[0b000000] = 0x0020;  // None of the sextants - space
    result[0b010101] =
        0x258c;  // Three leftmost sextants - equivalent to two leftmost quadrants
    result[0b101010] =
        0x2590;  // Three rightmost sextants - equivalent to two rightmost quadrants
    result[0b111111] = 0x2588;  // All the sextants - full block
    for (size_t ix = 0b000001; ix < 0b010101; ix++) {
        result[ix] = 0x1fb00 + ix - 1;
    }
    for (size_t ix = 0b010101 + 1; ix < 0b101010; ix++) {
        result[ix] = 0x1fb00 + ix - 2;
    }
    for (size_t ix = 0b101010 + 1; ix < 0b111111; ix++) {
        result[ix] = 0x1fb00 + ix - 3;
    }

    return result;
}()};

static constexpr uint8_t mvbit(uint32_t v, uint8_t src_pos, uint8_t dst_pos) {
    return (!!(v & (1 << src_pos))) << dst_pos;
}

static constexpr auto BLOCKS_2x4_BRAILLE{[]() constexpr {
    std::array<uint32_t, 256> result;
    result[0] = 0x20;
    for (size_t ix = 1; ix < 256; ix++) {
        uint8_t pos = mvbit(ix, 0, 0) | mvbit(ix, 2, 1) | mvbit(ix, 4, 2) |
                      mvbit(ix, 1, 3) | mvbit(ix, 3, 4) | mvbit(ix, 5, 5) |
                      mvbit(ix, 6, 6) | mvbit(ix, 7, 7);
        result[ix] = 0x2800 + pos;
    }
    return result;
}()};

static std::map<uint32_t, uint8_t> BuildDecodingMap2x2() {
    std::map<uint32_t, uint8_t> result;
    for (size_t ix = 0; ix < sizeof(BLOCKS_2x2) / sizeof(BLOCKS_2x2[0]); ix++) {
        result.emplace(BLOCKS_2x2[ix], ix);
    }
    return result;
}
static std::map<uint32_t, uint8_t> BuildDecodingMap2x3() {
    std::map<uint32_t, uint8_t> result;
    for (size_t ix = 0; ix < BLOCKS_2x3.size(); ix++) {
        result.emplace(BLOCKS_2x3[ix], ix);
    }
    return result;
}
static std::map<uint32_t, uint8_t> BuildDecodingMap2x4() {
    std::map<uint32_t, uint8_t> result;
    for (size_t ix = 0; ix < BLOCKS_2x4_BRAILLE.size(); ix++) {
        result.emplace(BLOCKS_2x4_BRAILLE[ix], ix);
    }
    return result;
}

static const auto BLOCKS_2x2_DEC         = BuildDecodingMap2x2();
static const auto BLOCKS_2x3_DEC         = BuildDecodingMap2x3();
static const auto BLOCKS_2x4_BRAILLE_DEC = BuildDecodingMap2x4();

uint32_t GetBlockDrawingCharacter(uint8_t pixels, box_drawing_t set) {
    switch (set) {
        case box_drawing_t::BLK_2x2:
            return BLOCKS_2x2[pixels & 0x0f];
        case box_drawing_t::BLK_2x3:
            return BLOCKS_2x3[pixels & 0x3f];
        case box_drawing_t::BLK_2x4_BRAILLE:
            return BLOCKS_2x4_BRAILLE[pixels & 0xff];
        default:
            return 0x20;
    }
}

bool CharacterToPixels(uint8_t &pixels, box_drawing_t set, uint32_t ch) {
    if (set == box_drawing_t::BLK_2x2) {
        auto it2x2 = BLOCKS_2x2_DEC.find(ch);
        if (it2x2 != BLOCKS_2x2_DEC.end()) {
            pixels = it2x2->second;
            return true;
        }
    } else if (set == box_drawing_t::BLK_2x3) {
        auto it2x3 = BLOCKS_2x3_DEC.find(ch);
        if (it2x3 != BLOCKS_2x3_DEC.end()) {
            pixels = it2x3->second;
            return true;
        }
    } else if (set == box_drawing_t::BLK_2x4_BRAILLE) {
        auto it2x4 = BLOCKS_2x4_BRAILLE_DEC.find(ch);
        if (it2x4 != BLOCKS_2x4_BRAILLE_DEC.end()) {
            pixels = it2x4->second;
            return true;
        }
    }
    return false;
}
