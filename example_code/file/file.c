#include <stdio.h>

int main(int argc, char** argv) {
    FILE* handle = fopen("./letters.txt", "w+");
    for (char c = 'a'; c <= 'z'; c++)
        fputc(c, handle);
    fclose(handle);
}
