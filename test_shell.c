#define _XOPEN_SOURCE 600
#include <pty.h> 
//STD needed for forkpty, 
//pty == psudeo terminal 

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void write_and_flush(int fd, const char *msg){
    write(fd, msg, strlen(msg));
    fsync(fd); //fsync ensures the files data is properly written to disk
}

int main() {
    int master_fd;
    pid_t pid = forkpty(&master_fd, NULL, NULL, NULL);

    if (pid < 0){
        perror("forkpty");
        exit(1);
    }

    // Child runs the shell
    //execl over execvp because we know the exact path, 
    //no need to search path using vp
    if (pid == 0){
        execl("./s3test", "./s3test", NULL);
        perror("execl");
        exit(1);
    }

    // Parent is the test harness
    //clean the buffer to prevent leftover rubbish and prevent leaks 
    char buffer[4096];
    for (int i = 0; i < 4096; i++){
        buffer[i] = 0;
    }

   // test sends command to shell 
   //Master PTY -> sees as user input
   // sleep, give shell time to run
    write_and_flush(master_fd, "echo hi | wc -c\n");
    sleep(1); 

    //read the output 
    int n = read(master_fd, buffer, sizeof(buffer)-1); 
    buffer[n] = '\0';

    printf("Shell output:\n%s\n", buffer);

    // Check expected result using strstr ofc substring in a string
    if (strstr(buffer, "3") != NULL){
        printf("TEST PASS\n");
    } else{
        printf("TEST FAIL\n");
    }

    //Signal send to kill the process if needed - Nuke, can't be ignored.
    // Kill means send signal.
    kill(pid, SIGKILL);
    return 0;
}
