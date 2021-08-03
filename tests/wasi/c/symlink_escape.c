#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main()
{
  FILE *file = fopen("/sandbox/data/outside", "r");
  assert(file == NULL);
  assert(errno == ENOTCAPABLE);
}
