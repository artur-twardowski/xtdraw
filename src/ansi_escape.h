#ifndef ANSI_ESCAPE_H
#define ANSI_ESCAPE_H

#include <iostream>

#include <iostream>

class AnsiEscape {
public:
    explicit AnsiEscape(std::ostream &os) : out_stream(os) {}
    void ClearScreen();
    void SetCursorPosition(int row, int col);
    void ShowCursor();
    void HideCursor();

private:
    std::ostream &out_stream;
};

#endif // ANSI_ESCAPE_H
