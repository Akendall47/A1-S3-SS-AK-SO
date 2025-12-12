#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <termios.h>

// For terminal control (declared as extern, defined in terminal_control.c)
extern struct termios oldt, newt;

typedef struct History {
    char *line;
    struct History *prev;
    struct History *next;
} History;

// A pain to get these variables working :(
extern History *history_head;
extern History *history_tail;

void enable_raw_mode(void);
void disable_raw_mode(void);
void move_cursor_left(int n);
void move_cursor_right(int n);
void add_ls_colour(char *args[], int argsc);

void add_history(char *new_line);
void clear_and_redraw(char *shell_prompt, char *line, size_t pos);

#endif