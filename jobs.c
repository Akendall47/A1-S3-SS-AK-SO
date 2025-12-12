#include "jobs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

static Job job_table[MAX_JOBS];
static int max_job_id = 0;

void init_jobs(void){
    for (int i = 0; i < MAX_JOBS; i++){
        job_table[i].id = 0;
    }
}

int add_job(pid_t pgid, JobState state, const char *cmd){
    for (int i = 0; i < MAX_JOBS; i++){
        if (job_table[i].id == 0) {
            job_table[i].id = ++max_job_id;
            job_table[i].pgid =  pgid;
            job_table[i].state = state;
            strncpy (job_table[i].cmd, cmd, sizeof(job_table[i].cmd) - 1);
            job_table[i].cmd[sizeof(job_table[i].cmd) - 1] = '\0';
            return job_table[i].id;
        }
    }
    fprintf(stderr, "Error: Job table full.\n");
    return -1;
}

void remove_job_by_pgid(pid_t pgid){
    for (int i = 0; i <  MAX_JOBS; i++){
        if (job_table [i].id != 0 &&  job_table[i].pgid == pgid){
            job_table[i].id = 0;
            return;
        }
    }
}

Job *find_job_by_id(int id ){
    for (int i = 0; i < MAX_JOBS; i++){
        if (job_table[i].id == id){
            return &job_table[i];
        }
    }
    return NULL;
}

void print_jobs(void){
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_table[i].id != 0) {
            printf ("[%d] %s\t%s\n", 
                job_table[i].id, 
                (job_table[i].state == JOB_RUNNING) ? "RUNNING" : "STOPPED",
                job_table[i].cmd) ;
        }
     }
}

void wait_for_job(pid_t pgid, const char *cmd){
    int status;
    waitpid(-pgid, &status, WUNTRACED);

    if (WIFSTOPPED(status)){
        // job stopped
        //check if it's already in table
        int job_id = -1;
        for (int i = 0; i < MAX_JOBS; i++){
            if (job_table[i].id != 0 && job_table[i].pgid == pgid) {
                job_table[i].state = JOB_STOPPED;
                job_id  = job_table[i].id;
                break;
            }
        }

        //if it not in table it was fg job  , can add now
        if (job_id == -1){
            job_id =  add_job(pgid, JOB_STOPPED, cmd);
        }

        printf("\n[%d] STOPPED\t%s\n", job_id, cmd);
    } 
    else if(WIFEXITED(status) || WIFSIGNALED(status)){
        remove_job_by_pgid(pgid);
    }
}

void put_job_in_foreground(Job *j, int continue_job){
    tcsetpgrp(STDIN_FILENO, j->pgid);

    if (continue_job){
        if (kill(-j->pgid, SIGCONT) < 0) perror("kill (SIGCONT)");
    }

    // Pass existing command name back to wait
    wait_for_job(j->pgid, j->cmd);

    tcsetpgrp(STDIN_FILENO, getpgrp());
}

void put_job_in_background(Job *j, int continue_job){
    if (continue_job) {
        if (kill(-j->pgid, SIGCONT) < 0) perror("kill (SIGCONT)");
    }
    j->state = JOB_RUNNING;
    printf("[%d] %s &\n", j->id, j->cmd);
}

