#include "lexer.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser_tok.h"

extern int errno;

char *read_entire_file(char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        perror("Could not open file: ");
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

    token_array_free(&(l->tokens));
    FREE(l->start);
    FREE(l->filename);
}

token_t new_openpar(void)
{
    token_t res;
    res.abs_offset = -1;
    res.col = -1;
    res.line = -1;
    res.lexeme = malloc(1);
    *(res.lexeme) = '(';
    res.type = DELIMITER;
    return res;
}

int get_end_of_splitter(char *s)
{
    // List of splitters:
    // ' ', '\n', '=', '==', '+', '*', '-', '/', '%', '&&', '||', '(', ')', '{', '}', '>', '<', ';', '++', '--', '@(unary)', '&(unary)'

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
        if (s[0] == '!' && s[1] == '=')
            return 2;
        if (s[0] == '>' && s[1] == '=')
            return 2;
        if (s[0] == '<' && s[1] == '=')
            return 2;
    }
    char c = *s;
    if (c == '=' || c == '+' || c == '*' || c == '/' || c == '*' || c == '-' || c == '%' || c == '(' || c == ')' || c == '{' || c == '}' || c == '<' || c == '>' || c == ';' || c == ',' || c == '!' || c == '&' || c == '@')
        return 1;
    else
        return -1;
}

int is_keyword(char *s)
{

    char *keywords[] = {"auto", "else", "if", "return", "while", "for"};
    int n_k = sizeof(keywords) / sizeof(char *);
    for (int i = 0; i < n_k; i++)
    {
        if (strcmp(s, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

int is_identifier(char *s)
{
    char autorised[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
    int l = strlen(s);
    int l2 = sizeof(autorised);

    for (int i = 0; i < l; i++)
    {
        int cond = 0;
        for (int j = 0; j < l2; j++)
            cond |= s[i] == autorised[j];
        if (!cond)
            return 0;
    }
    return 1;
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
    case '&':
    case '@':
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

error_reporter_t create_error_l(lexer_t l, err_type_t t, error_t error)
{
    error_reporter_t err;
    err.col = l.col;
    err.line = l.line;
    err.abs_offset = l.remaining - l.start;
    err.text = l.start;
    err.filename = l.filename;
    err.t = t;
    err.error = error;
    return err;
}
void step_lexer(lexer_t *l)
{

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
            error_reporter_t syntax_err = create_error_l(*l, SYNTAX_ERROR, INVALID_STRLIT);
            print_syntax_error(syntax_err);
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
            error_reporter_t syntax_err = create_error_l(*l, SYNTAX_ERROR, UNCLOSED_STRLIT);
            print_syntax_error(syntax_err);
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
            error_reporter_t syntax_err = create_error_l(*l, SYNTAX_ERROR, INVALID_STRLIT);
            print_syntax_error(syntax_err);
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
            tok.abs_offset = l->remaining - l->start;
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
            error_reporter_t syntax_err = create_error_l(*l, SYNTAX_ERROR, INVALID_CHRLIT);
            print_syntax_error(syntax_err);
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

            error_reporter_t syntax_err = create_error_l(*l, SYNTAX_ERROR, UNCLOSED_CHRLIT);
            print_syntax_error(syntax_err);
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
            error_reporter_t syntax_err = create_error_l(*l, SYNTAX_ERROR, INVALID_CHRLIT);
            print_syntax_error(syntax_err);
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
            tok.abs_offset = l->remaining - l->start;
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
        tok.abs_offset = l->remaining - l->start;
        token_array_push(&l->tokens, tok);
        l->remaining += k - 1;
    }

    else if (is_splitter(*l->remaining))
    {
        int end = get_end_of_splitter(l->remaining);
        if (end == -1)
        {
            error_reporter_t syntax_err = create_error_l(*l, SYNTAX_ERROR, UNEXP_DELIM);
            print_syntax_error(syntax_err);
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
            tok.abs_offset = l->remaining - l->start;
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
        tok.abs_offset = l->remaining - l->start;

        if (is_keyword(buffer))
            tok.type = KEYWORD;
        else
        {
            if (is_identifier(tok.lexeme))
                tok.type = IDENTIFIER;
            else
            {
                // Syntax error
                error_reporter_t syntax_err = create_error_l(*l, SYNTAX_ERROR, UNEXP_CHAR);
                print_syntax_error(syntax_err);
                l->flag |= SYNTAX_ERROR;
                l->remaining++;
                l->col++;
                return;
            }
        }
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

void lex_prog(lexer_t *l)
{
    while (*(l->remaining))
        step_lexer(l);
}