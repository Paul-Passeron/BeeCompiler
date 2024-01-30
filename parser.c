#include "parser.h"
#include "string.h"
#include "error.h"

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
    ast_t fun_def = new_ast((node_t){
        ast_function_def, {.ast_function_def = {.t = peek_token(*p, 0, NULL), .args = args, .body = NULL}}});
    ast_stack_push(&p->scope, fun_def);
    p->current += k + 1;
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
    ast_t fun_callargs = new_ast((node_t){
        ast_funccallargs, {.ast_funccallargs = {.args = malloc(sizeof(ast_t) * 256), .capacity = 256, .length = 0}}});
    ast_stack_push(&p->scope, fun_call);
    ast_stack_push(&p->scope, fun_callargs);
}

int is_empty_expression(ast_t a)
{
    if (a->tag != ast_expression)
        return 0;
    if (a->data.ast_expression.expression == NULL)
        return 1;
    return 0;
}

void fold_scope(parser_t *p)
{
    if (p->scope.length == 0)
    {
        return;
    }
    ast_t popped = ast_stack_pop(&p->scope);
    // printf("POPPED: ");
    // print_tag(*popped);
    // printf("\n");
    if (p->scope.length == 0)
    {
        ast_stack_push(&p->prog, popped);
        return;
    }

    if (is_empty_expression(popped))
    {
        return;
    }

    ast_t folder = ast_stack_pop(&p->scope);

    // printf("FOLDER: ");
    // print_tag(*folder);
    // printf("\n");
    switch (folder->tag)
    {
    case ast_bin_op:
    {
        struct ast_bin_op *data = &folder->data.ast_bin_op;
        if (data->l == NULL)
        {
            // There is a problem considering how we are parsing expressions for the moment
            printf("Error: Lhs NULL\n");
            exit(4);
        }
        // put it in the rhs
        data->l = popped;
        ast_stack_push(&p->scope, folder);
    }
    break;
    case ast_if_stat:
    {
        struct ast_if_stat data = folder->data.ast_if_stat;
        (void)data;
    }
    break;
    case ast_for_loop:
    {
        struct ast_for_loop data = folder->data.ast_for_loop;
        (void)data;
    }
    break;
    case ast_while_loop:
    {
        struct ast_while_loop data = folder->data.ast_while_loop;
        (void)data;
    }
    break;
    case ast_function_def:
    {
        struct ast_function_def *data = &folder->data.ast_function_def;
        data->body = popped;
        ast_stack_push(&p->scope, folder);
    }
    break;
    case ast_function_call:
    {
        struct ast_funccallargs arguments = popped->data.ast_funccallargs;
        struct ast_function_call *def = &folder->data.ast_function_call;
        def->args = arguments.args;
        def->capacity = arguments.capacity;
        def->arity = arguments.length;
        free(popped);
        ast_stack_push(&p->scope, folder);
    }
    break;
    case ast_assignement:
    {
        struct ast_assignement *data = &folder->data.ast_assignement;
        data->rhs = popped;
        ast_stack_push(&p->scope, folder);
        (void)data;
    }
    break;
    case ast_identifier:
    {
        struct ast_identifier data = folder->data.ast_identifier;
        (void)data;
    }
    break;
    case ast_literal:
    {
        struct ast_literal data = folder->data.ast_literal;
        (void)data;
    }
    break;
    case ast_unary_op:
    {
        struct ast_unary_op data = folder->data.ast_unary_op;
        (void)data;
    }
    break;
    case ast_expression:
    {
        struct ast_expression *data = &folder->data.ast_expression;
        if (data->expression == NULL)
        {
            data->expression = popped;
            ast_stack_push(&p->scope, folder);
        }
        else
        {
            // printf("Error: Could not fold expression\n");
            // exit(3);
            ast_stack_push(&p->scope, folder);
            fold_scope(p);
            ast_t expr = new_ast((node_t){
                ast_expression, {.ast_expression = {.expression = NULL}}});
            ast_stack_push(&p->scope, expr);
            ast_stack_push(&p->scope, popped);
            fold_scope(p);
        }
    }
    break;
    case ast_program:
    {
        struct ast_program data = folder->data.ast_program;
        (void)data;
    }
    break;
    case ast_return:
    {
        struct ast_return data = folder->data.ast_return;
        (void)data;
    }
    break;
    case ast_funccallargs:
    {
        struct ast_funccallargs *data = &folder->data.ast_funccallargs;
        data->args[data->length++] = popped;
        ast_stack_push(&p->scope, folder);
    }
    break;
    case ast_scope:
    {
        struct ast_scope *data = &folder->data.ast_scope;
        data->statements[data->length++] = popped;
        ast_stack_push(&p->scope, folder);
    }
    break;
    case ast_auto:
    {
        // Should be an lvalue but impossible since is parsed in the step_parse func
        printf("Error: Lonely auto keyword..\n");
        exit(5);
    }
    break;
    }
}
void step_parser(parser_t *p)
{

    parser_token_t curr = peek_type(*p, 0, NULL);
    // printf("CURR(%d, %d:%d): %d\n", p->current, p->scope.length, p->scope.length > 0 ? ast_stack_peek(&p->scope)->tag : -1, curr);
    if (curr == err_tok)
    {
        p->current++;
    }
    else if (curr == del_openbra)
    {

        ast_t s = new_ast((node_t){
            ast_scope, {.ast_scope = {.statements = malloc(sizeof(ast_t) * 256), .capacity = 256, .length = 0}}});
        ast_stack_push(&p->scope, s);
        ast_t expr = new_ast((node_t){
            ast_expression, {.ast_expression = {.expression = NULL}}});
        ast_stack_push(&p->scope, expr);
        p->current++;
    }
    else if (curr == del_closebra)
    {
        fold_scope(p);
        p->current++;
    }
    else if (curr == del_openparen)
    {
        ast_t expr = new_ast((node_t){
            ast_expression, {.ast_expression = {.expression = NULL}}});
        ast_stack_push(&p->scope, expr);
        p->current++;
    }
    else if (curr == del_closeparen)
    {
        fold_scope(p);
        p->current++;
    }
    else if (curr == del_comma)
    {
        fold_scope(p);
        ast_t arg = new_ast((node_t){
            ast_expression, {.ast_expression = {.expression = NULL}}});
        ast_stack_push(&p->scope, arg);
        p->current++;
    }
    else if (curr == del_semicol)
    {
        while (ast_stack_peek(&p->scope)->tag != ast_scope)
            fold_scope(p);
        ast_t arg = new_ast((node_t){
            ast_expression, {.ast_expression = {.expression = NULL}}});
        ast_stack_push(&p->scope, arg);
        p->current++;
    }
    else if (curr == tok_strlit || curr == tok_numlit || curr == tok_charlit)
    {
        if (p->scope.length > 0)
        {
            ast_t peeked = ast_stack_peek(&p->scope);
            if (is_empty_expression(peeked))
            {
                ast_t lit = new_ast((node_t){ast_literal, {.ast_literal = {.t = peek_token(*p, 0, NULL)}}});
                ast_t popped = ast_stack_pop(&p->scope);
                popped->data.ast_expression.expression = lit;
                ast_stack_push(&p->scope, popped);
            }
        }
        p->current++;
    }

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
        else if (next == op_assign)
        {
            ast_t assignement = new_ast((node_t){
                ast_assignement, {.ast_assignement = {.t = peek_token(*p, 0, NULL), .rhs = NULL}}});
            ast_stack_push(&p->scope, assignement);
            p->current++;
        }
        else
        {
            // Add identifier to what should be an expression
            ast_t peeked = ast_stack_peek(&p->scope);
            if (peeked->tag != ast_expression)
            {
                // printf("TODO: Problem: identifier can only be added to expressions. got: ");
                // print_tag(*peeked);
                // printf("\n");
                // exit(3);

                // No problem, let's deal with it
                ast_t popped = ast_stack_pop(&p->scope);
                ast_t expr = new_ast((node_t){ast_expression, {.ast_expression = {.expression = popped}}});
                ast_stack_push(&p->scope, expr);
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
    else if (curr == key_auto)
    {
        token_t var = peek_token(*p, 1, NULL);
        parser_token_t eq = peek_type(*p, 2, NULL);

        if (var.type != IDENTIFIER)
        {
            printf("Expected LHS\n");
            exit(10);
        }
        ast_t auto_var = new_ast((node_t){
            ast_auto, {.ast_auto = {.t = var}}});
        ast_stack_push(&p->scope, auto_var);
        fold_scope(p);
        if (eq == op_assign)
        {
            // gotta parse expression
            ast_t ass = new_ast((node_t){
                ast_assignement, {.ast_assignement = {.t = var}}});
            ast_stack_push(&p->scope, ass);

            p->current++;
        }
        else if (eq == del_semicol)
        {
            // we're basically done
        }
        ast_t expr = new_ast((node_t){
            ast_expression, {.ast_expression = {.expression = NULL}}});
        ast_stack_push(&p->scope, expr);
        p->current += 2;
    }

    else
    {
        p->current++;
    }
}

ast_t prog_to_ast(parser_t *p)
{

    return new_ast((node_t){
        ast_program,
        {.ast_program = {
             .capacity = p->prog.capacity,
             .length = p->prog.length,
             .program = p->prog.data}}});
}

void build_ast(parser_t *p)
{
    while (p->current < p->tokens.length)
        step_parser(p);
    while (p->scope.length > 0)
        fold_scope(p);
}