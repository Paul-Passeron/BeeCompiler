#ifndef TOKEN_H
#define TOKEN_H
#include "common.h"
#include "type.h"

typedef enum
{
    TYPE,
    KEYWORD,
    DELIMITER,
    NUM_LIT,
    STRING_LIT,
    CHAR_LIT,
    IDENTIFIER,
    OPERATION
} token_type_t;

typedef struct
{
    int line;
    int col;
    int abs_offset;
    char *lexeme;
    token_type_t type;
} token_t;

typedef struct
{
    token_t *data;
    int length;
    int capacity;
} token_array_t;

void token_array_create(token_array_t *s);
void token_array_free(token_array_t *s);

int token_array_empty(token_array_t s);

void token_array_push(token_array_t *s, token_t v);
token_t token_array_pop(token_array_t *s);

void print_token(token_t tok);

#endif // TOKEN_H