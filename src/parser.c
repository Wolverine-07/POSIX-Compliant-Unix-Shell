#include "shell.h"
#include "parser.h"

// This preliminary check remains the same as in Part A.
bool is_valid_syntax(const char *input) {
    bool expect_command = true; 
    bool last_was_special = false;
    for (int i = 0; input[i] != '\0'; ++i) {
        if (input[i] == ' ' || input[i] == '\t' || input[i] == '\r' || input[i] == '\n') continue;
        if (strchr("|&;<>", input[i])) {
            if (last_was_special) {
                if (input[i-1] == '>' && input[i] == '>') continue;
                return false; 
            }
            if (expect_command && (input[i] == '|' || input[i] == ';')) return false;
            last_was_special = true;
            expect_command = true;
            if (input[i] == '>' && input[i + 1] == '>') i++;
        } else {
            last_was_special = false;
            expect_command = false;
        }
    }
    if (last_was_special && input[strlen(input)-1] != '&') return false;
    return true;
}

// New function to tokenize the input string
char **tokenize(char *input, int *argc) {
    char *delim = " \t\r\n";
    char **tokens = malloc(64 * sizeof(char *));
    if (!tokens) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    int count = 0;
    char *token = strtok(input, delim);
    while (token != NULL) {
        tokens[count++] = token;
        token = strtok(NULL, delim);
    }
    tokens[count] = NULL; // Null-terminate the array
    *argc = count;
    return tokens;
}