#include <stdio.h>
#include "lexer.h"
#include <stdlib.h>
#include "parser.h"

void usage(char *name)
{
    printf("Usage:\n");
    printf("%s <input> <output>\n", name);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        usage(argv[0]);
        exit(1);
    }

    const char *input = argv[1];
    const char *output = argv[2];
    (void)output;

    lexer_t lexer;
    parser_t parser;
    lexer_create(&lexer, input);
    while (*lexer.remaining)
        step_lexer(&lexer);
    parser_create(&parser, lexer);
    ast_t ast = parse_program(&parser);

    free_ast(ast);
    parser_free(&parser);
    lexer_free(&lexer);
    return 0;
}