#ifndef _JOBS_H_
#define _JOBS_H_

#include <sys/types.h>
#include <signal.h>

// Max number of jobs to track
#define MAX_JOBS 20

typedef enum {
    JOB_RUNNING,
    JOB_STOPPED
} JobState;

typedef struct {
    int id;             
    pid_t pgid;         //process group ID
    JobState state;
    char cmd[1024];     // copy of cmd line
} Job;

void init_jobs();

//add job to table
int add_job(pid_t pgid, JobState state, const char *cmd);
void remove_job_by_pgid(pid_t pgid);

Job *find_job_by_id(int id);

void print_jobs();

// Wait specifically for a foreground process group
// replaces our 'reap' from before
void wait_for_job(pid_t pgid, const char *cmd);

void put_job_in_foreground(Job *j, int continue_job);
void put_job_in_background(Job *j, int continue_job);

#endif