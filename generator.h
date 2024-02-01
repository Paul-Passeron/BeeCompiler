#ifndef GENERATOR_H
#define GENERATOR_H
#include "ast.h"
#include "parser_tok.h"
#include <stdio.h>

typedef struct
{
    ast_t ast;
    FILE *out;
} generator_t;

generator_t create_generator(ast_t a, char *filename);

#endif // GENERATOR_H
