// --- START: Replace the top part of your executor.c file with this ---



#include "shell.h"

#include "executor.h"

#include "jobs.h"

#include "intrinsics.h"

#include "parser.h"

#include <ctype.h>
#include <errno.h>



#define MAX_ARGS 64

// External reference to next_job_id from jobs.c
extern int next_job_id;



// NEW: A struct to represent a single redirection

typedef struct {

enum { REDIR_IN, REDIR_OUT, REDIR_APPEND } type;

char* filename;

} Redirection;



// UPDATED: SimpleCommand now holds a list of redirections

typedef struct {

char *argv[MAX_ARGS];

int argc;

char full_command[1024];

Redirection redirections[MAX_ARGS];

int redirection_count;

} SimpleCommand;



typedef struct {

SimpleCommand *commands;

int num_commands;

} CommandGroup;



// Internal function prototypes

static CommandGroup* parse_cmd_group(char *input);

static void free_cmd_group(CommandGroup *group);

static void run_cmd_group(CommandGroup *group, bool is_background);

static void execute_cmd_group(char* cmd_group_string, bool is_background);



// --- END: Replace the top part of your executor.c file with this ---





// (The process_line and execute_cmd_group functions remain the same as the last fix)

void process_line(char *input) {

char *input_copy = strdup(input);

char *current = input;

char *end = input + strlen(input);

while (current < end) {

char *separator = strpbrk(current, ";&");

bool is_background = false;

if (!separator && current[strlen(current) - 1] == '&') {

is_background = true;

current[strlen(current) - 1] = '\0';

} else if (separator) {

if (*separator == '&') is_background = true;

*separator = '\0';

}

execute_cmd_group(current, is_background);

if (separator) current = separator + 1;

else break;

}

free(input_copy);

}



static void execute_cmd_group(char* cmd_group_string, bool is_background) {

while (isspace((unsigned char)*cmd_group_string)) cmd_group_string++;

if (*cmd_group_string == '\0') return;

CommandGroup *group = parse_cmd_group(cmd_group_string);

if (group && group->num_commands == 1 && is_parent_builtin(group->commands[0].argv[0])) {

handle_intrinsic(group->commands[0].argv, group->commands[0].argc);

} else if (group && group->num_commands > 0) {

run_cmd_group(group, is_background);

}

free_cmd_group(group);

}





// --- START: Replace your parse_cmd_group function with this ---

static CommandGroup* parse_cmd_group(char *input) {
    CommandGroup *group = malloc(sizeof(CommandGroup));
    if (!group) return NULL;
    group->commands = malloc(sizeof(SimpleCommand) * 16);
    if (!group->commands) { free(group); return NULL; }
    group->num_commands = 0;

    char *original_pipeline = strdup(input); // for job display

    char *saveptr_pipe = NULL;
    char *command_str = strtok_r(input, "|", &saveptr_pipe);
    while (command_str) {
        if (group->num_commands >= 16) break;

        // trim leading/trailing whitespace for each segment
        while (isspace((unsigned char)*command_str)) command_str++;
        size_t seg_len = strlen(command_str);
        while (seg_len > 0 && isspace((unsigned char)command_str[seg_len - 1])) command_str[--seg_len] = '\0';

        SimpleCommand *cmd = &group->commands[group->num_commands++];
        cmd->argc = 0;
        cmd->redirection_count = 0;
        cmd->full_command[0] = '\0';
        if (group->num_commands == 1 && original_pipeline) {
            strncpy(cmd->full_command, original_pipeline, sizeof(cmd->full_command) - 1);
            cmd->full_command[sizeof(cmd->full_command) - 1] = '\0';
        }

        char *saveptr_token = NULL;
        char *token = strtok_r(command_str, " \t\r\n", &saveptr_token);
        while (token) {
            if (strcmp(token, "<") == 0) {
                cmd->redirections[cmd->redirection_count].type = REDIR_IN;
                cmd->redirections[cmd->redirection_count++].filename = strtok_r(NULL, " \t\r\n", &saveptr_token);
            } else if (strcmp(token, ">") == 0) {
                cmd->redirections[cmd->redirection_count].type = REDIR_OUT;
                cmd->redirections[cmd->redirection_count++].filename = strtok_r(NULL, " \t\r\n", &saveptr_token);
            } else if (strcmp(token, ">>") == 0) {
                cmd->redirections[cmd->redirection_count].type = REDIR_APPEND;
                cmd->redirections[cmd->redirection_count++].filename = strtok_r(NULL, " \t\r\n", &saveptr_token);
            } else {
                if (cmd->argc < MAX_ARGS - 1) cmd->argv[cmd->argc++] = token;
            }
            token = strtok_r(NULL, " \t\r\n", &saveptr_token);
        }
        cmd->argv[cmd->argc] = NULL;
        command_str = strtok_r(NULL, "|", &saveptr_pipe);
    }

    free(original_pipeline);
    return group;
}

// --- END: Replace your parse_cmd_group function ---





// --- START: Replace your run_cmd_group function with this ---

static void run_cmd_group(CommandGroup *group, bool is_background) {

int num_pipes = group->num_commands - 1;

pid_t pgid = 0;

int pipe_fds[num_pipes][2];



for (int i = 0; i < num_pipes; i++) if (pipe(pipe_fds[i]) < 0) { perror("pipe"); return; }



for (int i = 0; i < group->num_commands; i++) {

pid_t pid = fork();

if (pid < 0) { perror("fork"); return; }


if (pid == 0) { // Child Process

signal(SIGINT, SIG_DFL); signal(SIGTSTP, SIG_DFL);

signal(SIGTTIN, SIG_DFL); signal(SIGTTOU, SIG_DFL);

pgid = (pgid == 0) ? getpid() : pgid;

setpgid(0, pgid);

// Only the parent should manipulate terminal foreground process group; remove from child to avoid SIGTTOU stops



// Set up I/O from pipes

if (i > 0) dup2(pipe_fds[i - 1][0], STDIN_FILENO);

if (i < num_pipes) dup2(pipe_fds[i][1], STDOUT_FILENO);


// Set up I/O from files (this will override pipe I/O if specified)

int input_fd = -1, output_fd = -1;
bool had_input_error = false;
bool had_output_error = false;

for (int j = 0; j < group->commands[i].redirection_count; j++) {

Redirection *r = &group->commands[i].redirections[j];

if (r->type == REDIR_IN) {
    // Handle input redirections
    if (input_fd != -1) close(input_fd);
    
    input_fd = open(r->filename, O_RDONLY);
    if (input_fd < 0) { 
        fprintf(stderr, "No such file or directory\n");
        had_input_error = true;
        // Continue with next redirection instead of exiting
    }
} else {
    // Handle output redirections
    if (output_fd != -1) close(output_fd);
    
    int flags = O_WRONLY | O_CREAT | (r->type == REDIR_APPEND ? O_APPEND : O_TRUNC);
    output_fd = open(r->filename, flags, 0644);
    if (output_fd < 0) { 
        fprintf(stderr, "Unable to create file for writing\n");
        had_output_error = true;
        // Continue with next redirection instead of exiting
    }
}

}

// Only set up stdin if we didn't have any input errors
if (!had_input_error && input_fd != -1) { 
    dup2(input_fd, STDIN_FILENO); 
    close(input_fd); 
} else if (had_input_error) {
    // If we had an input error, exit after error messages were printed
    exit(1);
}

// Only set up stdout if we didn't have any output errors
if (!had_output_error && output_fd != -1) { 
    dup2(output_fd, STDOUT_FILENO); 
    close(output_fd); 
} else if (had_output_error) {
    // If we had an output error, exit after error messages were printed
    exit(1);
}



// Child must close ALL original pipe fds

for (int j = 0; j < num_pipes; j++) {

close(pipe_fds[j][0]);

close(pipe_fds[j][1]);

}


if (handle_intrinsic(group->commands[i].argv, group->commands[i].argc)) exit(0);

execvp(group->commands[i].argv[0], group->commands[i].argv);

fprintf(stderr, "%s: Command not found!\n", group->commands[i].argv[0]);

exit(127);

}

pgid = (pgid == 0) ? pid : pgid;

setpgid(pid, pgid);

}



//

// CRITICAL: Parent must close ALL pipe file descriptors

// This allows children to receive EOF when writers finish

//

for (int i = 0; i < num_pipes; i++) {

if (pipe_fds[i][0] >= 0) close(pipe_fds[i][0]);
if (pipe_fds[i][1] >= 0) close(pipe_fds[i][1]);

}



if (!is_background) {
    int status;
    pid_t pid;
    bool job_stopped = false;
    int active_procs = group->num_commands;  // Track how many processes are still active

    if (isatty(STDIN_FILENO)) {
        tcsetpgrp(STDIN_FILENO, pgid);
        
        // Wait for all processes in the pipeline to complete or for one to be stopped
        while (active_procs > 0) {
            pid = waitpid(-pgid, &status, WUNTRACED);
            
            if (pid < 0) {
                if (errno == ECHILD) {
                    // No more children to wait for
                    break;
                }
                // Interrupted by signal, continue waiting
                if (errno == EINTR) {
                    continue;
                }
                perror("waitpid");
                break;
            }
            
            if (WIFSTOPPED(status)) {
                // A process was stopped, mark the job as stopped
                job_stopped = true;
                break;  // Stop waiting and give control back to shell
            } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
                // A process has completed, decrement counter
                active_procs--;
            }
        }
        
        tcsetpgrp(STDIN_FILENO, SHELL_PGID);
    } else {
        // Non-interactive mode has similar logic but simpler
        while (active_procs > 0) {
            pid = waitpid(-pgid, &status, WUNTRACED);
            if (pid < 0) {
                if (errno == ECHILD) break;
                if (errno == EINTR) continue;
                perror("waitpid");
                break;
            }
            
            if (WIFSTOPPED(status)) {
                job_stopped = true;
                break;
            } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
                active_procs--;
            }
        }
    }

    if (job_stopped) {
        jobs_add(pgid, group->commands[0].full_command, STOPPED);
        printf("[%d]+ Stopped\t\t%s\n", next_job_id - 1, group->commands[0].full_command);
        fflush(stdout);
    }

} else {

jobs_add(pgid, group->commands[0].full_command, RUNNING);

}

}

// --- END: Replace your run_cmd_group function ---



// (free_cmd_group can remain the same)

static void free_cmd_group(CommandGroup *group) {

if (group) {

free(group->commands);

free(group);

}

}