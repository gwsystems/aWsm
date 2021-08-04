#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char** environ;

int main(int argc, char* argv[argc]) {
    char**      env_var         = environ;
    const char* expected_string = "SHOULD_PASS=1";

    for (char** cursor = environ; *(cursor) != NULL; cursor++) {
        if (strcmp(*(env_var++), expected_string) == 0) { exit(EXIT_SUCCESS); }
    }

    exit(EXIT_FAILURE);
}
