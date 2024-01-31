#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "stack.h"
#include "lexer.h"
#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "parser_tok.h"

typedef struct
{
    token_array_t tokens;
    int current;
    stack_t scope_variables;
    ast_stack_t scope;
    ast_stack_t prog;
    char *filename;
    char *text;
} parser_t;

void parser_create(parser_t *p, lexer_t l); // should copy tokens so it cannot be freed from lexer
void parser_free(parser_t *p);

void parser_tok_name(parser_token_t t);

ast_t parse_expression(parser_t *p);
ast_t parse_program(parser_t *p);
ast_t parse_var_declaration(parser_t *p);
ast_t parse_var_assignment(parser_t *p);
ast_t parse_if_statement(parser_t *p);
ast_t parse_for_loop(parser_t *p);
ast_t parse_while_loop(parser_t *p);
ast_t parse_return_statement(parser_t *p);
ast_t parse_function_call(parser_t *p);
ast_t parse_function_def(parser_t *p);
ast_t build_ast(parser_t *p);
ast_t prog_to_ast(parser_t *p);

#endif // PARSER_H