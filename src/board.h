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
    uint32_t character{' '};        // Unicode character (any Unicode character supported)
    uint8_t  foreground_color{15};  // Foreground color (0-255)
    uint8_t  background_color{0};   // Background color (0-255)
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
    enum struct CursorMode { ENTIRE_CHARACTER, BLK_2x2, BLK_2x3, BLK_2x4 };
    Board(const BoxDimensions &viewport, uint16_t width = 80, uint16_t height = 25);

    void SetCell(uint32_t character, uint8_t bg_color = 0, uint8_t fg_color = 15);
    void TogglePixel();
    bool IsTogglingAvailable() const;

    void SetCursorPosition(uint16_t x, uint16_t y);
    void MoveCursor(int16_t x, int16_t y);
    void GetCursorPosition(uint16_t &x, uint16_t &y, uint8_t &sx, uint8_t &sy) const;

    const BoardCell &GetCell(uint16_t row, uint16_t col) const;
    const BoardCell &GetCellUnderCursor() const;

    void       SetCursorMode(CursorMode mode) { cursor_mode = mode; }
    CursorMode GetCursorMode() const { return cursor_mode; }

    void Render(TerminalIO &terminal_io);
    void RenderCursor(TerminalIO &terminal_io, bool show_placeholder);

    uint16_t GetCols() const { return width; }
    uint16_t GetRows() const { return height; }

    bool IsValidCoord(uint16_t row, uint16_t col) const;

   private:
    BoxDimensions          window;
    uint16_t               width;
    uint16_t               height;
    uint16_t               cursor_x;
    uint16_t               cursor_y;
    uint8_t                cursor_sx{0};
    uint8_t                cursor_sy{0};
    uint16_t               view_left;
    uint16_t               view_top;
    CursorMode             cursor_mode{CursorMode::ENTIRE_CHARACTER};
    std::vector<BoardCell> grid;
};

#endif  // BOARD_H
