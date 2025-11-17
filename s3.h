#ifndef _S3_H_
#define _S3_H_

///See reference for what these libraries provide
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>


///Constants for array sizes, defined for clarity and code readability
#define MAX_LINE 1024
#define MAX_ARGS 128
#define MAX_PROMPT_LEN 256

///Enum for readable argument indices (use where required)
enum ArgIndex
{
    ARG_PROGNAME,
    ARG_1,
    ARG_2,
    ARG_3,
};

///With inline functions, the compiler replaces the function call 
///with the actual function code;
///inline improves speed and readability; meant for short functions (a few lines).
///the static here avoids linker errors from multiple definitions (needed with inline).
// archie - updated to waitpid to have mroe control - and make more scalable if needed 
static inline void reap(pid_t pid)
{
    if (pid > 0){
        waitpid(pid, NULL, 0);
    }
}

///Shell I/O and related functions (add more as appropriate)
void init_lwd(char lwd[]);
int is_cd(char line[]);
void run_cd(char *args[], int argsc, char lwd[]);
void read_command_line(char line[], char lwd[]);
void construct_shell_prompt(char shell_prompt[]);
void parse_command(char line[], char *args[], int *argsc);

///Child functions (add more as appropriate)
void child(char *args[], int argsc);
void child_with_output_redirected(char *args[], int argsc, char *output_file, int append);
void child_with_input_redirected(char *args[], int argsc, char *input_file);

///Program launching functions (add more as appropriate)
pid_t launch_program(char *args[], int argsc);
pid_t launch_program_with_redirection(char *args[], int argsc);
pid_t launch_pipeline(char *stages[], int n);

///Redirection detection and parsing
int command_with_redirection(char line[]);
int find_redirection_operator(char *args[], int argsc, char **filename, int *append);

// Pipe Specific
int command_has_pipes(char line[]);
int split_pipeline(char *line, char *stages[], int *count);

#endif