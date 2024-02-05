#ifndef FUNDEF_TABLE_H
#define FUNDEF_TABLE_H

#include "type.h"
#include "ast.h"

typedef struct
{
    type_t ret_type;
    type_t *args_types;
    int arity;
    int cap;
} fundef_t;

typedef struct
{
    char **name_table;
    fundef_t *sig_table;
    int length;
    int cap;
} fundef_table_t;

fundef_t fundef_create(void);

void fundef_table_create(fundef_table_t *t);
void fundef_table_append(fundef_table_t *t, fundef_t f, char *name);
fundef_t fundef_from_ast(ast_t fundef);
int fundef_arg_size(fundef_t f);
fundef_t find_fundef(fundef_table_t f, char *name);

void load_functions(fundef_table_t *f, ast_t program);

#endif // FUNDEF_TABLE_H