#ifndef _TERMINAL_H_
#define _TERMINAL_H_


// For terminal control
static struct termios oldt, newt;

void enable_raw_mode();
void disable_raw_mode();
void move_cursor_left(int n);
void move_cursor_right(int n);

#endif