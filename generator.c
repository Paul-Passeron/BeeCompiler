#include "generator.h"
#include <stdlib.h>
#include <string.h>
#include "fundef_table.h"

int label = 0;
int scope = 0;
generator_t create_generator(ast_t a, char *filename)
{
    FILE *f = fopen(filename, "wb");
    if (f == NULL)
    {
        perror("Could not open file: ");
        exit(1);
    }
    stack_t *s = malloc(sizeof(stack_t));
    stack_create(s);

    // adding putchar as a preloaded function for the moment
    fundef_t putchar_fundef = fundef_create();
    putchar_fundef.args_types[0] = (type_t){.type = type_char_t, .t = NULL};
    putchar_fundef.arity = 1;
    putchar_fundef.ret_type = (type_t){.type = type_void, .t = NULL};
    generator_t g = {
        .ast = a,
        .scope = s,
        .out = f};
    fundef_table_create(&g.table);
    fundef_table_append(&g.table, putchar_fundef, "putchar");
    load_functions(&g.table, a);
    return g;
}

void destroy_generator(generator_t g)
{
    fclose(g.out);
    stack_free(g.scope);
}

void print_entry(generator_t g)
{
    FILE *f = g.out;
    fprintf(f, "format ELF64 executable 3\n");
    fprintf(f, "segment readable executable\n");
    fprintf(f, "entry start\n");
    fprintf(f, "FUNCTION_putchar:\n");
    fprintf(f, "    push rbp\n");
    fprintf(f, "    mov rbp, rsp\n");
    fprintf(f, "    mov rax, 1\n");
    fprintf(f, "    mov rdi, 1\n");
    fprintf(f, "    mov rsi, rbp\n");
    fprintf(f, "    add rsi, 16\n");
    fprintf(f, "    mov rdx, 1\n");
    fprintf(f, "    syscall\n");
    fprintf(f, "    pop rbp\n");
    fprintf(f, "    ret\n");
    fprintf(f, "start:\n");
    fprintf(f, "    call FUNCTION_main\n");
}

void print_exit(generator_t g)
{
    FILE *f = g.out;
    fprintf(f, "    mov rdi, rax\n");
    fprintf(f, "    mov rax, 60\n");
    // fprintf(f, "    mov rdi, %d\n", exit_code);
    fprintf(f, "    syscall\n");
}

void print_function_intro(generator_t g, char *function_name)
{
    FILE *f = g.out;
    fprintf(f, "FUNCTION_%s:\n", function_name);
    fprintf(f, "    push rbp\n");
    fprintf(f, "    mov rbp, rsp\n");
    fprintf(f, "    sub rsp, 64\n");
}

void print_function_outro(generator_t g)
{
    FILE *f = g.out;
    fprintf(f, "    add rsp, 64\n");
    fprintf(f, "    pop rbp\n");
    fprintf(f, "    ret\n");
}

int generate_label(generator_t g)
{
    // FILE *f = g.out;
    (void)g;
    label++;
    // fprintf(f, "label_%d:\n", label++);
    return label -
           1;
}

void generate_expression(generator_t g, ast_t expression)
{
    FILE *f = g.out;

    // move the result in rax
    if (expression->tag == ast_identifier)
    {
        token_t t = expression->data.ast_identifier.t;
        int found;
        stack_val_t e = get_scope_elem(t.lexeme, *g.scope, &found);
        if (!found)
        {
            printf("Identifier not found :'%s'!\n", t.lexeme);
            exit(1);
        }
        else
        {
            // maybe will have to put specifier, e.g byte, word, byte ptr...
            char c = '+';
            if (!e.is_arg)
                c = '-';
            fprintf(f, "    mov rax, [rbp %c %d]\n ", c, e.address);
        }
    }
    else if (expression->tag == ast_literal)
    {
        token_t t = expression->data.ast_literal.t;
        if (strcmp(t.lexeme, "\'\\n\'") == 0)
            fprintf(f, "    mov rax, 10\n");
        else
            fprintf(f, "    mov rax, %s\n", t.lexeme);
    }
    else if (expression->tag == ast_function_call)
        generate_function_call(g, expression);
    else if (expression->tag == ast_bin_op)
    {
        FILE *f = g.out;
        struct ast_bin_op data = expression->data.ast_bin_op;
        generate_expression(g, data.l);
        fprintf(f, "    push rax\n");
        // fprintf(f, "    mov rax, 0\n");
        generate_expression(g, data.r);
        fprintf(f, "    pop rbx\n");

        if (get_type(data.t) == op_plus)
            fprintf(f, "    add rax, rbx\n");
        if (get_type(data.t) == op_minus)
        {
            fprintf(f, "    sub rbx, rax\n");
            fprintf(f, "    mov rax, rbx\n");
        }
        else if (get_type(data.t) == op_mult)
            fprintf(f, "    mul rbx\n");
        else if (get_type(data.t) == op_div)
        {
            fprintf(f, "    mov rcx, rax\n");
            fprintf(f, "    mov rax, rbx\n");
            fprintf(f, "    mov rbx, rcx\n");
            fprintf(f, "    cdq\n");
            fprintf(f, "    mov rdx, 0\n");
            fprintf(f, "    div rbx\n");
        }
        else if (get_type(data.t) == op_mod)
        {
            fprintf(f, "    mov rdx, 0\n");
            fprintf(f, "    mov rcx, rax\n");
            fprintf(f, "    mov rax, rbx\n");
            fprintf(f, "    mov rbx, rcx\n");
            fprintf(f, "    cdq\n");
            fprintf(f, "    div rbx\n");
            fprintf(f, "    mov rax, rdx\n");
        }
    }
}

void generate_simple_assignment(generator_t g, char *name, ast_t rhs)
{
    FILE *f = g.out;
    int found = 0;
    scope_elem_t var = get_scope_elem(name, *g.scope, &found);
    if (!found)
    {
        printf("Identifier '%s' not found.\n", name);
        exit(1);
    }
    generate_expression(g, rhs);
    fprintf(f, "    mov rbx, rbp\n");
    fprintf(f, "    add rbx, %d\n", var.address);
    fprintf(f, "    mov [rbx], rax\n");
}

void generate_variable_declaration(generator_t g, ast_t decl)
{
    // FILE *f = g.out;

    struct ast_auto data = decl->data.ast_auto;
    char *name = data.t.lexeme;
    int size = size_of_type(*data.type);
    scope_elem_t var;
    var.address = 16;

    if (g.scope->length > 0)
    {
        scope_elem_t last_var = g.scope->data[g.scope->length - 1];
        var.address = last_var.address + last_var.n_bytes;
    }
    var.n_bytes = size;
    var.identifier = name;
    var.scope_index = scope;
    var.is_arg = 0;
    stack_push(g.scope, var);
    // printf("Pushed !\n");

    if (data.rhs != NULL)
    {
        // Rhs now
        // generate_expression(g, data.rhs);
        // fprintf(f, "    mov rbx, rbp\n");
        // fprintf(f, "    add rbx, %d\n", var.address);
        // fprintf(f, "    mov [rbx], rax\n");
        generate_simple_assignment(g, name, data.rhs);
    }
}

void generate_general_assignment(generator_t g, ast_t assign)
{
    struct ast_assignment data = assign->data.ast_assignment;
    // Temporary solution
    generate_simple_assignment(g, data.t.lexeme, data.rhs);
}

void generate_return(generator_t g, ast_t ret_stmt)
{
    struct ast_return data = ret_stmt->data.ast_return;
    generate_expression(g, data.expression);
    print_function_outro(g);
}

void generate_if_else_statement(generator_t g, ast_t stmt)
{
    FILE *f = g.out;
    struct ast_if_stat data = stmt->data.ast_if_stat;
    // int test_cond = generate_label(g);
    int else_body = generate_label(g);
    int end_body = generate_label(g);
    // fprintf(f, "label_%d:\n", test_cond);
    generate_expression(g, data.cond);
    fprintf(f, "    cmp rax, 0\n");
    fprintf(f, "    je label_%d\n", else_body);
    generate_statement(g, data.body);
    fprintf(f, "    jmp label_%d\n", end_body);
    fprintf(f, "label_%d:\n", else_body);
    if (data.other != NULL)
        generate_statement(g, data.other);
    fprintf(f, "label_%d:\n", end_body);
}

void generate_while_statement(generator_t g, ast_t stmt)
{
    FILE *f = g.out;
    struct ast_while_loop data = stmt->data.ast_while_loop;
    int test_cond = generate_label(g);
    int end_body = generate_label(g);
    fprintf(f, "label_%d:\n", test_cond);
    generate_expression(g, data.cond);
    fprintf(f, "    cmp rax, 0\n");
    fprintf(f, "    je label_%d\n", end_body);
    generate_statement(g, data.body);
    fprintf(f, "    jmp label_%d\n", test_cond);

    fprintf(f, "label_%d:\n", end_body);
}

void generate_statement(generator_t g, ast_t stmt)
{
    if (stmt->tag == ast_return)
        generate_return(g, stmt);
    else if (stmt->tag == ast_function_call)
        generate_function_call(g, stmt);
    else if (stmt->tag == ast_if_stat)
        generate_if_else_statement(g, stmt);
    else if (stmt->tag == ast_while_loop)
        generate_while_statement(g, stmt);
    else if (stmt->tag == ast_auto)
        generate_variable_declaration(g, stmt);
    else if (stmt->tag == ast_scope)
        generate_scope(g, stmt);
    else if (stmt->tag == ast_assignment)
        generate_general_assignment(g, stmt);
}

void generate_scope(generator_t g, ast_t scope_tree)
{
    int prev_scope = scope;
    scope++;
    struct ast_scope data = scope_tree->data.ast_scope;
    for (int i = 0; i < data.length; i++)
        generate_statement(g, data.statements[i]);
    get_rid_of_last_scope(g.scope, prev_scope);
    scope = prev_scope;
}

void generate_function_call(generator_t g, ast_t funcall)
{
    FILE *f = g.out;
    struct ast_function_call data = funcall->data.ast_function_call;
    char *name = data.called->data.ast_identifier.t.lexeme;
    fundef_t fun = find_fundef(g.table, name);
    int size = fundef_arg_size(fun);
    for (int i = data.arity - 1; i >= 0; i--)
    {
        ast_t expr = data.args[i];
        generate_expression(g, expr);
        // for the moment can only work on functions of which called is an identifier present in the table
        // We have to lookup here the number of bytes of the argument
        print_push_on_stack(g, (int)size_of_type(fun.args_types[i]));
    }
    fprintf(f, "    call FUNCTION_%s\n", name);
    fprintf(f, "    add rsp, %d\n", size);
}

void generate_function_def(generator_t g, ast_t fun)
{
    FILE *f = g.out;
    if (fun->tag != ast_function_def)
        exit(17);
    struct ast_function_def fundef = fun->data.ast_function_def;
    char *function_name = fun->data.ast_function_def.t.lexeme;
    print_function_intro(g, function_name);

    // Pushing function arguments on the generator scope
    int offset = 16;
    int init_length = g.scope->length;

    for (int i = 0; i < fundef.arity; i++)
    {
        struct ast_fundef_arg arg_data = fundef.args[i]->data.ast_fundef_arg;

        type_t t = *arg_data.type;
        int size = size_of_type(t);
        // int size = 8;
        char *name = arg_data.arg->data.ast_identifier.t.lexeme;
        stack_val_t var;
        var.address = offset;
        var.n_bytes = size;
        offset += size;
        var.identifier = name;
        var.scope_index = i;
        var.is_arg = 1;
        stack_push(g.scope, var);
    }

    fprintf(f, "    ; Function code here\n");
    generate_scope(g, fundef.body);
    print_function_outro(g);
    fprintf(f, "\n");
    // print_stack(*g.scope);
    while (g.scope->length > init_length)
        (void)stack_pop(g.scope);
}

void generate_program(generator_t g)
{
    FILE *f = g.out;
    print_entry(g);
    print_exit(g);
    fprintf(f, "\n");

    struct ast_program data = g.ast->data.ast_program;
    for (int i = 0; i < data.length; i++)
    {
        ast_t block = data.program[i];
        if (block->tag == ast_function_def)
        {
            generate_function_def(g, block);
            fprintf(f, "\n");
        }
    }
}

void print_push_on_stack(generator_t g, int n_bytes)
{
    // push contents of rax onto the stack
    FILE *f = g.out;
    fprintf(f, "    sub rsp, %d\n", n_bytes);
    fprintf(f, "    mov [rsp], ");
    switch (n_bytes)
    {
    case 1:
        fprintf(f, "byte al");
        break;
    case 2:
        fprintf(f, "word ax");
        break;
    case 4:
        fprintf(f, "dword eax");
        break;
    case 8:
        fprintf(f, "rax");
        break;
    default:
        break;
    }
    fprintf(f, "\n");
}