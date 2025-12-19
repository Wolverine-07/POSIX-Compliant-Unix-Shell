#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "shell.h"

// The main entry point for processing an entire line of input,
// including handling ';' and '&' operators.
void process_line(char *input);

#endif // EXECUTOR_H