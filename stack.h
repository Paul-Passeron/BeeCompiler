#ifndef STACK_H
#define STACK_H
#include "common.h"

typedef struct
{
    int address;
    char *identifier;
    int n_bytes;
    int scope_index;
    int is_arg;

} scope_elem_t;

typedef scope_elem_t stack_val_t;

typedef struct
{
    stack_val_t *data;
    int length;
    int capacity;
} stack_t;

void stack_create(stack_t *s);
void stack_free(stack_t *s);

int stack_empty(stack_t s);

void stack_push(stack_t *s, stack_val_t v);
stack_val_t stack_pop(stack_t *s);
stack_val_t stack_peek(stack_t *s);

scope_elem_t get_scope_elem(char *identifier, stack_t s, int *found);

void print_stack(stack_t s);
void get_rid_of_last_scope(stack_t *s, int scope);

int get_first_var_offset(stack_t s);
#endif // STACK_H