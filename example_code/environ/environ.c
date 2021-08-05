#include <stdio.h>
#include <stdlib.h>

extern char** environ;

int main(int argc, char* argv[argc]) {
    char** env_var = environ;

    while (*(env_var) != NULL) {
        fprintf(stderr, "%s\n", *(env_var++));
    };

    return 0;
}
