#include <unistd.h>

#include <functional>
#include <iomanip>
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

        ProcessInput(terminal_io.ReadKey());
        usleep(10000);
    }
    return true;
}

void App::ProcessInput(uint32_t keycode) {
    static const std::map<std::string, uint32_t> kInsertChar{
        {"0", ' '},     {"1", 0x1fb00}, {"2", 0x1fb01}, {"3", 0x1fb02}, {"4", 0x1fb03},
        {"5", 0x1fb04}, {"!", 0x256d},  {"@", 0x252c},  {"#", 0x256e},  {"$", 0x251c},
        {"%", 0x253c},  {"^", 0x2524},  {"&", 0x2570},  {"*", 0x2534},  {"(", 0x256f},
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
        if (board.GetCursorMode() == Board::CursorMode::ENTIRE_CHARACTER) {
            uint32_t c = board.GetCellUnderCursor().character;
            if (c == ' ') {
                board.SetCell('X');
            } else {
                board.SetCell(' ');
            }
        } else {
            board.TogglePixel();
        }
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

    RedrawCharacterPicker(board.GetCursorMode() == Board::CursorMode::ENTIRE_CHARACTER);

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

        terminal_io.SetCursorPosition(83, 12);
        SetColor(terminal_io, colors.cursor_info);
        const auto cursor_mode = board.GetCursorMode();
        if (cursor_mode == Board::CursorMode::ENTIRE_CHARACTER) {
            std::ostringstream line;
            line << std::setw(4) << cx << "  |";
            line << std::setw(4) << cy << "  |";
            terminal_io.Write(line.str(), 14);
        } else {
            std::ostringstream line;
            line << std::setw(4) << cx << "." << int(csx) << "|";
            line << std::setw(4) << cy << "." << int(csy) << "|";
            terminal_io.Write(line.str(), 14);
        }
        if (cursor_mode == Board::CursorMode::ENTIRE_CHARACTER) {
            SetColor(terminal_io, colors.cursor_mode);
            terminal_io.Write(" CHAR");
        } else {
            SetColor(terminal_io, board.IsTogglingAvailable() ? colors.cursor_mode : colors.cursor_mode_unavail);
            switch (cursor_mode) {
                case Board::CursorMode::BLK_2x2:
                    terminal_io.Write(" 2x2 ");
                    break;
                case Board::CursorMode::BLK_2x3:
                    terminal_io.Write(" 2x3 ");
                    break;
                default:
                    break;
            }
        }
        uint32_t           ch = board.GetCellUnderCursor().character;
        std::ostringstream os;
        os << "| " << std::setw(8) << std::hex << ch << "  ";
        SetColor(terminal_io, colors.cursor_info);
        terminal_io.Write(os.str());
        terminal_io.Write(ch);
        redraw_cursor_info = false;
    }
}

void App::GetCharacterSubset(std::vector<uint32_t> &chars, bool &double_width, std::string &subset_name,
                             size_t set_index) {
    struct SetDescriptor {
        std::string                                name;
        bool                                       double_width{false};
        std::vector<std::pair<uint32_t, uint32_t>> subsets;
    };
    const std::vector<SetDescriptor> kSets = {
        {.name = "ASCII and Latin-1", .subsets = {{32, 126}, {161, 0xAC}, {0xAE, 255}}},
        {.name = "Greek", .subsets = {{0x370, 0x3ff}}},
        {.name = "Cyrillic", .subsets = {{0x400, 0x4ff}}},
        {.name = "Braille", .subsets = {{0x2800, 0x28ff}}},
    };
    chars.resize(256);
    const auto &set    = set_index < kSets.size() ? kSets[set_index] : kSets[0];
    size_t      ins_ix = 0;
    for (const auto &[subset_lo, subset_hi] : set.subsets) {
        for (size_t ix = subset_lo; ix <= subset_hi; ix++) {
            chars[ins_ix++] = ix;
        }
    }
    double_width = set.double_width;
    subset_name  = set.name;
}

void App::RedrawCharacterPicker(bool active) {
    std::vector<uint32_t> active_set;
    bool                  double_width;
    std::string           subset_name;
    GetCharacterSubset(active_set, double_width, subset_name, active_set_ix);
    terminal_io.SetCursorPosition(83, 0);
    SetColor(terminal_io, colors.char_picker_header);
    terminal_io.Write(subset_name, 32);
    for (size_t y = 0; y < 8; y++) {
        terminal_io.SetCursorPosition(83, y + 1);
        for (size_t x = 0; x < 32; x++) {
            if (y * 32 + x == picker_char_ix) {
                SetColor(terminal_io, active ? colors.char_picker_cursor : colors.char_picker_inactive_cursor);
            } else {
                SetColor(terminal_io, active ? colors.char_picker_normal : colors.char_picker_inactive);
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
    terminal_io.SetCursorPosition(83 + 22, 9);
    SetColor(terminal_io, colors.char_picker_info);
    std::ostringstream line;
    line << std::hex << std::setw(8) << active_set[picker_char_ix];
    terminal_io.Write(line.str(), 10);
}

void App::OnTerminationSignal() {
    terminal_io.ShowCursor();
    terminal_io.RestoreTerminal();
    app_running = false;
}

void App::OnResizeSignal() {}

}  // namespace xtdraw
