#include <stdio.h>

unsigned long int fib(unsigned long int n) {
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}

int main(int argc, char** argv) {
    unsigned long n = 0, r;
    scanf("%lu", &n);
    r = fib(n);
    printf("%lu\n", r);

    return 0;
}
