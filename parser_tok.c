#include <stdio.h>
#include "parser_tok.h"
#include <string.h>
void parser_tok_name(parser_token_t t)
{
    switch (t)
    {

    case key_auto:
        printf("key_auto");
        break;
    case key_else:
        printf("key_else");
        break;
    case key_if:
        printf("key_if");
        break;
    case key_return:
        printf("key_return");
        break;
    case key_while:
        printf("key_while");
        break;
    case key_for:
        printf("key_for");
        break;
    case op_assign:
        printf("op_assign");
        break;
    case op_eq:
        printf("op_eq");
        break;
    case op_plus:
        printf("op_plus");
        break;
    case op_minus:
        printf("op_minus");
        break;
    case op_mult:
        printf("op_mult");
        break;
    case op_div:
        printf("op_div");
        break;
    case op_mod:
        printf("op_mod");
        break;
    case op_not:
        printf("op_not");
        break;
    case op_and:
        printf("op_and");
        break;
    case op_or:
        printf("op_or");
        break;
    case op_grtr:
        printf("op_grtr");
        break;
    case op_lssr:
        printf("op_lssr");
        break;
    case del_openbra:
        printf("del_openbra");
        break;
    case del_closebra:
        printf("del_closebra");
        break;
    case del_openparen:
        printf("del_openparen");
        break;
    case del_closeparen:
        printf("del_closeparen");
        break;
    case del_semicol:
        printf("del_semicol");
        break;
    case del_comma:
        printf("del_comma");
        break;
    case tok_iden:
        printf("tok_iden");
        break;
    case tok_strlit:
        printf("tok_strlit");
        break;
    case tok_numlit:
        printf("tok_numlit");
        break;
    case tok_charlit:
        printf("tok_charlit");
        break;
    default:
        printf("err_tok");
    }
}

int get_precedence(parser_token_t t)
{
    switch (t)
    {
    case op_assign:
        return 14;
    case op_eq:
        return 7;
    case op_diff:
        return 7;
    case op_plus:
        return 2;
    case op_minus:
        return 2;
    case op_mult:
        return 3;
    case op_div:
        return 3;
    case op_mod:
        return 3;
    case op_not:
        return 2;
    case op_and:
        return 11;
    case op_or:
        return 12;
    case op_grtr:
        return 6;
    case op_lssr:
        return 6;
    case op_grtr_eq:
        return 6;
    case op_lssr_eq:
        return 6;
    case op_deref:
        return 2;
    case op_incr:
        return 1;
    case op_decr:
        return 1;
    case op_address:
        return 2;
        // case del_openparen:
        //     return 1;
        // case del_closeparen:
        //     return 1;
    default:
        break;
    }
    return -1;
}

int get_assoc(parser_token_t t)
{
    // 0: ltor
    // 1: rtol
    switch (t)
    {
    case op_assign:
        return 1;
    case op_eq:
        return 0;
    case op_diff:
        return 0;
    case op_plus:
        return 0;
    case op_minus:
        return 0;
    case op_mult:
        return 0;
    case op_div:
        return 0;
    case op_mod:
        return 0;
    case op_not:
        return 1;
    case op_and:
        return 0;
    case op_or:
        return 0;
    case op_grtr:
        return 0;
    case op_lssr:
        return 0;
    case op_grtr_eq:
        return 0;
    case op_lssr_eq:
        return 0;
    case op_deref:
        return 1;
    case op_incr:
        return 0;
    case op_decr:
        return 0;
    case op_address:
        return 1;
        // case del_openparen:
        //     return 1;
        // case del_closeparen:
        //     return 1;
    default:
        break;
    }
    return -1;
}

int is_right_asso(parser_token_t t)
{
    return get_assoc(t) == 0;
}

parser_token_t get_type(token_t t)
{
    if (t.lexeme == NULL)
    {
        return err_tok;
    }
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
                return op_assign;
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
            case '&':
                return op_address;
            case '@':
                return op_deref;
            default:
                return err_tok;
            }
        }
        break;
        case 2:
        {
            if (strcmp(t.lexeme, "==") == 0)
                return op_eq;
            if (strcmp(t.lexeme, "&&") == 0)
                return op_and;
            if (strcmp(t.lexeme, "||") == 0)
                return op_or;
            if (strcmp(t.lexeme, "!=") == 0)
                return op_diff;
            if (strcmp(t.lexeme, ">=") == 0)
                return op_grtr_eq;
            if (strcmp(t.lexeme, "<=") == 0)
                return op_lssr_eq;
            return err_tok;
        }
        break;
        }
        break;
    }
    }
    return err_tok;
}
