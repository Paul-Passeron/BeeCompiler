#include "common.h"

int is_splitter(char c)
{
    char splitters[] = " \n=+-*/;{}()<>\%|&,\"\'!";
    int splitters_count = sizeof(splitters) - 1;

    for (int i = 0; i < splitters_count; i++)
    {
        if (c == splitters[i])
            return 1;
    }
    return 0;
}

int is_whitespace(char c)
{
    if (c == ' ' || c == '\n')
        return 1;
    return 0;
}