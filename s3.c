#include "s3.h"
#include "lexer_fsm.h"
#include "jobs.h" 

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

// void read_command_line(char line[], char lwd[]){
//     char shell_prompt[MAX_PROMPT_LEN];
//     construct_shell_prompt(shell_prompt);
//     printf("%s", shell_prompt);

//     if (fgets(line, MAX_LINE, stdin) == NULL) {
//         printf("\n");
//         exit(0); 
//     }
//     line[strcspn(line, "\n")] = '\0';
// }

void trim_whitespace(char line[]){
    char *start = line;
    char *end;
    while (*start && isspace((unsigned char)*start)) start++;
    if (*start == '\0'){ 
        line[0] = '\0'; return; 
    }
    end = start + strlen(start) - 1;

    while (end > start && isspace((unsigned char)*end)){
        end--;
    }
    *(end + 1) = '\0';

    if (start != line){
        memmove(line, start, (end - start) + 2);
    }
}

void globbing(char *args[], int argsc){
    if (argsc > 1){
        glob_t globbuf;
        glob(args[1], 0, NULL, &globbuf);
        execvp(args[0], globbuf.gl_pathv);
    }
}

void parse_command(char line[],  char *args[], int *argsc, int *is_bg){
    Token *tokens =  NULL;
    int token_count = 0;
    *is_bg = 0; 
    
    if (lexer_tokenize_all(line, &tokens, &token_count) < 0){
        *argsc = 0;
        args[0] = NULL;
        return;
    }
    
    *argsc = 0;
    for (int i = 0; i < token_count && *argsc < MAX_ARGS; i++){
        if (tokens[i].type == TOKEN_WORD){
            args[*argsc] = strdup(tokens[i].value);
            (*argsc)++;
        } 
        else if (tokens[i].type  == TOKEN_AMPERSAND){
            *is_bg = 1;
            break; 
        }
        else if (tokens[i].type == TOKEN_REDIR_OUT ||
                   tokens[i].type == TOKEN_REDIR_APP ||
                   tokens[i].type == TOKEN_REDIR_IN){
            break;
        } else if (tokens[i].type == TOKEN_PIPE){
            break;
        }
    }
    args[*argsc] = NULL;
    lexer_free_tokens(tokens, token_count);
}

// Used for pipeline stages 
int find_redirection_operator(char *args[], int argsc, char **filename, int *append){
    *append = 0;
    *filename = NULL;
    char line[MAX_LINE] = "";
    for (int i = 0; i < argsc; i++){
        if (i > 0)  strcat(line, " ");
        strcat(line, args[i]);
    }
    Token *tokens = NULL;
    int token_count = 0;
    if (lexer_tokenize_all(line,  &tokens, &token_count) < 0){ 
        return 0;
    }
    int result = 0;
    for (int i = 0; i < token_count; i++){
        if (tokens[i].type ==  TOKEN_REDIR_APP){
            if (i + 1 < token_count && tokens[i+1].type ==  TOKEN_WORD) { 
                *filename = strdup(tokens[i + 1].value);
                *append = 1; result = 1; break;
            }
        } else if (tokens[i].type == TOKEN_REDIR_OUT){
            if (i + 1 < token_count && tokens[i+1].type == TOKEN_WORD){
                *filename = strdup(tokens[i + 1].value);
                result = 1; break;
            }
        } else if (tokens[i].type == TOKEN_REDIR_IN){
            if (i + 1 < token_count && tokens[i+1].type == TOKEN_WORD){
                *filename = strdup(tokens[i + 1].value);
                result = 2; break;
            }
        }
    }
    lexer_free_tokens(tokens, token_count);
    return result;
}

int find_redirection_in_line(const char *line, char **filename, int *append) {
    Token *tokens = NULL;
    int token_count = 0;
    *append = 0;
    *filename = NULL;
    
    if (lexer_tokenize_all(line, &tokens, &token_count) < 0){
        return 0;
    }
    
    int result = 0;
    for (int i = 0; i < token_count; i++){
        if (tokens[i].type == TOKEN_REDIR_APP){ 
            if (i + 1 < token_count && tokens[i +1].type == TOKEN_WORD){
                *filename = strdup(tokens[i + 1].value);
                *append = 1; 
                result = 1; 
                break;
            }
        } else if (tokens[i].type == TOKEN_REDIR_OUT){
            if (i + 1 < token_count && tokens[i+ 1].type == TOKEN_WORD){
                *filename = strdup(tokens[i + 1].value);
                result = 1; 
                break;
            }
        } else if (tokens[i].type == TOKEN_REDIR_IN){
            if (i + 1 < token_count && tokens[i+1].type == TOKEN_WORD) {
                *filename = strdup(tokens[i +1].value);
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
    if (lexer_tokenize_all(line, &tokens, &token_count) < 0) {
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
    if (lexer_tokenize_all(line, &tokens, &token_count) < 0) { 
        *count = 0; 
        return 0; 
    }
    
    static char stage_bufs[MAX_ARGS][MAX_LINE];
    *count = 0;
    int stage_start = 0;
    
    for (int i = 0; i <= token_count; i++){
        if (i == token_count || tokens[i].type == TOKEN_PIPE){
            if (*count >= MAX_ARGS){
                break;
            } 
            stage_bufs[*count][0] = '\0'; 
            for (int j = stage_start; j < i; j++) {
                if (tokens[j].type == TOKEN_WORD){
                    if (j > stage_start){
                        strcat(stage_bufs[*count], " ");
                    }
                    strcat(stage_bufs[*count], tokens[j].value);
                }else if (tokens[j].type == TOKEN_REDIR_OUT){ 
                    strcat(stage_bufs[*count], " > ");
                } else if (tokens[j].type == TOKEN_REDIR_APP) { 
                    strcat(stage_bufs[*count], " >> ");
                } else if (tokens[j].type == TOKEN_REDIR_IN){ 
                    strcat(stage_bufs[*count], " < "); 
                }
                // ignore ampersand in pipe reconstruction
            }
            stages[*count] = stage_bufs[*count];
            (*count)++;
            stage_start = i + 1;
        }
    }
    lexer_free_tokens(tokens, token_count);
    return *count > 0;
}

void run_cd(char *args[], int argsc, char lwd[]){
    char cwd[MAX_LINE];
    if (args[1] != NULL && strcmp(args[1], "-") == 0){
        if (getcwd(cwd, sizeof(cwd)) == NULL){
            perror("getcwd() error");
        }
        if (chdir(lwd) != 0){
            perror("cd failed");
        }
        else strcpy(lwd, cwd);
    } else{
        if (getcwd(cwd, sizeof(cwd)) != NULL){
             strcpy(lwd, cwd);
        }
        else perror("getcwd() error");

        if (args[1] == NULL){
            const char *home = getenv("HOME");
            if (!home){
                home = "/";
            }
            if (chdir(home) != 0){ 
                perror("cd failed");
            }
        } else if (chdir(args[1]) != 0) perror("cd failed");
    }
}

int is_cd(char line[]){
    char temp[MAX_PROMPT_LEN];
    strcpy(temp, line);
    char *token = strtok(temp, " ");
    if (token != NULL && strcmp(token, "cd") == 0) {
        return 1;
    }
    return 0;
}

int is_subshell(char line[]){
    size_t len = strlen(line);
    if (line[0] == '(' && line[len - 1] == ')') {
        return 1;
    }
    return 0;
}

void run_subshell(char *token, char *lwd){
    int is_bg = 0;
    size_t len = strlen(token);
    if (token[len-1] == '&'){
        is_bg = 1;
        token[len-1] = '\0';
    }
    
    pid_t pid = fork();

    if (pid < 0){
        perror("fork failed");
        return;
    } 
    else if (pid == 0){ 
        setpgid(0, 0); 
        if (!is_bg){
             tcsetpgrp (STDIN_FILENO, getpid()); 
             signal(SIGINT, SIG_DFL);
             signal(SIGTSTP, SIG_DFL) ;
        }

        token = token + 1;
        len = strlen(token);
        if (len > 0 && token[len - 1] == ')'){
            token[len - 1] = '\0';
        }
        process_input(token, lwd);
        exit(0); 
    } 
    else{
        setpgid(pid, pid); 
        if (!is_bg){
            wait_for_job(pid, token);
            tcsetpgrp(STDIN_FILENO, getpid()); 
        } else{
            // capture return value instead of using max_job_id
            int jid = add_job(pid, JOB_RUNNING, token);
            printf("[%d] %d\n", jid, pid);
        }
    }
}

void child(char *args[], int argsc){
    globbing(args, argsc);
    execvp(args[0], args);
    perror("execvp failed");
    exit(1);
}

void child_with_output_redirected(char *args[], int argsc, char *output_file, int append){
    int fd;
    if (append) fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
    else fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    
    if (fd < 0){ 
        perror("open failed"); exit(1); 
    }
    if (dup2(fd, STDOUT_FILENO) < 0){ 
        perror("dup2 failed"); close(fd); exit(1);
    }
    close(fd);
    
    globbing(args, argsc);
    execvp(args[0], args);
    perror("execvp failed");
    exit(1);
}

void child_with_input_redirected(char *args[], int argsc, char *input_file){
    int fd = open(input_file, O_RDONLY);
    if (fd < 0){ 
        perror("open failed"); 
        exit(1); 
    }
    if (dup2(fd, STDIN_FILENO) < 0){
         perror("dup2 failed"); close(fd); exit(1);
    }
    close(fd); 

    globbing(args, argsc);
    execvp(args[0], args);
    perror("execvp failed");
    exit(1);
}

//not used as much now
void launch_program(char *args[], int argsc){
    if (argsc > 0 && strcmp(args[0], "exit") == 0){
        printf("Exiting shell.\n");
        exit(0);
    }
    pid_t pid = fork();
    if (pid < 0) { 
        perror("fork"); 
        return;
     }
    else if (pid == 0) { 
        child(args, argsc); 
    }
    else { waitpid(pid, NULL, 0); 
    }
}

void launch_program_with_redirection(char *args[], int argsc, const char *original_line, int is_bg){
     if (argsc > 0 && strcmp(args[0], "exit") == 0) {
         printf("Exiting shell.\n");
         exit(0);
     }
     
     char *filename = NULL;
     int append = 0;
     int redir_type = find_redirection_in_line(original_line, &filename, &append);
     
     pid_t pid = fork();
     
     if (pid < 0){
         perror("fork failed");
         return;
     }
     else if (pid == 0){
         setpgid(0, 0);
         if (!is_bg) {
             tcsetpgrp(STDIN_FILENO, getpid());
         }
         
         signal(SIGINT, SIG_DFL);
         signal(SIGTSTP, SIG_DFL);
         signal(SIGTTOU, SIG_DFL);

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
     else {
         setpgid(pid, pid); 

         if (!is_bg) {
             wait_for_job(pid, original_line); 
             tcsetpgrp(STDIN_FILENO, getpid()); 
         } else {
             int jid = add_job(pid, JOB_RUNNING, original_line);
             printf("[%d] %d\n", jid, pid);
         }
     }
 }

void launch_pipeline(char *stages[], int n, int is_bg){
    int in_fd = -1;
    int pfd[2];
    pid_t group_id = 0; 

    for (int i = 0; i < n; i++)
     {
         int is_last = (i == n - 1);
         if (!is_last){
             if (pipe(pfd) < 0){ perror("pipe failed"); exit(1); }
         }

         pid_t pid = fork();

         if (pid < 0){
             perror("fork failed");
             exit(1);
         }
         else if (pid == 0)
         {
            if (i == 0) setpgid(0, 0); 
            else setpgid(0, group_id); 
            
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            if (in_fd != -1){ dup2(in_fd, STDIN_FILENO); close(in_fd); }
            if (!is_last){ close(pfd[0]); dup2(pfd[1], STDOUT_FILENO); close(pfd[1]); }

            char buf[MAX_LINE];
            strncpy(buf, stages[i], sizeof(buf));
            buf[sizeof(buf)-1] = '\0';
            char *args[MAX_ARGS];
            int argsc, bg_dummy; 
            parse_command(buf, args, &argsc, &bg_dummy);

            char *fname = NULL;
            int append = 0;
            // Note: find_redirection_operator uses its own tokenizer logic
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

        if (i == 0) group_id = pid;
        setpgid(pid, group_id); 

        if (in_fd != -1) close(in_fd);
        if (!is_last){ close(pfd[1]); in_fd = pfd[0]; }
     }

    if (in_fd != -1) close(in_fd);

    if (!is_bg) {
        tcsetpgrp(STDIN_FILENO, group_id); 
        wait_for_job(group_id, "pipeline"); 
        tcsetpgrp(STDIN_FILENO, getpid()); 
    } else {
        int jid = add_job(group_id, JOB_RUNNING, "pipeline");
        printf("[%d] %d\n", jid, group_id);
    }
}

void process_input(char line[], char *lwd){
    char *args[MAX_ARGS];
    int argsc;
    int is_bg = 0;
    
    char *start = line;
    int len = strlen(line);
    int parenthesis_count = 0;

    for (int i = 0; i <= len; i++){
        if (line[i] == '(') parenthesis_count++;
        else if (line[i] == ')') parenthesis_count--;

        if ((line[i] == ';' && parenthesis_count == 0) || line[i] == '\0'){
            line[i] = '\0';
            char *token = start;
            trim_whitespace(token);

            if (strlen(token) > 0) {
                if (is_subshell(token)){
                    run_subshell(token, lwd);
                }
                else if (is_cd(token)) {
                    parse_command(token, args, &argsc, &is_bg);
                    run_cd(args, argsc, lwd);
                } 
                else if (strncmp(token, "jobs", 4) == 0) {
                     print_jobs();
                }
                else if (strncmp(token, "fg", 2) == 0) {
                    parse_command(token, args, &argsc, &is_bg);
                    if (argsc > 1) {
                        int jid = atoi(args[1]);
                        Job *j = find_job_by_id(jid);
                        if (j) put_job_in_foreground(j, 1);
                        else printf("Job [%d] not found\n", jid);
                    }
                }
                else if (strncmp(token, "bg", 2) == 0) {
                    parse_command(token, args, &argsc, &is_bg);
                    if (argsc > 1) {
                        int jid = atoi(args[1]);
                        Job *j = find_job_by_id(jid);
                        if (j) put_job_in_background(j, 1);
                        else printf("Job [%d] not found\n", jid);
                    }
                }
                else if (command_has_pipes(token)){
                    int pipe_bg = 0;
                    size_t tlen = strlen(token);
                    if (token[tlen-1] == '&') { pipe_bg = 1; token[tlen-1] = '\0'; }

                    char line_copy[MAX_LINE];
                    strncpy(line_copy, token, sizeof(line_copy));
                    
                    char *stages[MAX_ARGS];
                    int n = 0;
                    split_pipeline(line_copy, stages, &n);
                    if (n > 0) launch_pipeline(stages, n, pipe_bg);
                }
                else {
                    parse_command(token, args, &argsc, &is_bg);
                    if (argsc > 0){
                        launch_program_with_redirection(args, argsc, token, is_bg);
                    }
                }
            }
            start = &line[i+1];
        }
    }
}

void read_command_line(char line[], size_t length){
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);
    fflush(stdout);
    enable_raw_mode();

    size_t pos = 0;
    size_t cursor = 0;

    while (1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) <= 0)
            break;

        if (c == '\n') {
            line[pos] = '\0';
            write(STDOUT_FILENO, "\n", 1);
            break;
        }

        else if (c == '\t') {
            // TODO: call your tab-completion logic
            continue;
        }

        else if (c == 127) {  // Backspace
            if (cursor > 0) {
                // Remove character at cursor-1
                memmove(&line[cursor - 1], &line[cursor], pos - cursor);
                cursor--;
                pos--;

                // Redraw line
                printf("\r\033[K");
                printf("%s", shell_prompt);
                fwrite(line, 1, pos, stdout);

                // Move cursor to correct position
                move_cursor_left(pos - cursor);
            }
        }

        else if (c == 0x1B) { // Escape sequence
            char seq[2];
            if (read(STDIN_FILENO, &seq, 2) == 2) {
                if (seq[0] == '[') {
                    if (seq[1] == 'D') {  // Left arrow
                        if (cursor > 0) {
                            move_cursor_left(1);
                            cursor--;
                        }
                    }
                    else if (seq[1] == 'C') { // Right arrow
                        if (cursor < pos) {
                            move_cursor_right(1);
                            cursor++;
                        }
                    }
                }
            }
        }

        else {
            if (pos < length - 1) {
                // Insert character at cursor
                memmove(&line[cursor + 1], &line[cursor], pos - cursor);
                line[cursor] = c;
                pos++;
                cursor++;

                // Redraw line
                printf("\r\033[K");
                printf("%s", shell_prompt);
                fwrite(line, 1, pos, stdout);

                // Move cursor to correct position
                move_cursor_left(pos - cursor);
            }
        }
    }

    disable_raw_mode();
}