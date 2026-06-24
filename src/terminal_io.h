#ifndef TERMINAL_IO_H
#define TERMINAL_IO_H

#include <iostream>
#include <termios.h>
#include <string>
#include <stdint.h>
#include <optional>

std::string KeyCodeToString(uint32_t keycode, char special_delim_left = '<', char special_delim_right = '>');

class TerminalIO {
public:
    explicit TerminalIO(std::ostream &os) : out_stream(os) {}
    
    // Initialize terminal raw mode
    bool EnableRawMode();
    
    // Restore terminal to original state
    void RestoreTerminal();
    
    // Read keyboard input and return complete sequence as string
    uint32_t ReadKey();

    void Write(const std::string &data);
    void Write(uint32_t character);
    void Flush();
    
    // ANSI escape sequence operations
    void ClearScreen();
    void SetCursorPosition(int col, int row);
    void ShowCursor();
    void HideCursor();
    void SetColor(std::optional<uint8_t> bg, std::optional<uint8_t> fg);
    
    // Destructor to ensure cleanup
    ~TerminalIO();

private:
    uint32_t ReadUTF8(uint8_t first);
    std::ostream &out_stream;
    termios original_termios{};
    bool raw_mode_enabled = false;
};

#endif // TERMINAL_IO_H
