#ifndef COMMON_H
#define COMMON_H

#define FREE(ptr)     \
    do                \
    {                 \
        free((ptr));  \
        (ptr) = NULL; \
    } while (0)

#endif // COMMON_H