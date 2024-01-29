#include "parser.h"
#include "string.h"
#include "error.h"

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
    p->filename = l.filename;
    p->text = l.start;
}

void parser_free(parser_t *p)
{
    for (int i = 0; i < p->tokens.length; i++)
        free(p->tokens.data[i].lexeme);
    token_array_free(&p->tokens);
    free_ast(p->ast);
    // check if scope is empty (it should be)
}

token_t peek_token(parser_t p, int i, int *flag)
{
    if (p.current + 1 < p.tokens.length)
    {
        if (flag)
            *flag = 1;
        return p.tokens.data[p.current + i];
    }
    else if (flag != NULL)
        *flag = 0;
    return (token_t){0};
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

parser_token_t peek_type(parser_t p, int i, int *flag)
{
    token_t t = peek_token(p, i, flag);
    if (flag != NULL)
    {
        if (*flag)
            return get_type(t);
    }
    else
    {
        if (t.col == 0)
            return err_tok;
    }
    return get_type(t);
}

error_reporter_t create_error_p(parser_t p, err_type_t t, error_t err)
{
    error_reporter_t error;
    token_t tok = peek_token(p, 0, NULL);
    error.col = tok.col;
    error.line = tok.line;
    error.abs_offset = tok.abs_offset;
    error.filename = p.filename;
    error.text = p.text;
    error.t = t;
    error.error = err;
    return error;
}

token_array_t parse_functiondef_args(parser_t p, int begin, int l)
{
    // begin est l'indice du 1er argument
    token_array_t t;
    token_array_create(&t);
    int flip_flop = 0;
    for (int i = 0; i < l; i++)
    {
        token_t tok = peek_token(p, begin + i, NULL);
        if (!flip_flop && tok.type != IDENTIFIER)
        {
            // raise syntax error here
            // function def with invalid arguments
            // printf("TODO: Syntax error (invalid argument(s))\n");

            error_reporter_t err = create_error_p(p, SYNTAX_ERROR, INVALID_FUNDEF);
            print_syntax_error(err);
            exit(3);
        }
        if (!flip_flop)
            token_array_push(&t, tok);
        if (flip_flop)
        {
            parser_token_t type = get_type(tok);
            if (type != del_comma)
            {
                // raise syntax error here
                // function def with invalid separator between argument (not a comma)
                error_reporter_t err = create_error_p(p, SYNTAX_ERROR, INVALID_FUNCALL);
                print_syntax_error(err);
                exit(3);
            }
        }
        flip_flop = !flip_flop;
    }
    return t;
}

void parse_functiondef_no_body(parser_t *p, int k)
{
    // Problem if we get err_tok

    token_array_t args = parse_functiondef_args(*p, p->current + 2, k - 2);
    // printf("Number of arguments: %d\n", args.length);
    ast_t fun_def = new_ast((node_t){
        ast_function_def, {.ast_function_def = {.t = peek_token(*p, 0, NULL), .args = args, .body = NULL}}});
    ast_stack_push(&p->scope, fun_def);
    p->current += k;
}

// Maybe not useful as it will be handled by the litteral parser...
token_array_t *parse_functioncall_args(parser_t *p, int begin, int n, int *l)
{
    int length = 0;
    int capacity = 256;
    token_array_t *args = malloc(sizeof(token_array_t) * capacity);
    for (int i = 0; i < n; i++)
    {
        parser_token_t type = peek_type(*p, begin + i, NULL);
        if (type == del_comma)
        {
            if (length >= capacity - 1)
            {
                capacity *= 2;
                args = realloc(args, capacity);
                token_array_create(&args[length++]);
            }
        }
        else
        {
            token_array_push(&args[length - 1], peek_token(*p, begin + i, NULL));
        }
    }
    *l = length;
    return args;
}

void parse_functioncall_headonly(parser_t *p)
{
    ast_t fun_call = new_ast((node_t){
        ast_function_call, {.ast_function_call = {.t = peek_token(*p, 0, NULL), .args = NULL, .arity = -1, .capacity = -1}}});
    p->current += 1;
    ast_stack_push(&p->scope, fun_call);
}

void fold_scope(parser_t *p)
{
    if (p->scope.length == 0)
    {
        printf("Error: Cannot fold empty ast stack\n");
        exit(4);
    }
    ast_t popped = ast_stack_pop(&p->scope);
    if (p->scope.length == 1)
    {
        // Add it to the main program
        int *l = &p->ast->data.ast_program.length;
        int *cap = &p->ast->data.ast_program.length;

        if (*l >= *cap - 1)
        {
            *cap *= 2;
            p->ast->data.ast_program.program = realloc(p->ast->data.ast_program.program, *cap * sizeof(ast_t));
            p->ast->data.ast_program.program[*l++] = popped;
        }
        return;
    }
    ast_t folder = ast_stack_pop(&p->scope);

    switch (folder->tag)
    {
    case ast_bin_op:
    {
        struct ast_bin_op data = folder->data.ast_bin_op;
        if (data.l == NULL)
        {
            // There is a problem considering how we are parsing expressions for the moment
            printf("Error: Lhs NULL\n");
            exit(4);
                }
    }
    break;
    case ast_if_stat:
    {
        struct ast_if_stat data = folder->data.ast_if_stat;
    }
    break;
    case ast_for_loop:
    {
        struct ast_for_loop data = folder->data.ast_for_loop;
    }
    break;
    case ast_while_loop:
    {
        struct ast_while_loop data = folder->data.ast_while_loop;
    }
    break;
    case ast_function_def:
    {
        struct ast_function_def data = folder->data.ast_function_def;
    }
    break;
    case ast_function_call:
    {
        struct ast_function_call data = folder->data.ast_function_call;
    }
    break;
    case ast_assignement:
    {
        struct ast_assignement data = folder->data.ast_assignement;
    }
    break;
    case ast_identifier:
    {
        struct ast_identifier data = folder->data.ast_identifier;
    }
    break;
    case ast_literal:
    {
        struct ast_literal data = folder->data.ast_literal;
    }
    break;
    case ast_unary_op:
    {
        struct ast_unary_op data = folder->data.ast_unary_op;
    }
    break;
    case ast_expression:
    {
        struct ast_expression data = folder->data.ast_expression;
    }
    break;
    case ast_program:
    {
        struct ast_program data = folder->data.ast_program;
    }
    break;
    case ast_return:
    {
        struct ast_return data = folder->data.ast_return;
    }
    break;
    }

    ast_stack_push(&p->scope, folder);
}

void step_parser(parser_t *p)
{
    // Maybe delegate treating statements by other functions (mainly for operations)

    // Supposing there still are tokens
    parser_token_t curr = peek_type(*p, 0, NULL);

    if (curr == del_openbra)
    {
        // NEW SCOPE
        ast_t peeked = ast_stack_peek(&p->scope);
        if (peeked->tag == ast_function_call)
        {
            ast_t arg = new_ast((node_t){
                ast_expression, {.ast_expression = {.expression = NULL}}});
            ast_stack_push(&p->scope, arg);
            p->current++;
        }
        else
        {
            // TODO
        }
        p->current++;
    }
    else if (curr == del_closebra)
    {
        fold_scope(p);
        p->current++;
    }
    else if (curr == del_comma)
    {
        fold_scope(p);
        // make sure that we are in a function call
        ast_t peeked = ast_stack_peek(&p->scope);
        if (peeked->tag == ast_function_call)
        {
            ast_t arg = new_ast((node_t){
                ast_expression, {.ast_expression = {.expression = NULL}}});
            ast_stack_push(&p->scope, arg);
            p->current++;
        }
        else
        {
            p->current -= 3;
            if (p->current >= 0)
            {
                error_reporter_t err = create_error_p(*p, SYNTAX_ERROR, INVALID_FUNCALL);
                print_syntax_error(err);
            }
            else
            {
                error_reporter_t err = create_error_p(*p, SYNTAX_ERROR, -1);
                print_syntax_error(err);
            }
            exit(3);
        }
        p->current++;
    }
    else if (curr == del_closeparen)
    {
        // FOLD SCOPE => will have for instance successfully parsed arguments of function call
        p->current++;
    }
    // Trying to parse it as a function definition
    else if (curr == tok_iden)
    {
        parser_token_t next = peek_type(*p, 1, NULL);

        // IS FUNCTION DEF
        if (next == del_openparen)
        {
            int k;
            for (k = 1; peek_type(*p, k, NULL) != del_closeparen; k++)
                ;
            if (peek_type(*p, k + 1, NULL) == del_openbra)
            {
                parse_functiondef_no_body(p, k);
                return;
            }
            else
            {
                // IS FUNCTION_CALL
                parse_functioncall_headonly(p);
                return;
            }
        }
        else
        {
            // Add identifier to what should be an expression
            ast_t peeked = ast_stack_peek(&p->scope);
            if (peeked->tag != ast_expression)
            {
                printf("TODO: Problem: identifier can only be added to expressions\n");
                exit(3);
            }
            ast_t iden = new_ast((node_t){ast_identifier, {.ast_identifier = {.t = peek_token(*p, 0, NULL)}}});
            if (peeked->data.ast_expression.expression == NULL)
            {
                // be careful here
                peeked->data.ast_expression.expression = iden;
                return;
            }
            p->current++;
        }
    }
    else
    {
        p->current++;
    }
}