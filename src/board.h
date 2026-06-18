#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include <cstddef>

/**
 * Structure representing a single cell on the board
 */
struct BoardCell {
    uint32_t character;      // Unicode character (any Unicode character supported)
    uint8_t foreground_color; // Foreground color (0-255)
    uint8_t background_color; // Background color (0-255)
};

/**
 * Board class for managing a 2D grid of characters
 */
class Board {
public:
    static constexpr int DEFAULT_COLS = 80;
    static constexpr int DEFAULT_ROWS = 25;

    /**
     * Constructor with optional custom dimensions
     */
    Board(int cols = DEFAULT_COLS, int rows = DEFAULT_ROWS);

    /**
     * Destructor
     */
    ~Board();

    /**
     * Initialize board with space characters
     */
    void Initialize();

    /**
     * Set a cell at position (row, col)
     */
    void SetCell(int row, int col, uint32_t character, uint8_t fg_color = 7, uint8_t bg_color = 0);

    /**
     * Get a cell at position (row, col)
     */
    const BoardCell& GetCell(int row, int col) const;

    /**
     * Render the board to the terminal
     */
    void Render(class TerminalIO& terminal_io);

    /**
     * Get board dimensions
     */
    int GetCols() const { return cols; }
    int GetRows() const { return rows; }

    /**
     * Validate coordinates
     */
    bool IsValidCoord(int row, int col) const;

private:
    int cols;
    int rows;
    BoardCell* grid;

    /**
     * Get index from row and col
     */
    int GetIndex(int row, int col) const { return row * cols + col; }
};

#endif // BOARD_H
