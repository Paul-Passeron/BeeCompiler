#include <stdlib.h>
#include <stdio.h>
#include "token.h"

void token_array_create(token_array_t *s)
{
    int init_cap = 265;
    s->capacity = init_cap;
    s->length = 0;
    s->data = malloc(sizeof(token_t) * init_cap);
    if (s->data == NULL)
    {
        printf("Could not allocate memory\n");
        exit(1);
    }
}
void token_array_free(token_array_t *s)
{
    FREE(s->data);
    s->capacity = 0;
    s->length = 0;
}

int token_array_empty(token_array_t s)
{
    {
        return s.length == 0;
    }
}

void token_array_push(token_array_t *s, token_t v)
{
    {
        if (s->length == s->capacity)
        {
            s->capacity *= 2;
            s->data = realloc(s->data, sizeof(token_t) * s->capacity);
            if (s->data == NULL)
            {
                printf("Could not allocate memory\n");
                exit(1);
            }
        }
        s->data[s->length++] = v;
    }
}
token_t token_array_pop(token_array_t *s)
{
    if (s->length == 0)
    {
        printf("Cannot pop empty array\n");
        exit(2);
    }
    return s->data[--s->length];
}
void print_type(token_type_t t)
{
    switch (t)
    {
    case KEYWORD:
        printf("Keyword");
        break;
    case DELIMITER:
        printf("Delimeter");
        break;
    case NUM_LIT:
        printf("Num Literal");
        break;
    case STRING_LIT:
        printf("String Literal");
        break;
    case CHAR_LIT:
        printf("Char Literal");
        break;
    case IDENTIFIER:
        printf("Identifier");
        break;
    case OPERATION:
        printf("OPERATION");
        break;
    }
}

void print_token(token_t tok)
{
    printf("\n{");
    printf("   col:line: %d; %d\n", tok.col, tok.line);
    printf("    lexeme: %s\n", tok.lexeme);
    printf("    type: ");
    print_type(tok.type);
    printf("\n}\n");
}