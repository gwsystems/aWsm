#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(int arg, char** argv) {
    double g = atof(argv[1]);
    double i = atof(argv[2]);
    float  f = (float)g;
    double h = (float)i;

    printf("%f\n", f);
    printf("%f\n", floorf(f));
    printf("%f\n", ceilf(f));
    printf("%f\n", floor(g));
    printf("%f\n", ceil(g));
    printf("%f\n", fmin(g, i));
    printf("%f\n", fminf(f, h));
    printf("%f\n", fmax(g, i));
    printf("%f\n", fmaxf(f, h));
}
