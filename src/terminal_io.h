#ifndef TERMINAL_IO_H
#define TERMINAL_IO_H

#include <stdint.h>
#include <termios.h>

#include <iostream>
#include <optional>
#include <string>
#include <variant>

#include "types.h"
#include "ansi_output.h"

std::string KeyCodeToString(uint32_t keycode, char special_delim_left = '<',
                            char special_delim_right = '>');

class TerminalIO: public xtdraw::ANSIOutput {
   public:
    explicit TerminalIO(std::ostream &os);
    virtual ~TerminalIO();

    bool     EnableRawMode();
    void     RestoreTerminal();
    uint32_t ReadKey();
    void     WriteWindowed(const std::string &data, size_t window_size);
    void     Flush();

    void ClearScreen();
    void SetCursorPosition(int col, int row);
    void ShowCursor();
    void HideCursor();

   private:
    uint32_t      ReadUTF8(uint8_t first);
    std::ostream &out_stream;
    termios       original_termios{};
    bool          raw_mode_enabled = false;
};

#endif  // TERMINAL_IO_H
