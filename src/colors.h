#ifndef COLORS_H
#define COLORS_H

#include "terminal_io.h"
#include <cassert>

namespace xtdraw {
static inline constexpr uint8_t Xterm256RGB(uint8_t r, uint8_t g, uint8_t b) {
    assert(r < 6 && g < 6 && b < 6);
    return 16 + r * 36 + g * 6 + b;
}
static inline constexpr uint8_t Xterm256Gray(uint8_t intensity) {
    assert(intensity < 24);
    return intensity + 232;
}

struct ColorPair {
    TerminalIO::Color background, foreground;
};
struct Colors {
    ColorPair char_picker_header{Xterm256RGB(0,1,2), uint8_t{0}};
    ColorPair char_picker_normal{uint8_t{4}, uint8_t{7}};
    ColorPair char_picker_cursor{uint8_t{2}, uint8_t{15}};
    ColorPair char_picker_inactive{uint8_t{8}, uint8_t{7}};
    ColorPair char_picker_inactive_cursor{uint8_t{7}, uint8_t{15}};
    ColorPair char_picker_info{Xterm256RGB(0,1,2), uint8_t{15}};

    ColorPair cursor_info{uint8_t{4}, uint8_t{15}};
    ColorPair cursor_mode{uint8_t{4}, uint8_t{11}};
    ColorPair cursor_mode_unavail{uint8_t{4}, uint8_t{9}};
};

void SetColor(TerminalIO &terminal_io, const ColorPair &color_pair);
}  // namespace xtdraw

#endif
