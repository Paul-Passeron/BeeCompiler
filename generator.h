#ifndef GENERATOR_H
#define GENERATOR_H
#include "ast.h"
#include "parser_tok.h"
#include "fundef_table.h"
#include <stdio.h>

int label = 0;

typedef struct
{
    ast_t ast;
    stack_t scope;
    FILE *out;
    fundef_table_t table;
} generator_t;

generator_t create_generator(ast_t a, char *filename);
void destroy_generator(generator_t g);
void print_entry(generator_t g);
void print_exit(generator_t g);
void generate_program(generator_t g);
void print_push_on_stack(generator_t g, int n_bytes);
void generate_function_call(generator_t g, ast_t funcall);
void generate_expression(generator_t g, ast_t expression);

#endif // GENERATOR_H
