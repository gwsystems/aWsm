/* 
This test was derived from Node.js source code located at the following URL:
https://github.com/nodejs/node/blob/d872aaf1cf20d5b6f56a699e2e3a64300e034269/test/wasi/c/ftruncate.c

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
#include <unistd.h>

#define BASE_DIR "/sandbox/data"
#define OUTPUT_DIR BASE_DIR "/testdir"
#define PATH OUTPUT_DIR "/output.txt"

int main(void)
{
  struct stat st;
  int fd;

  (void)st;
  assert(0 == mkdir(OUTPUT_DIR, 0755));

  fd = open(PATH, O_CREAT | O_WRONLY, 0666);
  assert(fd != -1);

  /* Verify that the file is initially empty. */
  assert(0 == fstat(fd, &st));
  assert(st.st_size == 0);
  assert(0 == lseek(fd, 0, SEEK_CUR));

  /* Increase the file size using ftruncate(). */
  assert(0 == ftruncate(fd, 500));
  assert(0 == fstat(fd, &st));
  assert(st.st_size == 500);
  assert(0 == lseek(fd, 0, SEEK_CUR));

  /* Truncate the file using ftruncate(). */
  assert(0 == ftruncate(fd, 300));
  assert(0 == fstat(fd, &st));
  assert(st.st_size == 300);
  assert(0 == lseek(fd, 0, SEEK_CUR));

  assert(0 == close(fd));
  return 0;
}
