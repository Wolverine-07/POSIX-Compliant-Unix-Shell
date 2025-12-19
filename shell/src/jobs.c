#include "jobs.h"

#define MAX_JOBS 64

// The Job struct is defined in jobs.h
static Job job_list[MAX_JOBS];
int next_job_id = 1; // Now globally accessible

// Forward declarations for internal helper functions
static Job* find_job_by_pgid(pid_t pgid);
static Job* find_job_by_jid(int jid);
static Job* get_latest_job(void);

void jobs_init(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        job_list[i].active = false;
    }
}

void jobs_add(pid_t pgid, const char* command, JobState state) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!job_list[i].active) {
            job_list[i].pgid = pgid;
            job_list[i].job_id = next_job_id++;
            job_list[i].state = state;
            strncpy(job_list[i].command, command, sizeof(job_list[i].command) - 1);
            job_list[i].command[sizeof(job_list[i].command) - 1] = '\0';
            job_list[i].active = true;
            
            if (state == RUNNING) {
                 printf("[%d] %d\n", job_list[i].job_id, pgid);
            }
            return;
        }
    }
    fprintf(stderr, "shell: Error: too many background jobs\n");
}

void jobs_reap(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        Job* job = find_job_by_pgid(getpgid(pid));
        if (!job) continue;

        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            // Print job completion message immediately for background jobs
            printf("[%d]+ Done\t\t%s\n", job->job_id, job->command);
            fflush(stdout); // Ensure immediate output
            job->active = false;
        } else if (WIFSTOPPED(status)) {
            job->state = STOPPED;
        }
    }
}

void jobs_kill_all(void) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_list[i].active) {
            kill(-job_list[i].pgid, SIGKILL);
        }
    }
}

void do_activities(void) {
    Job temp_jobs[MAX_JOBS];
    int count = 0;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_list[i].active) temp_jobs[count++] = job_list[i];
    }
    // Simple bubble sort
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (strcmp(temp_jobs[j].command, temp_jobs[j+1].command) > 0) {
                Job temp = temp_jobs[j];
                temp_jobs[j] = temp_jobs[j+1];
                temp_jobs[j+1] = temp;
            }
        }
    }
    for (int i = 0; i < count; i++) {
        printf("[%d] : %s - %s\n", temp_jobs[i].pgid, temp_jobs[i].command,
               temp_jobs[i].state == RUNNING ? "Running" : "Stopped");
    }
}

void do_ping(char** args, int argc) {
    if (argc != 3) { fprintf(stderr, "ping: Invalid syntax\n"); return; }
    pid_t pid = atoi(args[1]);
    int sig = atoi(args[2]);
    if (kill(pid, 0) == -1) { printf("No such process found\n"); return; }
    kill(pid, sig % 32);
    printf("Sent signal %d to process with pid %d\n", sig, pid);
}

void do_fg(char** args, int argc) {
    Job* job = (argc == 1) ? get_latest_job() : find_job_by_jid(atoi(args[1]));
    if (!job) { printf("No such job\n"); return; }

    printf("%s\n", job->command);
    tcsetpgrp(STDIN_FILENO, job->pgid);
    if (job->state == STOPPED) { kill(-job->pgid, SIGCONT); }

    int status;
    waitpid(-job->pgid, &status, WUNTRACED);
    tcsetpgrp(STDIN_FILENO, SHELL_PGID);

    if (WIFSTOPPED(status)) {
        job->state = STOPPED;
        printf("\n[%d]+ Stopped\t\t%s\n", job->job_id, job->command);
    } else {
        job->active = false;
    }
}

void do_bg(char** args, int argc) {
    if (argc != 2) { fprintf(stderr, "bg: Invalid syntax\n"); return; }
    Job* job = find_job_by_jid(atoi(args[1]));
    if (!job) { printf("No such job\n"); return; }
    if (job->state == RUNNING) { printf("Job already running\n"); return; }
    
    kill(-job->pgid, SIGCONT);
    job->state = RUNNING;
    printf("[%d] %s &\n", job->job_id, job->command);
}

//
// FIX: The missing function definitions are now here.
//
static Job* find_job_by_pgid(pid_t pgid) {
    if (pgid < 1) return NULL;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_list[i].active && job_list[i].pgid == pgid) {
            return &job_list[i];
        }
    }
    return NULL;
}

static Job* find_job_by_jid(int jid) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_list[i].active && job_list[i].job_id == jid) return &job_list[i];
    }
    return NULL;
}

static Job* get_latest_job() {
    Job* latest = NULL;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (job_list[i].active) {
            if (!latest || job_list[i].job_id > latest->job_id) latest = &job_list[i];
        }
    }
    return latest;
}