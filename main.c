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

    parser_t parser;
    parser_create(&parser, lexer);

    build_ast(&parser);
    ast_t ast = prog_to_ast(&parser);

    pretty_print(ast);
    free(ast);
    parser_free(&parser);
    lexer_free(&lexer);
    return 0;
}