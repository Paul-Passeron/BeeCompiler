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
        free_ast(data.condition);
        free_ast(data.body);
        free_ast(data.iterator);
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
    case ast_assignment:
    {
        struct ast_assignment data = a->data.ast_assignment;
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
    case ast_expression:
    {
        struct ast_expression data = a->data.ast_expression;
        free_ast(data.expression);
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
    break;
    case ast_scope:
    {
        struct ast_scope data = a->data.ast_scope;
        for (int i = 0; i < data.length; i++)
        {
            free_ast(data.statements[i]);
        }
        free(data.statements);
    }
    break;
    case ast_funccallargs:
    {
        struct ast_funccallargs data = a->data.ast_funccallargs;
        for (int i = 0; i < data.length; i++)
        {
            free_ast(data.args[i]);
        }
        free(data.args);
    }
    break;
    case ast_auto:
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

    // TODO: There is a problem around here but I need to rewrite the freeing code anyway
    for (int i = 0; i < s->length - 1; i++)
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
    s->length -= 1;
    return s->data[s->length];
}

ast_t ast_stack_peek(ast_stack_t *s)
{

    if (s->length == 0)
    {
        printf("Cannot peek empty ast stack\n");
        exit(2);
    }
    return s->data[s->length - 1];
}

void print_tag(node_t t)
{

    switch (t.tag)
    {
    case ast_bin_op:
        printf("ast_bin_op");
        break;
    case ast_if_stat:
        printf("ast_if_stat");
        break;
    case ast_for_loop:
        printf("ast_for_loop");
        break;
    case ast_while_loop:
        printf("ast_while_loop");
        break;
    case ast_function_def:
        printf("ast_function_def");
        break;
    case ast_function_call:
        printf("ast_function_call");
        break;
    case ast_assignment:
        printf("ast_assignment");
        break;
    case ast_identifier:
        printf("ast_identifier");
        break;
    case ast_literal:
        printf("ast_literal");
        break;
    case ast_unary_op:
        printf("ast_unary_op");
        break;
    case ast_expression:
        printf("ast_expression");
        break;
    case ast_program:
        printf("ast_program");
        break;
    case ast_return:
        printf("ast_return");
        break;
    case ast_funccallargs:
        printf("ast_funccallargs");
        break;
    case ast_scope:
        printf("ast_scope");
        break;
    case ast_auto:
        printf("ast_auto");
        break;
    }
}

void pretty_print_aux(ast_t a, int prof)
{
    if (a == NULL)
        return;
    for (int i = 0; i < prof; i++)
        printf("   ");
    print_tag(*a);
    switch (a->tag)
    {
    case ast_bin_op:
    {
        struct ast_bin_op data = a->data.ast_bin_op;
        printf("%s:\n", data.t.lexeme);
        for (int i = 0; i < prof + 1; i++)
            printf("   ");
        printf("LHS:\n");
        pretty_print_aux(data.l, prof + 2);
        for (int i = 0; i < prof + 1; i++)
            printf("   ");
        printf("RHS:\n");
        pretty_print_aux(data.r, prof + 2);
    }
    break;
    case ast_if_stat:
    {
        struct ast_if_stat data = a->data.ast_if_stat;
        (void)data;
    }
    break;
    case ast_for_loop:
    {
        struct ast_for_loop data = a->data.ast_for_loop;
        (void)data;
    }
    break;
    case ast_while_loop:
    {
        struct ast_while_loop data = a->data.ast_while_loop;
        (void)data;
    }
    break;
    case ast_function_def:
    {
        struct ast_function_def data = a->data.ast_function_def;
        printf("\n");
        for (int i = 0; i < prof + 1; i++)
            printf("   ");
        printf("[name]: ");
        printf("%s\n", data.t.lexeme);
        if (data.args.length > 0)
        {
            for (int i = 0; i < prof + 1; i++)
                printf("   ");
            printf("[args]: ");
            for (int i = 0; i < data.args.length; i++)
            {
                printf("'%s' ", data.args.data[i].lexeme);
            }
            printf("\n");
        }
        for (int i = 0; i < prof + 1; i++)
            printf("   ");
        printf("[body]:\n");
        pretty_print_aux(data.body, prof + 2);
    }
    break;
    case ast_function_call:
    {
        struct ast_function_call data = a->data.ast_function_call;
        printf("\n");

        for (int i = 0; i < prof + 1; i++)
            printf("   ");
        printf("[name]: ");
        printf("%s\n", data.t.lexeme);
        if (data.arity > 0)
        {
            for (int i = 0; i < prof + 1; i++)
                printf("   ");
            printf("[args]:\n");
            for (int i = 0; i < data.arity; i++)
            {
                pretty_print_aux(data.args[i], prof + 2);
                printf("\n");
            }
        }
    }
    break;
    case ast_assignment:
    {
        struct ast_assignment data = a->data.ast_assignment;
        printf("\n");
        for (int i = 0; i < prof + 1; i++)
            printf("   ");
        printf("LHS: %s\n", data.t.lexeme);
        for (int i = 0; i < prof + 1; i++)
            printf("   ");
        printf("RHS:\n");
        pretty_print_aux(data.rhs, prof + 2);
    }
    break;
    case ast_identifier:
    {
        struct ast_identifier data = a->data.ast_identifier;
        printf(": %s\n", data.t.lexeme);
    }
    break;
    case ast_literal:
    {
        struct ast_literal data = a->data.ast_literal;
        printf("\n");
        for (int i = 0; i < prof + 2; i++)
            printf("   ");
        printf("%s\n", data.t.lexeme);
    }
    break;
    case ast_unary_op:
    {
        struct ast_unary_op data = a->data.ast_unary_op;
        (void)data;
    }
    break;
    case ast_expression:
    {
        struct ast_expression data = a->data.ast_expression;
        printf("\n");

        pretty_print_aux(data.expression, prof + 2);
    }
    break;
    case ast_program:
    {
        struct ast_program data = a->data.ast_program;
        printf("\n");

        for (int i = 0; i < data.length; i++)
            pretty_print_aux(data.program[i], prof + 2);
        printf("\n");
    }
    break;
    case ast_return:
    {
        struct ast_return data = a->data.ast_return;
        printf("\n");
        pretty_print_aux(data.expression, prof + 2);
    }
    break;
    case ast_funccallargs:
    {
        struct ast_funccallargs data = a->data.ast_funccallargs;
        (void)data;
    }
    break;
    case ast_scope:
    {
        struct ast_scope data = a->data.ast_scope;
        printf("\n");

        for (int i = 0; i < data.length; i++)
        {
            pretty_print_aux(data.statements[i], prof + 2);
            printf("\n");
        }
    }
    break;
    case ast_auto:
    {
        struct ast_auto data = a->data.ast_auto;
        printf(": %s\n", data.t.lexeme);
    }
    break;
    }
}

void pretty_print(ast_t a) { pretty_print_aux(a, 0); }

ast_t new_expr(void)
{
    return new_ast((node_t){
        ast_expression, {.ast_expression = {.expression = NULL}}});
}

ast_t wrap_in_expr(ast_t a)
{
    if (a->tag == ast_identifier)
    {
        printf("LEXEME OF ID IS : %s\n", a->data.ast_identifier.t.lexeme);
    }
    if (a->tag != ast_expression && a->tag != ast_scope && a->tag != ast_funccallargs && a->tag != ast_return)
    {
        ast_t expr = new_expr();
        expr->data.ast_expression.expression = a;
        return expr;
    }
    return a;
}