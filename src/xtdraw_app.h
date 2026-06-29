#ifndef XTDRAW_APP_H
#define XTDRAW_APP_H

#include "board.h"
#include "terminal_io.h"

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
    void       GetCharacterSubset(std::vector<uint32_t> &chars, bool &double_width, std::string &subset_name,
                                  size_t set_index);
    TerminalIO terminal_io;
    Board      board;

    bool redraw_board{true};
    bool redraw_cursor_info{true};
    bool app_running{true};

    uint16_t picker_char_ix{0};
    uint16_t active_set_ix{0};
};
}  // namespace xtdraw

#endif
