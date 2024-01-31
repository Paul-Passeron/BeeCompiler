#include "parser.h"
#include "parser_tok.h"
#include "error.h"
// ast_t parse_expression(parser_t *p);
// ast_t parse_program(parser_t *p);
// ast_t parse_var_declaration(parser_t *p);
// ast_t parse_var_assignment(parser_t *p);
// ast_t parse_if_statement(parser_t *p);
// ast_t parse_for_loop(parser_t *p);
// ast_t parse_while_loop(parser_t *p);
// ast_t parse_return_statement(parser_t *p)
// ast_t parse_function_call(parser_t *p);
// ast_t parse_function_def(parser_t *p);

ast_t parse_binary_expression(parser_t *p);
ast_t parse_function_def(parser_t *p);
ast_t parse_statement(parser_t *p);

ast_t build_ast(parser_t *p);
ast_t prog_to_ast(parser_t *p);

token_t peek_token(parser_t p, int k)
{
    if (p.current + k >= p.tokens.length)
        return (token_t){0};
    return p.tokens.data[p.current + k];
}

parser_token_t peek_type(parser_t p, int k)
{
    return get_type(peek_token(p, k));
}
token_t peek_next_token(parser_t p)
{
    return peek_token(p, 0);
}
parser_token_t peek_next_type(parser_t p)
{
    return peek_type(p, 0);
}

error_reporter_t create_error_p(parser_t p, err_type_t t, error_t err)
{
    error_reporter_t error;
    token_t tok = peek_next_token(p);
    error.col = tok.col;
    error.line = tok.line;
    error.abs_offset = tok.abs_offset;
    error.filename = p.filename;
    error.text = p.text;
    error.t = t;
    error.error = err;
    return error;
}

void parser_create(parser_t *p, lexer_t l)
{

    // copying token array
    token_array_create(&p->tokens);
    for (int i = 0; i < l.tokens.length; i++)
        token_array_push(&p->tokens, l.tokens.data[i]);
    stack_create(&p->scope_variables);
    p->current = 0;
    ast_stack_create(&p->scope);
    ast_stack_create(&p->prog);
    p->filename = l.filename;
    p->text = l.start;
}

void parser_free(parser_t *p)
{
    for (int i = 0; i < p->tokens.length; i++)
        free(p->tokens.data[i].lexeme);
    token_array_free(&p->tokens);
    ast_stack_free(&p->scope);
    ast_stack_free(&p->prog);
    stack_free(&p->scope_variables);
}

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
        parser_tok_name(got);
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
    ast_t if_body = parse_statement(p);
    ast_t else_body = NULL;
    // parsing the body of the else statement if there is one
    if (peek_next_type(*p) == key_else)
    {
        p->current++;
        else_body = parse_statement(p);
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
    ast_t condition = parse_expression(p);
    // Then we expect the 2nd semicolon
    expect(del_semicol, peek_next_type(*p));
    ast_t iterator = parse_expression(p);
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
    expect(del_closeparen, peek_next_type(*p));
    // We parse the body of the while loop
    ast_t while_body = parse_statement(p);
    return new_ast((node_t){
        ast_while_loop, {.ast_while_loop = {.body = while_body, .cond = condition}}});
}

ast_t parse_return_statement(parser_t *p)
{
    // production rule:
    // <return statement> ::= return <expression> ;
    // We expect the return keyword
    expect(key_return, peek_next_type(*p));
    p->current++;
    // We parse the expression that is returned
    ast_t returned_expression = parse_expression(p);
    // We expect a semicolon
    expect(del_semicol, peek_next_type(*p));
    return new_ast((node_t){
        ast_return, {.ast_return = {.expression = returned_expression}}});
}

ast_t parse_argument_list(parser_t *p)
{
    // production rule
    // <argument list> := expression | expression , <argument list>
    ast_stack_t s;
    ast_stack_create(&s);
    ast_stack_push(&s, parse_expression(p));
    while (peek_next_type(*p) == del_comma)
        ast_stack_push(&s, parse_expression(p));

    return new_ast((node_t){
        ast_funccallargs, {.ast_funccallargs = {.args = s.data, .capacity = s.capacity, .length = s.length}}});
}

ast_t parse_function_call(parser_t *p)
{
    // production rule:
    // <function call> ::= <identifier>(<argument list>) | <identifier>()
    // We expect an identifier
    expect(tok_iden, peek_next_type(*p));
    token_t function = peek_next_token(*p);
    p->current++;
    // We expect an open parenthesis
    expect(del_openparen, peek_next_type(*p));
    p->current++;
    // We check whether or not there are arguments
    ast_t *args = NULL;
    int arity = 0;
    int capacity = 0;

    if (peek_next_type(*p) != del_closeparen)
    {
        // Function call with at least 1 argument
        ast_t arg_list = parse_argument_list(p);
        args = arg_list->data.ast_funccallargs.args;
        arity = arg_list->data.ast_funccallargs.length;
        capacity = arg_list->data.ast_funccallargs.capacity;
    }
    return new_ast((node_t){
        ast_function_call, {.ast_function_call = {.args = args, .arity = arity, .capacity = capacity, .t = function}}});
}

ast_t parse_compound_statement(parser_t *p)
{
    // production rule:
    // <commpound statement> ::= {<statement list>}
    // reminder:
    // <statement list> ::= <statement> | <statement> <statement_list>
    // We expect an open bracket
    ast_stack_t scope;
    ast_stack_create(&scope);
    expect(del_openbra, peek_next_type(*p));
    p->current++;
    // parsing the statement list until we encounter the associated closing bracket
    while (peek_next_type(*p) != del_closebra)
        ast_stack_push(&scope, parse_statement(p));
    return new_ast((node_t){ast_scope, {.ast_scope = {.capacity = scope.capacity, .length = scope.length, .statements = scope.data}}});
}

ast_t parse_identifier(parser_t *p)
{
    // No need for production rule, identifiers are as defined by the lexer (see lexer.c)
    // We expect an identifier
    expect(tok_iden, peek_next_type(*p));
    token_t identifier = peek_next_token(*p);
    p->current++;
    return new_ast((node_t){
        ast_identifier, {.ast_identifier = {.t = identifier}}});
}

ast_stack_t parse_fundef_arg_list(parser_t *p)
{
    // production rule:
    // <fundef arg list > ::= <identifier> | <identifier>, <fundef arg list>
    // we expect an identifier
    ast_stack_t args;
    ast_stack_create(&args);

    expect(tok_iden, peek_next_type(*p));
    ast_stack_push(&args, parse_identifier(p));
    while (peek_next_type(*p) == del_comma)
        ast_stack_push(&args, parse_identifier(p));
    return args;
}

ast_t parse_function_def(parser_t *p)
{
    // production rule:
    // <function def> ::= <identifier>()<compound statement> | <identifier>(<fundef arg list>)<compound statement>
    // expect an identifier
    expect(tok_iden, peek_next_type(*p));
    token_t function = peek_next_token(*p);
    p->current++;
    // We expect an open parenthesis
    expect(del_openparen, peek_next_type(*p));
    p->current++;

    ast_stack_t args = {0};

    if (peek_next_type(*p) == del_closeparen)
    {
        // no argument in function def
        args = parse_fundef_arg_list(p);
    }
    // expect a closing parenthesis
    expect(del_closeparen, peek_next_type(*p));
    p->current++;
    ast_t body = parse_compound_statement(p);
    return new_ast((node_t){
        ast_function_def, {.ast_function_def = {.args = args.data, .arity = args.length, .capacity = args.capacity, .t = function, .body = body}}});
}