#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "stack.h"
#include "lexer.h"
#include <stdlib.h>
#include <stdio.h>
#include "common.h"

typedef struct
{
    token_array_t tokens;
    int current;
    ast_t ast;
    stack_t scope;
} parser_t;

void parser_create(parser_t *p, lexer_t l); // should copy tokens so it cannot be freed from lexer
void parser_free(parser_t *p);

// grows the ast
void step_parser(parser_t *p);

#endif // PARSER_H