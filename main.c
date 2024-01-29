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

    print_token_array(lexer);

    parser_t parser;
    parser_create(&parser, lexer);

       // printf("Length: %d\n", parser.scope.length);
    build_ast(&parser);
    ast_t ast = prog_to_ast(&parser);

    printf("\n\n");
    pretty_print(ast);

    lexer_free(&lexer);
    parser_free(&parser);

    return 0;
}