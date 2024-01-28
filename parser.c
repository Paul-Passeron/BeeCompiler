#include "parser.h"

void parser_create(parser_t *p, lexer_t l)
{

    // copying token array
    token_array_create(&p->tokens);
    for (int i = 0; i < l.tokens.length; i++)
        token_array_push(&p->tokens, l.tokens.data[i]);
    stack_create(&p->scope);
    p->ast = new_ast(empty_program());
    p->current = 0;
}

void parser_free(parser_t *p)
{
    for (int i = 0; i < p->tokens.length; i++)
        free(p->tokens.data[i].lexeme);
    token_array_free(&p->tokens);
    free_ast(p->ast);
}
