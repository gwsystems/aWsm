#include <assert.h>
#include <poll.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
  struct pollfd fds[4];
  time_t before, now;
  int ret;

  // Test sleep() behavior.
  time(&before);
  sleep(1);
  time(&now);
  assert(now - before >= 1);

  // Test poll() timeout behavior.
  fds[0] = (struct pollfd){.fd = -1, .events = 0, .revents = 0};
  time(&before);
  ret = poll(fds, 1, 2000);
  time(&now);
  assert(ret == 0);
  assert(now - before >= 2);

  fds[0] = (struct pollfd){.fd = 1, .events = POLLOUT, .revents = 0};
  fds[1] = (struct pollfd){.fd = 2, .events = POLLOUT, .revents = 0};

  ret = poll(fds, 2, -1);
  assert(ret == 2);
  assert(fds[0].revents == POLLOUT);
  assert(fds[1].revents == POLLOUT);

  // Make a poll() call with duplicate file descriptors.
  fds[0] = (struct pollfd){.fd = 1, .events = POLLOUT, .revents = 0};
  fds[1] = (struct pollfd){.fd = 2, .events = POLLOUT, .revents = 0};
  fds[2] = (struct pollfd){.fd = 1, .events = POLLOUT, .revents = 0};
  fds[3] = (struct pollfd){.fd = 1, .events = POLLIN, .revents = 0};

  ret = poll(fds, 2, -1);
  assert(ret == 2);
  assert(fds[0].revents == POLLOUT);
  assert(fds[1].revents == POLLOUT);
  assert(fds[2].revents == 0);
  assert(fds[3].revents == 0);

  // Test timeout
  fds[0] = (struct pollfd){.fd = 0, .events = POLLIN, .revents = 0};
  ret = poll(fds, 1, 2000);
  assert(ret == 0);

  return 0;
}
