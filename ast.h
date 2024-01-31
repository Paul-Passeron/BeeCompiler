#ifndef AST_H
#define AST_H

#include "token.h"
#include "common.h"
#include "stack.h"

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
        ast_assignment,
        ast_identifier,
        ast_literal,
        ast_unary_op,
        ast_expression,
        ast_program,
        ast_return,
        ast_funccallargs,
        ast_scope,
        ast_auto,
        // ast_tmp_expr,
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
            node_t *condition;
            node_t *iterator;
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
            int arity;
            int capacity;
        } ast_function_call;

        struct ast_assignment
        {
            token_t t; // lhs
            node_t *rhs;
        } ast_assignment;

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

        struct ast_expression
        {
            node_t *expression;
        } ast_expression;

        struct ast_program
        {
            node_t **program;
            int length;
            int capacity;
        } ast_program;

        struct ast_return
        {
            node_t *expression;
        } ast_return;

        struct ast_funccallargs
        {
            node_t **args;
            int length;
            int capacity;
        } ast_funccallargs;

        struct ast_scope
        {
            node_t **statements;
            int length;
            int capacity;
        } ast_scope;

        struct ast_auto
        {
            token_t t;
        } ast_auto;

        // struct ast_tmp_expr
        // {
        //     stack_t precedence;
        //     node_t **expressions;
        //     int length;
        //     int capacity;
        //     int min_precedence;
        // } ast_tmp_expr;

    } data;
};
typedef node_t *ast_t;

ast_t new_ast(node_t node);

node_t empty_program();

void free_ast(ast_t a);

typedef struct
{
    ast_t *data;
    int length;
    int capacity;
} ast_stack_t;

void ast_stack_create(ast_stack_t *s);
void ast_stack_free(ast_stack_t *s);
void ast_stack_push(ast_stack_t *s, ast_t a);
ast_t ast_stack_pop(ast_stack_t *s);
ast_t ast_stack_peek(ast_stack_t *s);

void print_tag(node_t t);

void pretty_print(ast_t a);

ast_t wrap_in_expr(ast_t a);
ast_t new_expr(void);
#endif // AST_H