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
}

void print_exit(generator_t g, int exit_code)
{
    FILE *f = g.out;
    fprintf(f, "mov rax, 60\n");
    fprintf(f, "mov rdi, %d\n", exit_code);
    fprintf(f, "syscall\n");
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

void generate_function_def(generator_t g, ast_t funcall)
{
    FILE *f = g.out;
    if (funcall->tag != ast_function_def)
        exit(17);
    char *function_name = funcall->data.ast_function_def.t.lexeme;
    print_function_intro(g, function_name);
    fprintf(f, "   ; Function code here\n");
    print_function_outro(g);
    fprintf(f, "\n");
}

void generate_program(generator_t g)
{
    FILE *f = g.out;
    print_entry(g);
    print_exit(g, 0);
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