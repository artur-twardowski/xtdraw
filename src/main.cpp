#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "ansi_escape.h"

// Global TerminalIO instance
TerminalIO* g_terminal_io = nullptr;

/**
 * Handle cleanup on exit
 */
void cleanup() {
    if (g_terminal_io) {
        g_terminal_io->ShowCursor();
        g_terminal_io->RestoreTerminal();
    }
}

/**
 * Signal handler for graceful exit
 */
void signal_handler(int) {
    cleanup();
    exit(0);
}

/**
 * Process and display keyboard input
 */
void process_input(const std::string& sequence) {
    if (sequence.empty()) {
        return;
    }

    unsigned char ch = sequence[0];

    // Handle special keys and printable characters
    if (ch == 'q' || ch == 'Q') {
        // Exit the application
        cleanup();
        exit(0);
    } else if (ch == '\033') {
        // Escape sequence (arrow keys, function keys, etc.)
        if (sequence.length() >= 3 && sequence[1] == '[') {
            switch (sequence[2]) {
                case 'A':
                    std::cout << "UP arrow pressed\n" << std::flush;
                    break;
                case 'B':
                    std::cout << "DOWN arrow pressed\n" << std::flush;
                    break;
                case 'C':
                    std::cout << "RIGHT arrow pressed\n" << std::flush;
                    break;
                case 'D':
                    std::cout << "LEFT arrow pressed\n" << std::flush;
                    break;
            }
        }
    } else if (ch == '\n' || ch == '\r') {
        std::cout << "ENTER pressed\n" << std::flush;
    } else if (ch == 127 || ch == '\b') {
        std::cout << "BACKSPACE pressed\n" << std::flush;
    } else if (ch == 9) {
        std::cout << "TAB pressed\n" << std::flush;
    } else if (ch == 27) {
        std::cout << "ESC pressed\n" << std::flush;
    } else if (ch < 32) {
        std::cout << "Control character: 0x" << std::hex << (int)ch << std::dec << "\n" << std::flush;
    } else if (ch >= 32 && ch < 127) {
        std::cout << "Key pressed: '" << ch << "' (ASCII: " << (int)ch << ")\n" << std::flush;
    } else if (ch >= 128) {
        std::cout << "Extended character: 0x" << std::hex << (int)ch << std::dec << "\n" << std::flush;
    }
}

/**
 * Main event loop
 */
void event_loop(TerminalIO& terminal_io) {
    std::cout << "Raw Mode Terminal Application\n"
             << "============================""\n"
             << "Press 'q' to quit.\n"
             << "Try pressing arrow keys, letters, etc.\n"
             << std::flush;

    while (true) {
        std::string key_sequence = terminal_io.ReadKeySequence();
        process_input(key_sequence);
        usleep(10000); // Small delay to prevent busy-waiting (10ms)
    }
}


int main() {
    // Create TerminalIO instance
    TerminalIO terminal_io(std::cout);
    g_terminal_io = &terminal_io;

    // Enable raw mode
    if (!terminal_io.EnableRawMode()) {
        std::cerr << "Failed to enable raw mode\n";
        return 1;
    }

    // Register cleanup function to be called on exit
    atexit(cleanup);

    // Register signal handlers for graceful cleanup
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Clear the screen
    terminal_io.ClearScreen();

    // Run the event loop
    event_loop(terminal_io);

    return 0;
}
