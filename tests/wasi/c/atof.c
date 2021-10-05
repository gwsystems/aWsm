#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        printf("Arg %d: '%s'\n", i, argv[i]);
    }
    char* endptr = "not set";
    float pi     = strtof(argv[1], &endptr);
    if (pi == 0) {
        perror("strtod failed");
        printf("Start: %p\n", argv[1]);
        printf("Remaining: %p\n", endptr);
    }
    printf("%.2lf\n", pi);
}
