#include "stack.h"
#include <stdlib.h>
#include "string.h"
#include <stdio.h>

void stack_create(stack_t *s)
{
    int init_cap = 265;
    s->capacity = init_cap;
    s->length = 0;
    s->data = malloc(sizeof(stack_val_t) * init_cap);
    if (s->data == NULL)
    {
        printf("Could not allocate memory\n");
        exit(1);
    }
}

void stack_free(stack_t *s)
{
    FREE(s->data);
    s->capacity = 0;
    s->length = 0;
}

void stack_push(stack_t *s, stack_val_t v)
{
    if (s->length == s->capacity)
    {
        s->capacity *= 2;
        s->data = realloc(s->data, sizeof(stack_val_t) * s->capacity);
        if (s->data == NULL)
        {
            printf("Could not allocate memory\n");
            exit(1);
        }
    }
    s->data[s->length++] = v;
}

stack_val_t stack_pop(stack_t *s)
{
    if (s->length == 0)
    {
        printf("Cannot pop empty stack\n");
        exit(2);
    }
    return s->data[--s->length];
}

stack_val_t stack_peek(stack_t *s)
{
    if (s->length == 0)
    {
        printf("Cannot peek empty stack\n");
        exit(2);
    }
    return s->data[s->length - 1];
}

scope_elem_t get_scope_elem(char *identifier, stack_t s, int *found)
{
    while (!stack_empty(s))
    {
        scope_elem_t t = stack_pop(&s);
        if (strcmp(t.identifier, identifier) == 0)
        {
            *found = 1;
            return t;
        }
    }
    *found = 0;
    return (scope_elem_t){0};
}

int stack_empty(stack_t s)
{
    return s.length == 0;
}
