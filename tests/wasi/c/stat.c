/* 
This test was derived from Node.js source code located at the following URL:
https://github.com/nodejs/node/blob/d872aaf1cf20d5b6f56a699e2e3a64300e034269/test/wasi/c/stat.c

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

#include <sys/stat.h>

#include <assert.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#define BASE_DIR "/sandbox/data"
#define OUTPUT_DIR BASE_DIR "/testdir"
#define PATH OUTPUT_DIR "/output.txt"
#define SIZE 500

int main(void)
{
  struct timespec times[2];
  struct stat st;
  int fd;
  int ret;
  off_t pos;

  // (void)st;
  ret = mkdir(OUTPUT_DIR, 0755);
  assert(ret == 0);

  fd = open(PATH, O_CREAT | O_WRONLY, 0666);
  assert(fd != -1);

  pos = lseek(fd, SIZE - 1, SEEK_SET);
  assert(pos == SIZE - 1);

  ret = (int)write(fd, "", 1);
  assert(ret == 1);

  ret = fstat(fd, &st);
  assert(ret == 0);
  assert(st.st_size == SIZE);

  times[0].tv_sec = 4;
  times[0].tv_nsec = 0;
  times[1].tv_sec = 9;
  times[1].tv_nsec = 0;
  assert(0 == futimens(fd, times));
  assert(0 == fstat(fd, &st));
  assert(4 == st.st_atime);
  assert(9 == st.st_mtime);

  ret = close(fd);
  assert(ret == 0);

  ret = access(PATH, R_OK);
  assert(ret == 0);

  ret = stat(PATH, &st);
  assert(ret == 0);
  assert(st.st_size == SIZE);

  ret = unlink(PATH);
  assert(ret == 0);

  ret = stat(PATH, &st);
  assert(ret == -1);

  ret = stat(OUTPUT_DIR, &st);
  assert(ret == 0);
  assert(S_ISDIR(st.st_mode));

  ret = rmdir(OUTPUT_DIR);
  assert(ret == 0);

  ret = stat(OUTPUT_DIR, &st);
  assert(ret == -1);

  return 0;
}
