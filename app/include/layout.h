#ifndef XTDRAW_LAYOUT_H
#define XTDRAW_LAYOUT_H

#include <stdint.h>
namespace xtdraw {
struct Layout {
    uint16_t draw_area_left;
    uint16_t draw_area_top;
    uint16_t draw_area_width;
    uint16_t draw_area_height;

    bool     char_picker_visible;
    uint16_t char_picker_left;
    uint16_t char_picker_top;

    bool     palette_visible;
    uint16_t palette_left;
    uint16_t palette_top;

    uint16_t cursor_pos_left;
    uint16_t cursor_pos_top;
    uint16_t status_line;
    uint16_t last_char_col;
};
}  // namespace xtdraw

#endif  // XTDRAW_LAYOUT_H
