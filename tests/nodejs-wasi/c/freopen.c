#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
  FILE *file_orig = fopen("/sandbox/input.txt", "r");
  if (file_orig != NULL)
  {
    // strerror(errno);
    exit(EXIT_FAILURE);
  }

  FILE *file_new = freopen("/sandbox/input2.txt", "r", file_orig);
  assert(file_new != NULL);

  int c = fgetc(file_new);
  while (c != EOF)
  {
    int wrote = fputc((char)c, stdout);
    assert(wrote != EOF);
    c = fgetc(file_new);
  }
}
