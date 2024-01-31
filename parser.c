#include "parser.h"
#include "parser_tok.h"

// ast_t parse_expression(parser_t *p);
// ast_t parse_program(parser_t *p);
// ast_t parse_var_declaration(parser_t *p);
// ast_t parse_var_assignment(parser_t *p);
// ast_t parse_if_statement(parser_t *p);
// ast_t parse_for_loop(parser_t *p);
// ast_t parse_while_loop(parser_t *p);

ast_t parse_binary_expression(parser_t *p);
ast_t parse_function_call(parser_t *p);
ast_t parse_function_def_signature(parser_t *p);
ast_t parse_statement(parser_t *p);

ast_t build_ast(parser_t *p);
ast_t prog_to_ast(parser_t *p);

parser_token_t peek_next_type(parser_t p);
token_t peek_next_token(parser_t p);
parser_token_t peek_type(parser_t p, int k);
token_t peek_token(parser_t p, int k);

int is_eof(parser_t p)
{
    return p.current >= p.tokens.length;
}

void expect(parser_token_t expect, parser_token_t got)
{
    if (expect != got)
    {
        printf("Expected '");
        parser_tok_name(expect);
        printf("' token. Got: ");
        parser_tok_name(t);
        printf(".\n");
        exit(1);
    }
}

ast_t parse_var_declaration(parser_t *p)
{
    // We expect it to be auto (or maybe a type name in the future)
    // production rule:
    // <var_declaration> ::= auto <identifier>
    expect(key_auto, peek_next_type(*p));
    p->current++;
    token_t variable = peek_next_token(*p);
    expect(tok_iden, peek_next_type(*p));
    if (peek_type(*p, 1) == del_semicol)
        p->current++;
    else if (peek_type(*p, 1) != op_assign)
    {
        // There is a problem
        // maybe multiple variable names consecutively ?
        printf("TODO: Syntax Error: Invalid variable definition.\n");
        exit(1);
    }

    return new_ast((node_t){
        ast_auto, {.ast_auto.t = variable}});
}

ast_t parse_var_assignment(parser_t *p)
{
    // production rule:
    // <var_assignment> ::= <identifier> = <expression>
    // We expect an identifier and then assign sign
    expect(tok_iden, peek_next_type(*p));
    token_t variable = peek_next_token(*p);
    p->current++;
    expect(op_assign, peek_next_type(*p));
    p->current++;
    // Then we parse the expression
    return new_ast((node_t){
        ast_assignment, {.ast_assignment = {.t = variable, .rhs = parse_expression(p)}}});
}

ast_t parse_program(parser_t *p)
{
    // parse every statements until end of file
    // production rule:
    // <program> ::= <statement list>
    // <statement list> ::= <statement> | <statement> <statement_list>
    while (!is_eof(*p))
        ast_stack_push(&p->prog, parse_statement(p));
    return prog_to_ast(p);
}

ast_t parse_if_statement(parser_t *p)
{
    // production rule:
    // <if_statement> ::= if (<expression>) <statement> | if (<expression>) <statement> else <statement>
    // expect an if keyword
    expect(key_if, peek_next_type(*p));
    // then we expect open parenthesis to start condition expression
    p->current++;
    expect(del_openparen, peek_next_type(*p));
    p->current++;
    ast_t condition = parse_expression(p);
    // expecting the closing parenthesis signaling the end of the condition
    expect(del_closeparen, peek_next_type(*p));
    p->current++;
    // Then we parse the body of the if statement
    ast_t if_body = parse_statement(*p);
    ast_t else_body = NULL;
    // parsing the body of the else statement if there is one
    if (peek_next_type(*p) == key_else)
    {
        p->current++;
        else_body = parse_statement(*p);
    }
    return new_ast((node_t){
        ast_if_stat, {.ast_if_stat = {.body = if_body, .cond = condition, .other = else_body}}});
}

ast_t parse_for_loop(parser_t *p)
{
    // production rule:
    // <for_loop> ::= for (<expression> ; <expression>; <expression>) <statement>
    // We expect the for keyword
    expect(key_for, peek_next_type(*p));
    p->current++;
    // We expect an open parenthesis
    expect(del_openparen, peek_next_type(*p));
    p->current++;
    ast_t init = parse_expression(p);
    // Then we expect a semicolon
    expect(del_semicol, peek_next_type(*p));
    p->current++;
    ast_t condition = parse_expression(*p);
    // Then we expect the 2nd semicolon
    expect(del_semicol, peek_next_type(*p));
    ast_t iterator = parse_expression(*p);
    // We expect the final closing parenthesis
    expect(del_closeparen, peek_next_type(*p));
    p->current++;
    // parsing the body of the loop
    ast_t for_body = parse_statement(p);
    return new_ast((node_t){
        ast_for_loop, {.ast_for_loop = {.condition = condition, .init = init, .iterator = iterator, .body = for_body}}});
}

ast_t parse_while_loop(parser_t *p)
{
    // production rule:
    // <while_statement> ::= while (<expression>) <statement>
    // We expect the while keyword
    expect(key_while, peek_next_type(*p));
    p->current++;
    // We expect an open parenthesis
    expect(del_openparen, peek_next_type(*p));
    p->current++;
    // We parse the condition expression
    ast_t condition = parse_expression(p);
    // We expect a closed parenthesis
    expect(del_closeparen, peek_next_token(*p));
    // We parse the body of the while loop
    ast_t while_body = parse_statement(p);
    return new_ast((node_t){
        ast_while_loop, {.ast_while_loop = {.body = while_body, .cond = condition}}});
}