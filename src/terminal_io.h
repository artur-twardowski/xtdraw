#ifndef TERMINAL_IO_H
#define TERMINAL_IO_H

#include <stdint.h>
#include <termios.h>

#include <iostream>
#include <optional>
#include <string>
#include <variant>

std::string KeyCodeToString(uint32_t keycode, char special_delim_left = '<', char special_delim_right = '>');

class TerminalIO {
   public:
    struct rgb_t {
        uint8_t r, g, b;
    };
    enum struct color_mode_t { COLOR_8, COLOR_256, COLOR_RGB };
    typedef std::variant<uint8_t, rgb_t> color_t;
    explicit TerminalIO(std::ostream &os) : out_stream(os), last_bg_color(uint8_t{0}), last_fg_color(uint8_t{15}) {}

    bool     EnableRawMode();
    void     RestoreTerminal();
    uint32_t ReadKey();
    void     Write(const std::string &data);
    void     Write(uint32_t character);
    void     Flush();

    void ClearScreen();
    void SetCursorPosition(int col, int row);
    void ShowCursor();
    void HideCursor();
    void SetColor(std::optional<color_t> bg, std::optional<color_t> fg);

    // Destructor to ensure cleanup
    ~TerminalIO();

   private:
    uint32_t      ReadUTF8(uint8_t first);
    std::ostream &out_stream;
    termios       original_termios{};
    bool          raw_mode_enabled = false;
    color_t       last_bg_color, last_fg_color;
    color_mode_t  color_mode;
};

#endif  // TERMINAL_IO_H
