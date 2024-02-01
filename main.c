#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"
#include "generator.h"

void usage(char *name)
{
    printf("Usage:\n");
    printf("%s <input> <output>\n", name);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        usage(argv[0]);
        exit(1);
    }

    char *input = NULL;
    char *output = NULL;
    int print_tree = 0;
    int print_lexemes = 0;
    for (int i = 1; i < argc; i++)
    {
        if (*argv[i] == '-')
        {
            // This is a flag
            if (argv[i][1] == 't')
                print_tree = 1;
            else if (argv[i][1] == 'l')
                print_lexemes = 1;
            else
            {
                printf("Unknown flag '%.2s'\n", argv[i]);
                exit(1);
            }
        }
        else if (input == NULL)
            input = argv[i];
        else if (output == NULL)
            output = argv[i];
    }
    (void)output;

    lexer_t lexer;
    parser_t parser;
    lexer_create(&lexer, input);
    lex_prog(&lexer);
    if (print_lexemes)
        print_token_array(lexer);
    parser_create(&parser, lexer);
    ast_t ast = parse_program(&parser);
    if (print_tree)
        pretty_print(ast);
    free_ast(ast);
    parser_free(&parser);
    lexer_free(&lexer);
    return 0;
}