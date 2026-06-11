#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <signal.h>
#include "ansi_escape.h"

// Global variables for terminal state restoration
termios original_termios;
bool raw_mode_enabled = false;

/**
 * Restore terminal to original state
 */
void restore_terminal() {
    if (raw_mode_enabled) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
        raw_mode_enabled = false;
    }
}

/**
 * Handle cleanup on exit
 */
void cleanup() {
    restore_terminal();
    AnsiEscape::show_cursor();
}

/**
 * Signal handler for graceful exit
 */
void signal_handler(int) {
    cleanup();
    exit(0);
}

/**
 * Enable raw mode for terminal
 */
bool enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
        perror("tcgetattr");
        return false;
    }

    // Register cleanup function to be called on exit
    atexit(cleanup);

    // Register signal handlers for graceful cleanup
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    termios raw = original_termios;

    // Disable canonical mode and echo
    raw.c_lflag &= ~(ICANON | ECHO);

    // Set minimum characters to read and timeout
    raw.c_cc[VMIN] = 0;   // Non-blocking read
    raw.c_cc[VTIME] = 0;  // No timeout

    // Disable output processing
    raw.c_oflag &= ~(OPOST);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        return false;
    }

    raw_mode_enabled = true;
    return true;
}

/**
 * Handle keyboard input
 */
void handle_input() {
    unsigned char ch;
    ssize_t bytes_read = read(STDIN_FILENO, &ch, 1);

    if (bytes_read == -1) {
        perror("read");
        return;
    }

    if (bytes_read == 0) {
        return; // No input available
    }

    // Handle special keys and printable characters
    if (ch == 'q' || ch == 'Q') {
        // Exit the application
        cleanup();
        exit(0);
    } else if (ch == '\033') {
        // Escape sequence (arrow keys, function keys, etc.)
        unsigned char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) > 0) {
            if (seq[0] == '[') {
                if (read(STDIN_FILENO, &seq[1], 1) > 0) {
                    switch (seq[1]) {
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
void event_loop() {
    std::cout << "Raw Mode Terminal Application\n"
             << "============================""\n"
             << "Press 'q' to quit.\n"
             << "Try pressing arrow keys, letters, etc.\n"
             << std::flush;

    while (true) {
        handle_input();
        usleep(10000); // Small delay to prevent busy-waiting (10ms)
    }
}

int main() {
    // Enable raw mode
    if (!enable_raw_mode()) {
        std::cerr << "Failed to enable raw mode\n";
        return 1;
    }

    // Clear the screen
    AnsiEscape::clear_screen();

    // Run the event loop
    event_loop();

    return 0;
}
