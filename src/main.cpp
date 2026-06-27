#include <iostream>
#include <map>
#include <unistd.h>
#include <signal.h>
#include "charset.h"
#include "terminal_io.h"
#include "board.h"
#include <iomanip>
#include <sstream>

// Global TerminalIO instance
TerminalIO* g_terminal_io = nullptr;
Board *g_board = nullptr;

bool redraw_board = true;
bool redraw_cursor_info = true;

/**
 * Handle cleanup on exit
 */
void OnClose() {
    if (g_terminal_io) {
        g_terminal_io->ShowCursor();
        g_terminal_io->RestoreTerminal();
    }
}

/**
 * Signal handler for graceful exit
 */
void OnSignal(int) {
    OnClose();
    exit(0);
}

/**
 * Process and display keyboard input
 */
void ProcessInput(uint32_t seq) {
    static const std::map<std::string, uint32_t> kInsertChar {
        {"0", ' '},
        {"1", 0x1fb00},
        {"2", 0x1fb01},
        {"3", 0x1fb02},
        {"4", 0x1fb03},
        {"5", 0x1fb04},

        {"!", 0x256d},
        {"@", 0x252c},
        {"#", 0x256e},
        {"$", 0x251c},
        {"%", 0x253c},
        {"^", 0x2524},
        {"&", 0x2570},
        {"*", 0x2534},
        {"(", 0x256f},
    };
    // Handle special keys and printable characters
    if (!seq) {
        return;
    }

    auto move_cursor = [&](int dx, int dy) {
        g_board->MoveCursor(dx, dy);
        redraw_board = true;
        redraw_cursor_info = true;
    };
    auto change_cursor_mode = [&](Board::CursorMode mode) {
        g_board->SetCursorMode(mode);
        redraw_board = true;
        redraw_cursor_info = true;
    };

    const std::string seq_str = KeyCodeToString(seq);
    if (seq == 'q' || seq == 'Q') {
        // Exit the application
        OnClose();
        exit(0);
    } else if (seq_str == "<Up>") {
        move_cursor(0, -1);
    } else if (seq_str == "<Down>") {
        move_cursor(0, 1);
    } else if (seq_str == "<Left>") {
        move_cursor(-1, 0);
    } else if (seq_str == "<Right>") {
        move_cursor(1, 0);
    } else if (seq_str == "<F1>") {
        change_cursor_mode(Board::CursorMode::ENTIRE_CHARACTER);
    } else if (seq_str == "<F2>") {
        change_cursor_mode(Board::CursorMode::BLK_2x2);
    } else if (seq_str == "<F3>") {
        change_cursor_mode(Board::CursorMode::BLK_2x3);
    } else if (seq_str == " ") {
        g_board->TogglePixel();
        redraw_board = true;
    } else {
        auto it = kInsertChar.find(seq_str);
        if (it != kInsertChar.end()) {
            g_board->SetCell(it->second);
            redraw_board = true;
        }
    }
}

void RedrawBoard(bool &redraw_board, TerminalIO &terminal_io, Board &board, uint32_t draw_frame) {
    if (redraw_board) {
        board.Render(terminal_io);
    }

    if (redraw_board || draw_frame % 16 == 0) {
        board.RenderCursor(terminal_io, draw_frame > 0);
    }
    if (redraw_board) {
        redraw_board = false;
    }
}

void RedrawCursorInfo(bool &redraw_cursor_info, TerminalIO &terminal_io, Board &board) {
        if (redraw_cursor_info) {
            uint16_t cx, cy;
            uint8_t csx, csy;
            board.GetCursorPosition(cx, cy, csx, csy);

            terminal_io.SetCursorPosition(0, 18);
            terminal_io.SetColor(uint8_t{4}, uint8_t{15});
            if (board.GetCursorMode() == Board::CursorMode::ENTIRE_CHARACTER) {
                terminal_io.Write(std::to_string(cx) + ", " + std::to_string(cy));
            } else {
                terminal_io.Write(std::to_string(cx) + "." + std::to_string(csx) + ", " + std::to_string(cy) + "." + std::to_string(csy));
                if (board.IsTogglingAvailable()) {
                    terminal_io.SetColor({}, uint8_t{11});
                } else {
                    terminal_io.SetColor({}, uint8_t{9});
                }
                terminal_io.Write(" T");
            }
            std::ostringstream os;
            os << " " << std::hex << board.GetCellUnderCursor().character << "   ";
            terminal_io.SetColor(uint8_t{4}, uint8_t{15});
            terminal_io.Write(os.str());
            redraw_cursor_info = false;
        }
}

void event_loop(TerminalIO& terminal_io) {
    static uint32_t draw_frame = 0;
    while (true) {
        draw_frame++;
        if (draw_frame == 32) {
            draw_frame = 0;
        }

        RedrawBoard(redraw_board, terminal_io, *g_board, draw_frame);
        RedrawCursorInfo(redraw_cursor_info, terminal_io, *g_board);

        terminal_io.SetColor(uint8_t{0}, uint8_t{255});
        terminal_io.SetCursorPosition(90, 5);

        ProcessInput(terminal_io.ReadKey());
        usleep(10000);
    }
}


int main() {
    // Create TerminalIO instance
    TerminalIO terminal_io(std::cout);
    Board board({0, 0, 80, 16}, 120, 60);
    g_terminal_io = &terminal_io;
    g_board = &board;

    // Enable raw mode
    if (!terminal_io.EnableRawMode()) {
        std::cerr << "Failed to enable raw mode\n";
        return 1;
    }
    terminal_io.HideCursor();

    // Register cleanup function to be called on exit
    atexit(OnClose);

    // Register signal handlers for graceful cleanup
    signal(SIGINT, OnSignal);
    signal(SIGTERM, OnSignal);

    // Clear the screen
    terminal_io.ClearScreen();

    // Run the event loop
    event_loop(terminal_io);

    return 0;
}
