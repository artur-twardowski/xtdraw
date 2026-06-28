#ifndef CHARSET_H
#define CHARSET_H

#include <stdint.h>

enum struct line_weight_t : unsigned int { THIN = 0, THICK, DOUBLE, NONE };

enum struct box_drawing_t { BLK_2x2, BLK_2x3 };

static inline const uint8_t ATTR_SOLID           = 0x00;
static inline const uint8_t ATTR_DASH2           = 0x01;
static inline const uint8_t ATTR_DASH3           = 0x02;
static inline const uint8_t ATTR_DASH4           = 0x03;
static inline const uint8_t ATTR_ROUNDED_CORNERS = 0x04;

struct box_drawing_spec_t {
    line_weight_t north{line_weight_t::NONE}, south{line_weight_t::NONE}, east{line_weight_t::NONE},
        west{line_weight_t::NONE};
    uint8_t attributes{0};
};

uint32_t GetBoxDrawingCharacter(const box_drawing_spec_t &spec);

uint32_t GetBlockDrawingCharacter(uint8_t pixels, box_drawing_t set);
bool     CharacterToPixels(uint8_t &pixels, box_drawing_t set, uint32_t ch);

#endif
