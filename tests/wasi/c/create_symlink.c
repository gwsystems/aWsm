#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
  const char *target = "/sandbox/data/dummy.txt";
  const char *linkpath = "/sandbox/data/test_link";
  char readlink_result[128];
  size_t result_size = sizeof(readlink_result);

  int rc;

  if ((rc = symlink(target, linkpath)) != 0)
  {
    fprintf(stderr, "Error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  };
  assert(readlink(linkpath, readlink_result, result_size) ==
         strlen(target) + 1);
  assert(0 == strcmp(readlink_result, target));

  FILE *file = fopen(linkpath, "r");
  assert(file != NULL);

  int c = fgetc(file);
  while (c != EOF)
  {
    int wrote = fputc(c, stdout);
    assert(wrote != EOF);
    c = fgetc(file);
  }
}
