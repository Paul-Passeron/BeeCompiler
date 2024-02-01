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
    return (generator_t){
        .ast = a,
        .out = f};
}

void destroy_generator(generator_t g)
{
    fclose(g.out);
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