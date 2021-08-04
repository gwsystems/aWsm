/* 
This test was derived from Node.js source code located at the following URL:
https://github.com/nodejs/node/blob/d872aaf1cf20d5b6f56a699e2e3a64300e034269/test/wasi/c/readdir.c

It retains the the Node.js license as follows:

MIT License
-----------

Copyright Node.js contributors. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*/

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
  DIR *dir;
  struct dirent *entry;
  int cnt;

  dir = opendir("/sandbox/data/readdir");
  assert(dir != NULL);

  cnt = 0;
  errno = 0;
  while (NULL != (entry = readdir(dir)))
  {
    if (strcmp(entry->d_name, "input.txt") == 0 ||
        strcmp(entry->d_name, "input2.txt") == 0 ||
        strcmp(entry->d_name, "notadir") == 0)
    {
      assert(entry->d_type == DT_REG);
    }
    else if (strcmp(entry->d_name, "subdir") == 0)
    {
      assert(entry->d_type == DT_DIR);
    }
    else
    {
      assert(0);
    }

    cnt++;
  }

  assert(errno == 0);
  assert(cnt == 4);
  closedir(dir);
  return 0;
}
