#include "s3.h"
#include "terminal_control.h"

History *history_head = NULL;
History *history_tail = NULL;

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
    if (n > 0) {
        printf("\033[%dD", n);
    }
    fflush(stdout);
}

void move_cursor_right(int n){
    if (n > 0) {
        printf("\033[%dC", n);
    }
    fflush(stdout);
}

// Used for giving colour output for ls command like standard terminal
void add_ls_colour(char *args[], int argsc){
    if (args[0] && strcmp(args[0], "ls") == 0) {

        char **new_args = malloc(sizeof(argsc)+2);

        for (int i = 0; i < argsc; i++)
            new_args[i] = args[i];

        // Add colour flag to new arguments
        new_args[argsc] = "--color=always";
        new_args[argsc+1] = NULL;

        execvp("ls", new_args);
        perror("execvp");
        exit(1);
    }
}


void add_history(char *new_line) {
    if (strlen(new_line) == 0) return; // Don't add empty lines

    // So duplicates of the same command after each other aren't just added
    if (history_tail && strcmp(history_tail->line, new_line) == 0) return;

    History *new_node = malloc(sizeof(History));
    new_node->line = strdup(new_line); 
    new_node->next = NULL;
    new_node->prev = history_tail;

    if (history_tail) {
        history_tail->next = new_node;
    } else {
        history_head = new_node;
    }
    history_tail = new_node;
}

void clear_and_redraw(char *shell_prompt, char *line, size_t pos) {
    printf("\r\033[K");
    printf(GREEN "%s\x1b[0m", shell_prompt);
    fwrite(line, 1, pos, stdout);
}