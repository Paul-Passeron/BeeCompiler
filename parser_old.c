#include "parser.h"
#include "string.h"
#include "error.h"

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
                printf("Type: %d\n", type);
                // raise syntax error here
                // function def with invalid separator between argument (not a comma)
                error_reporter_t err = create_error_p(p, SYNTAX_ERROR, INVALID_FUNDEF);
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

void fold_expressions(parser_t *p)
{
    if (p->scope.length > 1)
    {
        ast_t pop = ast_stack_pop(&p->scope);
        ast_t fall = ast_stack_pop(&p->scope);
        if (fall->tag == ast_expression)
        {
            free(fall); // OMG FREE FALL MENTIONNED !!!!
            ast_stack_push(&p->scope, pop);
            // fold_expressions(p);
        }
        else
        {
            ast_stack_push(&p->scope, fall);
            ast_stack_push(&p->scope, pop);
        }
    }
    else
        return;
}

void fold_scope(parser_t *p)
{
    fold_expressions(p);

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
        return;

    ast_t folder = ast_stack_pop(&p->scope);
    if (popped->tag == ast_expression && folder->tag == ast_expression)
    {
        if (folder->data.ast_expression.expression == NULL)
        {
            printf("Here\n");
            free(folder);
            ast_stack_push(&p->scope, popped);
            // fold_scope(p);
            return;
        }
    }
    popped = wrap_in_expr(popped);

    // printf("FOLDER: ");
    // print_tag(*folder);
    // printf("\n");
    switch (folder->tag)
    {
    case ast_bin_op:
    {
        printf("Here\n");
        struct ast_bin_op *data = &folder->data.ast_bin_op;
        if (data->l == NULL)
        {
            // There is a problem considering how we are parsing expressions for the moment
            printf("Error: Lhs NULL\n");
            exit(4);
        }
        // put it in the rhs
        data->r = popped;
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

        struct ast_return *data = &folder->data.ast_return;
        data->expression = popped;
        ast_stack_push(&p->scope, folder);
    }
    break;
    case ast_funccallargs:
    {
        struct ast_funccallargs *data = &folder->data.ast_funccallargs;
        data->args[data->length++] = popped;
        printf("Lexeme: %s\n", popped->data.ast_expression.expression->data.ast_identifier.t.lexeme);

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

int is_bin_op(parser_token_t t)
{
    return op_eq <= t && t <= op_diff;
}
int is_function_def(parser_t p)
{
    if (peek_type(p, 0, NULL) != tok_iden)
    {
        return 0;
    }
    if (peek_type(p, 1, NULL) != del_openparen)
        return 0;
    int k = 2;
    int scope = 1;
    while (p.current + k < p.tokens.length && scope > 0)
    {
        parser_token_t curr = peek_type(p, k, NULL);
        k++;
        if (curr == del_openbra || curr == del_openparen)
            scope++;
        else if (curr == del_closebra || curr == del_closeparen)
            scope--;
    }
    if (peek_type(p, k, NULL) == del_openbra)
        return 1;
    return 0;
}

int is_function_call(parser_t p)
{
    if (peek_type(p, 0, NULL) != tok_iden)
        return 0;
    if (peek_type(p, 1, NULL) != del_openparen)
        return 0;
    return !is_function_def(p);
}

ast_t parse_primary(parser_t *p)
{
    printf("Parse Primary\n");

    // parse single identifiers and function calls (no function defs)
    // parse literals
    // that's all ...?

    token_t tok = peek_token(*p, 0, NULL);
    parser_token_t t = get_type(tok);
    if (t == tok_iden)
    {
        if (is_function_call(*p))
        {
            // OMG a function call ???
            printf("That's a function call, well done !\n");
            parse_functioncall_headonly(p);
            int l = p->scope.length - 2;
            while (p->scope.length != l)
            {

                ast_t arg = parse_expression(p);
                ast_stack_push(&p->scope, arg);
                parser_token_t t = peek_type(*p, 0, NULL);

                if (!(t == tok_iden || t == tok_charlit || t == tok_numlit || t == tok_strlit))
                {
                    step_parser(p);
                }
            }

            return ast_stack_pop(&p->scope);
        }
        else
        {
            ast_t ide = new_ast((node_t){ast_identifier, {.ast_identifier = {.t = peek_token(*p, 0, NULL)}}});
            ast_t popped = ast_stack_pop(&p->scope);
            popped->data.ast_expression.expression = ide;
            p->current++;

            return popped;
        }
    }
    else if (t == tok_charlit || t == tok_strlit || t == tok_numlit)
    {
        // Litteral
        if (p->scope.length > 0)
        {
            ast_t peeked = ast_stack_peek(&p->scope);
            if (is_empty_expression(peeked))
            {
                ast_t lit = new_ast((node_t){ast_literal, {.ast_literal = {.t = peek_token(*p, 0, NULL)}}});
                ast_t popped = ast_stack_pop(&p->scope);
                popped->data.ast_expression.expression = lit;
                p->current++;
                return popped;
            }
        }
    }
    p->current++;
    return new_expr();
}
ast_t parse_expression(parser_t *p)
{
    printf("Parse Expression\n");
    // ast_stack_push(&p->scope, new_expr());
    // ast_stack_push(&p->scope, new_expr());
    ast_t prim = parse_primary(p);
    return parse_expression_1(prim, p, 0);
}

ast_t parse_expression_1(ast_t lhs, parser_t *p, int min_expression)
{
    printf("Parse Expression 1\n");
    token_t lookahead = peek_token(*p, 0, NULL);
    parser_token_t t = get_type(lookahead);
    printf("Parse Expression 2\n");
    printf("T is : %d\n", t);
    fflush(stdout);
    // ast_t lhs = ast_stack_pop(&p->scope);
    while (is_bin_op(t) && get_precedence(t) > min_expression)
    {

        token_t op = lookahead;
        ast_t rhs = parse_primary(p);
        // p->current++;

        lookahead = peek_token(*p, 0, NULL);
        t = get_type(lookahead);

        while ((is_bin_op(t) && get_precedence(t) > get_precedence(get_type(op))) || (is_right_asso(t) && get_precedence(t) == get_precedence(get_type(op))))
        {
            printf("Hereaaa\n");

            rhs = parse_expression_1(rhs, p, get_precedence(get_type(op)) + (get_precedence(t) > get_precedence(get_type(op)) ? 1 : 0));
            p->current++;
            lookahead = peek_token(*p, 1, NULL);
            t = get_type(lookahead);
        }
        lhs = new_ast((node_t){
            ast_bin_op, {.ast_bin_op = {.l = lhs, .r = rhs, .t = op}}});
    }
    // ast_stack_push(&p->scope, lhs);
    return lhs;
}

void step_parser(parser_t *p)
{
    // What it handles :

    //  - function defs  (done)
    //  - if statements
    //  - for and while loops
    // delegates the treatment of expressions (can ask to parse an expression starting at current token)
    printf("Step_Parser: %d\n", p->current);

    parser_token_t curr = peek_type(*p, 0, NULL);
    if (curr == err_tok)
        return;

    if (p->scope.length < 1)
        ast_stack_push(&p->scope, new_expr());
    else if (ast_stack_peek(&p->scope)->tag != ast_expression)
        ast_stack_push(&p->scope, new_expr());
    switch (curr)
    {
    case tok_iden:
    {
        // is it a function def ?
        if (is_function_def(*p))
        {
            printf("Here\n");
            fflush(stdout);
            int k = 2;
            int scope = 1;
            while (p->current + k < p->tokens.length && scope > 0)
            {
                parser_token_t curr = peek_type(*p, k, NULL);
                k++;
                if (curr == del_openbra || curr == del_openparen)
                    scope++;
                else if (curr == del_closebra || curr == del_closeparen)
                    scope--;
            }
            parse_functiondef_no_body(p, k - 1);
            ast_stack_push(&p->scope, new_expr());
        }
        // is it a function call ?
        // else if (is_function_call(*p))
        //    // parse_functioncall_headonly(p);

        // // ->simply a variable
        else
        {
            // Would that work with function calls tooo ????
            if (is_function_call(*p))
                printf("FunCall\n");
            else
                printf("Variable\n");
            ast_stack_push(&p->scope, parse_expression(p));
            // fold_scope(p);
        }
    }
    break;
    case del_closebra:
    {
        fold_scope(p);
        p->current++;
    }
    break;
    case del_closeparen:
    {
        fold_scope(p);
        p->current++;
    }
    break;
    case del_openbra:
    {
        ast_t s = new_ast((node_t){
            ast_scope, {.ast_scope = {.statements = malloc(sizeof(ast_t) * 256), .capacity = 256, .length = 0}}});
        ast_stack_push(&p->scope, s);
        ast_t expr = new_ast((node_t){
            ast_expression, {.ast_expression = {.expression = NULL}}});
        ast_stack_push(&p->scope, expr);
        p->current++;
    }
    break;
    case del_openparen:
    {
        ast_t expr = new_ast((node_t){
            ast_expression, {.ast_expression = {.expression = NULL}}});
        ast_stack_push(&p->scope, expr);
        p->current++;
    }
    break;
    case del_comma:
    {
        fold_scope(p);
        ast_t arg = new_ast((node_t){
            ast_expression, {.ast_expression = {.expression = NULL}}});
        ast_stack_push(&p->scope, arg);
        p->current++;
    }
    break;
    case del_semicol:
    {
        if (p->scope.length > 1)
            while (p->scope.length > 0 && ast_stack_peek(&p->scope)->tag != ast_scope)
                fold_scope(p);
        ast_t arg = new_ast((node_t){
            ast_expression, {.ast_expression = {.expression = NULL}}});
        ast_stack_push(&p->scope, arg);
        p->current++;
    }
    break;
    case tok_charlit:
    case tok_numlit:
    case tok_strlit:
        ast_stack_push(&p->scope, parse_expression(p));

        break;
    default:
        p->current++;
        break;
    }
}
void step_parser_old(parser_t *p)
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
        // p->current++;
        parse_expression(p);
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
            // parse_expression(p);
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
    else if (curr == key_return)
    {
        ast_t ret = new_ast((node_t){
            ast_return, {.ast_return = {.expression = NULL}}});
        ast_stack_push(&p->scope, ret);
        ast_t expr = new_ast((node_t){
            ast_expression, {.ast_expression = {.expression = NULL}}});
        ast_stack_push(&p->scope, expr);
        p->current++;
    }
    else if (is_bin_op(curr))
    {
        // ast_t lhs = ast_stack_pop(&p->scope);

        // ast_t plus = new_ast((node_t){
        //     ast_bin_op, {.ast_bin_op = {.l = lhs, .r = NULL, .t = peek_token(*p, 0, NULL)}}});
        // ast_stack_push(&p->scope, plus);
        // ast_t expr = new_ast((node_t){ast_expression, {.ast_expression = {.expression = NULL}}});
        // ast_stack_push(&p->scope, expr);
        // p->current++;

        // should not happen !!!

        printf("Unreachable (bin op) op\n");
        exit(7);
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