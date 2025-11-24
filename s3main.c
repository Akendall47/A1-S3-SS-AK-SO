#include "s3.h"


int main(int argc, char *argv[]) {
    char line[MAX_LINE];
    char lwd[MAX_PROMPT_LEN-6]; 
    init_lwd(lwd);

    while (1) {
        read_command_line(line, lwd);
        process_input(line, lwd); // merged our prevoious commands so can call recursivley for subshells
    }
    return 0;
}