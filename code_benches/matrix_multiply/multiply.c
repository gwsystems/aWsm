#include <stdio.h>
#include <stdlib.h>

#define MATRIX_SIZE 1700

struct matrix {
    int data[MATRIX_SIZE][MATRIX_SIZE];
};

int main() {
    srand(12388846);

    struct matrix* a = malloc(sizeof(struct matrix));
    struct matrix* b = malloc(sizeof(struct matrix));

    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            a->data[i][j] = rand();
            b->data[i][j] = rand();
        }
    }

    struct matrix *c = malloc(sizeof(struct matrix));
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            int sum = 0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                sum += a->data[i][k] + b->data[k][j];
            }
            c->data[i][j] = sum;
        }
    }

    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            printf("%d", c->data[i][j]);
        }
        printf("\n");
    }

    return 0;
}