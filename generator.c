#include "generator.h"
#include <stdlib.h>

generator_t create_generator(ast_t a, char *filename)
{
    FILE *f = fopen(filename, "rb");
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