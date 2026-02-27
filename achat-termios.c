
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

/* ----------------- TERM -----------------*/

// Local Var
struct termios term;

// Restore original terminal properties
void restore_terminal(void) {
	tcsetattr(STDIN_FILENO,TCSANOW,&term);
}

// Set terminal to non-blocking input
void enable_nonblocking_input(void) {
    // Get attributes
    tcgetattr(STDIN_FILENO, &term);
    // Set exit
    atexit(restore_terminal);
    // Set Flags
    term.c_lflag &= ~(ICANON | ECHO); // raw input
    // Set attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    // Set stdin to non blocking
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}
