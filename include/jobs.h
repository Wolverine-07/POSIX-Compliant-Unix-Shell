#ifndef JOBS_H
#define JOBS_H

#include "shell.h"

// Declare next_job_id as extern for use in other files
extern int next_job_id;

typedef enum {
    RUNNING,
    STOPPED
} JobState;

typedef struct {
    pid_t pgid;         // Process group ID for the job
    int job_id;
    char command[1024]; // The full command string
    JobState state;
    bool active;
} Job;

void jobs_init(void);
void jobs_add(pid_t pgid, const char* command, JobState state);
void jobs_reap(void);
void jobs_kill_all(void);

// New functions for Part E
void do_activities(void);
void do_ping(char** args, int argc);
void do_fg(char** args, int argc);
void do_bg(char** args, int argc);

#endif // JOBS_H