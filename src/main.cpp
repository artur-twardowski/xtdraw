#include <iostream>
#include <map>
#include <unistd.h>
#include <signal.h>
#include "terminal_io.h"
#include "board.h"
#include <iomanip>

// Global TerminalIO instance
TerminalIO* g_terminal_io = nullptr;
Board *g_board = nullptr;

bool redraw = true;

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
    const std::string seq_str = KeyCodeToString(seq);
    if (seq == 'q' || seq == 'Q') {
        // Exit the application
        OnClose();
        exit(0);
    } else if (seq_str == "<Up>") {
        g_board->MoveCursor(0, -1);
        redraw = true;
    } else if (seq_str == "<Down>") {
        g_board->MoveCursor(0, 1);
        redraw = true;
    } else if (seq_str == "<Left>") {
        g_board->MoveCursor(-1, 0);
        redraw = true;
    } else if (seq_str == "<Right>") {
        g_board->MoveCursor(1, 0);
        redraw = true;
    } else if (seq_str == "<F1>") {
        g_board->SetCursorMode(Board::CursorMode::ENTIRE_CHARACTER);
    } else if (seq_str == "<F2>") {
        g_board->SetCursorMode(Board::CursorMode::BLK_2x2);
    } else if (seq_str == " ") {
        g_board->TogglePixel();
        redraw = true;
    } else {
        auto it = kInsertChar.find(seq_str);
        if (it != kInsertChar.end()) {
            g_board->SetCell(it->second);
            redraw = true;
        }
    }
}

/**
 * Main event loop
 */
void event_loop(TerminalIO& terminal_io) {
    static uint32_t draw_frame = 0;
    while (true) {
        draw_frame++;
        if (draw_frame == 32) {
            draw_frame = 0;
        }

        if (redraw) {
            g_board->Render(terminal_io);
        }

        if (redraw || draw_frame % 16 == 0) {
            g_board->RenderCursor(terminal_io, draw_frame > 0);
        }
        if (redraw) {
            redraw = false;
        }
        ProcessInput(terminal_io.ReadKey());
        usleep(10000);
    }
}


int main() {
    // Create TerminalIO instance
    TerminalIO terminal_io(std::cout);
    Board board({20, 2, 80, 16}, 120, 60);
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
