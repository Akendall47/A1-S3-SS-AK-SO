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

/// Check: for redirection operators >
//strstr: search for substring in a string
int command_with_redirection(char line[]){
    if (strstr(line, ">>") != NULL || 
        strstr(line, ">") != NULL || 
        strstr(line, "<") != NULL){
        return 1;
    }
    return 0;
}

// return: >, >> == 1 ; input: < == 2 ; None: == 0 
// extract filename after opertor 
//append flag set for >>
// null terminates args array for execvp
int find_redirection_operator(char *args[], int argsc, char **filename, int *append) {
    *append = 0;
    
    for (int i = 0; i < argsc; i++) {
        if (strcmp(args[i], ">>") == 0){
            if (i + 1 < argsc) {
                *filename = args[i + 1];
                *append = 1;
                args[i] = NULL;
                return 1;
            }
        } else if (strcmp(args[i], ">") == 0){ //overwrite 
            if (i + 1 < argsc) {
                *filename = args[i + 1];
                args[i] = NULL;
                return 1;
            }
        } else if (strcmp(args[i], "<") == 0){
            ///Input redirection
            if (i + 1 < argsc) {
                *filename = args[i + 1];
                args[i] = NULL;
                return 2;
            }
        }
    }
    
    return 0;
}

///Launch related functions
void child(char *args[], int argsc)
{
    ///Implement this function:

    ///Use execvp to load the binary 
    ///of the command specified in args[ARG_PROGNAME].
    ///For reference, see the code in lecture 3.

    //take agrument name, and arg list

    /*Debug - print args to see what excvp is called with
    for (int i = 0; args[i] != NULL; ++i) {
        fprintf(stderr, "execvp will run: arg[%d] = '%s'\n", i, args[i]);
    }*/

    execvp(args[0], args);
    perror("execvp failed");
    exit(1);
}

void child_with_output_redirected(char *args[], int argsc, char *output_file, int append){
    int fd;
    
    // open file and set flags as needed
    // 0644: 3 permission groups: owner R/W, everyone else just R
    if (append){
        // append:  create if doesn't exist, append if it does
        fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    } else{
        // overwrite: create if doesn't exist, truncate if it does ie. start with empty file
        fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    
    if (fd < 0){
        perror("open failed");
        exit(1);
    }
    
    // Redirect stdout to file
    if (dup2(fd, STDOUT_FILENO) < 0){
        perror("dup2 failed");
        close(fd);
        exit(1);
    }
    
    close(fd);
    
    execvp(args[0], args);
    perror("execvp failed");
    exit(1);
}

// can code be resused here ? encapsulate error handling into own function/method? 

void child_with_input_redirected(char *args[], int argsc, char *input_file) {
    int fd;
    
    // Open file for reading
    fd = open(input_file, O_RDONLY);
    
    if (fd < 0) {
        perror("open failed");
        exit(1);
    }
    
    // redirect stdin from the file
    if (dup2(fd, STDIN_FILENO) < 0) {
        perror("dup2 failed");
        close(fd);
        exit(1);
    }
    
    close(fd); 

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
        child(args, argsc);
    }
    else //parent, pause, while child exexcutes
    {
        int status;

        //over: int pid == wait(NULL)
        //as more flexible.. 1+ child processes., Flags for non-blocking
        waitpid(pid, &status, 0);
    }
}

void launch_program_with_redirection(char *args[], int argsc) {
    ///Handle exit before fork
    if (argsc > 0 && strcmp(args[0], "exit") == 0) {
        printf("Exiting shell.\n");
        exit(0);
    }
    
    char *filename = NULL;
    int append = 0;
    int redir_type = find_redirection_operator(args, argsc, &filename, &append);
    
    if (redir_type == 0){
        ///No redirection found shoudlnt really happen
        launch_program(args, argsc);
        return;
    }
    
    pid_t pid = fork();
    
    if (pid < 0){
        perror("fork failed");
        exit(1);
    } else if (pid == 0) { ///Child process
        if (redir_type == 1){
            child_with_output_redirected(args, argsc, filename, append);
        } else if (redir_type == 2){
            child_with_input_redirected(args, argsc, filename);
        }
    } else{ 
        int status;
        waitpid(pid, &status, 0);
    }
}