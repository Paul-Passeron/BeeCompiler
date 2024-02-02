#ifndef GENERATOR_H
#define GENERATOR_H
#include "ast.h"
#include "parser_tok.h"
#include <stdio.h>

typedef struct
{
    ast_t ast;
    stack_t scope;
    FILE *out;
} generator_t;

generator_t create_generator(ast_t a, char *filename);
void destroy_generator(generator_t g);
void print_entry(generator_t g);
void print_exit(generator_t g, int exit_code);
void generate_program(generator_t g);
#endif // GENERATOR_H
