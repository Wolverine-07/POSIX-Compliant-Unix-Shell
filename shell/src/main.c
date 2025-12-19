#include "shell.h"
#include "prompt.h"
#include "input.h"
#include "parser.h"
#include "jobs.h"
#include "executor.h"
#include "intrinsics.h"

// Global variable definitions
char SHELL_HOME[PATH_MAX];
char PREVIOUS_CWD[PATH_MAX] = "";
pid_t SHELL_PGID;
bool PREVIOUS_CWD_IS_SET = false; 

// E.3: Signal Handlers
// These handlers do nothing, their purpose is to interrupt blocking syscalls like waitpid.
// The main logic is handled in the loops where those syscalls are.
void sigint_handler(int sig) { (void)sig; }
void sigtstp_handler(int sig) { (void)sig; }

int main(void) {
    // Make the shell interactive and grab terminal control only if running on a terminal
    if (isatty(STDIN_FILENO)) {
        SHELL_PGID = getpgrp();
        while (tcgetpgrp(STDIN_FILENO) != SHELL_PGID) {
            kill(-SHELL_PGID, SIGTTIN);
        }
        tcsetpgrp(STDIN_FILENO, SHELL_PGID);
        
        // E.3: Install signal handlers for the shell
        signal(SIGINT, sigint_handler);
        signal(SIGTSTP, sigtstp_handler);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
    } else {
        // Running non-interactively, just get our process group
        SHELL_PGID = getpgrp();
    }

    // Initialize shell home directory
    if (getcwd(SHELL_HOME, sizeof(SHELL_HOME)) == NULL) {
        perror("getcwd"); exit(EXIT_FAILURE);
    }
    strncpy(PREVIOUS_CWD, SHELL_HOME, sizeof(PREVIOUS_CWD) - 1);

    // Initialize subsystems
    log_init();
    jobs_init();

    while (1) {
        jobs_reap();
        display_prompt();
        char *input = read_input();

        if (input == NULL) { // E.3: Handle Ctrl-D
            printf("logout\n");
            jobs_kill_all();
            break; 
        }

        // Check if this is a "log execute" command with pipes
        if (strlen(input) > 0) {
            // Check if the command is "log execute N | ..."
            char *cmd_copy = strdup(input);
            char *token = strtok(cmd_copy, " \t");
            int is_log_execute_with_pipe = 0;
            int history_index = 0;
            char *remaining_pipeline = NULL;
            
            if (token && strcmp(token, "log") == 0) {
                token = strtok(NULL, " \t");
                if (token && strcmp(token, "execute") == 0) {
                    token = strtok(NULL, " \t");
                    if (token) {
                        history_index = atoi(token);
                        // Check if there's a pipe after the log execute command
                        char *pipe_check = strtok(NULL, " \t");
                        if (pipe_check && strcmp(pipe_check, "|") == 0) {
                            // Capture the rest of the pipeline
                            char *rest = strtok(NULL, "");
                            if (rest) {
                                remaining_pipeline = strdup(rest);
                                is_log_execute_with_pipe = 1;
                            }
                        }
                    }
                }
            }
            free(cmd_copy);
            
            // Add command to log
            char *input_copy_for_log = strdup(input);
            log_add(input_copy_for_log);
            free(input_copy_for_log);

            if (is_log_execute_with_pipe && history_index > 0) {
                char *historical_cmd = log_get_command(history_index);
                if (historical_cmd) {
                    // Create the full pipeline: "historical_cmd | remaining_pipeline"
                    char *full_pipeline = malloc(strlen(historical_cmd) + strlen(remaining_pipeline) + 4);
                    sprintf(full_pipeline, "%s | %s", historical_cmd, remaining_pipeline);

                    // Process the full pipeline
                    if (is_valid_syntax(full_pipeline)) {
                        process_line(full_pipeline);
                    } else {
                        fprintf(stderr, "Invalid Syntax in pipeline!\n");
                    }

                    free(full_pipeline);
                    free(historical_cmd);
                } else {
                    fprintf(stderr, "log: invalid index\n");
                }
                if (remaining_pipeline) free(remaining_pipeline);
            } else {
                // Check if the command is "log execute N" (without pipes)
                cmd_copy = strdup(input);
                token = strtok(cmd_copy, " \t");
                int is_log_execute = 0;
                history_index = 0;
                
                if (token && strcmp(token, "log") == 0) {
                    token = strtok(NULL, " \t");
                    if (token && strcmp(token, "execute") == 0) {
                        token = strtok(NULL, " \t");
                        if (token) {
                            history_index = atoi(token);
                            // Check if there's no pipe after the log execute command
                            char *pipe_check = strtok(NULL, " \t");
                            if (!pipe_check) {
                                is_log_execute = 1;
                            }
                        }
                    }
                }
                free(cmd_copy);

                if (is_log_execute && history_index > 0) {
                    // Get the command from history and execute it
                    char *historical_cmd = log_get_command(history_index);
                    if (historical_cmd) {
                        // Print the command being executed
                        printf("%s\n", historical_cmd);
                        
                        // Process it
                        if (is_valid_syntax(historical_cmd)) {
                            process_line(historical_cmd);
                        } else {
                            fprintf(stderr, "Invalid Syntax in historical command!\n");
                        }
                        free(historical_cmd);
                    } else {
                        fprintf(stderr, "log: invalid index\n");
                    }
                } else if (!is_valid_syntax(input)) {
                    fprintf(stderr, "Invalid Syntax!\n");
                } else {
                    process_line(input);
                }
            }
        }
        free(input);
    }
    return 0;
}