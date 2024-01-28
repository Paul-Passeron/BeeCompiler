#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "stack.h"

typedef struct
{
    token_array_t tokens;
    ast_t ast;
    stack_t scope;
} parser_t;

#endif // PARSER_H