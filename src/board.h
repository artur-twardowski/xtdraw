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

/**
 * Board class for managing a 2D grid of characters
 */
class Board {
public:
    /**
     * Constructor with optional custom dimensions
     */
    Board(uint16_t cols = 80, uint16_t rows = 25);

    /**
     * Set a cell at position (row, col)
     */
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
    uint16_t GetCols() const { return cols; }
    uint16_t GetRows() const { return rows; }

    /**
     * Validate coordinates
     */
    bool IsValidCoord(uint16_t row, uint16_t col) const;

private:
    uint16_t cols;
    uint16_t rows;
    uint16_t cursor_x;
    uint16_t cursor_y;
    const uint16_t board_draw_x{20};
    std::vector<BoardCell> grid;

    /**
     * Get index from row and col
     */
    int GetIndex(int row, int col) const { return row * cols + col; }
};

#endif // BOARD_H
