#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    fprintf(stderr, "Argc: %d\n", argc);
    for (int i = 0; i < argc; i++)
    {
        printf("Arg %d: %s\n", i, argv[i]);
    }
    exit(EXIT_SUCCESS);
}
