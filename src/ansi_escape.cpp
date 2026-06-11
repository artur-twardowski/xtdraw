#include "ansi_escape.h"

static constexpr const char ESC = '\033';

void AnsiEscape::ClearScreen() {
    // Clear entire screen and move cursor to home (0,0)
    out_stream << ESC << "[2J" << ESC << "[H" << std::flush;
}

void AnsiEscape::SetCursorPosition(int row, int col) {
    out_stream << ESC << row << ";" << col << "H" << std::flush;
}

void AnsiEscape::ShowCursor() {
    out_stream << ESC << "[?25h" << std::flush;
}

void AnsiEscape::HideCursor() {
    out_stream << ESC << "[?25l" << std::flush;
}
