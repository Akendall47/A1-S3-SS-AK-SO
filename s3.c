#include "s3.h"

///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[])
{
    char cwd[MAX_LINE];

    if (getcwd(cwd, sizeof(cwd)) == NULL){
        perror("getcwd() error");
        exit(1);
    }

    strcpy(shell_prompt, "[");       // start with '['
    strcat(shell_prompt, cwd);       // add current working directory
    strcat(shell_prompt, " ");       // add a space
    strcat(shell_prompt, "s3");     // add your custom name
    strcat(shell_prompt, "]$ "); 
    
    // strcpy(shell_prompt, "[s3]$ ");
}

void init_lwd(char lwd[]){
    char cwd[MAX_LINE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
       strcpy(lwd, cwd);
   } else {
       perror("getcwd() error");
   }
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[], char lwd[])
{
    //Note note sure why we need lwd
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

void run_cd(char *args[], int argsc, char lwd[]){
    char cwd[MAX_LINE];

    if (args[1] != NULL && strcmp(args[1], "-") == 0){
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd() error");
        }

        if (chdir(lwd) != 0){
            perror("cd failed");
        }else{
            strcpy(lwd, cwd);
        }
    }

    else{
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            strcpy(lwd, cwd);
        } else {
            perror("getcwd() error");
        }

        if (args[1] == NULL){
            if (chdir("/home") != 0){
                perror("cd failed");
            }
        }else if (chdir(args[1]) != 0){
            perror("cd failed");
        }
}
    
}

int is_cd(char line[]){
    char temp[MAX_PROMPT_LEN];
    strcpy(temp, line);
    char *token = strtok(temp, " "); // This modifies the original line so I've made a copy so it modifies a temp value
    if (token != NULL && strcmp(token, "cd") == 0){
        return 1;
    }
    return 0;
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
int find_redirection_operator(char *args[], int argsc, char **filename, int *append){
    *append = 0;
    
    for (int i = 0; i < argsc; i++){
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
        }else if (strcmp(args[i], "<") == 0){
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

    //Debug if needed - print args to see what excvp is called with
    // for (int i = 0; args[i] != NULL; ++i) {
    //     fprintf(stderr, "execvp will run: arg[%d] = '%s'\n", i, args[i]);
    // }

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

void child_with_input_redirected(char *args[], int argsc, char *input_file){
    int fd;
    
    // Open file for reading
    fd = open(input_file, O_RDONLY);
    
    if (fd < 0){
        perror("open failed");
        exit(1);
    }
    
    // redirect stdin from the file
    if (dup2(fd, STDIN_FILENO) < 0){
        perror("dup2 failed");
        close(fd);
        exit(1);
    }
    
    close(fd); 

    execvp(args[0], args);
    perror("execvp failed");
    exit(1);
}

//obviously using strchr to  find | pipe
int command_has_pipes(char line[]){
    return strchr(line, '|') != NULL;
}


static char *trim(char *s){
//////// an efificiency thing - as parse a lot of text
// only which part of memory we return changes 
//remove leading spaces or tabs from string in-place
// eg " hello " -> "hello"
//
//loop moves ptr s forward until first non-space char
// chekc if str empty "  " -> ""
// move ptr e back while trailing spaces are found
    while (*s==' '||*s=='\t') s++;
    if (*s==0) return s;
    char *e = s + strlen(s) - 1;
    while (e>s && (*e==' '||*e=='\t')) *e-- = '\0';
    return s;
}

//split into indivudal stages
//strtok split at every | ---> store in stages 
int split_pipeline(char *line, char *stages[], int *count){
    *count = 0;
    char *p = strtok(line, "|");
    while (p && *count < MAX_ARGS) {
        stages[(*count)++] = trim(p);
        p = strtok(NULL, "|");
    }
    return *count > 0;
}

///
// Launches return PID so main can Reap - more control/flexible - and readable
///
pid_t launch_pipeline(char *stages[], int n)
 {
    int in_fd = -1;     // read end of previous pipe
    int pfd[2];         // pipe for current stage
    pid_t pids[MAX_ARGS];
    int count = 0;      // how many children created

    for (int i = 0; i < n; i++)
     {
         int is_last = (i == n - 1);
         if (!is_last)
         {
             if (pipe(pfd) < 0)
             {
                 perror("pipe failed");
                 exit(1);
             }
         }

         pid_t pid = fork();

         if (pid < 0)
         {
             perror("fork failed");
             exit(1);
         }
         else if (pid == 0)
         {
            ///CHILD
            //if Not first: then stdin <== from previous pipe
            if (in_fd != -1)
            {
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            ///if not last: stdout ==> pipe write-end ie next pipe
            if (!is_last)
            {
                close(pfd[0]);              // close read end
                dup2(pfd[1], STDOUT_FILENO);
                close(pfd[1]);
            }
//
// buf used as a fixed array to hold copy of the command - one pipleine stage 
// with strncpy copying up to sizeof buf from stages[] into buf
//strncpy has no null termintor so must be done manually
            char buf[MAX_LINE];
            strncpy(buf, stages[i], sizeof(buf));
            buf[sizeof(buf)-1] = '\0';

            char *args[MAX_ARGS];
            int argsc;
            parse_command(buf, args, &argsc);

            ///Check redirection 
            char *fname = NULL;
            int append = 0;
            int redir_type = find_redirection_operator(args, argsc, &fname, &append);


            if (redir_type == 2 && i == 0)
            {
                child_with_input_redirected(args, argsc, fname);
            }
            else if (redir_type == 1 && is_last)
            {
                child_with_output_redirected(args, argsc, fname, append);
            }
            else if (redir_type != 0)
            {
                fprintf(stderr, "Redirection not allowed in middle of pipeline\n");
                _exit(1);
            }
            else
            {
                child(args, argsc);
            }
         }

        ///PARENT
        pids[count++] = pid;   // record pid

        if (in_fd != -1)  // close previous pipe read end
            close(in_fd);
        if (!is_last)  // if not the last stage, move pipe read end forward
        {
            close(pfd[1]);     // close write end
            in_fd = pfd[0];    // save read end for next stage
        }
     }
    // close leftover read end in parent
    if (in_fd != -1) close(in_fd);

    // wait for all children and return last child's pid (or -1 if none)
    pid_t last = -1;
    for (int i = 0; i < count; ++i) {
        int status;
        waitpid(pids[i], &status, 0);
        last = pids[i];
    }
    return last;
} 

pid_t launch_program(char *args[], int argsc)
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
        return -1;     // parent returns error; main() should not reap
    }
    else if (pid == 0)  //child
    {   
        //find a programme called eg ls and run seperatley
        child(args, argsc);       // execvp never returns on success
    }
    // parent does NOT wait here anymore;
    // main() will call waitpid(pid, ...)
    return pid;
}

pid_t launch_program_with_redirection(char *args[], int argsc) {
     ///Handle exit before fork
     if (argsc > 0 && strcmp(args[0], "exit") == 0) {
         printf("Exiting shell.\n");
         exit(0);
     }
     
     char *filename = NULL;
     int append = 0;
     int redir_type = find_redirection_operator(args, argsc, &filename, &append);
     
     if (redir_type == 0){
        ///No redirection found: fall back to normal launcher and return its pid
        return launch_program(args, argsc);
     }
     
     pid_t pid = fork();
     
     if (pid < 0)
     {
         perror("fork failed");
         return -1;     // parent returns error; main() must not reap this
     }
     else if (pid == 0)   ///child
     {
         if (redir_type == 2)
         {
             child_with_input_redirected(args, argsc, filename);
         }
         else if (redir_type == 1)
         {
             child_with_output_redirected(args, argsc, filename, append);
         }
         else
         {
             child(args, argsc);
         }
     }
 
     return pid;
 }