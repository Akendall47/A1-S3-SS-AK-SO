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


#endif