#include <unistd.h>

#include <functional>
#include <map>
#include <sstream>

#include "xtdraw_app.h"

namespace xtdraw {
App::App() : terminal_io(std::cout), board({0, 0, 80, 16}, 128, 64) {}
bool App::Init() {
    if (!terminal_io.EnableRawMode()) {
        std::cerr << "Failed to enable raw mode\n";
        return false;
    }
    terminal_io.HideCursor();
    terminal_io.ClearScreen();

    return true;
}

bool App::Run() {
    static uint32_t draw_frame = 0;
    while (app_running) {
        draw_frame++;
        if (draw_frame == 32) {
            draw_frame = 0;
        }

        RedrawBoard(draw_frame);
        RedrawCursorInfo();

        terminal_io.SetColor(uint8_t{0}, uint8_t{255});
        terminal_io.SetCursorPosition(90, 5);

        ProcessInput(terminal_io.ReadKey());
        usleep(10000);
    }
    return true;
}

void App::ProcessInput(uint32_t keycode) {
    static const std::map<std::string, uint32_t> kInsertChar{
        {"0", ' '},    {"1", 0x1fb00}, {"2", 0x1fb01}, {"3", 0x1fb02}, {"4", 0x1fb03}, {"5", 0x1fb04},
        {"!", 0x256d}, {"@", 0x252c},  {"#", 0x256e},  {"$", 0x251c},  {"%", 0x253c},  {"^", 0x2524},
        {"&", 0x2570}, {"*", 0x2534},  {"(", 0x256f},
    };
    // Handle special keys and printable characters
    if (!keycode) {
        return;
    }

    auto move_cursor = [&](int dx, int dy) {
        board.MoveCursor(dx, dy);
        redraw_board       = true;
        redraw_cursor_info = true;
    };
    auto change_cursor_mode = [&](Board::CursorMode mode) {
        board.SetCursorMode(mode);
        redraw_board       = true;
        redraw_cursor_info = true;
    };

    const std::string seq_str = KeyCodeToString(keycode);
    if (seq_str == "q" || seq_str == "Q") {
        // Exit the application
        OnTerminationSignal();
        exit(0);
    } else if (seq_str == "<Up>" || seq_str == "k") {
        move_cursor(0, -1);
    } else if (seq_str == "<Down>" || seq_str == "j") {
        move_cursor(0, 1);
    } else if (seq_str == "<Left>" || seq_str == "h") {
        move_cursor(-1, 0);
    } else if (seq_str == "<Right>" || seq_str == "l") {
        move_cursor(1, 0);
    } else if (seq_str == "[") {
        if (active_set_ix > 0) active_set_ix--;
    } else if (seq_str == "]") {
        active_set_ix++;
    } else if (seq_str == "H") {
        picker_char_ix = (picker_char_ix - 1) & 0xFF;
    } else if (seq_str == "L") {
        picker_char_ix = (picker_char_ix + 1) & 0xFF;
    } else if (seq_str == "J") {
        picker_char_ix = (picker_char_ix + 32) & 0xFF;
    } else if (seq_str == "K") {
        picker_char_ix = (picker_char_ix - 32) & 0xFF;
    } else if (seq_str == "<F1>") {
        change_cursor_mode(Board::CursorMode::ENTIRE_CHARACTER);
    } else if (seq_str == "<F2>") {
        change_cursor_mode(Board::CursorMode::BLK_2x2);
    } else if (seq_str == "<F3>") {
        change_cursor_mode(Board::CursorMode::BLK_2x3);
    } else if (seq_str == " ") {
        board.TogglePixel();
        redraw_board = true;
    } else {
        auto it = kInsertChar.find(seq_str);
        if (it != kInsertChar.end()) {
            board.SetCell(it->second);
            redraw_board = true;
        }
    }
}

void App::RedrawBoard(uint32_t draw_frame) {
    if (redraw_board) {
        board.Render(terminal_io);
    }
    RedrawCharacterPicker();

    if (redraw_board || draw_frame % 16 == 0) {
        board.RenderCursor(terminal_io, draw_frame > 0);
    }
    if (redraw_board) {
        redraw_board = false;
    }

}

void App::RedrawCursorInfo() {
    if (redraw_cursor_info) {
        uint16_t cx, cy;
        uint8_t  csx, csy;
        board.GetCursorPosition(cx, cy, csx, csy);

        terminal_io.SetCursorPosition(0, 18);
        terminal_io.SetColor(uint8_t{4}, uint8_t{15});
        if (board.GetCursorMode() == Board::CursorMode::ENTIRE_CHARACTER) {
            terminal_io.Write(std::to_string(cx) + ", " + std::to_string(cy));
        } else {
            terminal_io.Write(std::to_string(cx) + "." + std::to_string(csx) + ", " + std::to_string(cy) + "." +
                              std::to_string(csy));
            if (board.IsTogglingAvailable()) {
                terminal_io.SetColor({}, uint8_t{11});
            } else {
                terminal_io.SetColor({}, uint8_t{9});
            }
            terminal_io.Write(" T");
        }
        std::ostringstream os;
        os << " " << std::hex << board.GetCellUnderCursor().character << "   ";
        terminal_io.SetColor(uint8_t{4}, uint8_t{15});
        terminal_io.Write(os.str());
        redraw_cursor_info = false;
    }
}

void App::GetCharacterSubset(std::vector<uint32_t> &chars, bool &double_width, size_t set_index) {
    chars.resize(256);
    if (set_index == 0) {
        size_t ins_ix = 0;
        for (size_t ix = 32; ix < 127; ix++) {
            chars[ins_ix++] = ix;
        }
        for (size_t ix = 161; ix < 173; ix++) {
            chars[ins_ix++] = ix;
        }
        for (size_t ix = 174; ix < 256; ix++) {
            chars[ins_ix++] = ix;
        }
    } else {
        for (size_t ix = 0; ix < 256; ix++) {
            chars[ix] = set_index * 256 + ix;
        }
    }
    double_width = false;
}

void App::RedrawCharacterPicker() {
    std::vector<uint32_t> active_set;
    bool double_width;
    GetCharacterSubset(active_set, double_width, active_set_ix);
    for (size_t y = 0; y < 8; y++) {
        terminal_io.SetCursorPosition(83, y + 1);
        for (size_t x = 0; x < 32; x++) {
            if (y * 32 + x == picker_char_ix) {
                terminal_io.SetColor(uint8_t{2}, uint8_t{15});
            } else {
                terminal_io.SetColor(uint8_t{4}, uint8_t{7});
            }
            uint32_t ix = y * 32 + x;
            if (ix < active_set.size()) {
                uint32_t ch = active_set[ix];
                if (ch >= ' ') {
                    terminal_io.Write(ch);
                } else {
                    terminal_io.Write(' ');
                }
            }
            
        }
    }
}

void App::OnTerminationSignal() {
    terminal_io.ShowCursor();
    terminal_io.RestoreTerminal();
    app_running = false;
}

void App::OnResizeSignal() {}

}  // namespace xtdraw
