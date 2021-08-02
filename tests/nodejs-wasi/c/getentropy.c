#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_LEN 256

int main()
{
  char buf[BUF_LEN] = {0};
  int r = getentropy(buf, BUF_LEN);
  assert(r == 0);

  for (int i = 0; i < BUF_LEN; i++)
  {
    if (buf[i] != 0)
    {
      return EXIT_SUCCESS;
    }
  }

  return EXIT_FAILURE;
}
