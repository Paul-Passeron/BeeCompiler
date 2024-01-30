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
    stack_t scope_variables;
    ast_stack_t scope;
    ast_stack_t prog;
    char *filename;
    char *text;
} parser_t;

void parser_create(parser_t *p, lexer_t l); // should copy tokens so it cannot be freed from lexer
void parser_free(parser_t *p);

// grows the ast
// void step_parser(parser_t *p);

typedef enum
{
    // keywords
    key_auto,
    key_else,
    key_if,
    key_return,
    key_while,
    key_for,

    // operators
    op_assign,
    op_eq,
    op_plus,
    op_minus,
    op_mult,
    op_div,
    op_mod,
    op_not,
    op_and,
    op_or,
    op_grtr,
    op_lssr,
    op_lssr_eq,
    op_grtr_eq,
    op_diff,
    op_deref,
    op_address,
    op_incr,
    op_decr,
    // delimeters
    del_openbra,
    del_closebra,
    del_openparen,
    del_closeparen,
    del_semicol,
    del_comma,

    // others
    tok_iden,
    tok_strlit,
    tok_numlit,
    tok_charlit,

    err_tok,

} parser_token_t;

void parser_tok_name(parser_token_t t);

ast_t prog_to_ast(parser_t *p);
void build_ast(parser_t *p);
void parse_primary(parser_t *p);
void step_parser(parser_t *p);
void parse_expression_1(parser_t *p, int min_expression);
void parse_expression(parser_t *p);
#endif // PARSER_H