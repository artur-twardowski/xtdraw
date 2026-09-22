#include <unistd.h>

#include <cstring>
#include <iomanip>
#include <map>
#include <sstream>
#include <variant>

#include "core/terminal_io.h"
#include "core/utils.h"

namespace xtdraw {

static constexpr const char ESC = '\033';
TerminalIO::TerminalIO(std::ostream &os) : xtdraw::ANSIOutput(os), out_stream(os) {}

bool TerminalIO::EnableRawMode() {
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
        perror("tcgetattr");
        return false;
    }

    termios raw = original_termios;

    // Disable canonical mode and echo
    raw.c_lflag &= ~(ICANON | ECHO);

    // Set minimum characters to read and timeout
    raw.c_cc[VMIN]  = 0;  // Non-blocking read
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

uint32_t TerminalIO::ReadUTF8(uint8_t first) {
    size_t   count = 0;
    uint8_t  buf[3];
    uint32_t result;
    if ((first & 0b11100000) == 0b11000000) {
        result = (first & 0b00011111);
        count  = 1;
    } else if ((first & 0b11110000) == 0b11100000) {
        result = (first & 0b00001111);
        count  = 2;
    } else if ((first & 0b11111000) == 0b11110000) {
        result = (first & 0b00000111);
        count  = 3;
    }

    if (read(STDIN_FILENO, buf, count) > 0) {
        for (size_t ix = 0; ix < count; ix++) {
            if ((buf[ix] & 0b11000000) == 0b10000000) {
                result = (result << 6) | (buf[ix] & 0b00111111);
            } else {
                return 0xFFFFFFFE;
            }
        }
        return result;
    } else {
        return 0xFFFFFFFF;
    }
}

uint32_t TerminalIO::ReadKey() {
    uint32_t result = 0;
    uint8_t  ch;
    ssize_t  bytes_read = read(STDIN_FILENO, &ch, 1);
    static const uint32_t CSI_SEQUENCE = (0b01 << 30);
    static const uint32_t SS_SEQUENCE = (0b10<< 30);
    static const uint32_t SS3_SEQUENCE = 0x80;
    static const uint32_t OTHER_SEQUENCE = (0b11 << 30);
    static const uint32_t OVERFLOW_SEQUENCE = OTHER_SEQUENCE | (0b11 << 28);
    static const uint32_t TERMINAL_SIZE = OTHER_SEQUENCE | (0b01 << 28);

    // Interpretation of result based on most significant bits:
    // 00...: Unicode character
    // 01...: CSI sequence (most of special keys on the keyboard)
    // 10...: SS2/SS3 sequences (some special keys):
    //     100000ee eeeddddd cccccbbb bbaaaaaa - up to 5 bytes from the sequence
    // 11...: other:
    //     1111...: overflow sequence
    //     1101rrrr rrrrrrrr cccccccc cccccccc - current cursor position/terminal dimensions:
    //                                           c - number of columns (0..65535)
    //                                           r - number of rows (0..4095)

    if (bytes_read == -1) {
        perror("read");
        return 0;
    }

    if (bytes_read == 0) {
        return 0;
    }

    // Handle escape sequences (arrow keys, function keys, etc.)
    if (ch == ESC) {
        unsigned char next_ch;

        // Read the next character to determine sequence type
        if (read(STDIN_FILENO, &next_ch, 1) > 0) {
            // Standard CSI (Control Sequence Introducer) sequences: ESC [
            if (next_ch == '[') {
                result |= CSI_SEQUENCE;
                unsigned char seq_ch;

                // Read parameter bytes and intermediate bytes (0x30-0x3F and 0x20-0x2F)
                size_t bitshift = 0;
                std::string complete_sequence;
                while (read(STDIN_FILENO, &seq_ch, 1) > 0) {
                    complete_sequence += seq_ch;
                    // Check if this is a final byte (0x40-0x7E)
                    // Common final bytes: A-Z (cursor movement), ~ (function keys), m
                    // (color), etc.
                    if (seq_ch >= 0x20 && seq_ch <= 0x3F) {
                        uint8_t part =
                            seq_ch - 0x20;  // values from 0x00 to 0x1F -> 5 bits needed
                        if (bitshift <= 24) {
                            result |= (part << bitshift);
                            bitshift += 5;
                        } else {
                            result = OVERFLOW_SEQUENCE;
                        }
                    } else if (seq_ch >= 0x40 && seq_ch <= 0x7E) {
                        uint8_t part =
                            seq_ch - 0x40;  // values from 0x00 to 0x3F -> 6 bits needed
                        if (bitshift <= 24) {
                            result |= (part << bitshift);
                            bitshift += 6;
                        } else {
                            result = OVERFLOW_SEQUENCE;
                        }
                        break;
                    }
                }

                if (complete_sequence.ends_with('R')) {
                    try {
                        ssize_t pos1 = complete_sequence.find(';');
                        ssize_t pos2 = complete_sequence.size() - 1;
                        uint16_t rows = std::stoi(complete_sequence.substr(0, pos1));
                        uint16_t cols = std::stoi(complete_sequence.substr(pos1+1,pos2));
                        result = TERMINAL_SIZE | ((rows & 0xfff) << 16) | (cols & 0xffff);
                    } catch (...) { /* Leave the original value of result */ }
                }
            }
            // SS2 sequences: ESC N (single shift, rarely used)
            else if (next_ch == 'N') {
                unsigned char seq_ch;
                if (read(STDIN_FILENO, &seq_ch, 1) > 0) {
                    result = SS_SEQUENCE | (seq_ch);
                }
            }
            // SS3 sequences: ESC O (used by some terminals for function keys)
            else if (next_ch == 'O') {
                unsigned char seq_ch;
                if (read(STDIN_FILENO, &seq_ch, 1) > 0) {
                    result = SS_SEQUENCE | SS3_SEQUENCE | (seq_ch);
                }
            } else {
                result = OTHER_SEQUENCE | 0x100 | next_ch;
            }
        }
    } else if (ch > 0x7F) {
        result = ReadUTF8(ch);
    } else {
        result = ch;
    }

    return result;
}

void TerminalIO::ClearScreen() {
    // Clear entire screen and move cursor to home (0,0)
    out_stream << ESC << "[2J" << ESC << "[H" << std::flush;
}

void TerminalIO::SetCursorPosition(int col, int row) {
    out_stream << ESC << "[" << (row + 1) << ";" << (col + 1) << "H" << std::flush;
}

void TerminalIO::ShowCursor() { out_stream << ESC << "[?25h" << std::flush; }

void TerminalIO::HideCursor() { out_stream << ESC << "[?25l" << std::flush; }
void TerminalIO::RequestCursorPosition() {
    out_stream << ESC << "[6n" << std::flush;
}
void TerminalIO::RequestTerminalDimensions() {
    out_stream << ESC << "[9999;9999f" << ESC << "[6n" << std::flush;
}

void TerminalIO::WriteWindowed(const std::string &data, size_t window_size) {
    if (data.size() < window_size) {
        Write(data);
        for (size_t ix = data.size(); ix < window_size; ix++) {
            Write(' ');
        }
    } else {
        Write(data.substr(0, window_size));
    }
}
void TerminalIO::Flush() { out_stream.flush(); }

TerminalIO::~TerminalIO() { RestoreTerminal(); }

static const std::map<uint32_t, std::string> SPECIAL_KEYCODES{
    {0x01, "C-a"},
    {0x02, "C-b"},
    {0x03, "C-c"},
    {0x04, "C-d"},
    {0x05, "C-e"},
    {0x06, "C-f"},
    {0x07, "C-g"},
    {0x08, "C-h"},
    {0x09, "C-i"},
    {0x0A, "C-j"},
    {0x0B, "C-k"},
    {0x0C, "C-l"},
    {'<', "LT"},
    {'>', "GT"},
    {0x800000d0, "F1"},
    {0x800000d1, "F2"},
    {0x800000d2, "F3"},
    {0x800000d3, "F4"},
    {0x4000fab1, "F5"},
    {0x4000faf1, "F6"},
    {0x4000fb11, "F7"},
    {0x4000fb31, "F8"},
    {0x4000fa12, "F9"},
    {0x4000fa32, "F10"},
    {0x4000fa72, "F11"},
    {0x4000fa92, "F12"},
    {0x40085771, "C-F1"},
    {0x4008d771, "C-F2"},
    {0x40084b71, "S-F1"},
    {0x40084f71, "M-F1"},
    {0x40085b71, "C-S-F1"},
    {0x40000001, "Up"},
    {0x40000002, "Down"},
    {0x40000003, "Right"},
    {0x40000004, "Left"},
    {0x400007d2, "Ins"},
    {0x400007d3, "Del"},
};

std::string KeyCodeToString(uint32_t keycode, char special_delim_left,
                            char special_delim_right) {
    auto it = SPECIAL_KEYCODES.find(keycode);
    if (it != SPECIAL_KEYCODES.end()) {
        return special_delim_left + it->second + special_delim_right;
    } else if (keycode <= 0x1F) {
        return std::string("") + special_delim_left + "x" + char(keycode + 0x20) +
               special_delim_right;
    }
    if (keycode <= 0x0010FFFF) {
        return xtdraw::EncodeUTF8(keycode);
    } else {
        std::ostringstream os;
        os << special_delim_left << std::hex << std::setw(8) << std::setfill('0')
           << keycode << special_delim_right;
        return os.str();
    }
}

}  // namespace xtdraw
