#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>

// Validates the syntax of the input command string
bool is_valid_syntax(const char *input);

// Tokenizes a string into an array of arguments
char **tokenize(char *input, int *argc);

#endif // PARSER_H