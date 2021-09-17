#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    double pi = atof(argv[1]);
    printf("%.2lf\n", pi);
}
