#ifndef LEXER_H
#define LEXER_H

char *read_entire_file(char *filename);

#include "token.h"

typedef enum
{
    SYNTAX_ERROR
} flags_t;

typedef enum
{
    UNEXP_DELIM,
    UNCLOSED_STRLIT,
    INVALID_STRLIT,
    UNCLOSED_CHRLIT,
    INVALID_CHRLIT
} syntax_error_t;

typedef struct
{
    int col;
    int line;
    token_array_t tokens;
    char *remaining;
    char *start;
    char *filename;
    int flag;
    int file_length;
} lexer_t;

void lexer_create(lexer_t *l, char *filename);
void lexer_free(lexer_t *l);

void step_lexer(lexer_t *l);

void print_token_array(lexer_t l);

#endif // LEXER_H