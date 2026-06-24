#include "terminal_io.h"
#include <unistd.h>
#include <cstring>
#include <map>
#include <sstream>
#include <iomanip>
#include <variant>

static constexpr const char ESC = '\033';

static constexpr uint8_t ExtractBits(uint32_t in, uint8_t lsb, uint8_t count) {
    const uint32_t mask = ((1 << count) - 1) << lsb;
    return (in & mask) >> lsb;
}

bool operator==(const TerminalIO::color_t &c1, const TerminalIO::color_t &c2) {
    using rgb_t = TerminalIO::rgb_t;
    if (std::holds_alternative<uint8_t>(c1) && std::holds_alternative<uint8_t>(c2)) {
        return std::get<uint8_t>(c1) == std::get<uint8_t>(c2);
    } else if (std::holds_alternative<rgb_t>(c1) && std::holds_alternative<rgb_t>(c2)) {
        const rgb_t &rgb1 = std::get<rgb_t>(c1);
        const rgb_t &rgb2 = std::get<rgb_t>(c2);
        return rgb1.r == rgb2.r && rgb1.g == rgb2.g && rgb1.b == rgb2.b;
    } else {
        return false;
    }
}

bool operator!=(const TerminalIO::color_t &c1, const TerminalIO::color_t &c2) {
    return !(c1 == c2);
}

static std::string EncodeUTF8(uint32_t keycode) {
    char result[5];
    if (keycode <= 0x7F) {
        result[0] = ExtractBits(keycode, 0, 7);
        result[1] = '\0';
    } else if (keycode <= 0x7FF) {
        result[0] = 0b11000000 | ExtractBits(keycode, 6, 5);
        result[1] = 0b10000000 | ExtractBits(keycode, 0, 6);
        result[2] = '\0';
    } else if (keycode <= 0xFFFF) {
        result[0] = 0b11100000 | ExtractBits(keycode, 12, 4);
        result[1] = 0b10000000 | ExtractBits(keycode, 6, 6);
        result[2] = 0b10000000 | ExtractBits(keycode, 0, 6);
        result[3] = '\0';
    } else if (keycode <= 0x10FFFF) {
        result[0] = 0b11110000 | ExtractBits(keycode, 18, 3);
        result[1] = 0b10000000 | ExtractBits(keycode, 12, 6);
        result[2] = 0b10000000 | ExtractBits(keycode, 6, 6);
        result[3] = 0b10000000 | ExtractBits(keycode, 0, 6);
        result[4] = '\0';
    }
    return result;
}

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

uint32_t TerminalIO::ReadUTF8(uint8_t first) {
    size_t count = 0;
    uint8_t buf[3];
    uint32_t result;
    if ((first & 0b11100000) == 0b11000000) {
        result = (first & 0b00011111);
        count = 1;
    } else if ((first & 0b11110000) == 0b11100000) {
        result = (first & 0b00001111);
        count = 2;
    } else if ((first & 0b11111000) == 0b11110000) {
        result = (first & 0b00000111);
        count = 3;
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
    uint8_t ch;
    ssize_t bytes_read = read(STDIN_FILENO, &ch, 1);

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
                result |= (1 << 30);
                unsigned char seq_ch;
                
                // Read parameter bytes and intermediate bytes (0x30-0x3F and 0x20-0x2F)
                size_t bitshift = 0;
                while (read(STDIN_FILENO, &seq_ch, 1) > 0) {
                    // Check if this is a final byte (0x40-0x7E)
                    // Common final bytes: A-Z (cursor movement), ~ (function keys), m (color), etc.
                    if (bitshift >= 24) {
                        std::cout << "<too long>";
                        break;
                    }
                    if (seq_ch >= 0x20 && seq_ch <= 0x3F) {
                        uint8_t part = seq_ch - 0x20; // values from 0x00 to 0x1F -> 5 bits needed
                        result |= (part << bitshift);
                        bitshift += 5;
                    } else if (seq_ch >= 0x40 && seq_ch <= 0x7E) {
                        uint8_t part = seq_ch - 0x40; // values from 0x00 to 0x3F -> 6 bits needed
                        result |= (part << bitshift);
                        bitshift += 6;
                        break;
                    }

                }
            }
            // SS2 sequences: ESC N (single shift, rarely used)
            else if (next_ch == 'N') {
                unsigned char seq_ch;
                if (read(STDIN_FILENO, &seq_ch, 1) > 0) {
                    result = (2 << 30) | (seq_ch);
                }
            }
            // SS3 sequences: ESC O (used by some terminals for function keys)
            else if (next_ch == 'O') {
                unsigned char seq_ch;
                if (read(STDIN_FILENO, &seq_ch, 1) > 0) {
                    result = (2 << 30) | 0x80 | (seq_ch);
                }
            } else {
                result = (3 << 30) | 0x100 | next_ch;
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
    out_stream << ESC << "[" << (row + 1) << ";" << (col+1) << "H" << std::flush;
}

void TerminalIO::ShowCursor() {
    out_stream << ESC << "[?25h" << std::flush;
}

void TerminalIO::HideCursor() {
    out_stream << ESC << "[?25l" << std::flush;
}

void TerminalIO::Write(const std::string &data) {
    out_stream << data;
}
void TerminalIO::Write(uint32_t character) {
    out_stream << EncodeUTF8(character);
}
void TerminalIO::Flush() {
    out_stream.flush();
}

void TerminalIO::SetColor(std::optional<color_t> bg, std::optional<color_t> fg) {
    bool put_bg = (bg.has_value() && last_bg_color != *bg);
    bool put_fg = (fg.has_value() && last_fg_color != *fg);
    if (!put_bg && !put_fg) {
        return;
    }
    out_stream << ESC << "[";
    if (put_bg) {
        out_stream << "48;5;" << (int)std::get<uint8_t>(*bg);
        last_bg_color = *bg;
    }
    if (put_fg) {
        if (put_bg) {
            out_stream << ";";
        }
        out_stream << "38;5;" << (int)std::get<uint8_t>(*fg);
        last_fg_color = *fg;
    }
    out_stream << "m";
}

TerminalIO::~TerminalIO() {
    RestoreTerminal();
}

static const std::map<uint32_t, std::string> SPECIAL_KEYCODES {
    {0x01, "C-a"},
    {0x02, "C-b"},
    {0x03, "C-c"},
    {0x04, "C-d"},
    {0x05, "C-e"},
    {0x06, "C-f"},
    {0x07, "C-g"},
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


std::string KeyCodeToString(uint32_t keycode, char special_delim_left, char special_delim_right) {
    auto it = SPECIAL_KEYCODES.find(keycode);
    if (it != SPECIAL_KEYCODES.end()) {
        return special_delim_left + it->second + special_delim_right;
    } else if (keycode <= 0x1F) {
        return std::string("") + special_delim_left + "x" + char(keycode + 0x20) + special_delim_right;
    }if (keycode <= 0x0010FFFF) {
        return EncodeUTF8(keycode);
    } else {
        std::ostringstream os;
        os << special_delim_left 
           << std::hex << std::setw(8) << std::setfill('0') << keycode
           << special_delim_right;
        return os.str();
    }
}
