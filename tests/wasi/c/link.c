/* 
This test was derived from Node.js source code located at the following URL:
https://github.com/nodejs/node/blob/d872aaf1cf20d5b6f56a699e2e3a64300e034269/test/wasi/c/link.c

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
#include <unistd.h>
#include <sys/stat.h>

#define OLD "/sandbox/data/input.txt"
#define NEW "/sandbox/data/hard_link.txt"

int main() {
  struct stat st_old;
  struct stat st_new;

  assert(0 == stat(OLD, &st_old));
  assert(0 == link(OLD, NEW));
  assert(0 == stat(NEW, &st_new));
  assert(st_old.st_ino == st_new.st_ino);
  return 0;
}
