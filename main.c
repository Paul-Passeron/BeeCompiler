#include <stdio.h>
#include "lexer.h"
#include <stdlib.h>
#include "parser.h"

void usage(char *name)
{
    printf("Usage:\n");
    printf("%s <filename>\n", name);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        usage(argv[0]);
        exit(1);
    }

    lexer_t lexer;
    lexer_create(&lexer, argv[1]);
    while (*lexer.remaining)
        step_lexer(&lexer);

    // print_token_array(lexer);

    parser_t parser;
    parser_create(&parser, lexer);

    while (parser.current < parser.tokens.length)
        step_parser(&parser);

    printf("Length: %d\n", parser.scope.length);
    int i = 0;
    while (parser.scope.length > 0)
    {
        ast_t popped = ast_stack_pop(&parser.scope);
        printf("%d: %d\n", i, popped->tag);
        i++;
        // free_ast(popped);
    }

    lexer_free(&lexer);
    parser_free(&parser);

    return 0;
}