#include "s3.h"
#include "lexer_fsm.h"

void construct_shell_prompt(char shell_prompt[]){
    char cwd[MAX_LINE];

    if (getcwd(cwd, sizeof(cwd)) == NULL){
        perror("getcwd() error");
        exit(1);
    }

    strcpy(shell_prompt, "[");       
    strcat(shell_prompt, cwd);       
    strcat(shell_prompt, " ");       
    strcat(shell_prompt, "s3");     
    strcat(shell_prompt, "]$ "); 
}

void init_lwd(char lwd[]){
    char cwd[MAX_LINE];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
       strcpy(lwd, cwd);
   } else {
       perror("getcwd() error");
   }
}

void read_command_line(char line[], char lwd[]){
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);

    if (fgets(line, MAX_LINE, stdin) == NULL) {
        perror("fgets failed");
        exit(1);
    }

    line[strcspn(line, "\n")] = '\0';
}

void run_cd(char *args[], int argsc, char lwd[]){
    char cwd[MAX_LINE];

    if (args[1] != NULL && strcmp(args[1], "-") == 0){
        if (getcwd(cwd, sizeof(cwd)) == NULL){
            perror("getcwd() error");
        }

        if (chdir(lwd) != 0){
            perror("cd failed");
        }else{
            strcpy(lwd, cwd);
        }
    }
    else {
        if (getcwd(cwd, sizeof(cwd)) != NULL){
            strcpy(lwd, cwd);
        } else{
            perror("getcwd() error");
        }

        if (args[1] == NULL){
            const char *home = getenv("HOME");
            if (!home) home = "/";

            if (chdir(home) != 0){
                perror("cd failed");
            }
        }
        else if (chdir(args[1]) != 0){
            perror("cd failed");
        }
    }
}

int is_cd(char line[]){
    char temp[MAX_PROMPT_LEN];
    strcpy(temp, line);
    char *token = strtok(temp, " ");
    if (token != NULL && strcmp(token, "cd") == 0){
        return 1;
    }
    return 0;
}

int is_subshell(char line[]){
    size_t len = strlen(line);
    if (line[0] == '(' && line[len - 1] == ')'){
        return 1;
    }
    return 0;
}


void run_subshell(char *token, char *lwd){
    pid_t pid = fork();

    if (pid < 0){
        perror("fork failed");
        exit(1);
    } 
    else if (pid == 0) { 
        token = token + 1;
        size_t len = strlen(token);
        if (len > 0 && token[len - 1] == ')'){
            token[len - 1] = '\0';
        }
        process_input(token, lwd);
        exit(0); 
    } 
    else{
        int status;
        waitpid(pid, &status, 0);
    }
}


void trim_whitespace(char line[]){
    char *start = line;
    char *end;

    while (*start && isspace((unsigned char)*start))
        start++;

    if (*start == '\0') {
        line[0] = '\0';
        return;
    }

    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end))
        end--;

    *(end + 1) = '\0';

    if (start != line)
        memmove(line, start, (end - start) + 2);
}


void parse_command(char line[], char *args[], int *argsc){
    Token *tokens = NULL;
    int token_count = 0;
    
    //uses FSM lexer instead of your string manipulation
    if (lexer_tokenize_all(line, &tokens, &token_count) < 0){
        *argsc = 0;
        args[0] = NULL;
        return;
    }
    
    //Extract words into args array
    *argsc = 0;
    for (int i = 0; i < token_count && *argsc < MAX_ARGS; i++){
        if (tokens[i].type == TOKEN_WORD) {
            args[*argsc] = strdup(tokens[i].value); // Copy so we can free tokens
            (*argsc)++;
        } else if (tokens[i].type == TOKEN_REDIR_OUT ||
                   tokens[i].type == TOKEN_REDIR_APP ||
                   tokens[i].type == TOKEN_REDIR_IN){
            //stop at redirection operators
            break;
        } else if (tokens[i].type == TOKEN_PIPE){
            //Stop at pipe
            break;
        }
    }
    
    args[*argsc] = NULL; //null terminate for execvp
    
    lexer_free_tokens(tokens, token_count);
}

//used for pipeline stages 
int find_redirection_operator(char *args[], int argsc, char **filename, int *append){
    *append = 0;
    *filename = NULL;
    
    // Reconstruct line from args to tokenize
    char line[MAX_LINE] = "";
    for (int i = 0; i < argsc; i++){
        if (i > 0) strcat(line, " ");
        strcat(line, args[i]);
    }
    
    Token *tokens = NULL;
    int token_count = 0;
    
    if (lexer_tokenize_all(line, &tokens, &token_count) < 0){
        return 0;
    }
    
    //Find >> or > or <
    int result = 0;
    for (int i = 0; i < token_count; i++){
        if (tokens[i].type == TOKEN_REDIR_APP){
            if (i + 1 < token_count && tokens[i+1].type == TOKEN_WORD){
                *filename = strdup(tokens[i + 1].value);
                *append = 1;
                result = 1;
                break;
            }
        } else if (tokens[i].type == TOKEN_REDIR_OUT){
            if (i + 1 < token_count && tokens[i+1].type == TOKEN_WORD){
                *filename = strdup(tokens[i + 1].value);
                result = 1;
                break;
            }
        } else if (tokens[i].type == TOKEN_REDIR_IN){
            if (i + 1 < token_count && tokens[i+1].type == TOKEN_WORD){
                *filename = strdup(tokens[i + 1].value);
                result = 2;
                break;
            }
        }
    }
    
    lexer_free_tokens(tokens, token_count);
    return result;
}

int find_redirection_in_line(const char *line, char **filename, int *append){
    Token *tokens = NULL;
    int token_count = 0;
    
    *append = 0;
    *filename = NULL;
    
    if (lexer_tokenize_all(line, &tokens, &token_count) < 0){
        return 0;
    }
    
    //Find >> or > or <
    int result = 0;
    for (int i = 0; i < token_count; i++){
        if (tokens[i].type == TOKEN_REDIR_APP){ 
            if (i + 1 < token_count && tokens[i+1].type == TOKEN_WORD){
                *filename = strdup(tokens[i + 1].value);
                *append = 1;
                result = 1; 
                break;
            }
        } else if (tokens[i].type == TOKEN_REDIR_OUT) {
            if (i + 1 < token_count && tokens[i+1].type == TOKEN_WORD){
                *filename = strdup(tokens[i + 1].value);
                result = 1;
                break;
            }
        } else if (tokens[i].type == TOKEN_REDIR_IN){
            if (i + 1 < token_count && tokens[i+1].type == TOKEN_WORD){
                *filename = strdup(tokens[i + 1].value);
                result = 2;
                break;
            }
        }
    }
    
    lexer_free_tokens(tokens, token_count);
    return result;
}

int command_has_pipes(char line[]){
    Token *tokens = NULL;
    int token_count = 0;
    
    if (lexer_tokenize_all(line, &tokens, &token_count) < 0){
        return 0;
    }
    
    int has_pipe = 0;
    for (int i = 0; i < token_count; i++){
        if (tokens[i].type == TOKEN_PIPE){
            has_pipe = 1;
            break;
        }
    }
    
    lexer_free_tokens(tokens, token_count);
    return has_pipe;
}

int split_pipeline(char *line, char *stages[], int *count){
    Token *tokens = NULL;
    int token_count = 0;
    
    if (lexer_tokenize_all(line, &tokens, &token_count) < 0){
        *count = 0;
        return 0;
    }
    
    //Split at pipe operators
    static char stage_bufs[MAX_ARGS][MAX_LINE];
    *count = 0;
    int stage_start = 0;
    
    for (int i = 0; i <= token_count; i++) {
        if (i == token_count || tokens[i].type == TOKEN_PIPE){
            if (*count >= MAX_ARGS) break;
            
            //  reconstruct stage from tokens
            stage_bufs[*count][0] = '\0';
            for (int j = stage_start; j < i; j++){
                if (tokens[j].type == TOKEN_WORD){
                    if (j > stage_start) strcat(stage_bufs[*count], " ");
                    strcat(stage_bufs[*count], tokens[j].value);
                } else if (tokens[j].type == TOKEN_REDIR_OUT){
                    strcat(stage_bufs[*count], " > ");
                } else if (tokens[j].type == TOKEN_REDIR_APP) {
                    strcat(stage_bufs[*count], " >> ");
                } else if (tokens[j].type == TOKEN_REDIR_IN){
                    strcat(stage_bufs[*count], " < ");
                }
            }
            
            stages[*count] = stage_bufs[*count];
            (*count)++;
            stage_start = i + 1;
        }
    }
    
    lexer_free_tokens(tokens, token_count);
    return *count > 0;
}

void process_input(char line[], char *lwd){
    char *args[MAX_ARGS];
    int argsc;
    
    char *start = line;
    int len = strlen(line);
    int parenthesis_count = 0;

    //loop through looking for semicolons and parentheses
    for (int i = 0; i <= len; i++){
        if (line[i] == '('){
            parenthesis_count++;
        }
        else if (line[i] == ')'){
            parenthesis_count--;
        }

        if ((line[i] == ';' && parenthesis_count == 0) || line[i] == '\0'){
            line[i] = '\0';

            char *token = start;
            trim_whitespace(token);

            if (strlen(token) > 0) {
                argsc = 0;

                if (is_subshell(token)){
                    run_subshell(token, lwd);
                }
                else if (is_cd(token)) {
                    parse_command(token, args, &argsc);
                    run_cd(args, argsc, lwd);
                } 

                else if (command_has_pipes(token)){
                    char line_copy[MAX_LINE];
                    strncpy(line_copy, token, sizeof(line_copy));
                    line_copy[sizeof(line_copy)-1] = '\0';

                    char *stages[MAX_ARGS];
                    int n = 0;

                    split_pipeline(line_copy, stages, &n);

                    if (n > 0){
                        launch_pipeline(stages, n);
                    }
                }

                else {
                    parse_command(token, args, &argsc);
                    if (argsc == 0){
                        continue;
                    }
                    // pass original token so redirection can be detected
                    pid_t pid = launch_program_with_redirection(args, argsc, token);
                    reap(pid);
                }
            }

            start = &line[i+1];
        }
    }
}

void child(char *args[], int argsc)
{
    execvp(args[0], args);
    perror("execvp failed");
    exit(1);
}

void child_with_output_redirected(char *args[], int argsc, char *output_file, int append){
    int fd;
    
    //open file and set flags as needed
    // 0644: 3 permission groups: owner R/W, everyone else just 
    if (append){
        fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    } else{
        fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    
    if (fd < 0){
        perror("open failed");
        exit(1);
    }
    
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
    
    fd = open(input_file, O_RDONLY);
    
    if (fd < 0){
        perror("open failed");
        exit(1);
    }
    
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

void launch_program(char *args[], int argsc)
{
    if (argsc > 0 && strcmp(args[0], "exit") == 0){
        printf("Exiting shell.\n");
        exit(0);
    }

    pid_t pid = fork();

    if (pid < 0){
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0){   
        child(args, argsc);
    }
    else{
        int status;
        waitpid(pid, &status, 0);
    }
}

// now takes original line to properly detect redirection
pid_t launch_program_with_redirection(char *args[], int argsc, const char *original_line){
     if (argsc > 0 && strcmp(args[0], "exit") == 0) {
         printf("Exiting shell.\n");
         exit(0);
     }
     
     char *filename = NULL;
     int append = 0;
     int redir_type = find_redirection_in_line(original_line, &filename, &append);
     
     if (redir_type == 0){
        launch_program(args, argsc);
        return 0;
     }
     
     pid_t pid = fork();
     
     if (pid < 0){
         perror("fork failed");
         return -1;
     }
     else if (pid == 0){
         if (redir_type == 2){
             child_with_input_redirected(args, argsc, filename);
         }
         else if (redir_type == 1){
             child_with_output_redirected(args, argsc, filename, append);
         }
         else{
             child(args, argsc);
         }
     }
 
     return pid;
 }

pid_t launch_pipeline(char *stages[], int n){
    int in_fd = -1;
    int pfd[2];
    pid_t pids[MAX_ARGS];
    int count = 0;

    for (int i = 0; i < n; i++)
     {
         int is_last = (i == n - 1);
         if (!is_last){
             if (pipe(pfd) < 0){
                 perror("pipe failed");
                 exit(1);
             }
         }

         pid_t pid = fork();

         if (pid < 0){
             perror("fork failed");
             exit(1);
         }
         else if (pid == 0)
         {
            if (in_fd != -1){
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            if (!is_last){
                close(pfd[0]);
                dup2(pfd[1], STDOUT_FILENO);
                close(pfd[1]);
            }

            char buf[MAX_LINE];
            strncpy(buf, stages[i], sizeof(buf));
            buf[sizeof(buf)-1] = '\0';

            char *args[MAX_ARGS];
            int argsc;
            parse_command(buf, args, &argsc);

            char *fname = NULL;
            int append = 0;
            int redir_type = find_redirection_operator(args, argsc, &fname, &append);

            if (redir_type == 2 && i == 0){
                child_with_input_redirected(args, argsc, fname);
            }
            else if (redir_type == 1 && is_last){
                child_with_output_redirected(args, argsc, fname, append);
            }
            else if (redir_type != 0){
                fprintf(stderr, "Redirection not allowed in middle of pipeline\n");
                _exit(1);
            }
            else{
                child(args, argsc);
            }
         }

        pids[count++] = pid;

        if (in_fd != -1){
            close(in_fd);
        }
        if (!is_last){
            close(pfd[1]);
            in_fd = pfd[0];
        }
     }

    if (in_fd != -1){
        close(in_fd);
    }

    pid_t last = -1;
    for (int i = 0; i < count; ++i){
        int status;
        waitpid(pids[i], &status, 0);
        last = pids[i];
    }
    return last;
}