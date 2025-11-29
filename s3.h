#ifndef _S3_H_
#define _S3_H_

#include "lexer_fsm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>



///Constants for array sizes, defined for clarity and code readability
#define MAX_LINE 1024
#define MAX_ARGS 128
#define MAX_PROMPT_LEN 256

///Enum for readable argument indices
enum ArgIndex
{
    ARG_PROGNAME,
    ARG_1,
    ARG_2,
    ARG_3,
};

static inline void reap(pid_t pid)
{
    if (pid > 0){
        waitpid(pid, NULL, 0);
    }
}

/// shell I/O
void init_lwd(char lwd[]);
int is_cd(char line[]);
void run_cd(char *args[], int argsc, char lwd[]);
void read_command_line(char line[], char lwd[]);
void construct_shell_prompt(char shell_prompt[]);

//takes a bg ptr now
void parse_command(char line[], char *args[], int *argsc, int *is_bg);
int is_subshell(char line[]);
void process_input(char line[], char *lwd);
void run_subshell(char line[], char *lwd);
void trim_whitespace(char line[]);

/// child functions
void child(char *args[], int argsc);
void child_with_output_redirected(char *args[], int argsc, char *output_file, int append);
void child_with_input_redirected(char *args[], int argsc, char *input_file);

///launchers
void launch_program(char *args[], int argsc);
void launch_program_with_redirection(char *args[], int argsc, const char *original_line, int is_bg);
void launch_pipeline(char *stages[], int n, int is_bg);

/// Redirection
int find_redirection_in_line(const char *line, char **filename, int *append);
int find_redirection_operator(char *args[], int argsc, char **filename, int *append);

// Pipes
int command_has_pipes(char line[]);
int split_pipeline(char *line, char *stages[], int *count);

#endif