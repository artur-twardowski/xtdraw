#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "terminal_io.h"

// Global TerminalIO instance
TerminalIO* g_terminal_io = nullptr;

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
void ProcessInput(const std::string& sequence) {
    if (sequence.empty()) {
        return;
    }

    unsigned char ch = sequence[0];

    // Handle special keys and printable characters
    if (ch == 'q' || ch == 'Q') {
        // Exit the application
        OnClose();
        exit(0);
    } else {
        for (unsigned char ch: sequence) {
            if (ch < 32 || ch >= 127) {
                std::cout << "<" << std::hex << (int)ch << ">";
            } else {
                std::cout << ch;
            }
        }
        std::cout.flush();
    }
}

/**
 * Main event loop
 */
void event_loop(TerminalIO& terminal_io) {
    while (true) {
        std::string key_sequence = terminal_io.ReadKeySequence();
        ProcessInput(key_sequence);
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
