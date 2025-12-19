#include "shell.h"
#include "intrinsics.h"
#include "executor.h"
#include "jobs.h"

#define MAX_LOG_SIZE 15

// Static variables for the command log system
static char* command_log[MAX_LOG_SIZE];
static int log_count = 0;
static int log_start = 0;

// Forward declarations for all intrinsic command functions
static void do_hop(char **args, int argc);
static void do_reveal(char **args, int argc);
static void do_log(char **args, int argc);
static char* get_log_path(void);
static int compare_strings(const void *a, const void *b);


// Main dispatcher for all intrinsic commands
bool handle_intrinsic(char **args, int argc) {
    if (strcmp(args[0], "hop") == 0) { do_hop(args, argc); return true; }
    if (strcmp(args[0], "reveal") == 0) { do_reveal(args, argc); return true; }
    if (strcmp(args[0], "log") == 0) { do_log(args, argc); return true; }
    if (strcmp(args[0], "activities") == 0) { do_activities(); return true; }
    if (strcmp(args[0], "ping") == 0) { do_ping(args, argc); return true; }
    if (strcmp(args[0], "fg") == 0) { do_fg(args, argc); return true; }
    if (strcmp(args[0], "bg") == 0) { do_bg(args, argc); return true; }
    return false;
}

bool is_parent_builtin(const char* cmd) {
    if (strcmp(cmd, "hop") == 0) {
        return true;
    }
    // In the future, you might add "exit", "export", etc. here.
    return false;
}
// B.1: hop command logic (Corrected Version)
static void do_hop(char **args, int argc) {
    if (argc == 1) { // hop or hop ~
        char current_dir[PATH_MAX];
        if (getcwd(current_dir, sizeof(current_dir)) == NULL) { perror("getcwd"); return; }
        if (chdir(SHELL_HOME) == 0) {
            strncpy(PREVIOUS_CWD, current_dir, sizeof(PREVIOUS_CWD) - 1);
            PREVIOUS_CWD_IS_SET = true;
        }
        return;
    }

    for (int i = 1; i < argc; i++) {
        char current_dir[PATH_MAX];
        if (getcwd(current_dir, sizeof(current_dir)) == NULL) { perror("getcwd"); return; }

        char *target = args[i];
        int chdir_res = -1;

        if (strcmp(target, "-") == 0) {
            if (!PREVIOUS_CWD_IS_SET) { fprintf(stderr, "hop: OLDPWD not set\n"); continue; }
            char temp[PATH_MAX];
            strncpy(temp, PREVIOUS_CWD, sizeof(temp) - 1);
            printf("%s\n", PREVIOUS_CWD);
            chdir_res = chdir(PREVIOUS_CWD);
            if (chdir_res == 0) strncpy(PREVIOUS_CWD, current_dir, sizeof(PREVIOUS_CWD) - 1);
        } else {
            if (strcmp(target, "~") == 0) chdir_res = chdir(SHELL_HOME);
            else if (strcmp(target, ".") == 0) { chdir_res = 0; continue; }
            else if (strcmp(target, "..") == 0) chdir_res = chdir("..");
            else chdir_res = chdir(target);
            
            if (chdir_res == 0) {
                strncpy(PREVIOUS_CWD, current_dir, sizeof(PREVIOUS_CWD) - 1);
                PREVIOUS_CWD_IS_SET = true; // FIX: Set the flag on any successful hop
            } else {
                fprintf(stderr, "No such directory!\n");
            }
        }
    }
}

// B.2: reveal command logic (Corrected Version)
static void do_reveal(char **args, int argc) {
    bool show_all = false, line_by_line = false;
    char *path_arg = NULL;

    for (int i = 1; i < argc; i++) {
        if (args[i][0] == '-') {
            if (strlen(args[i]) == 1) {
                if (path_arg != NULL) { fprintf(stderr, "reveal: Invalid Syntax!\n"); return; }
                path_arg = args[i];
                continue;
            }
            for (size_t j = 1; j < strlen(args[i]); j++) {
                if (args[i][j] == 'a') show_all = true;
                else if (args[i][j] == 'l') line_by_line = true;
            }
        } else {
            if (path_arg != NULL) { fprintf(stderr, "reveal: Invalid Syntax!\n"); return; }
            path_arg = args[i];
        }
    }

    char target_path[PATH_MAX];
    if (path_arg == NULL) { getcwd(target_path, sizeof(target_path)); }
    else if (strcmp(path_arg, "-") == 0) {
        if (!PREVIOUS_CWD_IS_SET) { fprintf(stderr, "No such directory!\n"); return; } // FIX: Check flag
        strncpy(target_path, PREVIOUS_CWD, sizeof(target_path));
    } else if (strcmp(path_arg, "~") == 0) strncpy(target_path, SHELL_HOME, sizeof(target_path));
    else if (strcmp(path_arg, ".") == 0) getcwd(target_path, sizeof(target_path));
    else if (strcmp(path_arg, "..") == 0) {
        getcwd(target_path, sizeof(target_path));
        char *last_slash = strrchr(target_path, '/');
        if (last_slash != NULL && last_slash != target_path) *last_slash = '\0';
        else if (last_slash == target_path && strlen(target_path) > 1) *(last_slash + 1) = '\0';
    } else {
         strncpy(target_path, path_arg, sizeof(target_path));
    }

    DIR *d = opendir(target_path);
    if (!d) { fprintf(stderr, "No such directory!\n"); return; }
    
    char *files[2048]; int file_count = 0;
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (!show_all && dir->d_name[0] == '.') continue;
        files[file_count++] = strdup(dir->d_name);
    }
    closedir(d);
    qsort(files, file_count, sizeof(char *), compare_strings);
    for (int i = 0; i < file_count; i++) {
        printf("%s%s", files[i], line_by_line ? "\n" : "  ");
        free(files[i]);
    }
    if (!line_by_line && file_count > 0) printf("\n");
}


// (The rest of the functions: log, activities, ping, fg, bg, etc. remain the same)
// Helper for qsort
static int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// B.3: log command logic
static void do_log(char **args, int argc) {
    if (argc == 1) {
        for (int i = 0; i < log_count; i++) {
            printf("%s\n", command_log[(log_start + i) % MAX_LOG_SIZE]);
        }
    } else if (argc == 2 && strcmp(args[1], "purge") == 0) {
        for (int i = 0; i < log_count; i++) free(command_log[i]);
        log_count = 0; log_start = 0;
        FILE *f = fopen(get_log_path(), "w");
        if (f) fclose(f);
    } else if (argc == 3 && strcmp(args[1], "execute") == 0) {
        int index = atoi(args[2]);
        if (index > 0 && index <= log_count) {
            // For piped log execute commands, main.c will handle execution
            // Just print the command for user feedback
            int actual_index = (log_start + log_count - index) % MAX_LOG_SIZE;
            printf("%s\n", command_log[actual_index]);
            return;
        } else {
            fprintf(stderr, "log: invalid index\n");
        }
    }
}

// Helper for log persistence
static char* get_log_path() {
    static char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s/.cshell_log", getenv("HOME"));
    return path;
}

void log_init() {
    FILE *f = fopen(get_log_path(), "r");
    if (!f) return;
    char *line = NULL; size_t len = 0;
    while (getline(&line, &len, f) != -1) {
        if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = '\0';
        char* command_copy = strdup(line);
        log_add(command_copy);
        free(command_copy);
    }
    free(line);
    fclose(f);
}

void log_add(const char *command) {
    if (strncmp(command, "log", 3) == 0 && (command[3] == ' ' || command[3] == '\0')) return;
    if (log_count > 0 && strcmp(command, command_log[(log_start + log_count - 1) % MAX_LOG_SIZE]) == 0) return;

    if (log_count == MAX_LOG_SIZE) {
        free(command_log[log_start]);
        log_start = (log_start + 1) % MAX_LOG_SIZE;
    } else {
        log_count++;
    }
    command_log[(log_start + log_count - 1) % MAX_LOG_SIZE] = strdup(command);

    FILE *f = fopen(get_log_path(), "w");
    if (!f) return;
    for (int i = 0; i < log_count; i++) {
        fprintf(f, "%s\n", command_log[(log_start + i) % MAX_LOG_SIZE]);
    }
    fclose(f);
}

// Get a command from history by index (1-based)
char* log_get_command(int index) {
    if (index <= 0 || index > log_count) {
        return NULL;
    }
    int actual_index = (log_start + log_count - index) % MAX_LOG_SIZE;
    return strdup(command_log[actual_index]);
}