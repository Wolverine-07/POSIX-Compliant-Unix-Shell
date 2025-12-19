#ifndef SHELL_H
#define SHELL_H

// Standard POSIX headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/wait.h> 
#include <sys/stat.h> 
#include <fcntl.h>    
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>   // For signal handling
#include <termios.h>  // For terminal control

// For boolean type
#include <stdbool.h>

// Global variable to store the shell's home directory
extern char SHELL_HOME[PATH_MAX];
// Global variable for the previous working directory
extern char PREVIOUS_CWD[PATH_MAX];
// Global variable for the shell's process group ID
extern pid_t SHELL_PGID;
extern bool PREVIOUS_CWD_IS_SET;

#endif // SHELL_H