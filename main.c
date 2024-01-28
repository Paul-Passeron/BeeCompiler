#include <stdio.h>
#include "stack.h"
#include "lexer.h"
#include <stdlib.h>
#include "common.h"

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
    {
        step_lexer(&lexer);
    }
    print_token_array(lexer);

    lexer_free(&lexer);

    return 0;
}