#include "error.h"
#include <stdio.h>
#include "common.h"

void print_error_preface(error_reporter_t r)
{
    printf("\033[1m%s:%d:%d:\033[0m ", r.filename, r.line, r.col);
}

void print_syntax_error(error_reporter_t r)
{
    print_error_preface(r);
    printf("\033[1m\033[31mSyntax Error:\033[00m\033[0m ");
    char *remaining = r.text + r.abs_offset;
    switch (r.error)
    {
    case UNEXP_DELIM:
    {
        char c = *(remaining + 1);
        if (c && is_splitter(c) && !is_whitespace(c))
            printf("Unexpected delimeter '\033[1;33m%c%c\033[00m'.\n", *remaining, c);
        else
            printf("Unexpected delimeter '\033[1;33m%c\033[00m'.\n", *remaining);
    }
    break;
    case UNCLOSED_STRLIT:
        printf("Unclosed string literal: '\033[1;33m%.10s\033[00m...'\n", remaining);
        break;
    case INVALID_STRLIT:
        printf("Invalid string literal: '\033[1;33m%.10s\033[00m...'\n", remaining);
        break;
    case UNCLOSED_CHRLIT:
        printf("Unclosed char literal: '\033[1;33m%.10s\033[00m...'\n", remaining);
        break;
    case INVALID_CHRLIT:
        printf("Invalid char literal: '\033[1;33m%.10s\033[00m...'\n", remaining);
        break;
    case INVALID_FUNDEF:
        printf("Invalid function definition: '\033[1;33m%.11s\033[00m...'\n", remaining);
        break;
    case INVALID_FUNCALL:
        printf("Invalid function call: '\033[1;33m%.11s\033[00m...'\n", remaining);
        break;
    default:
        printf("Uknown error !\n");
        break;
    }
    fflush(stdout);
}
