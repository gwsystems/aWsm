/*
This test was derived from Node.js source code located at the following URL:
https://github.com/nodejs/node/blob/d872aaf1cf20d5b6f56a699e2e3a64300e034269/test/wasi/c/poll.c

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
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(void) {
    struct pollfd fds[4];
    time_t        before, now;
    int           ret;

    // Test sleep() behavior.
    time(&before);
    sleep(1);
    time(&now);
    assert(now - before >= 1);

    // Test poll() timeout behavior.
    fds[0] = (struct pollfd){ .fd = -1, .events = 0, .revents = 0 };
    time(&before);
    ret = poll(fds, 1, 2000);
    time(&now);
    assert(ret == 0);
    assert(now - before >= 2);

    fds[0] = (struct pollfd){ .fd = 1, .events = POLLOUT, .revents = 0 };
    fds[1] = (struct pollfd){ .fd = 2, .events = POLLOUT, .revents = 0 };

    ret = poll(fds, 2, -1);
    assert(ret == 2);
    assert(fds[0].revents == POLLOUT);
    assert(fds[1].revents == POLLOUT);

    // Make a poll() call with duplicate file descriptors.
    fds[0] = (struct pollfd){ .fd = 1, .events = POLLOUT, .revents = 0 };
    fds[1] = (struct pollfd){ .fd = 2, .events = POLLOUT, .revents = 0 };
    fds[2] = (struct pollfd){ .fd = 1, .events = POLLOUT, .revents = 0 };
    fds[3] = (struct pollfd){ .fd = 1, .events = POLLIN, .revents = 0 };

    ret = poll(fds, 2, -1);
    assert(ret == 2);
    assert(fds[0].revents == POLLOUT);
    assert(fds[1].revents == POLLOUT);
    assert(fds[2].revents == 0);
    assert(fds[3].revents == 0);

    // Test timeout
    // Commented out because relying on STDIN is too fragile
    // This should try to read from a socket, but this is probably
    // impossible due to the WASI spec flux around sockets
    // fds[0] = (struct pollfd){.fd = 0, .events = POLLIN, .revents = 0};
    // ret = poll(fds, 1, 2000);
    // assert(ret == 0);

    return 0;
}
