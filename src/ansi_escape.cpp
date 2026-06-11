#include "ansi_escape.h"

void AnsiEscape::write_escape_sequence(const char* sequence) {
    std::cout << sequence << std::flush;
}

void AnsiEscape::clear_screen() {
    // Clear entire screen and move cursor to home (0,0)
    write_escape_sequence("\033[2J\033[H");
}

void AnsiEscape::set_cursor_position(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H" << std::flush;
}

void AnsiEscape::show_cursor() {
    write_escape_sequence("\033[?25h");
}

void AnsiEscape::hide_cursor() {
    write_escape_sequence("\033[?25l");
}
