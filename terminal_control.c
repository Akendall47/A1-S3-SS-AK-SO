#include "s3.h"
#include "terminal_control.h"

// Used to pass every character to the program rather than waiting until the enter key getting rid of fgets()
void enable_raw_mode(){
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void disable_raw_mode(){
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

void move_cursor_left(int n){
    printf("\x1b[%dD", n);
    fflush(stdout);
}

void move_cursor_right(int n){
    printf("\x1b[%dC", n);
    fflush(stdout);
}