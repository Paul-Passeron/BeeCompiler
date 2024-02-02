#include "type.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

type_type_t regular_type_type_from_lexeme(char *s)
{
    if (strcmp(s, "i64") == 0)
        return type_i64;

    if (strcmp(s, "i32") == 0)
        return type_i32;

    if (strcmp(s, "i16") == 0)
        return type_i16;

    if (strcmp(s, "i8") == 0)
        return type_i8;

    if (strcmp(s, "u64") == 0)
        return type_u64;

    if (strcmp(s, "u32") == 0)
        return type_u32;

    if (strcmp(s, "u16") == 0)
        return type_u16;

    if (strcmp(s, "u8") == 0)
        return type_u8;

    if (strcmp(s, "char") == 0)
        return type_char_t;

    if (strcmp(s, "void") == 0)
        return type_void;
    return type_error;
}

type_t *regular_type_from_lexeme(char *s)
{
    type_t *ptr = malloc(sizeof(type_t));
    if (ptr == NULL)
    {
        printf("Not enough memory\n");
        exit(1);
    }
    *ptr = (type_t){.type = regular_type_type_from_lexeme(s), .t = NULL};
    return ptr;
}

size_t size_of_type(type_t t)
{

    switch (t.type)
    {
    case type_i64:
        return 8;
    case type_i32:
        return 4;
    case type_i16:
        return 2;
    case type_i8:
        return 1;
    case type_u64:
        return 8;
    case type_u32:
        return 4;
    case type_u16:
        return 2;
    case type_u8:
        return 1;
    case type_char_t:
        return 1;
    case type_void:
        return 0;
    case type_pointer_t:
        return POINTER_SIZE;
    default:
        break;
    }
    return 0;
}

void print_reg_type(type_type_t t)
{
    switch (t)
    {
    case type_i64:
        printf("type_i64");
        break;
    case type_i32:
        printf("type_i32");
        break;
    case type_i16:
        printf("type_i16");
        break;
    case type_i8:
        printf("type_i8");
        break;
    case type_u64:
        printf("type_u64");
        break;
    case type_u32:
        printf("type_u32");
        break;
    case type_u16:
        printf("type_u16");
        break;
    case type_u8:
        printf("type_u8");
        break;
    case type_char_t:
        printf("type_char_t");
        break;
    case type_pointer_t:
        printf("type_pointer_t");
        break;
    case type_error:
        printf("type_error");
        break;
    case type_void:
        printf("type_void");
        break;
    }
}

void print_var_type(type_t *t)
{
    if (t->type != type_pointer_t)
    {
        print_reg_type(t->type);
    }
    else
    {
        printf("pointer-> ");
        print_var_type(t->t);
    }
}