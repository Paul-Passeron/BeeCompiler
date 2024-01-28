#ifndef AST_H
#define AST_H

#include "token.h"

typedef struct node_t node_t;

struct node_t
{
    enum
    {
        ast_bin_op,
        ast_if_stat,
        ast_for_loop,
        ast_while_loop,
        ast_function_def,
        ast_function_call,
        ast_assignement,
        ast_identifier,
        ast_literal,
        ast_unary_op,
    } tag;
    union
    {
        struct ast_bin_op
        {
            token_t t;
            node_t *l;
            node_t *r;
        } ast_bin_op;

        struct ast_if_stat
        {
            node_t *cond;
            node_t *body;
            node_t *other;
        } ast_if_stat;

        struct ast_for_loop
        {
            node_t *init;
            node_t *cond;
            node_t *body;
        } ast_for_loop;

        struct ast_while_loop
        {
            node_t *cond;
            node_t *body;
        } ast_while_loop;

        struct ast_function_def
        {
            token_t t;
            token_array_t args;
            node_t *body;
        } ast_function_def;

        struct ast_function_call
        {
            token_t t;
            node_t **args;
        } ast_function_call;

        struct ast_assignement
        {
            token_t t; // lhs
            node_t *rhs;
        } ast_assignement;

        struct ast_identifier
        {
            token_t t;
        } ast_identifier;

        struct ast_literal
        {
            token_t t;
        } ast_literal;

        struct ast_unary_op
        {
            token_t t;
            node_t *operand;
        } ast_unary_op;

    } data;
};

typedef node_t *ast_t;

#endif // AST_H