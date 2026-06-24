#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <cstddef>
#include <vector>

class TerminalIO;

/**
 * Structure representing a single cell on the board
 */
struct BoardCell {
    uint32_t character{' '};      // Unicode character (any Unicode character supported)
    uint8_t foreground_color{15}; // Foreground color (0-255)
    uint8_t background_color{0}; // Background color (0-255)
};

struct BoxDimensions {
    uint16_t left;
    uint16_t top;
    uint16_t width;
    uint16_t height;
};

/**
 * Board class for managing a 2D grid of characters
 */
class Board {
public:
    Board(const BoxDimensions &viewport, uint16_t width = 80, uint16_t height = 25);

    void SetCell(uint32_t character, uint8_t bg_color = 0, uint8_t fg_color = 15);

    void SetCursorPosition(uint16_t x, uint16_t y);
    void MoveCursor(int16_t x, int16_t y);

    /**
     * Get a cell at position (row, col)
     */
    const BoardCell& GetCell(uint16_t row, uint16_t col) const;

    /**
     * Render the board to the terminal
     */
    void Render(TerminalIO& terminal_io);

    void RenderCursor(TerminalIO &terminal_io, bool show_placeholder);

    /**
     * Get board dimensions
     */
    uint16_t GetCols() const { return width; }
    uint16_t GetRows() const { return height; }

    /**
     * Validate coordinates
     */
    bool IsValidCoord(uint16_t row, uint16_t col) const;

private:
    BoxDimensions window;
    uint16_t width;
    uint16_t height;
    uint16_t cursor_x;
    uint16_t cursor_y;
    uint16_t view_left;
    uint16_t view_top;
    std::vector<BoardCell> grid;
};

#endif // BOARD_H
