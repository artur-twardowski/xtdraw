#ifndef TERMINAL_IO_H
#define TERMINAL_IO_H

#include <iostream>
#include <termios.h>
#include <string>

class TerminalIO {
public:
    explicit TerminalIO(std::ostream &os) : out_stream(os) {}
    
    // Initialize terminal raw mode
    bool EnableRawMode();
    
    // Restore terminal to original state
    void RestoreTerminal();
    
    // Read keyboard input and return complete sequence as string
    std::string ReadKeySequence();
    
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
