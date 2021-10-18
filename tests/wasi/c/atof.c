#include <stdlib.h>

const char* pi_str = "3.14";

int main() {
    float pi = strtof(pi_str, NULL);
    return (int)pi;
}
