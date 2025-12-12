#include "s3.h"
#include "jobs.h"
#include <signal.h>

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv; // maybe use in future
    char line[MAX_LINE];
    char lwd[MAX_PROMPT_LEN-6];

    init_jobs();
    init_lwd(lwd);

    //ignore interactive signals in the shell itself
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    while (1) {
        read_command_line(line, sizeof(line));
        process_input(line, lwd); 
    }
    return 0;
}