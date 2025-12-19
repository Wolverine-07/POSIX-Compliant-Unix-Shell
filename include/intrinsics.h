#ifndef INTRINSICS_H
#define INTRINSICS_H

#include <stdbool.h>

// Initialize the log system
void log_init(void);

// Add a command to the log
void log_add(const char *command);

// Get command from history by index (1-based, with 1 being most recent)
// Returns NULL if index is invalid
char* log_get_command(int index);

// Returns true if the command was an intrinsic and was handled
bool handle_intrinsic(char **args, int argc);
bool is_parent_builtin(const char* cmd); 
#endif // INTRINSICS_H