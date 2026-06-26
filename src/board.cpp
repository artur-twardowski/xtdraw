#include "board.h"
#include "terminal_io.h"
#include "charset.h"

Board::Board(const BoxDimensions &viewport, uint16_t width, uint16_t height) : window(viewport), width(width), height(height) {
    grid.resize(width * height);
    view_left = 0;
    view_top = 0;
    cursor_x = 0;
    cursor_y = 0;
}

void Board::SetCell(uint32_t character, uint8_t bg_color, uint8_t fg_color) {
    auto &element = grid[cursor_y * width + cursor_x];
    element.character = character;
    element.background_color = bg_color;
    element.foreground_color = fg_color;
}

void Board::SetCursorPosition(uint16_t x, uint16_t y) {
    if (x < width) {
        cursor_x = x;
    } else {
        cursor_x = width-1;
    }
    if (y < height) {
        cursor_y = y;
    } else {
        cursor_y = height-1;
    }
}
void Board::MoveCursor(int16_t x, int16_t y) {
    if (x > 0) {
        if (cursor_x + x < width) {
            cursor_x += x;
        } else {
            cursor_x = width - 1;
        }
        while (cursor_x >= view_left + window.width - 2) {
            view_left++;
        }
    } else {
        if (cursor_x >= -x) {
            cursor_x += x;
        } else {
            cursor_x = 0;
        }
        while (cursor_x < view_left) {
            view_left--;
        }
    }
    if (y > 0) {
        if (cursor_y + y < height) {
            cursor_y += y;
        } else {
            cursor_y = height - 1;
        }
        while (cursor_y >= view_top + window.height - 2) {
            view_top++;
        }
    } else {
        if (cursor_y >= -y) {
            cursor_y += y;
        } else {
            cursor_y = 0;
        }
        while (cursor_y < view_top) {
            view_top--;
        }
    }
}

const BoardCell& Board::GetCell(uint16_t row, uint16_t col) const {
    static const BoardCell empty;
    if (IsValidCoord(row, col)) {
        return grid[row * width + col];
    } else {
        return empty;
    }
}

void Board::Render(TerminalIO &terminal_io) {
    const uint32_t kLeftTopCorner = GetBoxDrawingCharacter({
        .south=line_weight_t::THIN,
        .east=line_weight_t::THICK,
        .attributes=ATTR_ROUNDED_CORNERS
    });
    const uint32_t kRightTopCorner = GetBoxDrawingCharacter({
        .south=line_weight_t::THIN,
        .west=line_weight_t::THICK,
        .attributes=ATTR_ROUNDED_CORNERS
    });
    const uint32_t kLeftBottomCorner = 0x2570;
    const uint32_t kRightBottomCorner = 0x256f;
    const uint32_t kHorizontalLine = GetBoxDrawingCharacter({
        .east=line_weight_t::THIN,
        .west=line_weight_t::THIN,
    });
    const uint32_t kVerticalLine = GetBoxDrawingCharacter({
        .north=line_weight_t::THIN,
        .south=line_weight_t::THIN,
    });
    const uint8_t border_bg = 16;
    const uint8_t border_fg = 255;

    uint32_t rows = window.height - 2;
    uint32_t cols = window.width - 2;

    terminal_io.SetCursorPosition(window.left, window.top);
    terminal_io.SetColor(border_bg, border_fg);
    terminal_io.Write(kLeftTopCorner);
    for (size_t x = 0; x < cols; x++) {
        terminal_io.Write(kHorizontalLine);
    }
    terminal_io.Write(kRightTopCorner);

    for (uint32_t y = 0; y < rows; y++) {
        terminal_io.SetCursorPosition(window.left, window.top + y + 1);
        terminal_io.SetColor(border_bg, border_fg);
        terminal_io.Write(kVerticalLine);

        for (uint32_t x = 0; x < cols; x++) {
            const auto &cell = GetCell(view_top + y, view_left + x);
            terminal_io.SetColor(cell.background_color, cell.foreground_color);
            terminal_io.Write(cell.character);
        }
        terminal_io.Write(kVerticalLine);
    }
    terminal_io.SetCursorPosition(window.left, window.top + window.height - 1);
    terminal_io.SetColor(border_bg, border_fg);
    terminal_io.Write(kLeftBottomCorner);
    for (size_t x = 0; x < cols; x++) {
        terminal_io.Write(kHorizontalLine);
    }
    terminal_io.Write(kRightBottomCorner);
    terminal_io.Flush();
}

void Board::RenderCursor(TerminalIO &terminal_io, bool show_placeholder) {
    const auto &cell = GetCell(cursor_y, cursor_x);
    terminal_io.SetCursorPosition(
            cursor_x - view_left + window.left + 1,
            cursor_y - view_top + window.top + 1);
    if (show_placeholder) {
        terminal_io.SetColor(cell.foreground_color, cell.background_color);
        terminal_io.Write(cell.character);
    } else {
        terminal_io.SetColor(cell.background_color, cell.foreground_color);
        terminal_io.Write(cell.character);
    }
}

bool Board::IsValidCoord(uint16_t row, uint16_t col) const {
    return row < height && col < width;
}
