#include "ast.h"

#include <stdlib.h>
#include <stdio.h>

ast_t new_ast(node_t node)
{
    node_t *ptr = malloc(sizeof(node_t));
    if (ptr == NULL)
    {
        printf("Not enough memory\n");
        exit(1);
    }
    *ptr = node;
    return ptr;
}

node_t empty_program()
{
    return (node_t){
        ast_program, {.ast_program = {0}}};
}

void free_ast(ast_t a)
{
    switch (a->tag)
    {
    case ast_bin_op:
    {
        struct ast_bin_op data = a->data.ast_bin_op;
        free_ast(data.l);
        free_ast(data.r);
    }
    break;
    case ast_if_stat:
    {
        struct ast_if_stat data = a->data.ast_if_stat;
        free_ast(data.cond);
        free_ast(data.body);
        free_ast(data.other);
    }
    break;
    case ast_for_loop:
    {
        struct ast_for_loop data = a->data.ast_for_loop;
        free_ast(data.init);
        free_ast(data.cond);
        free_ast(data.body);
    }
    break;
    case ast_while_loop:
    {
        struct ast_while_loop data = a->data.ast_while_loop;
        free_ast(data.cond);
        free_ast(data.body);
    }
    break;
    case ast_function_def:
    {
        struct ast_function_def data = a->data.ast_function_def;
        token_array_free(&data.args);
        free_ast(data.body);
    }
    break;
    case ast_function_call:
    {
        struct ast_function_call data = a->data.ast_function_call;
        for (int i = 0; i < data.arity; i++)
            free_ast(data.args[i]);
    }
    break;
    case ast_assignement:
    {
        struct ast_assignement data = a->data.ast_assignement;
        free_ast(data.rhs);
    }
    break;
    case ast_identifier:
    {
        // struct ast_identifier data = a->data.ast_identifier;
        // Nothing else to do
    }
    break;
    case ast_literal:
    {
        // struct ast_literal data = a->data.ast_literal;
        // Nothing else to do
    }
    break;
    case ast_unary_op:
    {
        struct ast_unary_op data = a->data.ast_unary_op;
        free_ast(data.operand);
    }
    break;
    case ast_statement:
    {
        struct ast_statement data = a->data.ast_statement;
        free_ast(data.statement);
    }
    break;
    case ast_program:
    {
        struct ast_program data = a->data.ast_program;
        for (int i = 0; i < data.length; i++)
            free_ast(data.program[i]);
    }
    break;
    case ast_return:
    {
        struct ast_return data = a->data.ast_return;
        free_ast(data.expression);
    }
    default:
        break;
    }
    free(a); // No need to use FREE here
}

void ast_stack_create(ast_stack_t *s)
{
    int init_cap = 265;
    s->capacity = init_cap;
    s->length = 0;
    s->data = malloc(sizeof(ast_t) * init_cap);
    if (s->data == NULL)
    {
        printf("Could not allocate memory\n");
        exit(1);
    }
}
void ast_stack_free(ast_stack_t *s)
{
    for (int i = 0; i < s->length; i++)
        free_ast(s->data[i]);
    FREE(s->data);
    s->capacity = 0;
    s->length = 0;
}
void ast_stack_push(ast_stack_t *s, ast_t a)
{
    if (s->length == s->capacity)
    {
        s->capacity *= 2;
        s->data = realloc(s->data, sizeof(ast_t) * s->capacity);
        if (s->data == NULL)
        {
            printf("Could not allocate memory\n");
            exit(1);
        }
    }
    s->data[s->length++] = a;
}
ast_t ast_stack_pop(ast_stack_t *s)
{

    if (s->length == 0)
    {
        printf("Cannot pop empty ast stack\n");
        exit(2);
    }
    return s->data[--s->length];
}