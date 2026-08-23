#ifndef XTDRAW_APP_H
#define XTDRAW_APP_H

#include "board.h"
#include "colors.h"
#include "terminal_io.h"
#include <array>

namespace xtdraw {
class App {
   public:
    App();
    bool Init();
    bool Run();
    void OnTerminationSignal();
    void OnResizeSignal();

   private:
    void       ProcessInput(uint32_t keycode);
    void       RedrawBoard(uint32_t draw_frame);
    void       RedrawCursorInfo();
    void       RedrawCharacterPicker(bool active);
    uint32_t   GetActiveCharacter() const;
    void       RedrawColorPicker();
    Colors     colors;
    TerminalIO terminal_io;
    Board      board;

    bool redraw_board{true};
    bool redraw_cursor_info{true};
    bool redraw_color_picker{true};
    bool app_running{true};

    struct QuickSelectCharacter {
        uint16_t set_index{0};
        uint16_t char_index{0};
    };

    uint16_t picker_char_ix{0};
    uint16_t active_set_ix{0};
    ColorPair active_color{uint8_t{0}, uint8_t{255}};
    std::string last_char;

    std::array<QuickSelectCharacter, 20> quick_select_chars;
};
}  // namespace xtdraw

#endif
