#include "s3.h"

///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    strcpy(shell_prompt, "[s3]$ ");
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[])
{
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);

    ///See man page of fgets(...)
    //fgets: (ptr to array that stores input chars, max chars, input stream eg keyboard)

    if (fgets(line, MAX_LINE, stdin) == NULL)
    {
        perror("fgets failed");
        exit(1);
    }
    ///Remove newline (enter)
    line[strlen(line) - 1] = '\0';
}

void parse_command(char line[], char *args[], int *argsc)
{
    ///Implements simple tokenization (space delimited)
    ///Note: strtok puts '\0' (null) characters within the existing storage, 
    ///to split it into logical cstrings.
    ///There is no dynamic allocation.

    ///See the man page of strtok(...)
    // splits string into tokens using delimiter " " -stateful tokenzier
    char *token = strtok(line, " ");
    *argsc = 0;
    while (token != NULL && *argsc < MAX_ARGS - 1)
    {
        //store and increment ptr - one swoop - hence ++ is after 
        args[(*argsc)++] = token;

        //contiue tokenzing the same string 
        token = strtok(NULL, " ");
    }
    
    args[*argsc] = NULL; ///args must be null terminated
}

///Launch related functions
void child(char *args[], int argsc)
{
    ///Implement this function:

    ///Use execvp to load the binary 
    ///of the command specified in args[ARG_PROGNAME].
    ///For reference, see the code in lecture 3.

    //take agrument name, and arg list

    execvp(args[0], args);
    perror("execvp failed");
    exit(1);
}

void launch_program(char *args[], int argsc)
{
    ///Implement this function:

    ///fork() a child process.
    ///In the child part of the code,
    ///call child(args, argv)
    ///For reference, see the code in lecture 2.


    //handle exit before fork, as shell shoudl terminate, not child
    // user can type exit

    if (argsc > 0 && strcmp(args[0], "exit") == 0)
    {
        printf("Exiting shell.\n");
        exit(0);
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0)  //child
    {   
        //find a programme called eg ls and run seperatley
        execvp(args[0], args);
        perror("execvp failed");
        exit(1);
    }
    else //parent, pause, while child exexcutes
    {
        int status;

        //over: int pid == wait(NULL)
        //as more flexible.. 1+ child processes., Flags for non-blocking
        waitpid(pid, &status, 0);
    }
}