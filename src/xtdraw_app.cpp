#include <unistd.h>

#include <functional>
#include <iomanip>
#include <map>
#include <sstream>
#include <variant>

#include "colors.h"
#include "xtdraw_app.h"
#include "charset.h"

namespace xtdraw {
App::App() : 
    terminal_io(std::cout),
    board({0, 0, 80, 16}, 128, 64)
{
    for (uint8_t ix = 0; ix < quick_select_chars.size(); ix++) {
        quick_select_chars[ix] = {0, static_cast<uint16_t>('A' + ix)};
    }
}
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
        RedrawColorPicker();

        SetColor(terminal_io, colors.cursor_info);
        terminal_io.SetCursorPosition(83, 19);
        terminal_io.Write(last_char, 20);

        ProcessInput(terminal_io.ReadKey());
        usleep(10000);
    }
    return true;
}

void App::ProcessInput(uint32_t keycode) {
    static const std::map<std::string, size_t> kQuickSelectKeys = {
        {"1", 0}, {"2", 1}, {"3", 2}, {"4", 3}, {"5", 4},
        {"6", 5}, {"7", 6}, {"8", 7}, {"9", 8}, {"0", 9},
        {"!", 10}, {"@", 11}, {"#", 12}, {"$", 13}, {"%", 14},
        {"^", 15}, {"&", 16}, {"*", 17}, {"(", 18}, {")", 19},
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
    auto toggle_character_or_pixel = [&]() {
        if (board.GetCursorMode() == Board::CursorMode::ENTIRE_CHARACTER) {
            uint32_t current_char = board.GetCellUnderCursor().character;
            uint32_t selected_char = GetActiveCharacter();
            if (current_char != selected_char) {
                board.SetCell(selected_char);
            } else {
                board.SetCell(' ');
            }
        } else {
            board.TogglePixel();
        }
        redraw_board = true;
    };
    auto change_color = [&](TerminalIO::Color &c, int8_t delta) {
        if (std::holds_alternative<uint8_t>(c)) {
            auto &cv = std::get<uint8_t>(c);
            if (delta > 0) {
                cv++;
            } else {
                cv--;
            }
        }
        redraw_color_picker = true;
    };

    const std::string seq_str = KeyCodeToString(keycode);
    const bool dbl_width = IsCharacterSetDoubleWidth(active_set_ix);
    size_t picker_vertical_step = dbl_width ? 16 : 32;
    uint8_t picker_mask = dbl_width ? 0x7f : 0xff;
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
    } else if (seq_str == "{") {
        if (active_set_ix > 0) active_set_ix--;
    } else if (seq_str == "}") {
        active_set_ix++;
    } else if (seq_str == ";") {
        change_color(active_color.foreground, -1);
    } else if (seq_str == ":") {
        change_color(active_color.background, -1);
    } else if (seq_str == "'") {
        change_color(active_color.foreground, 1);
    } else if (seq_str == "\"") {
        change_color(active_color.background, 1);
    } else if (seq_str == "H") {
        picker_char_ix = (picker_char_ix - 1) & picker_mask;
    } else if (seq_str == "L") {
        picker_char_ix = (picker_char_ix + 1) & picker_mask;
    } else if (seq_str == "J") {
        picker_char_ix = (picker_char_ix + picker_vertical_step) & picker_mask;
    } else if (seq_str == "K") {
        picker_char_ix = (picker_char_ix - picker_vertical_step) & picker_mask;
    } else if (seq_str == "<F1>") {
        change_cursor_mode(Board::CursorMode::ENTIRE_CHARACTER);
    } else if (seq_str == "<F2>") {
        change_cursor_mode(Board::CursorMode::BLK_2x2);
    } else if (seq_str == "<F3>") {
        change_cursor_mode(Board::CursorMode::BLK_2x3);
    } else if (seq_str == "<F4>") {
        change_cursor_mode(Board::CursorMode::BLK_2x4);
    } else if (seq_str == " ") {
        toggle_character_or_pixel();
        board.SetCellColor(active_color);
    } else if (kQuickSelectKeys.find(seq_str) != kQuickSelectKeys.end()) {
        auto qsc = quick_select_chars[kQuickSelectKeys.at(seq_str)];
        if (board.GetCursorMode() == Board::CursorMode::ENTIRE_CHARACTER) {
            board.SetCell(GetCharFromSubset(qsc.set_index, qsc.char_index));
        }
        
    }
    last_char = seq_str;
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

        terminal_io.SetCursorPosition(83, 15);
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
                case Board::CursorMode::BLK_2x4:
                    terminal_io.Write(" 2x4 ");
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

void App::RedrawColorPicker() {
    uint16_t top_row           = 16;
    auto     is_selected_color = [](const TerminalIO::Color &color, uint8_t selection) {
        if (std::holds_alternative<uint8_t>(color)) {
            return selection == std::get<uint8_t>(color);
        }
        return false;
    };
    auto is_selected_color_component = [](const TerminalIO::Color &color, char component,
                                          uint8_t component_value) {
        if (std::holds_alternative<uint8_t>(color)) {
            uint8_t c = std::get<uint8_t>(color);
            if (c >= 16 && c < 232) {
                c -= 16;
                uint8_t r = c % 6;
                uint8_t g = (c / 6) % 6;
                uint8_t b = (c / 36) % 6;

                return (component == 'R' && r == component_value) ||
                       (component == 'G' && g == component_value) ||
                       (component == 'B' && b == component_value);
            } else {
                return false;
            }
        }
        return false;
    };
    auto print_mark = [&](uint8_t selection) -> std::string {
        bool selected_bg = is_selected_color(active_color.background, selection);
        bool selected_fg = is_selected_color(active_color.foreground, selection);
        if (selected_bg) {
            return selected_fg ? "X" : "B";
        } else {
            return selected_fg ? "F" : " ";
        }
    };
    if (redraw_color_picker) {
        // ROW 1
        terminal_io.SetCursorPosition(0, top_row);
        for (uint8_t c = 0; c < 16; c++) {
            SetColor(terminal_io, {uint8_t(c), uint8_t{15}});
            terminal_io.Write(" " + print_mark(c) + " ");
        }
        SetColor(terminal_io, {uint8_t{0}, uint8_t{0}});
        terminal_io.Write("    ");
        SetColor(terminal_io, active_color);
        terminal_io.Write("    ");

        terminal_io.SetCursorPosition(0, top_row + 1);
        for (uint8_t c = 0; c < 24; c++) {
            uint8_t ccode = Xterm256Gray(c);
            SetColor(terminal_io, {ccode, c < 16 ? Xterm256Gray(23) : Xterm256Gray(0)});
            terminal_io.Write(print_mark(ccode) + " ");
        }
        SetColor(terminal_io, {uint8_t{0}, uint8_t{0}});
        terminal_io.Write("    ");
        SetColor(terminal_io, active_color);
        terminal_io.Write("-\u2588\u2588-");
        terminal_io.SetCursorPosition(0, top_row + 2);

        for (const char &component : {'R', 'G', 'B'}) {
            SetColor(terminal_io, {uint8_t{0}, Xterm256RGB(component == 'R' ? 5 : 0,
                                                           component == 'G' ? 5 : 0,
                                                           component == 'B' ? 5 : 0)});
            terminal_io.Write(component);
            terminal_io.Write('[');
            for (uint8_t intensity = 0; intensity < 6; intensity++) {
                TerminalIO::Color ccode = Xterm256RGB(component == 'R' ? intensity : 0,
                                                      component == 'G' ? intensity : 0,
                                                      component == 'B' ? intensity : 0);
                SetColor(terminal_io, {ccode, uint8_t(255)});
                bool selected_bg = is_selected_color_component(active_color.background,
                                                               component, intensity);
                bool selected_fg = is_selected_color_component(active_color.foreground,
                                                               component, intensity);
                if (selected_bg) {
                    terminal_io.Write(selected_fg ? "X " : "B ");
                } else {
                    terminal_io.Write(selected_fg ? "F " : "  ");
                }
            }
            SetColor(terminal_io, {uint8_t{0}, Xterm256RGB(component == 'R' ? 5 : 0,
                                                           component == 'G' ? 5 : 0,
                                                           component == 'B' ? 5 : 0)});
            terminal_io.Write("]  ");
        }
        SetColor(terminal_io, {uint8_t{0}, uint8_t{0}});
        terminal_io.Write(" ");
        SetColor(terminal_io, active_color);
        terminal_io.Write("    ");
        terminal_io.SetCursorPosition(0, top_row + 2);

        redraw_color_picker = false;
    }
}

void App::RedrawCharacterPicker(bool active) {
    std::vector<uint32_t> active_set;
    bool                  double_width;
    std::string           subset_name;
    GetCharacterSubset(active_set, double_width, subset_name, active_set_ix);
    terminal_io.SetCursorPosition(83, 0);
    SetColor(terminal_io, colors.char_picker_header);
    terminal_io.Write(subset_name, 32);
    const size_t chars_in_row = double_width ? 16 : 32;
    for (size_t y = 0; y < 8; y++) {
        terminal_io.SetCursorPosition(83, y + 1);
        for (size_t x = 0; x < chars_in_row; x++) {
            if (y * chars_in_row + x == picker_char_ix) {
                SetColor(terminal_io, active ? colors.char_picker_cursor
                                             : colors.char_picker_inactive_cursor);
            } else {
                SetColor(terminal_io, active ? colors.char_picker_normal
                                             : colors.char_picker_inactive);
            }
            uint32_t ix = y * chars_in_row + x;
            if (ix < active_set.size()) {
                uint32_t ch = active_set[ix];
                if (ch >= ' ') {
                    terminal_io.Write(ch);
                    if (double_width) {
                        terminal_io.Write(' ');
                    }
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

    static const char kKeys[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};

    for (uint8_t row = 0; row < 3; row++) {
        terminal_io.SetCursorPosition(84, 11 + row);
        for (uint8_t col = 0; col < 10; col++) {
            terminal_io.Write(' ');
            if (row == 2) {
                terminal_io.Write(kKeys[col]);
            } else {
                std::vector<uint32_t> chars;
                const auto           &qsc = quick_select_chars[(row == 0 ? 10 : 0) + col];
                GetCharacterSubset(chars, qsc.set_index);
                terminal_io.Write(chars[qsc.char_index]);
            }
            terminal_io.Write(' ');
        }
    }
}

void App::OnTerminationSignal() {
    terminal_io.ShowCursor();
    terminal_io.RestoreTerminal();
    app_running = false;
}

void App::OnResizeSignal() {}
uint32_t App::GetActiveCharacter() const {
    std::vector<uint32_t> active_set;
    bool _b;
    std::string _s;
    GetCharacterSubset(active_set, _b, _s, active_set_ix);
    return active_set[picker_char_ix];
}

}  // namespace xtdraw
