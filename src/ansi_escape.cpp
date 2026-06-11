#include "ansi_escape.h"

void AnsiEscape::write_escape_sequence(const char* sequence) {
    std::cout << sequence << std::flush;
}

void AnsiEscape::clear_screen() {
    // Clear entire screen and move cursor to home (0,0)
    write_escape_sequence(ESC "[2J" ESC "[H");
}

void AnsiEscape::set_cursor_position(int row, int col) {
    std::cout << ESC "[" << row << ";" << col << "H" << std::flush;
}

void AnsiEscape::show_cursor() {
    write_escape_sequence(ESC "[?25h");
}

void AnsiEscape::hide_cursor() {
    write_escape_sequence(ESC "[?25l");
}
