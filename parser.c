#include "parser.h"
#include "string.h"

void parser_create(parser_t *p, lexer_t l)
{

    // copying token array
    token_array_create(&p->tokens);
    for (int i = 0; i < l.tokens.length; i++)
        token_array_push(&p->tokens, l.tokens.data[i]);
    stack_create(&p->scope_variables);
    p->ast = new_ast(empty_program());
    p->current = 0;
    ast_stack_create(&p->scope);
}

void parser_free(parser_t *p)
{
    for (int i = 0; i < p->tokens.length; i++)
        free(p->tokens.data[i].lexeme);
    token_array_free(&p->tokens);
    free_ast(p->ast);
    // check if scope is empty (it should be)
}

token_t current_token(parser_t p)
{
    return p.tokens.data[p.current];
}

parser_token_t get_type(token_t t)
{
    switch (t.type)
    {
    case KEYWORD:
    {
        if (strcmp(t.lexeme, "auto") == 0)
            return key_auto;
        if (strcmp(t.lexeme, "else") == 0)
            return key_else;
        if (strcmp(t.lexeme, "if") == 0)
            return key_if;
        if (strcmp(t.lexeme, "return") == 0)
            return key_return;
        if (strcmp(t.lexeme, "while") == 0)
            return key_while;
        if (strcmp(t.lexeme, "for") == 0)
            return key_for;
    }
    break;
    case DELIMITER:
    {
        switch (*t.lexeme)
        {
        case '(':
            return del_openparen;
        case ')':
            return del_closeparen;
        case '{':
            return del_openbra;
        case '}':
            return del_closebra;
        case ';':
            return del_semicol;
        case ',':
            return del_comma;
        default:
            return err_tok;
        }
    }
    break;
    case NUM_LIT:
        return tok_numlit;
    case STRING_LIT:
        return tok_strlit;
    case CHAR_LIT:
        return tok_charlit;
    case IDENTIFIER:
        return tok_iden;
    case OPERATION:
    {
        switch (strlen(t.lexeme))
        {
        case 1:
        {
            switch (*t.lexeme)
            {
            case '=':
                return op_eq;
            case '+':
                return op_plus;
            case '-':
                return op_minus;
            case '*':
                return op_mult;
            case '/':
                return op_div;
            case '%':
                return op_mod;
            case '!':
                return op_not;
            case '>':
                return op_grtr;
            case '<':
                return op_lssr;
            default:
                return err_tok;
            }
        }
        break;
        case 2:
        {
            if (strcmp(t.lexeme, "==") == 0)
                return op_assign;
            if (strcmp(t.lexeme, "&&") == 0)
                return op_and;
            if (strcmp(t.lexeme, "||") == 0)
                return op_or;
            return err_tok;
        }
        break;
        }
        break;
    }
    }
    return err_tok;
}

void step_parser(parser_t *p)
{
    token_t curr = current_token(*p);
    (void)curr;
}
