#include "lexer.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int errno;

int is_whitespace(char c)
{
    if (c == ' ' || c == '\n')
        return 1;
    return 0;
}

int is_splitter(char c)
{
    char splitters[] = " \n=+-*/;{}()<>\%|&,\"\'1234567890!";
    int splitters_count = sizeof(splitters) - 1;
    for (int i = 0; i < splitters_count; i++)
    {
        if (c == splitters[i])
            return 1;
    }
    return 0;
}

char *read_entire_file(char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        perror("Could not open file.\n");
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    size_t length = ftell(f);

    fseek(f, 0, SEEK_SET);
    char *buffer = malloc(length + 1);

    if (!fread(buffer, 1, length, f))
    {
        perror("Could not read file.\n");
        fclose(f);
        exit(2);
    }
    buffer[length] = 0;
    fclose(f);
    return buffer;
}

void print_error_preface(lexer_t l)
{
    printf("\033[1m%s:%d:%d:\033[0m ", l.filename, l.line, l.col);
}

void print_syntax_error(lexer_t l, syntax_error_t err)
{
    print_error_preface(l);
    printf("\033[1m\033[31mSyntax Error:\033[00m\033[0m ");
    switch (err)
    {
    case UNEXP_DELIM:
    {
        char c = *(l.remaining + 1);
        if (c && is_splitter(c) && !is_whitespace(c))
            printf("Unexpected delimeter '\033[1;33m%c%c\033[00m'.\n", *l.remaining, c);
        else
            printf("Unexpected delimeter '\033[1;33m%c\033[00m'.\n", *l.remaining);
    }
    break;
    case UNCLOSED_STRLIT:
        printf("Unclosed string literal: '\033[1;33m%.10s\033[00m...'\n", l.remaining);
        break;
    case INVALID_STRLIT:
        printf("Invalid string literal: '\033[1;33m%.10s\033[00m...'\n", l.remaining);
        break;
    case UNCLOSED_CHRLIT:
        printf("Unclosed char literal: '\033[1;33m%.10s\033[00m...'\n", l.remaining);
        break;
    case INVALID_CHRLIT:
        printf("Invalid char literal: '\033[1;33m%.10s\033[00m...'\n", l.remaining);
        break;
    default:
        break;
    }
    fflush(stdout);
}

void lexer_create(lexer_t *l, char *filename)
{
    l->col = 1;
    l->line = 1;
    token_array_create(&(l->tokens));
    l->start = read_entire_file(filename);
    l->remaining = l->start;
    int n = strlen(filename);
    l->filename = malloc(n);
    memcpy(l->filename, filename, n);
    l->flag = 0;
    l->file_length = n + 2;
}

void lexer_free(lexer_t *l)
{
    // No need to free tokens lexemes as they are used in the parser
    /*for (int i = 0; i < l->tokens.length; i++)
        free(l->tokens.data[i].lexeme);*/
    token_array_free(&(l->tokens));
    FREE(l->start);
    FREE(l->filename);
}

int get_end_of_splitter(char *s)
{
    // List of splitters:
    // ' ', '\n', '=', '==', '+', '*', '-', '/', '%', '&&', '||', '(', ')', '{', '}', '>', '<', ';', '++', '--'

    if (strlen(s) > 1)
    {
        if (s[0] == '=' && s[1] == '=')
            return 2;
        if (s[0] == '&' && s[1] == '&')
            return 2;
        if (s[0] == '|' && s[1] == '|')
            return 2;
        if (s[0] == '+' && s[1] == '+')
            return 2;
        if (s[0] == '-' && s[1] == '-')
            return 2;
    }
    char c = *s;
    if (c == '=' || c == '+' || c == '*' || c == '/' || c == '*' || c == '-' || c == '%' || c == '(' || c == ')' || c == '{' || c == '}' || c == '<' || c == '>' || c == ';' || c == ',' || c == '!')
        return 1;
    else
        return -1;
}

int is_keyword(char *s)
{
    char *keywords[] = {"auto", "else", "if", "return", "while", "for"};
    int n_k = sizeof(keywords) / sizeof(char *) - 1;
    for (int i = 0; i < n_k; i++)
    {
        if (strcmp(s, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

int get_next_split_offset(char *s)
{
    char *start = s;
    while (*s)
    {
        if (is_splitter(*s))
            return s - start;
        s++;
    }
    return s - start;
}

token_type_t delim_type(char *buffer)
{
    if (strlen(buffer) == 2)
        return OPERATION;
    switch (*buffer)
    {
    case '+':
    case '-':
    case '=':
    case '*':
    case '/':
    case '%':
    case '<':
    case '>':
    case '!':
        return OPERATION;
    default:
        return DELIMITER;
    }
}

int is_int_litteral(char *s, int k)
{

    // Only for base 10 for the moment
    int l = strlen(s);
    for (int i = 0; i < l && i < k; i++)
    {
        if (s[i] < '0' || s[i] > '9' || s[i] == '\n' || s[i] == ' ')
            return 0;
    }
    return 1;
}

int is_string_litteral(char *s)
{
    int l = strlen(s);
    if (s[0] != '\"' || s[l - 1] != '\"')
        return 0;
    for (int i = 1; i < l - 1; i++)
        if (s[i] == '\"')
            return 0;
    return 1;
}

int is_char_litteral(char *s)
{
    int l = strlen(s);
    if (l == 3)
        return s[0] == '\'' && s[2] == '\'';
    if (l == 4)
        return s[0] == '\'' && s[3] == '\'' && s[1] == '\\';
    return strcmp(s, "\\000") == 0;
}

void step_lexer(lexer_t *l)
{
    // No support for string, char and int litterals

    while (*l->remaining && is_whitespace(*l->remaining))
    {
        if (!*l->remaining)
            return;
        if (*l->remaining == ' ')
            l->col++;
        else
        {
            l->line++;
            l->col = 1;
        }
        l->remaining++;
    }

    if (!*l->remaining)
        return;
    // string lit
    if (*l->remaining == '\"')
    {
        char *start = l->remaining;
        int col = l->col;
        int line = l->line;
        l->remaining += 1;
        while (*l->remaining != '\"' && *l->remaining != 0 && *l->remaining != '\n')
        {
            l->col += 1;
            l->remaining += 1;
        }

        char *buffer = malloc(l->remaining - start + 2);
        memcpy(buffer, start, l->remaining - start + 1);
        buffer[l->remaining - start + 1] = 0;
        if (*l->remaining == '\n')
        {
            l->line += 1;
            l->col = 1;
            int tmp1 = l->col;
            int tmp2 = l->line;
            l->col = col;
            l->line = line;
            char *tmp3 = l->remaining;
            l->remaining = start;
            print_syntax_error(*l, INVALID_STRLIT);
            l->col = tmp1;
            l->line = tmp2;
            l->remaining = tmp3;
        }

        else if (*l->remaining != '\"')
        {
            int tmp1 = l->col;
            int tmp2 = l->line;
            l->col = col;
            l->line = line;
            char *tmp3 = l->remaining;
            l->remaining = start;
            print_syntax_error(*l, UNCLOSED_STRLIT);
            l->col = tmp1;
            l->line = tmp2;
            l->remaining = tmp3;
        }
        else if (!is_string_litteral(buffer))
        {
            int tmp1 = l->col;
            int tmp2 = l->line;
            l->col = col;
            l->line = line;
            char *tmp3 = l->remaining;
            l->remaining = start;
            print_syntax_error(*l, INVALID_STRLIT);
            l->col = tmp1;
            l->line = tmp2;
            l->remaining = tmp3;
        }
        else
        {
            // create token
            token_t tok;
            tok.col = col;
            tok.line = line;
            tok.lexeme = buffer;
            tok.type = STRING_LIT;
            token_array_push(&l->tokens, tok);
            l->remaining += 1;
        }
    }

    // char lit
    else if (*l->remaining == '\'')
    {
        char *start = l->remaining;
        int col = l->col;
        int line = l->line;
        l->remaining += 1;
        while (*l->remaining != '\'' && *l->remaining != 0 && *l->remaining != '\n')
        {
            l->col += 1;
            l->remaining += 1;
        }

        char *buffer = malloc(l->remaining - start + 2);
        memcpy(buffer, start, l->remaining - start + 1);
        buffer[l->remaining - start + 1] = 0;
        if (*l->remaining == '\n')
        {
            l->line += 1;
            l->col = 1;
            int tmp1 = l->col;
            int tmp2 = l->line;
            l->col = col;
            l->line = line;
            char *tmp3 = l->remaining;
            l->remaining = start;
            print_syntax_error(*l, INVALID_CHRLIT);
            l->col = tmp1;
            l->line = tmp2;
            l->remaining = tmp3;
        }

        else if (*l->remaining != '\'')
        {
            int tmp1 = l->col;
            int tmp2 = l->line;
            l->col = col;
            l->line = line;
            char *tmp3 = l->remaining;
            l->remaining = start;
            print_syntax_error(*l, UNCLOSED_CHRLIT);
            l->col = tmp1;
            l->line = tmp2;
            l->remaining = tmp3;
        }
        else if (!is_char_litteral(buffer))
        {
            int tmp1 = l->col;
            int tmp2 = l->line;
            l->col = col;
            l->line = line;
            char *tmp3 = l->remaining;
            l->remaining = start;
            print_syntax_error(*l, INVALID_CHRLIT);
            l->col = tmp1;
            l->line = tmp2;
            l->remaining = tmp3 + 1;
        }
        else
        {
            // create token
            token_t tok;
            tok.col = col;
            tok.line = line;
            tok.lexeme = buffer;
            tok.type = CHAR_LIT;
            token_array_push(&l->tokens, tok);
            l->remaining += 1;
        }
    }

    else if (is_int_litteral(l->remaining, 1))
    {
        int k = 1;
        int col = l->col;
        int line = l->line;

        while (l->remaining[k - 1] <= '9' && l->remaining[k - 1] >= '0')
        {
            l->col += 1;
            k += 1;
        }
        char *buffer = malloc(k);
        buffer[k - 1] = 0;
        memcpy(buffer, l->remaining, k - 1);
        token_t tok;
        tok.col = col;
        tok.line = line;
        tok.lexeme = buffer;
        tok.type = NUM_LIT;
        token_array_push(&l->tokens, tok);
        l->remaining += k - 1;
    }

    else if (is_splitter(*l->remaining))
    {
        int end = get_end_of_splitter(l->remaining);
        if (end == -1)
        {
            print_syntax_error(*l, UNEXP_DELIM);
            l->flag |= SYNTAX_ERROR;
            l->remaining++;
            l->col++;
            return;
        }
        else
        {
            char *buffer = malloc(end);
            memcpy(buffer, l->remaining, end + 1);
            buffer[end] = 0;
            token_t tok;
            tok.col = l->col;
            tok.line = l->line;
            tok.lexeme = buffer;
            tok.type = delim_type(buffer);
            token_array_push(&l->tokens, tok);
            l->col += end;
            l->remaining += end;
        }
    }
    else
    {
        int next_split_offset = get_next_split_offset(l->remaining);
        char *buffer = malloc(next_split_offset + 1);
        memcpy(buffer, l->remaining, next_split_offset);
        buffer[next_split_offset] = 0;
        token_t tok;
        tok.col = l->col;
        tok.line = l->line;
        tok.lexeme = buffer;

        if (is_keyword(buffer))
            tok.type = KEYWORD;
        else
            tok.type = IDENTIFIER;

        token_array_push(&l->tokens, tok);
        l->col += next_split_offset;
        l->remaining += next_split_offset;

        // TODO: add syntax error when identifier doesnt exist
    }
}

void print_token_array(lexer_t l)
{
    for (int i = 0; i < l.tokens.length; i++)
    {
        print_token(l.tokens.data[i]);
        printf("\n");
    }
}
