#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char** environ;

#define STRING_COUNT 3

int main(int argc, char* argv[argc]) {
    char**      env_var            = environ;
    const char* expected_strings[] = { "FOO=BAR", "BAR=BAZ", "BAZ=FOO" };
    bool        was_found[]        = { false, false, false };

    for (char** cursor = environ; *(cursor) != NULL; cursor++) {
        for (int i = 0; i < STRING_COUNT; i++) {
            if (!was_found[i]) {
                if (strcmp(*cursor, expected_strings[i]) == 0) {
                    was_found[i] = true;
                }
            }
        }
    }

    for (int i = 0; i < STRING_COUNT; i++) {
        if (was_found[i] == false) {
            exit(EXIT_FAILURE);
        }
    }

    exit(EXIT_SUCCESS);
}
