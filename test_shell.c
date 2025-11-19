#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_OUTPUT 8192

//forkpty() may eventually be neeeded to test Job control
// ie create pseudo terminal 

typedef struct {
    const char *input;
    const char *expected;
    const char *description;
} Test;

// Add as many tests as you want here
Test tests[] = {
    {"echo \"< | >\"", "< | >", "special chars in quotes"},
    {"echo 'hello > world'", "hello > world", "single quotes"},
    {"echo hi | wc -c", "3", "simple pipe"},
    {"echo hi > f1", "", "output redirection"},
    {"cat f1", "hi", "read redirected file"},
    {"echo first > f2", "", "create file"},
    {"echo second >> f2", "", "append to file"},
    {"cat f2", "second", "verify append worked"},
    {"echo \"a'b\" 'c\"d'", "a'b c\"d", "mixed quotes"},
    {"echo \"unterminated", "Error", "unterminated quote"},
    {"echo hi | tr a-z A-Z | tr -d O", "HI", "pipe chain"},
    {"echo X | wc -c > num", "", "pipe with redirect"},
    {"cat num", "2", "verify piped redirect"},
};

#define NUM_TESTS (sizeof(tests) / sizeof(tests[0]))

// Run shell with input, capture output
int run_shell_command(const char *input, char *output, size_t output_size){
    int pipefd[2];
    if (pipe(pipefd) == -1){
        perror("pipe");
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }

    if (pid == 0){
        // Child: run shell
        close(pipefd[0]); // Close read end
        
        // Redirect stdout and stderr to pipe
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        // Run shell and send command via stdin
        FILE *shell = popen("./s3", "w");
        if (!shell) {
            perror("popen");
            exit(1);
        }
        
        fprintf(shell, "%s\n", input);
        fprintf(shell, "exit\n");
        pclose(shell);
        exit(0);
    }

    // Parent: read output
    close(pipefd[1]); // Close write end

    size_t total = 0;
    ssize_t n;
    while (total < output_size - 1){
        n = read(pipefd[0], output + total, output_size - total - 1);
        if (n <= 0) break;
        total += n;
    }
    output[total] = '\0';

    close(pipefd[0]);

    // Wait for child
    int status;
    waitpid(pid, &status, 0);

    return 0;
}

int main() {
    char output[MAX_OUTPUT];
    int passed = 0;
    int failed = 0;
    printf(" Running %zu tests \n", NUM_TESTS);

    for (size_t i = 0; i < NUM_TESTS; i++){
        printf("[%zu/%zu] %s\n", i + 1, NUM_TESTS, tests[i].description);
        printf("      → %s\n", tests[i].input);

        if (run_shell_command(tests[i].input, output, sizeof(output)) == -1){
            printf("      FAIL - Could not run test\n\n");
            failed++;
            continue;
        }

        // Check if expected string is in output
        if (strstr(output, tests[i].expected)){
            printf("      PASS\n\n");
            passed++;
        } else{
            printf("      FAIL\n");
            printf("      Expected: \"%s\"\n", tests[i].expected);
            printf("      Got: \"%s\"\n\n", output);
            failed++;
        }
    }

    printf(" Results: %d/%zu passed, %d failed \n", 
           passed, NUM_TESTS, failed);

    // Cleanup test files
    system("rm -f f1 f2 num 2>/dev/null");
    return (failed == 0) ? 0 : 1;
}