#include "generator.h"
#include <stdlib.h>

generator_t create_generator(ast_t a, char *filename)
{
    FILE *f = fopen(filename, "wb");
    if (f == NULL)
    {
        perror("Could not open file: ");
        exit(1);
    }
    stack_t s;
    stack_create(&s);

    return (generator_t){
        .ast = a,
        .scope = s,
        .out = f};
}

void destroy_generator(generator_t g)
{
    fclose(g.out);
    stack_free(&g.scope);
}

void print_entry(generator_t g)
{
    FILE *f = g.out;
    fprintf(f, "format ELF64 executable 3\n");
    fprintf(f, "segment readable executable\n");
    fprintf(f, "entry start\n");
    fprintf(f, "start:\n");
    fprintf(f, "    mov rbx, 8\n");
    fprintf(f, "    push rbx\n");
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
}

void print_function_outro(generator_t g)
{
    FILE *f = g.out;
    fprintf(f, "    pop rbp\n");
    fprintf(f, "    ret\n");
}

void generate_expression(generator_t g, ast_t expression)
{
    // move the result in rax
    if (expression->tag == ast_identifier)
    {
        token_t t = expression->data.ast_identifier.t;
        int found;
        stack_val_t e = get_scope_elem(t.lexeme, g.scope, &found);
        if (!found)
        {
            printf("Identifier not found !\n");
            exit(1);
        }
        else
        {
            FILE *f = g.out;
            // maybe will have to put speciofer, e.g byte, word, byte ptr...
            fprintf(f, "    mov rax, [rbp + %d]\n ", e.address);
        }
    }
    else if (expression->tag == ast_literal)
    {
        token_t t = expression->data.ast_literal.t;
        if (t.type == NUM_LIT)
        {
            FILE *f = g.out;

            fprintf(f, "   mov rax, %s\n", t.lexeme);
        }
    }
}

void generate_return(generator_t g, ast_t ret_stmt)
{
    struct ast_return data = ret_stmt->data.ast_return;
    generate_expression(g, data.expression);
}

void generate_statement(generator_t g, ast_t stmt)
{
    if (stmt->tag == ast_return)
    {
        generate_return(g, stmt);
    }
}

void generate_scope(generator_t g, ast_t scope)
{
    struct ast_scope data = scope->data.ast_scope;
    for (int i = 0; i < data.length; i++)
    {
        generate_statement(g, data.statements[i]);
    }
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
    int init_length = g.scope.length;

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
        stack_push(&g.scope, var);
    }

    fprintf(f, "   ; Function code here\n");
    generate_scope(g, fundef.body);
    print_function_outro(g);
    fprintf(f, "\n");
    print_stack(g.scope);
    while (g.scope.length > init_length)
        (void)stack_pop(&g.scope);
}

void generate_program(generator_t g)
{
    FILE *f = g.out;
    print_entry(g);
    fprintf(f, "    add rsp, 8\n");

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