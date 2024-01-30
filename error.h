#ifndef ERROR_H
#define ERROR_H

typedef enum
{
    SYNTAX_ERROR
} err_type_t;

typedef enum
{
    UNEXP_DELIM,
    UNEXP_CHAR,
    UNCLOSED_STRLIT,
    INVALID_STRLIT,
    UNCLOSED_CHRLIT,
    INVALID_CHRLIT,

    INVALID_FUNCALL,
    INVALID_FUNDEF,
} error_t;

typedef struct
{
    char *filename;
    char *text;
    int abs_offset;
    int line;
    int col;
    err_type_t t;
    error_t error;

} error_reporter_t;

void print_error_preface(error_reporter_t r);
void print_syntax_error(error_reporter_t r);

#endif // ERROR_H