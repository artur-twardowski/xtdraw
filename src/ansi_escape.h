#ifndef ANSI_ESCAPE_H
#define ANSI_ESCAPE_H

#include <iostream>

/**
 * Utility class for ANSI escape code operations
 * Encapsulates all terminal control sequences used for cursor positioning,
 * screen clearing, and cursor visibility.
 */
class AnsiEscape {
public:
    /**
     * Clear the entire screen and move cursor to home position (0,0)
     */
    static void clear_screen();

    /**
     * Set cursor position to specified row and column
     * @param row Row number (1-based)
     * @param col Column number (1-based)
     */
    static void set_cursor_position(int row, int col);

    /**
     * Show the cursor
     */
    static void show_cursor();

    /**
     * Hide the cursor
     */
    static void hide_cursor();

private:
    // Private constructor to prevent instantiation
    AnsiEscape() = default;

    /**
     * Write ANSI escape sequence to stdout and flush
     * @param sequence The complete ANSI sequence
     */
    static void write_escape_sequence(const char* sequence);
};

#endif // ANSI_ESCAPE_H
