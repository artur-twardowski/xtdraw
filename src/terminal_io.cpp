#include "terminal_io.h"
#include <unistd.h>
#include <cstring>

static constexpr const char ESC = '\033';

bool TerminalIO::EnableRawMode() {
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
        perror("tcgetattr");
        return false;
    }

    termios raw = original_termios;

    // Disable canonical mode and echo
    raw.c_lflag &= ~(ICANON | ECHO);

    // Set minimum characters to read and timeout
    raw.c_cc[VMIN] = 0;   // Non-blocking read
    raw.c_cc[VTIME] = 0;  // No timeout

    // Disable output processing
    raw.c_oflag &= ~(OPOST);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        return false;
    }

    raw_mode_enabled = true;
    return true;
}

void TerminalIO::RestoreTerminal() {
    if (raw_mode_enabled) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
        raw_mode_enabled = false;
    }
}

std::string TerminalIO::ReadKeySequence() {
    unsigned char ch;
    ssize_t bytes_read = read(STDIN_FILENO, &ch, 1);

    if (bytes_read == -1) {
        perror("read");
        return "";
    }

    if (bytes_read == 0) {
        return ""; // No input available
    }

    std::string sequence;
    sequence += ch;

    // Handle escape sequences (arrow keys, function keys, etc.)
    if (ch == ESC) {
        unsigned char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) > 0) {
            sequence += seq[0];
            if (seq[0] == '[') {
                if (read(STDIN_FILENO, &seq[1], 1) > 0) {
                    sequence += seq[1];
                }
            }
        }
    }

    return sequence;
}

void TerminalIO::ClearScreen() {
    // Clear entire screen and move cursor to home (0,0)
    out_stream << ESC << "[2J" << ESC << "[H" << std::flush;
}

void TerminalIO::SetCursorPosition(int row, int col) {
    out_stream << ESC << "[" << row << ";" << col << "H" << std::flush;
}

void TerminalIO::ShowCursor() {
    out_stream << ESC << "[?25h" << std::flush;
}

void TerminalIO::HideCursor() {
    out_stream << ESC << "[?25l" << std::flush;
}

TerminalIO::~TerminalIO() {
    RestoreTerminal();
}
