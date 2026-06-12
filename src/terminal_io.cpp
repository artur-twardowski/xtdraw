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
        unsigned char next_ch;
        
        // Read the next character to determine sequence type
        if (read(STDIN_FILENO, &next_ch, 1) > 0) {
            sequence += next_ch;
            
            // Standard CSI (Control Sequence Introducer) sequences: ESC [
            if (next_ch == '[') {
                unsigned char seq_ch;
                
                // Read parameter bytes and intermediate bytes (0x30-0x3F and 0x20-0x2F)
                while (read(STDIN_FILENO, &seq_ch, 1) > 0) {
                    sequence += seq_ch;
                    
                    // Check if this is a final byte (0x40-0x7E)
                    // Common final bytes: A-Z (cursor movement), ~ (function keys), m (color), etc.
                    if (seq_ch >= 0x40 && seq_ch <= 0x7E) {
                        break;
                    }
                    
                    // Limit sequence length to prevent infinite loops
                    if (sequence.length() > 16) {
                        break;
                    }
                }
            }
            // SS2 sequences: ESC N (single shift, rarely used)
            else if (next_ch == 'N') {
                unsigned char seq_ch;
                if (read(STDIN_FILENO, &seq_ch, 1) > 0) {
                    sequence += seq_ch;
                }
            }
            // SS3 sequences: ESC O (used by some terminals for function keys)
            else if (next_ch == 'O') {
                unsigned char seq_ch;
                if (read(STDIN_FILENO, &seq_ch, 1) > 0) {
                    sequence += seq_ch;
                }
            }
            // Other escape sequences (ESC followed by single character)
            // These are complete as-is
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
