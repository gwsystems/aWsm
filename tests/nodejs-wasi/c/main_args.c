#include <assert.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv)
{
  assert(argc == 4);
  assert(0 == strcmp(argv[0], "main_args_vm"));
  assert(0 == strcmp(argv[1], "foo"));
  assert(0 == strcmp(argv[2], "-bar"));
  assert(0 == strcmp(argv[3], "--baz=value"));
  return 0;
}
