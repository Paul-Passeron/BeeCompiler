#ifndef TYPE_H
#define TYPE_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define POINTER_SIZE 4

typedef enum
{
    type_i64 = 0,
    type_i32,
    type_i16,
    type_i8,
    type_u64,
    type_u32,
    type_u16,
    type_u8,
    type_char_t,
    type_pointer_t,
    type_error,
} type_type_t;

typedef struct type
{
    type_type_t type;
    struct type *t;
} type_t;

type_t *regular_type_from_lexeme(char *s);

size_t size_of_type(type_t t);

#endif // TYPE_H
