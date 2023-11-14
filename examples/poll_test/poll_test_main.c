/****************************************************************************
 * apps/examples/poll_test/poll_test_main.c
 ****************************************************************************/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/timerfd.h>
#include <stdint.h>
#include <unistd.h>

#define TIMERFD_COUNT 8

static int fds[TIMERFD_COUNT] = {-1};

int create_timers(void)
{
	struct itimerspec its;

  clock_gettime(CLOCK_MONOTONIC, &its.it_value);

  // Set initial alarm for 3 seconds in the future
  its.it_value.tv_sec += 3;
  its.it_value.tv_nsec = 0;

  // Set interval to 1 second
  its.it_interval.tv_sec = 1;
  its.it_interval.tv_nsec = 0;

  for(int i = 0; i < TIMERFD_COUNT; i++)
  {
		fds[i] = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if(fds[i] < 0)
    {
      return fds[i];
    }

		if (timerfd_settime(fds[i], TFD_TIMER_ABSTIME, &its, NULL) < 0)
    {
      return errno;
    }
  }

  return 0;
}

void cleanup(void)
{
  for(int i = 0; i < TIMERFD_COUNT; i++)
  {
    if(fds[i] > 0)
    {
      close(fds[i]);
    }
  }
}

void timer_read(int fd)
{
  uint64_t expirations;
  read(fd, &expirations, sizeof(expirations));
}

size_t wait(void)
{
  struct pollfd pfds[TIMERFD_COUNT];
  const int timeout_ms = 10000;
  int ret;
  size_t mask = 0;

  for(int i = 0; i < TIMERFD_COUNT; i++)
  {
    pfds[i].fd = fds[i];
    pfds[i].events = POLLIN;
  }

  ret = poll(pfds, TIMERFD_COUNT, timeout_ms);
  if(ret < 0)
  {
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < TIMERFD_COUNT; i++)
  {
    if (pfds[i].revents == POLLIN)
    {
      mask |= (1 << i);
      timer_read(pfds[i].fd);
    }
  }
  return mask;
}


int main(int argc, char *argv[])
{
  size_t mask;
  int ret = create_timers();
  atexit(cleanup);

  if(ret < 0)
  {
    printf("Cannot create timers: %d\n", ret);
    exit(EXIT_FAILURE);
  }

  printf("Start poll\n");
  while(1)
  {
    mask = wait();
    printf("mask 0x%zX\n", mask);
  }

  exit(EXIT_SUCCESS);
}
