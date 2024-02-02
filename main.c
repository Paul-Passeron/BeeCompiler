#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    int use_fasm = 0;
    for (int i = 1; i < argc; i++)
    {
        if (*argv[i] == '-')
        {
            // This is a flag
            if (argv[i][1] == 't')
                print_tree = 1;
            else if (argv[i][1] == 'l')
                print_lexemes = 1;
            else if (argv[i][1] == 'c')
                use_fasm = 1;
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

    if (input == NULL)
    {
        printf("You should input an input filename\n");
        exit(1);
    }

    if (output == NULL)
    {
        printf("You should input an output filename\n");
        exit(1);
    }

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
    if (use_fasm)
    {
        generator_t g = create_generator(ast, output);
        generate_program(g);
        destroy_generator(g);
    }
    // free_ast(ast);

    char cmd[256];
    memset(cmd, 0, 256);
    parser_free(&parser);
    lexer_free(&lexer);
    if (use_fasm)
    {
        sprintf(cmd, "fasm %s %s.out && chmod u+x %s.out", output, output, output);
        printf("[CMD] : %s\n", cmd);
        system(cmd);
    }

    return 0;
}