#include "shell.h"
#include "prompt.h"

void display_prompt(void) {
    char username[LOGIN_NAME_MAX];
    struct utsname sys_info;
    char cwd[PATH_MAX];

    // Get username
    if (getlogin_r(username, sizeof(username)) != 0) {
        perror("getlogin_r");
        strcpy(username, "user");
    }

    // Get system name
    if (uname(&sys_info) != 0) {
        perror("uname");
        strcpy(sys_info.nodename, "system");
    }

    // Get current working directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        strcpy(cwd, "");
    }

    // A.1: Check if CWD is an ancestor of SHELL_HOME and replace with "~"
    char display_path[PATH_MAX];
    char *home_in_cwd = strstr(cwd, SHELL_HOME);
    
    if (home_in_cwd == cwd) { // Must be at the beginning of the string
        snprintf(display_path, sizeof(display_path), "~%s", cwd + strlen(SHELL_HOME));
    } else {
        strncpy(display_path, cwd, sizeof(display_path) - 1);
        display_path[sizeof(display_path) - 1] = '\0';
    }

    // Print the final prompt
    printf("<%s@%s:%s> ", username, sys_info.nodename, display_path);
    fflush(stdout); // Ensure prompt is displayed immediately
}