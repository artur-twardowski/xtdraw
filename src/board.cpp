#include "board.h"
#include "terminal_io.h"

Board::Board(uint16_t cols, uint16_t rows) : cols(cols), rows(rows) {
    grid.resize(cols * rows);
}

void Board::SetCell(uint32_t character, uint8_t bg_color, uint8_t fg_color) {
    auto &element = grid[cursor_y * cols + cursor_x];
    element.character = character;
    element.background_color = bg_color;
    element.foreground_color = fg_color;
}

void Board::SetCursorPosition(uint16_t x, uint16_t y) {
    if (x < cols) {
        cursor_x = x;
    } else {
        cursor_x = cols-1;
    }
    if (y < rows) {
        cursor_y = y;
    } else {
        cursor_y = rows-1;
    }
}
void Board::MoveCursor(int16_t x, int16_t y) {
    if (x > 0) {
        if (cursor_x + x < cols) {
            cursor_x += x;
        } else {
            cursor_x = cols - 1;
        }
    } else {
        if (cursor_x >= -x) {
            cursor_x += x;
        } else {
            cursor_x = 0;
        }
    }
    if (y > 0) {
        if (cursor_y + y < rows) {
            cursor_y += y;
        } else {
            cursor_y = rows - 1;
        }
    } else {
        if (cursor_y >= -y) {
            cursor_y += y;
        } else {
            cursor_y = 0;
        }
    }
}

const BoardCell& Board::GetCell(uint16_t row, uint16_t col) const {
    static const BoardCell empty;
    if (IsValidCoord(row, col)) {
        return grid[row * cols + col];
    } else {
        return empty;
    }
}

void Board::Render(TerminalIO &terminal_io) {
    const auto *cell = grid.data();
    for (uint32_t y = 0; y < rows; y++) {
        uint8_t last_fg = 0, last_bg = 0;
        terminal_io.SetCursorPosition(y, 0);
        terminal_io.SetColor(last_bg, last_fg);

        for (uint32_t x = 0; x < cols; x++) {
            if (cell->foreground_color != last_fg || cell->background_color != last_bg) {
                terminal_io.SetColor(cell->background_color, cell->foreground_color);
                last_fg = cell->foreground_color;
                last_bg = cell->background_color;
            }

            terminal_io.Write(cell->character);
            cell++;
        }
        terminal_io.Flush();
    }
}

void Board::RenderCursor(TerminalIO &terminal_io, bool show_cursor) {
    const auto &cell = GetCell(cursor_y, cursor_x);
    terminal_io.SetCursorPosition(cursor_y, cursor_x);
    terminal_io.SetColor(cell.background_color, cell.foreground_color);
    if (show_cursor) {
        if (cell.character == ' ') {
            terminal_io.Write('O');
        } else {
            terminal_io.Write(' ');
        }
    } else {
        terminal_io.Write(cell.character);
    }
}

bool Board::IsValidCoord(uint16_t row, uint16_t col) const {
    return row < rows && col < cols;
}
