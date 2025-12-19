#include "shell.h"
#include "input.h"

char *read_input(void) {
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    nread = getline(&line, &len, stdin);

    if (nread == -1) {
        // EOF (Ctrl-D) or error
        free(line);
        return NULL;
    }

    // Remove trailing newline character, if present
    if (nread > 0 && line[nread - 1] == '\n') {
        line[nread - 1] = '\0';
    }

    return line;
}