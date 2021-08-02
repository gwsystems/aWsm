#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
  FILE *file_orig = fopen("/sandbox/data/original.txt", "r");
  if (file_orig == NULL)
  {
    strerror(errno);
    exit(EXIT_FAILURE);
  }

  FILE *file_new = freopen("/sandbox/data/replacement.txt", "r", file_orig);
  if (file_new == NULL)
  {
    strerror(errno);
    exit(EXIT_FAILURE);
  }

  int c = fgetc(file_orig);
  while (c != EOF)
  {
    int wrote = fputc((char)c, stdout);
    assert(wrote != EOF);
    c = fgetc(file_orig);
  }
}
