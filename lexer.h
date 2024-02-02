#ifndef LEXER_H
#define LEXER_H

char *read_entire_file(char *filename);

#include "token.h"
#include "common.h"
#include "error.h"
#include "type.h"

typedef err_type_t flag_t;

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

// Should be called after the parser is done, otherwise some tokens lexemes might be freed
void lexer_free(lexer_t *l);

void step_lexer(lexer_t *l);

void print_token_array(lexer_t l);

void lex_prog(lexer_t *l);
#endif // LEXER_H