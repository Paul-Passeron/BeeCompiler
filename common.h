#ifndef COMMON_H
#define COMMON_H

#define FREE(ptr)     \
    do                \
    {                 \
        free((ptr));  \
        (ptr) = NULL; \
    } while (0)

int is_whitespace(char c);
int is_splitter(char c);

#endif // COMMON_H