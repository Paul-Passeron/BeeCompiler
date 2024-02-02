#ifndef PARSER_TOK_H
#define PARSER_TOK_H
#include "token.h"
typedef enum
{
    // keywords
    key_auto,
    key_else,
    key_if,
    key_return,
    key_while,
    key_for,

    // operators
    op_assign,
    op_eq,
    op_plus,
    op_minus,
    op_mult,
    op_div,
    op_mod,
    op_not,
    op_and,
    op_or,
    op_grtr,
    op_lssr,
    op_lssr_eq,
    op_grtr_eq,
    op_diff,
    op_deref,
    op_address,
    op_incr,
    op_decr,
    op_bit_and,
    op_bit_or,
    op_bit_xor,
    // delimeters
    del_openbra,
    del_closebra,
    del_openparen,
    del_closeparen,
    del_semicol,
    del_comma,

    // others
    tok_iden,
    tok_strlit,
    tok_numlit,
    tok_charlit,
    tok_type,

    err_tok,

} parser_token_t;

void parser_tok_name(parser_token_t t);
// int is_right_asso(parser_token_t t);
parser_token_t get_type(token_t t);
// int get_assoc(parser_token_t t);
// int get_precedence(parser_token_t t);

#endif // PARSER_TOK_H