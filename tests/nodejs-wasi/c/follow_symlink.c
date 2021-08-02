#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
  FILE *file = fopen("/sandbox/data/input_link.txt", "r");
  if (file == NULL)
  {
    fprintf(stderr, "Error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  };

  char c = fgetc(file);
  while (c != EOF)
  {
    int wrote = fputc(c, stdout);
    assert(wrote != EOF);
    c = fgetc(file);
  }
}
