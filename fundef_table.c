#include "fundef_table.h"

#include <stdio.h>
#include <string.h>
fundef_t fundef_create(void)
{
    fundef_t res;
    res.cap = 256;
    res.args_types = malloc(256 * sizeof(type_t));
    // memset(args_type, 0, 256 * sizeof(type_t));
    res.arity = 0;
    return res;
}

void fundef_table_create(fundef_table_t *t)
{
    t->cap = 256;
    t->name_table = malloc(t->cap * sizeof(char *));
    t->sig_table = malloc(t->cap * sizeof(fundef_t));
    t->length = 0;
}

void fundef_table_append(fundef_table_t *t, fundef_t f, char *name)
{
    if (t->length >= 256)
    {
        printf("Table is full !\n");
        exit(1);
    }
    t->name_table[t->length] = name;
    t->sig_table[t->length++] = f;
}

fundef_t fundef_from_ast(ast_t fundef)
{
    fundef_t res = fundef_create();
    struct ast_function_def data = fundef->data.ast_function_def;
    // char *name = data.t.lexeme;
    res.arity = data.arity;
    res.ret_type = *data.return_type;
    for (int i = 0; i < data.arity; i++)
    {
        struct ast_fundef_arg arg = data.args[i]->data.ast_fundef_arg;
        type_t t = *arg.type;
        res.args_types[i] = t;
    }
    return res;
}

void fundef_table_append_ast(fundef_table_t *t, ast_t fundef)
{
    struct ast_function_def data = fundef->data.ast_function_def;
    char *name = data.t.lexeme;
    fundef_table_append(t, fundef_from_ast(fundef), name);
}

fundef_t find_fundef(fundef_table_t f, char *name)
{
    for (int i = 0; i < f.length; i++)
    {
        if (strcmp(name, f.name_table[i]) == 0)
            return f.sig_table[i];
    }
    printf("Could not find function '%s'\n", name);
    exit(1);
}

int fundef_arg_size(fundef_t f)
{
    int cter = 0;
    for (int i = 0; i < f.arity; i++)
        cter += size_of_type(f.args_types[i]);
    return cter;
}

void load_functions(fundef_table_t *f, ast_t program)
{
    struct ast_program data = program->data.ast_program;

    for (int i = 0; i < data.length; i++)
    {
        if (data.program[i]->tag == ast_function_def)
            fundef_table_append_ast(f, data.program[i]);
    }
}