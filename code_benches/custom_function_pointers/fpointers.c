#include <stdio.h>
#include <stdlib.h>

#define FUNCTION_COUNT 10000
#define CALL_COUNT 500000

typedef int (*some_function_t)();

// Some functions that could get called
int e() {
    return 1;
}

int f() {
    return 2;
}

int g() {
    return 3;
}


int main() {
    some_function_t* functions = malloc(sizeof(some_function_t) * FUNCTION_COUNT);

    srand(12132513);
    for (int i = 0; i < FUNCTION_COUNT; i++) {
        switch (rand() % 3) {
            case 0:
                functions[i] = e;
                break;
            case 1:
                functions[i] = f;
                break;
            default:
                functions[i] = g;
                break;
        }
    }

    for (int i = FUNCTION_COUNT - 1; i >= 0; i--) {
        int sum = 0;
        for (int j = 0; j < CALL_COUNT; j++) {
            sum += functions[i]();
        }
        printf("%d ", sum);
    }

    return 0;
}

