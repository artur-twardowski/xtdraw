#ifndef TERMINAL_IO_H
#define TERMINAL_IO_H

#include <iostream>
#include <termios.h>

class TerminalIO {
public:
    explicit TerminalIO(std::ostream &os) : out_stream(os) {}
    
    // Initialize terminal raw mode
    bool EnableRawMode();
    
    // Restore terminal to original state
    void RestoreTerminal();
    
    // ANSI escape sequence operations
    void ClearScreen();
    void SetCursorPosition(int row, int col);
    void ShowCursor();
    void HideCursor();
    
    // Destructor to ensure cleanup
    ~TerminalIO();

private:
    std::ostream &out_stream;
    termios original_termios{};
    bool raw_mode_enabled = false;
};

#endif // TERMINAL_IO_H
